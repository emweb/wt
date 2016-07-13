/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WInPlaceEdit"
#include "Wt/WCssDecorationStyle"
#include "Wt/WContainerWidget"
#include "Wt/WPushButton"
#include "Wt/WText"
#include "Wt/WTheme"
#include "Wt/WLineEdit"

namespace Wt {

WInPlaceEdit::WInPlaceEdit(WContainerWidget *parent)
  : WCompositeWidget(parent),
    valueChanged_(this)
{
  create();
}

WInPlaceEdit::WInPlaceEdit(const WString& text, WContainerWidget *parent)
  : WCompositeWidget(parent),
    valueChanged_(this)
{
  create();
  setText(text);
}

WInPlaceEdit::WInPlaceEdit(bool buttons, 
			   const WString& text, WContainerWidget *parent)
  : WCompositeWidget(parent),
    valueChanged_(this)
{
  create();
  setText(text);
  setButtonsEnabled(buttons);
}

void WInPlaceEdit::create()
{
  setImplementation(impl_ = new WContainerWidget());
  setInline(true);

  text_ = new WText(WString::Empty, PlainText, impl_);
  text_->decorationStyle().setCursor(ArrowCursor);

  editing_ = new WContainerWidget(impl_);
  editing_->setInline(true);
  editing_->hide();

  edit_ = new WLineEdit(editing_);
  edit_->setTextSize(20);
  save_ = 0;
  cancel_ = 0;

  /*
   * This is stateless implementation heaven
   */
  text_->clicked().connect(text_, &WWidget::hide);
  text_->clicked().connect(editing_, &WWidget::show);
  text_->clicked().connect(edit_, &WWidget::setFocus);

  edit_->enterPressed().connect(edit_, &WFormWidget::disable);
  edit_->enterPressed().connect(this, &WInPlaceEdit::save);
  edit_->enterPressed().preventPropagation();

  edit_->escapePressed().connect(editing_, &WWidget::hide);
  edit_->escapePressed().connect(text_, &WWidget::show);
  edit_->escapePressed().connect(this, &WInPlaceEdit::cancel);
  edit_->escapePressed().preventPropagation();

  buttons_ = new WContainerWidget(editing_);
  buttons_->setInline(true);
  buttons_->addStyleClass("input-group-btn"); // FIXME !!!!

  setButtonsEnabled();
}

const WString& WInPlaceEdit::text() const
{
  return edit_->text();
}

void WInPlaceEdit::setText(const WString& text)
{
  empty_ = text.empty();

  if (!empty_)
    text_->setText(text);
  else
    text_->setText(placeholderText());

  edit_->setText(text);
}

void WInPlaceEdit::setEmptyText(const WString& text)
{
  setPlaceholderText(text);
}

void WInPlaceEdit::setPlaceholderText(const WString& text)
{
  placeholderText_ = text;

  edit_->setPlaceholderText(text);
  if (empty_)
    text_->setText(text);
}

const WString& WInPlaceEdit::placeholderText() const
{
  return placeholderText_;
}

void WInPlaceEdit::save()
{
  editing_->hide();
  text_->show();
  edit_->enable();
  if (save_)
    save_->enable();
  if (cancel_)
    cancel_->enable();

  bool changed
    = empty_ ? !edit_->text().empty() : edit_->text() != text_->text();

  if (changed) {
    setText(edit_->text());
    valueChanged().emit(edit_->text());
  }
}

void WInPlaceEdit::cancel()
{
  edit_->setText(empty_ ? WString::Empty : text_->text());
}

void WInPlaceEdit::setButtonsEnabled(bool enabled)
{
  if (enabled && !save_) {
    if (c2_.connected())
      c2_.disconnect();

    save_ = new WPushButton(tr("Wt.WInPlaceEdit.Save"), buttons_);
    cancel_ = new WPushButton(tr("Wt.WInPlaceEdit.Cancel"), buttons_);

    save_->clicked().connect(edit_, &WFormWidget::disable);
    save_->clicked().connect(save_, &WFormWidget::disable);
    save_->clicked().connect(cancel_, &WFormWidget::disable);
    save_->clicked().connect(this, &WInPlaceEdit::save);
    
    cancel_->clicked().connect(editing_, &WWidget::hide);
    cancel_->clicked().connect(text_, &WWidget::show);
    cancel_->clicked().connect(this, &WInPlaceEdit::cancel);
  } else if (!enabled && save_) {
    delete save_;
    save_ = 0;
    delete cancel_;
    cancel_ = 0;
    c2_ = edit_->blurred().connect(this, &WInPlaceEdit::save);
  }
}

void WInPlaceEdit::render(WFlags<RenderFlag> flags)
{
  if (save_ && flags & RenderFull)
    wApp->theme()->apply(this, editing_, InPlaceEditingRole);

  WCompositeWidget::render(flags);
}
}
