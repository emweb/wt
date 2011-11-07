/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "CommentView.h"
#include "PostView.h"
#include "../asciidoc/asciidoc.h"

#include "../model/BlogSession.h"
#include "../model/Comment.h"
#include "../model/Tag.h"
#include "../model/Token.h"
#include "../model/User.h"

#include <Wt/WAnchor>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WTextArea>

using namespace Wt;
namespace dbo = Wt::Dbo;

PostView::PostView(BlogSession& session, const std::string& basePath,
		   dbo::ptr<Post> post, RenderType type)
  : session_(session),
    basePath_(basePath),
    post_(post)
{
  viewType_ = Brief;
  render(type);
}

void PostView::renderTemplate(std::ostream& result)
{
  dbo::Transaction t(session_);

  WTemplate::renderTemplate(result);

  post_.purge();

  t.commit();
}

void PostView::resolveString(const std::string& varName,
			     const std::vector<WString>& args,
			     std::ostream& result)
{
  if (varName == "title")
    format(result, post_->title);
  else if (varName == "date")
    format(result, post_->date.toString("dddd, MMMM d, yyyy @ HH:mm"));
  else if (varName == "brief") {
    if (!post_->briefSrc.empty())
      format(result, post_->briefHtml, XHTMLText);
    else
      format(result, post_->bodyHtml, XHTMLText);
  } else if (varName == "brief+body") {
    format(result, "<div>" + post_->briefHtml + "</div>"
	   "<div id=\"" + basePath_ + post_->permaLink() + "/more\">"
	   "<div>" + post_->bodyHtml + "</div></div>", XHTMLUnsafeText);
  } else
    WTemplate::resolveString(varName, args, result);
}

void PostView::render(RenderType type)
{
  if (type != Edit)
    viewType_ = type;

  clear();

  switch (type) {
  case Detail: {
    setTemplateText(tr("blog-post"));

    commentCount_ = new WText(post_->commentCount());

    CommentView *comments = new CommentView(session_, post_->rootComment());
    session_.commentsChanged().connect(this, &PostView::updateCommentCount);

    bindWidget("comment-count", commentCount_);
    bindWidget("comments", comments);
    bindString("anchor", basePath_ + post_->permaLink());

    break;
  }
  case Brief: {
    setTemplateText(tr("blog-post-brief"));

    WAnchor *titleAnchor
      = new WAnchor(WLink(WLink::InternalPath, basePath_ + post_->permaLink()),
		    post_->title);
    bindWidget("title", titleAnchor);

    if (!post_->briefSrc.empty()) {
      WAnchor *moreAnchor 
	= new WAnchor(WLink(WLink::InternalPath,
			    basePath_ + post_->permaLink() + "/more"),
		      tr("blog-read-more"));
      bindWidget("read-more", moreAnchor);
    } else {
      bindString("read-more", WString::Empty);
    }

    commentCount_ = new WText("(" + post_->commentCount() + ")");

    WAnchor *commentsAnchor
      = new WAnchor(WLink(WLink::InternalPath,
			  basePath_ + post_->permaLink() + "/comments"));
    commentsAnchor->addWidget(commentCount_);
    bindWidget("comment-count", commentsAnchor);

    break; }
  case Edit: {
    setTemplateText(tr("blog-post-edit"));

    bindWidget("title-edit", titleEdit_ = new WLineEdit(post_->title));
    bindWidget("brief-edit", briefEdit_ = new WTextArea(post_->briefSrc));
    bindWidget("body-edit", bodyEdit_ = new WTextArea(post_->bodySrc));

    WPushButton *saveButton = new WPushButton(tr("save"));
    WPushButton *cancelButton = new WPushButton(tr("cancel"));
    bindWidget("save", saveButton);
    bindWidget("cancel", cancelButton);

    saveButton->clicked().connect(this, &PostView::saveEdit);
    cancelButton->clicked().connect(this, &PostView::showView);

    break; }
  }

  if (type == Detail || type == Brief) {
    if (session_.user() == post_->author) {
      WPushButton *publishButton;
      if (post_->state != Post::Published) {
	publishButton = new WPushButton(tr("publish"));
	publishButton->clicked().connect(this, &PostView::publish);
      } else {
	publishButton = new WPushButton(tr("retract"));
	publishButton->clicked().connect(this, &PostView::retract);
      }
      bindWidget("publish", publishButton);

      WPushButton *editButton = new WPushButton(tr("edit"));
      editButton->clicked().connect(this, &PostView::showEdit);
      bindWidget("edit", editButton);

      WPushButton *deleteButton = new WPushButton(tr("delete"));
      deleteButton->clicked().connect(this, &PostView::rm);
      bindWidget("delete", deleteButton);
    } else {
      bindString("publish", WString::Empty);
      bindString("edit", WString::Empty);
      bindString("delete", WString::Empty);
    }
  }

  WAnchor *postAnchor
    = new WAnchor(WLink(WLink::InternalPath,
			basePath_ + "author/" + post_->author->name.toUTF8()),
		  post_->author->name);
  bindWidget("author", postAnchor);
}

void PostView::saveEdit()
{
  dbo::Transaction t(session_);

  bool newPost = post_.id() == -1;

  Post *post = post_.modify();

  post->title = titleEdit_->text();
  post->briefSrc = briefEdit_->text();
  post->bodySrc = bodyEdit_->text();

  post->briefHtml = asciidoc(post->briefSrc);
  post->bodyHtml = asciidoc(post->bodySrc);

  if (newPost) {
    session_.add(post_);

    post->date = Wt::WDateTime::currentDateTime();
    post->state = Post::Unpublished;
    post->author = session_.user();

    dbo::ptr<Comment> rootComment = session_.add(new Comment);
    rootComment.modify()->post = post_;
  }

  session_.flush();

  render(viewType_);

  t.commit();
}

void PostView::showView()
{
  if (post_.id() == -1)
    delete this;
  else {
    dbo::Transaction t(session_);
    render(viewType_);
    t.commit();
  }
}

void PostView::publish()
{
  setState(Post::Published);
}

void PostView::retract()
{
  setState(Post::Unpublished);
}

void PostView::setState(Post::State state)
{
  dbo::Transaction t(session_);

  post_.modify()->state = state;
  if (state == Post::Published)
    post_.modify()->date = Wt::WDateTime::currentDateTime();

  render(viewType_);

  t.commit();
}

void PostView::showEdit()
{
  dbo::Transaction t(session_);

  render(Edit);

  t.commit();
}

void PostView::rm()
{
  dbo::Transaction t(session_);
  post_.remove();
  t.commit();

  delete this;
}

void PostView::updateCommentCount(dbo::ptr<Comment> comment)
{
  if (comment->post == post_) {
    std::string count = comment->post->commentCount();

    if (commentCount_->text().toUTF8()[0] == '(')
      commentCount_->setText("(" + count + ")");
    else
      commentCount_->setText(count);
  }
}
