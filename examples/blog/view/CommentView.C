/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "CommentView.h"

#include "../model/BlogSession.h"
#include "../model/Comment.h"
#include "../model/User.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>

namespace dbo = Wt::Dbo;

CommentView::CommentView(BlogSession& session, dbo::ptr<Comment> comment)
  : session_(session),
    comment_(comment)
{
  comment_ = comment;

  renderView();
}

CommentView::CommentView(BlogSession& session, long long parentId)
  : session_(session)
{
  dbo::ptr<Comment> parent = session_.load<Comment>(parentId);

  comment_ = Wt::cpp14::make_unique<Comment>();
  comment_.modify()->parent = parent;
  comment_.modify()->post = parent->post;

  edit();
}

bool CommentView::isNew() const
{
  return comment_.id() == -1;
}

void CommentView::edit()
{
  clear();

  dbo::Transaction t(session_);

  setTemplateText(tr("blog-edit-comment"));

  auto editArea = Wt::cpp14::make_unique<Wt::WTextArea>();
  editArea_ = editArea.get();
  editArea_->setText(comment_->textSrc());
  editArea_->setFocus();

  auto save = Wt::cpp14::make_unique<Wt::WPushButton>(tr("save"));
  save->clicked().connect(this, &CommentView::save);

  auto cancel = Wt::cpp14::make_unique<Wt::WPushButton>(tr("cancel"));
  cancel->clicked().connect(this, &CommentView::cancel);

  bindWidget("area", std::move(editArea));
  bindWidget("save", std::move(save));
  bindWidget("cancel", std::move(cancel));

  t.commit();
}

void CommentView::cancel()
{
  if (isNew())
    removeFromParent();
  else {
    dbo::Transaction t(session_);
    renderView();
    t.commit();
  }
}

void CommentView::renderTemplate(std::ostream& result)
{
  dbo::Transaction t(session_);

  WTemplate::renderTemplate(result);

  comment_.purge();

  t.commit();
}

void CommentView::resolveString(const std::string& varName,
                                const std::vector<Wt::WString>& args,
				std::ostream& result)
{
  if (varName == "author")
    format(result, comment_->author ? comment_->author->name : "anonymous");
  else if (varName == "date")
    format(result, comment_->date.timeTo(Wt::WDateTime::currentDateTime())
	   + " ago");
  else if (varName == "contents")
    format(result, comment_->textHtml(), Wt::TextFormat::XHTML);
  else
    WTemplate::resolveString(varName, args, result);
}

void CommentView::renderView()
{
  clear();

  bool isRootComment = !comment_->parent;
  setTemplateText(isRootComment ? tr("blog-root-comment")
		  : tr("blog-comment"));

  bindString("collapse-expand", Wt::WString::Empty); // NYI

  auto replyText
      = Wt::cpp14::make_unique<Wt::WText>(isRootComment ? tr("comment-add")
			       : tr("comment-reply"));
  replyText->setStyleClass("link");
  replyText->clicked().connect(this, &CommentView::reply);
  bindWidget("reply", std::move(replyText));

  bool mayEdit = session_.user()
    && (comment_->author == session_.user()
	|| session_.user()->role == User::Admin);

  if (mayEdit) {
    auto editText
        = Wt::cpp14::make_unique<Wt::WText>(tr("comment-edit"));
    editText->setStyleClass("link");
    editText->clicked().connect(this, &CommentView::edit);
    bindWidget("edit", std::move(editText));
  } else
    bindString("edit", Wt::WString::Empty);

  bool mayDelete
    = (session_.user() && session_.user() == comment_->author)
    || session_.user() == comment_->post->author; 

  if (mayDelete) {
    auto deleteText
        = Wt::cpp14::make_unique<Wt::WText>(tr("comment-delete"));
    deleteText->setStyleClass("link");
    deleteText->clicked().connect(this, &CommentView::rm);
    bindWidget("delete", std::move(deleteText));
  } else
    bindString("delete", Wt::WString::Empty);

  typedef std::vector< dbo::ptr<Comment> > CommentVector;
  CommentVector comments;
  {
    dbo::collection<dbo::ptr<Comment> > cmts
      = comment_->children.find().orderBy("date");
    comments.insert(comments.end(), cmts.begin(), cmts.end());
  }

  auto children
      = Wt::cpp14::make_unique<Wt::WContainerWidget>();
  for (int i = (int)comments.size() - 1; i >= 0; --i)
    children->addWidget(Wt::cpp14::make_unique<CommentView>(session_, comments[i]));

  bindWidget("children", std::move(children));
}

void CommentView::save()
{
  dbo::Transaction t(session_);

  bool isNew = comment_.id() == -1;

  Comment *comment = comment_.modify();

  comment->setText(editArea_->text());

  if (isNew) {
    session_.add(comment_);
    comment->date = Wt::WDateTime::currentDateTime();
    comment->author = session_.user();
    session_.commentsChanged().emit(comment_);
  }

  renderView();

  t.commit();
}

void CommentView::reply()
{
  dbo::Transaction t(session_);

  Wt::WContainerWidget *c = resolve<Wt::WContainerWidget *>("children");
  c->insertWidget(0, Wt::cpp14::make_unique<CommentView>(session_, comment_.id()));

  t.commit();
}

void CommentView::rm()
{
  dbo::Transaction t(session_);

  comment_.modify()->setDeleted();
  renderView();

  t.commit();
}
