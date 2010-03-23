/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "BlogSession.h"
#include "asciidoc/asciidoc.h"

#include "model/Comment.h"
#include "model/Post.h"
#include "model/Tag.h"
#include "model/User.h"

namespace dbo = Wt::Dbo;

BlogSession::BlogSession(const std::string& sqliteDb)
  : connection_(sqliteDb)
{
  setConnection(connection_);

  mapClass<Post>("post");
  mapClass<Comment>("comment");
  mapClass<User>("user");
  mapClass<Tag>("tag");

  try {
    dbo::Transaction t(*this);
    createTables();

    dbo::ptr<User> admin = add(new User());
    User *a = admin.modify();
    a->name = "admin";
    a->role = User::Admin;
    a->setPassword("admin");

    dbo::ptr<Post> post = add(new Post());
    Post *p = post.modify();

    p->state = Post::Published;
    p->author = admin;
    p->title = "Welcome!";
    p->briefSrc = "Welcome to your own blog.";
    p->bodySrc = "We have created for you an admin user with password admin";
    p->briefHtml = asciidoc(p->briefSrc);
    p->bodyHtml = asciidoc(p->bodySrc);
    p->date = Wt::WDateTime::currentDateTime();

    dbo::ptr<Comment> rootComment = add(new Comment());
    rootComment.modify()->post = post;

    t.commit();

    std::cerr << "Created database, and admin/admin user";
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "Using existing database";
  }
}

void BlogSession::setUser(dbo::ptr<User> user)
{
  user_ = user;
}
