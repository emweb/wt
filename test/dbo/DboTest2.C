/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <Wt/Dbo/Dbo>
#include <Wt/WDate>
#include <Wt/WDateTime>
#include <Wt/WTime>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Dbo/ptr_tuple>

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

  User *user = new User();
  user->name = "Joe";
  user->password = "Secret";
  user->role = User::Visitor;
  user->karma = 13;

  dbo::ptr<User> userPtr = session.add(user);

  User *user2 = new User();
  user2->name = "Daisy";
  user2->password = "Secret2";
  user2->role = User::Visitor;
  user2->karma = 12;

  session.add(user2);

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

  dbo::ptr<User> silly = session.add(new User());
  silly.remove();

  dbo::ptr<Post> post = session.add(new Post());
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

  dbo::ptr<Tag> cooking = session.add(new Tag());
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
      User *user = new User();
      std::ostringstream os;
      os << "User" << i;
      user->name = os.str();
      session.add(user);
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

    User *sn = new User();
    sn->name = "sn";
    session.add(sn);

    User *ln = new User();
    std::string s = "longname";
    for(int i = 0; i<99;i++)
      s+="longname";
    ln->name = s;
    session.add(ln);
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
