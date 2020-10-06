/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "CommentView.h"
#include "PostView.h"
#include "../asciidoc/asciidoc.h"

#include "../model/BlogSession.h"
#include "../model/User.h"

#include <Wt/WAnchor.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>

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
                             const std::vector<Wt::WString>& args,
			     std::ostream& result)
{
  if (varName == "title")
    format(result, post_->title);
  else if (varName == "date")
    format(result, post_->date.toString("dddd, MMMM d, yyyy @ HH:mm"));
  else if (varName == "brief") {
    if (!post_->briefSrc.empty())
      format(result, post_->briefHtml, Wt::TextFormat::XHTML);
    else
      format(result, post_->bodyHtml, Wt::TextFormat::XHTML);
  } else if (varName == "brief+body") {
    format(result, "<div>" + post_->briefHtml + "</div>"
	   "<div id=\"" + basePath_ + post_->permaLink() + "/more\">"
           "<div>" + post_->bodyHtml + "</div></div>", Wt::TextFormat::UnsafeXHTML);
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

    session_.commentsChanged().connect(this, &PostView::updateCommentCount);

    commentCount_ =
        bindWidget("comment-count", std::make_unique<Wt::WText>(post_->commentCount()));
    bindWidget("comments", std::make_unique<CommentView>(session_, post_->rootComment()));
    bindString("anchor", basePath_ + post_->permaLink());

    break;
  }
  case Brief: {
    setTemplateText(tr("blog-post-brief"));

    auto titleAnchor
      = std::make_unique<Wt::WAnchor>(Wt::WLink(Wt::LinkType::InternalPath,
                                          basePath_ + post_->permaLink()),
                                    post_->title);
    bindWidget("title", std::move(titleAnchor));

    if (!post_->briefSrc.empty()) {
      auto moreAnchor
        = std::make_unique<Wt::WAnchor>(Wt::WLink(Wt::LinkType::InternalPath,
                                            basePath_ + post_->permaLink() + "/more"),
                                      tr("blog-read-more"));
      bindWidget("read-more", std::move(moreAnchor));
    } else {
      bindString("read-more", Wt::WString::Empty);
    }

    auto commentsAnchor
      = std::make_unique<Wt::WAnchor>(Wt::WLink(Wt::LinkType::InternalPath,
                                    basePath_ + post_->permaLink() + "/comments"));
    commentCount_ =
        commentsAnchor->addWidget(std::make_unique<Wt::WText>("(" + post_->commentCount() + ")"));
    bindWidget("comment-count", std::move(commentsAnchor));

    break; }
  case Edit: {
    setTemplateText(tr("blog-post-edit"));

    titleEdit_ = bindWidget("title-edit", std::make_unique<Wt::WLineEdit>(post_->title));
    briefEdit_ = bindWidget("brief-edit", std::make_unique<Wt::WTextArea>(post_->briefSrc));
    bodyEdit_ = bindWidget("body-edit", std::make_unique<Wt::WTextArea>(post_->bodySrc));

    auto saveButton = bindWidget("save", std::make_unique<Wt::WPushButton>(tr("save")));
    auto cancelButton = bindWidget("cancel", std::make_unique<Wt::WPushButton>(tr("cancel")));

    saveButton->clicked().connect(this, &PostView::saveEdit);
    cancelButton->clicked().connect(this, &PostView::showView);

    break; }
  }

  if (type == Detail || type == Brief) {
    if (session_.user() == post_->author) {
      std::unique_ptr<Wt::WPushButton> publishButton;
      if (post_->state != Post::Published) {
        publishButton
            = std::make_unique<Wt::WPushButton>(tr("publish"));
	publishButton->clicked().connect(this, &PostView::publish);
      } else {
        publishButton
            = std::make_unique<Wt::WPushButton>(tr("retract"));
	publishButton->clicked().connect(this, &PostView::retract);
      }
      bindWidget("publish", std::move(publishButton));

      auto editButton(std::make_unique<Wt::WPushButton>(tr("edit")));
      editButton->clicked().connect(this, &PostView::showEdit);
      bindWidget("edit", std::move(editButton));

      auto deleteButton(std::make_unique<Wt::WPushButton>(tr("delete")));
      deleteButton->clicked().connect(this, &PostView::rm);
      bindWidget("delete", std::move(deleteButton));
    } else {
      bindString("publish", Wt::WString::Empty);
      bindString("edit", Wt::WString::Empty);
      bindString("delete", Wt::WString::Empty);
    }
  }

  auto postAnchor = std::make_unique<Wt::WAnchor>(Wt::WLink(Wt::LinkType::InternalPath,
			basePath_ + "author/" + post_->author->name.toUTF8()),
		  post_->author->name);
  bindWidget("author", std::move(postAnchor));
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

    dbo::ptr<Comment> rootComment = session_.add(std::make_unique<Comment>());
    rootComment.modify()->post = post_;
  }

  session_.flush();

  render(viewType_);

  t.commit();
}

void PostView::showView()
{
  if (post_.id() == -1)
    this->removeFromParent();
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

  this->removeFromParent();
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
