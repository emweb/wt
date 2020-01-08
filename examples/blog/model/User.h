// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef USER_H_
#define USER_H_

#include <Wt/Dbo/Types.h>

#include "Post.h"
#include "Token.h"

class Comment;

namespace dbo = Wt::Dbo;

typedef dbo::collection< dbo::ptr<Comment> > Comments;
typedef dbo::collection< dbo::ptr<Post> > Posts;
typedef dbo::collection< dbo::ptr<Token> > Tokens;

class User {
public:
  User();

  enum Role {
    Visitor = 0,
    Admin = 1
  };

  Wt::WString   name;
  Role      role;

  std::string password;
  std::string passwordMethod;
  std::string passwordSalt;
  int failedLoginAttempts;
  Wt::WDateTime lastLoginAttempt;

  std::string oAuthId;
  std::string oAuthProvider;

  Tokens authTokens;
  Comments comments;
  Posts posts;

  Posts latestPosts(int count = 10) const;
  Posts allPosts(Post::State state) const;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, name,                "name");
    dbo::field(a, password,            "password");
    dbo::field(a, passwordMethod,      "password_method");
    dbo::field(a, passwordSalt,        "password_salt");
    dbo::field(a, role,                "role");
    dbo::field(a, failedLoginAttempts, "failed_login_attempts");
    dbo::field(a, lastLoginAttempt,    "last_login_attempt");
    dbo::field(a, oAuthId,             "oauth_id");
    dbo::field(a, oAuthProvider,       "oauth_provider");

    dbo::hasMany(a, comments,   dbo::ManyToOne, "author");
    dbo::hasMany(a, posts,      dbo::ManyToOne, "author");
    dbo::hasMany(a, authTokens, dbo::ManyToOne, "user");
  }

  static dbo::dbo_traits<User>::IdType stringToId(const std::string &s);
};

DBO_EXTERN_TEMPLATES(User)

#endif // USER_H_
