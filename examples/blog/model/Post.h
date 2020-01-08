// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef POST_H_
#define POST_H_

#include <Wt/WDate.h>
#include <Wt/WString.h>

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>

#include "Comment.h"
#include "Tag.h"

class User;

namespace dbo = Wt::Dbo;

typedef dbo::collection< dbo::ptr<Comment> > Comments;
typedef dbo::collection< dbo::ptr<Tag> > Tags;

class Post : public dbo::Dbo<Post> {
public:
  enum State {
    Unpublished = 0,
    Published = 1
  };

  dbo::ptr<User> author;
  State          state;

  Wt::WDateTime  date;
  Wt::WString    title;
  Wt::WString    briefSrc;
  Wt::WString    briefHtml;
  Wt::WString    bodySrc;
  Wt::WString    bodyHtml;

  Comments       comments;
  Tags           tags;

  std::string permaLink() const;
  std::string commentCount() const;
  dbo::ptr<Comment> rootComment() const;
  std::string titleToUrl() const;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, state,     "state");
    dbo::field(a, date,      "date");
    dbo::field(a, title,     "title");
    dbo::field(a, briefSrc,  "brief_src");
    dbo::field(a, briefHtml, "brief_html");
    dbo::field(a, bodySrc,   "body_src");
    dbo::field(a, bodyHtml,  "body_html");

    dbo::belongsTo(a, author, "author");

    dbo::hasMany(a, comments, dbo::ManyToOne,  "post");
    dbo::hasMany(a, tags,     dbo::ManyToMany, "post_tag");
  }
};

DBO_EXTERN_TEMPLATES(Post)

#endif // POST_H_
