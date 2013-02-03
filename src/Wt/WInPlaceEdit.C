/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WInPlaceEdit"
#include "Wt/WCssDecorationStyle"
#include "Wt/WContainerWidget"
#include "Wt/WPushButton"
#include "Wt/WText"
#include "Wt/WLineEdit"

namespace Wt {

WInPlaceEdit::WInPlaceEdit(const WString& text, WContainerWidget *parent)
  : WCompositeWidget(parent),
    valueChanged_(this)
{
  setImplementation(impl_ = new WContainerWidget());
  setInline(true);

  text_ = new WText(WString::Empty, PlainText, impl_);
  text_->decorationStyle().setCursor(ArrowCursor);

  editing_ = new WContainerWidget(impl_);
  editing_->setInline(true);
  editing_->hide();
  editing_->addStyleClass("input-append"); // FIXME

  edit_ = new WLineEdit(editing_);
  edit_->setTextSize(20);
  save_ = 0;
  cancel_ = 0;

  /*
   * This is stateless implementation heaven
   */
  text_->clicked().connect(text_, &WWidget::hide);
  text_->clicked().connect(editing_, &WWidget::show);
  text_->clicked().connect(edit_, &WFormWidget::setFocus);

  edit_->enterPressed().connect(edit_, &WFormWidget::disable);
  edit_->enterPressed().connect(this, &WInPlaceEdit::save);
  edit_->enterPressed().preventPropagation();

  edit_->escapePressed().connect(editing_, &WWidget::hide);
  edit_->escapePressed().connect(text_, &WWidget::show);
  edit_->escapePressed().connect(this, &WInPlaceEdit::cancel);
  edit_->escapePressed().preventPropagation();

  setButtonsEnabled();

  setText(text);
}

const WString& WInPlaceEdit::text() const
{
  return edit_->text();
}

void WInPlaceEdit::setText(const WString& text)
{
  if (!text.empty()) {
    text_->setText(text);
    empty_ = false;
  } else {
    text_->setText(emptyText_);
    empty_ = true;
  }

  edit_->setText(text);
}

void WInPlaceEdit::setEmptyText(const WString& emptyText)
{
  emptyText_ = emptyText;
  if (empty_)
    text_->setText(emptyText_);
}

const WString& WInPlaceEdit::emptyText()
{
  return emptyText_;
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
  if (c1_.connected())
    c1_.disconnect();
  if (c2_.connected())
    c2_.disconnect();

  if (enabled) {
    save_ = new WPushButton(tr("Wt.WInPlaceEdit.Save"), editing_);
    cancel_ = new WPushButton(tr("Wt.WInPlaceEdit.Cancel"), editing_);

    save_->clicked().connect(edit_, &WFormWidget::disable);
    save_->clicked().connect(save_, &WFormWidget::disable);
    save_->clicked().connect(cancel_, &WFormWidget::disable);
    save_->clicked().connect(this, &WInPlaceEdit::save);
    
    cancel_->clicked().connect(editing_, &WWidget::hide);
    cancel_->clicked().connect(text_, &WWidget::show);
    cancel_->clicked().connect(this, &WInPlaceEdit::cancel);
  } else {
    delete save_;
    save_ = 0;
    delete cancel_;
    cancel_ = 0;
    c1_ = edit_->blurred().connect(edit_, &WFormWidget::disable);
    c2_ = edit_->blurred().connect(this, &WInPlaceEdit::save);
  }
}

}
