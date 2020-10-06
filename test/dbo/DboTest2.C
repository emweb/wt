/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo.h>
#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WTime.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/ptr_tuple.h>

#include "DboFixture.h"

namespace dbo = Wt::Dbo;

class Post;
typedef dbo::collection< dbo::ptr<Post> > Posts;

class User {
public:
  enum Role {
    Visitor = 0,
    Admin = 1,
    Alien = 42
  };

  std::string name;
  std::string password;
  Role        role;
  int         karma;

  Posts       posts;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, name, "name", 1000);
    dbo::field(a, password, "password");
    dbo::field(a, role,     "role");
    dbo::field(a, karma,    "karma");

    dbo::hasMany(a, posts, dbo::ManyToOne, "user");
  }
};

class Tag;
typedef dbo::collection< dbo::ptr<Tag> > Tags;

class Post {
public:
  std::string title;
  std::string contents;

  dbo::ptr<User> user;
  Tags tags;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, title, "title", 1000);
    dbo::field(a, contents, "contents");

    dbo::belongsTo(a, user, "user", Wt::Dbo::OnDeleteCascade);
    dbo::hasMany(a, tags, dbo::ManyToMany, "post_tags");
  }
};

class Tag {
public:
  std::string name;

  dbo::collection< dbo::ptr<Post> > posts;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, name, "name", 1000);

    dbo::hasMany(a, posts, dbo::ManyToMany, "post_tags");
  }
};

struct Dbo2Fixture : DboFixtureBase
{
  Dbo2Fixture() :
    DboFixtureBase()
  {
    session_->mapClass<User > ("user");
    session_->mapClass<Post > ("post");
    session_->mapClass<Tag > ("tag");

    try {
      session_->dropTables(); //todo:remove
    } catch (...) {
    }
    std::cout << "-------------------------- end of drop ----------------------*********" << std::endl;

    session_->createTables();
  }
};

BOOST_AUTO_TEST_CASE( dbo2_test1 )
{
  Dbo2Fixture f;

  dbo::Session& session = *f.session_;

  dbo::Transaction transaction(session);

  auto user = std::make_unique<User>();
  user->name = "Joe";
  user->password = "Secret";
  user->role = User::Visitor;
  user->karma = 13;

  dbo::ptr<User> userPtr = session.add(std::move(user));

  auto user2 = std::make_unique<User>();
  user2->name = "Daisy";
  user2->password = "Secret2";
  user2->role = User::Visitor;
  user2->karma = 12;

  session.add(std::move(user2));

  // simple queries: session.find()
  dbo::ptr<User> joe = session.find<User>("where \"name\" = ?").bind("Joe");
  std::cerr << "Joe has karma: " << joe->karma << std::endl;

  // any queries: session.query()
  dbo::ptr<User> joe2 = session.query< dbo::ptr<User> >
    ("select u from \"user\" u where \"name\" = ?").bind("Joe");
  std::cerr << "Indeed, Joe has karma: " << joe2->karma << std::endl;

  std::cerr << "Joe == Joe: " << (joe == joe2) << std::endl;

  // session.query() can return anything
  int count = session.query<int>
    ("select count(*) from \"user\" where \"name\" = ?").bind("Joe");
  std::cerr << "There is only " << count << " Joe." << std::endl;

  BOOST_REQUIRE(count == 1);

  // session.query() and session.find() can return a collection
  typedef dbo::collection< dbo::ptr<User> > Users;

  Users users = session.find<User>();
  std::cerr << "We have " << users.size() << " users:" << std::endl;
  for (Users::const_iterator i = users.begin(); i != users.end(); ++i)
    std::cerr << " user " << (*i)->name
	      << " with karma of " << (*i)->karma << std::endl;

  joe = session.find<User>("where \"name\" = ?").bind("Joe");    
  joe.modify()->karma++;
  joe.modify()->password = "public";

  dbo::ptr<User> silly = session.add(std::make_unique<User>());
  silly.remove();

  dbo::ptr<Post> post = session.add(std::make_unique<Post>());
  post.modify()->user = joe;

#if 0
  {
    dbo::ptr<Post> p;
    std::string msg;

    boost::tie(p, msg)
      = session.query<boost::tuple<dbo::ptr<Post>, std::string> >
      ("SELECT (doc), msg FROM db_function('Howdy!')").resultValue();
  }
#endif

  std::cerr << "Joe has " << joe->posts.size() << " posts." << std::endl;

  dbo::ptr<Tag> cooking = session.add(std::make_unique<Tag>());
  cooking.modify()->name = "Cooking";

  post.modify()->tags.insert(cooking);
  std::cerr << cooking->posts.size() << " post(s) tagged with Cooking."
    << std::endl;

  transaction.commit();

  //Test from oracle branch
  {
    dbo::Session& session = *f.session_;
    dbo::Transaction transactionTag(session);
    dbo::ptr<User> joe = session.find<User > ("where \"name\" = ?").bind("Joe");
    std::cerr << " joe name = " << joe->name << std::endl;
    std::cerr << " joe password = " << joe->password << std::endl;

    // test limit offset

    for (int i = 0; i < 10; i++) {
      auto user = std::make_unique<User>();
      std::ostringstream os;
      os << "User" << i;
      user->name = os.str();
      session.add(std::move(user));
    }

    int userNum = 9;
    Users allUsers = session.find<User > ().orderBy("\"name\" desc").limit(5);
    for (Users::const_iterator i = allUsers.begin();
      i != allUsers.end(); ++i) {
      dbo::ptr<User> user = *i;
      std::ostringstream os;
      os << "User" << userNum;
      //The test will fail if other user name (not joe, joe2, user1,user2...)
      //will be added to the DB before that test.
      BOOST_REQUIRE(user->name.compare(os.str()) == 0);
      userNum--;
    }

#ifndef FIREBIRD


    Users usersOffset =
      session.find<User > ().orderBy("\"name\" desc").limit(7).offset(3);

    userNum = 6;
    for (Users::const_iterator i = usersOffset.begin();
      i != usersOffset.end(); ++i) {
      dbo::ptr<User> user = *i;
      std::ostringstream os;
      os << "User" << userNum;
      BOOST_REQUIRE(user->name.compare(os.str()) == 0);
      userNum--;
    }
#endif //FIREBIRD

  }
  /*
   * Check that we fail gracefully when tying to bind too much
   */
  {
    dbo::Session& session = *f.session_;
    dbo::Transaction transactionBindTooMuch(session);
    
    bool caught = false;
    try {
      Users allUsers2 = session.find<User>().bind("Joe");
      transactionBindTooMuch.commit();
    } catch (std::exception& e) {
      std::cerr << "Catching exception: " << std::endl;
      std::cerr << "Catching exception: " << e.what() << std::endl;
      caught = true;
    }

    BOOST_REQUIRE(caught);
  }

  /*
   * Test bind 2 strings with different size
   */
  {
    dbo::Session& session = *f.session_;
    dbo::Transaction transactionBindTooMuch(session);

    auto sn = std::make_unique<User>();
    sn->name = "sn";
    session.add(std::move(sn));

    auto ln = std::make_unique<User>();
    std::string s = "longname";
    for(int i = 0; i<99;i++)
      s+="longname";
    ln->name = s;
    session.add(std::move(ln));
  }

  //test OnDeleteCascade
  {
    dbo::Transaction transaction2(session);

    Posts posts = session.find<Post>();
    BOOST_REQUIRE(posts.size() == 1);
    Tags tags = session.find<Tag>();
    BOOST_REQUIRE (tags.size() == 1);
    int postTagsCount =
        session.query<int>("select count(*) from \"post_tags\"");
    BOOST_REQUIRE (postTagsCount == 1);

    dbo::ptr<User> joe3 = session.query< dbo::ptr<User> >
        ("select u from \"user\" u where \"name\" = ?").bind("Joe");
    joe3.remove();

    transaction2.commit();
  }
  {
    dbo::Transaction transaction(session);

    Posts posts = session.find<Post>();
    BOOST_REQUIRE(posts.size() < 1);
    Tags tags = session.find<Tag>();
    BOOST_REQUIRE(tags.size() == 1);
    int postTagsCount =
        session.query<int>("select count(*) from \"post_tags\"");
    BOOST_REQUIRE (postTagsCount == 0);

    transaction.commit();
  }

}

/*
 * Add posts with identical content to all user blogs, using various methods.
 */

BOOST_AUTO_TEST_CASE( dbo2_test2 )
{
  Dbo2Fixture f;

  dbo::Session& session = *f.session_;

  typedef dbo::collection< dbo::ptr<User> > Users;

  Post commonPost;
  commonPost.contents = "Groupthink";
  const size_t USER_COUNT = 2;

  {
    dbo::Transaction transaction(session);

    for (size_t i = 0; i < USER_COUNT; i++) {
      auto user = std::make_unique<User>();
      std::ostringstream os;
      os << "User" << i;
      user->name = os.str();
      user->password = "Secret";
      user->role = User::Visitor;
      user->karma = i;
      session.add(std::move(user));
    }
  }

  {
    dbo::Transaction transaction(session);
    commonPost.title = "Post 1";
    std::cerr << "\nAdding '" << commonPost.title << "' to " << USER_COUNT
      << " user blogs iterating over user objects" << std::endl;

    /*
     * Complete user objects are retrieved from DB, although only the id is
     * needed. This could be costly with a more complex User record.
     */

    Users allUsers = session.find<User > ();
    for (Users::const_iterator i = allUsers.begin();
      i != allUsers.end(); ++i) {
      dbo::ptr<User> user = *i;

      commonPost.user = user;
      dbo::ptr<Post> post = session.add(std::make_unique<Post>(commonPost));
    }
  }

  {
    dbo::Transaction transaction(session);
    commonPost.title = "Post 2";
    std::cerr << "\nAdding '" << commonPost.title << "' to " << USER_COUNT
      << " user blogs using dbo::ptr link from existing posts" << std::endl;

    /*
     * If there is an existing dbo::ptr to a user, we can add new Posts
     * without retrieving the associated user by copying the dbo::ptr.
     * NOTE: This works because we have a single post, "Post 1", for each user.
     */

    Posts existingPosts = session.find<Post > ();

    for (Posts::const_iterator i = existingPosts.begin();
      i != existingPosts.end(); ++i) {

      commonPost.user = (*i)->user;
      dbo::ptr<Post> post = session.add(std::make_unique<Post>(commonPost));
    }
  }

  {
    dbo::Transaction transaction(session);
    commonPost.title = "Post 3";
    std::cerr << "\nAdding '" << commonPost.title << "' to " << USER_COUNT
      << " user blogs using insert through user objects" << std::endl;

    /*
     * With insert(), the full User object is retrieved for each post,
     * even though only the id is required. The User object is also
     * updated (incremented version number).
     */

    Users allUsers = session.find<User > ();
    for (Users::const_iterator i = allUsers.begin();
      i != allUsers.end(); ++i) {
      i->modify()->posts.insert(std::make_unique<Post>(commonPost));
    }
  }

  {
    dbo::Transaction transaction(session);
    commonPost.title = "Post 4";
    std::cerr << "\nAdding '" << commonPost.title << "' to " << USER_COUNT
      << " user blogs using load to retrieve user objects" << std::endl;

    typedef dbo::dbo_traits<User>::IdType UserId;
    typedef dbo::collection< UserId > UserIds;

    /*
     * With load(), the full User object is retrieved for each post,
     * even though only the id is required.
     */

    UserIds allUserIds = session.query<UserId > ("select u.\"id\" from \"user\" u");
    for (UserIds::const_iterator i = allUserIds.begin();
      i != allUserIds.end(); ++i) {
      commonPost.user = session.load<User>(*i);
      dbo::ptr<Post> post = session.add(std::make_unique<Post>(commonPost));
    }
  }

  {
    dbo::Transaction transaction(session);
    commonPost.title = "Post 5";
    std::cerr << "\nAdding '" << commonPost.title << "' to " << USER_COUNT
      << " user blogs using loadLazy to retrieve user objects" << std::endl;

    typedef dbo::dbo_traits<User>::IdType UserId;
    typedef dbo::collection< UserId > UserIds;

    /*
     * With loadLazy(), there is no need to retrieve the full User object
     * in order to obtain a dbo::ptr to the User for adding posts.
     */

    UserIds allUserIds = session.query<UserId > ("select u.\"id\" from \"user\" u");
    for (UserIds::const_iterator i = allUserIds.begin();
      i != allUserIds.end(); ++i) {
      commonPost.user = session.loadLazy<User>(*i);
      dbo::ptr<Post> post = session.add(std::make_unique<Post>(commonPost));
    }
  }

  {
    dbo::Transaction transaction(session);
    std::cerr << std::endl;

    /*
     * Each user should now have 5 posts
     */

    Users allUsers = session.find<User > ();
    for (Users::const_iterator i = allUsers.begin();
      i != allUsers.end(); ++i) {
      dbo::ptr<User> user = *i;

      BOOST_REQUIRE(user->posts.size() == 5);
    }
  }
}
