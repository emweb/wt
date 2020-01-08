// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef COMMENT_VIEW_H_
#define COMMENT_VIEW_H_

#include <Wt/WTemplate.h>
#include <Wt/Dbo/ptr.h>

namespace Wt {
  class WTextArea;
}

class BlogSession;
class Comment;

namespace dbo = Wt::Dbo;

class CommentView : public Wt::WTemplate
{
public:
  // For new comment, goes immediately to edit mode
  CommentView(BlogSession& session, long long parentId);

  // Existing comment
  CommentView(BlogSession& session, dbo::ptr<Comment> comment);

  virtual void resolveString(const std::string& varName,
                             const std::vector<Wt::WString>& args,
			     std::ostream& result);

protected:
  virtual void renderTemplate(std::ostream& result);

private:
  BlogSession& session_;
  dbo::ptr<Comment> comment_;
  Wt::WTextArea *editArea_;

  void reply();
  void edit();
  void rm();
  void save();
  void cancel();
  bool isNew() const;

  void renderView();
};

#endif // COMMENT_VIEW_H_
