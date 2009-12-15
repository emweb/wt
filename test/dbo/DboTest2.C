// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/bind.hpp>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/WDate>
#include <Wt/WDateTime>
#include <Wt/WTime>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Dbo/ptr_tuple>

#include "DboTest2.h"

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
    dbo::field(a, name,     "name");
    dbo::field(a, password, "password");
    dbo::field(a, role,     "role");
    dbo::field(a, karma,    "karma");

    dbo::hasMany(a, posts, dbo::ManyToOne, "user");
  }
};

class Tag;

class Post {
public:
  std::string title;
  std::string contents;

  dbo::ptr<User> user;

  dbo::collection< dbo::ptr<Tag> > tags;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, title, "title");
    dbo::field(a, contents, "contents");

    dbo::belongsTo(a, user, "user");
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
    dbo::field(a, name, "name");

    dbo::hasMany(a, posts, dbo::ManyToMany, "post_tags");
  }
};

void DboTest2::setup()
{
  connection_ = new dbo::backend::Sqlite3(":memory:");

  session_ = new dbo::Session();
  session_->setConnection(*connection_);

  session_->mapClass<User>("user");
  session_->mapClass<Post>("post");
  session_->mapClass<Tag>("tag");

  session_->createTables();
}

void DboTest2::teardown()
{
  delete session_;
  delete connection_;
}

void DboTest2::test1()
{
  setup();
  dbo::Session& session = *session_;

  {
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
    dbo::ptr<User> joe = session.find<User>("where name = ?").bind("Joe");
    std::cerr << "Joe has karma: " << joe->karma << std::endl;

    // any queries: session.query()
    dbo::ptr<User> joe2 = session.query< dbo::ptr<User> >
      ("select u from user u where name = ?").bind("Joe");
    std::cerr << "Indeed, Joe has karma: " << joe2->karma << std::endl;

    std::cerr << "Joe == Joe: " << (joe == joe2) << std::endl;

    // session.query() can return anything
    int count = session.query<int>
      ("select count(*) from user where name = ?").bind("Joe");
    std::cerr << "There is only " << count << " Joe." << std::endl;

    // session.query() and session.find() can return a collection
    typedef dbo::collection< dbo::ptr<User> > Users;

    Users users = session.find<User>();
    std::cerr << "We have " << users.size() << " users:" << std::endl;
    for (Users::const_iterator i = users.begin(); i != users.end(); ++i)
      std::cerr << " user " << (*i)->name
		<< " with karma of " << (*i)->karma << std::endl;

    joe = session.find<User>("where name = ?").bind("Joe");    
    joe.modify()->karma++;
    joe.modify()->password = "public";

    dbo::ptr<User> silly = session.add(new User());
    silly.remove();

    dbo::ptr<Post> post = session.add(new Post());
    post.modify()->user = joe;

    std::cerr << "Joe has " << joe->posts.size() << " posts." << std::endl;

    dbo::ptr<Tag> cooking = session.add(new Tag());
    cooking.modify()->name = "Cooking";

    post.modify()->tags.insert(cooking);
    std::cerr << cooking->posts.size() << " post(s) tagged with Cooking."
	      << std::endl;

    transaction.commit();
  }

  teardown();
}

DboTest2::DboTest2()
  : test_suite("dbotest2_test_suite")
{
  add(BOOST_TEST_CASE(boost::bind(&DboTest2::test1, this)));
}
