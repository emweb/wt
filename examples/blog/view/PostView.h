// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef POST_VIEW_H_
#define POST_VIEW_H_

#include <Wt/WTemplate.h>
#include <Wt/Dbo/ptr.h>

#include "../model/Post.h"

namespace Wt {
  class WText;
}

class BlogSession;

class Comment;

class PostView : public Wt::WTemplate
{
public:
  enum RenderType { Brief, Detail, Edit };

  PostView(BlogSession& session, const std::string& basePath,
	   dbo::ptr<Post> post, RenderType type);

  virtual void resolveString(const std::string& varName,
                             const std::vector<Wt::WString>& args,
			     std::ostream& result);

protected:
  virtual void renderTemplate(std::ostream& result);

private:
  BlogSession& session_;
  std::string basePath_;
  dbo::ptr<Post> post_;

  RenderType viewType_;
  Wt::WText *commentCount_;
  Wt::WLineEdit *titleEdit_;
  Wt::WTextArea *briefEdit_, *bodyEdit_;

  void render(RenderType type);
  void updateCommentCount(dbo::ptr<Comment> comment);
  void saveEdit();
  void showView();

  void publish();
  void retract();
  void showEdit();
  void rm();

  void setState(Post::State state);

  using WWebWidget::render;
};

#endif // POST_VIEW_H_
