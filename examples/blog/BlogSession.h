// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef BLOG_SESSION_H_
#define BLOG_SESSION_H_

#include <Wt/Dbo/Session>
#include <Wt/Dbo/ptr>
#include <Wt/WSignal>
#include <Wt/Dbo/backend/Sqlite3>

namespace dbo = Wt::Dbo;

class Comment;
class Post;
class User;

class BlogSession : public dbo::Session
{
public:
  BlogSession(const std::string& sqliteDb);

  void setUser(dbo::ptr<User> user);
  dbo::ptr<User> user() const { return user_; }

  Wt::Signal< dbo::ptr<Comment> >& commentsChanged() { return commentsChanged_; }

private:
  dbo::backend::Sqlite3 connection_;
  dbo::ptr<User> user_;
  Wt::Signal< dbo::ptr<Comment> > commentsChanged_;
};

#endif // BLOG_SESSION_H_
