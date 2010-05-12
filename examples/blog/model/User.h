// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef USER_H_
#define USER_H_

#include <Wt/Dbo/Types>

#include "Post.h"

class Comment;

namespace dbo = Wt::Dbo;

typedef dbo::collection< dbo::ptr<Comment> > Comments;
typedef dbo::collection< dbo::ptr<Post> > Posts;

class User {
public:
  enum Role {
    Visitor = 0,
    Admin = 1
  };

  std::string name;
  Role        role;

  Comments    comments;
  Posts       posts;

  Posts latestPosts(int count = 10) const;
  Posts allPosts(Post::State state) const;

  void setPassword(const std::string& password);
  bool authenticate(const std::string& password) const;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, name,      "name");
    dbo::field(a, password_, "password");
    dbo::field(a, role,      "role");

    dbo::hasMany(a, comments, dbo::ManyToOne, "author");
    dbo::hasMany(a, posts,    dbo::ManyToOne, "author");
  }

private:
  std::string password_;
};

DBO_EXTERN_TEMPLATES(User);

#endif // USER_H_
