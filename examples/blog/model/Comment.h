// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef COMMENT_H_
#define COMMENT_H_

#include <Wt/WDateTime.h>

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>

class Comment;
class Post;
class User;

namespace dbo = Wt::Dbo;

typedef dbo::collection<dbo::ptr<Comment> > Comments;

class Comment {
public:
  dbo::ptr<User>    author;
  dbo::ptr<Post>    post;
  dbo::ptr<Comment> parent;

  Wt::WDateTime         date;

  void setText(const Wt::WString& text);
  void setDeleted();

  const Wt::WString& textSrc() const { return textSrc_; }
  const Wt::WString& textHtml() const { return textHtml_; }

  Comments          children;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, date, "date");
    dbo::field(a, textSrc_, "text_source");
    dbo::field(a, textHtml_, "text_html");

    dbo::belongsTo(a, post, "post", dbo::OnDeleteCascade);
    dbo::belongsTo(a, author, "author");
    dbo::belongsTo(a, parent, "parent", dbo::OnDeleteCascade);

    dbo::hasMany(a, children, dbo::ManyToOne, "parent");
  }

private:
  Wt::WString textSrc_;
  Wt::WString textHtml_;
};

DBO_EXTERN_TEMPLATES(Comment)

#endif // COMMENT_H_
