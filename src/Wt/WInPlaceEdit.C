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

  text_ = new WText(text, PlainText, impl_);
  text_->decorationStyle().setCursor(ArrowCursor);

  edit_ = new WLineEdit(text, impl_);
  edit_->setTextSize(20);
  save_ = 0;
  cancel_ = 0;
  edit_->hide();

  /*
   * This is stateless implementation heaven
   */
  text_->clicked().connect(text_, &WWidget::hide);
  text_->clicked().connect(edit_, &WWidget::show);
  text_->clicked().connect(edit_, &WFormWidget::setFocus);

  edit_->enterPressed().connect(edit_, &WFormWidget::disable);
  edit_->enterPressed().connect(this, &WInPlaceEdit::save);
  edit_->enterPressed().preventDefaultAction();

  edit_->escapePressed().connect(edit_, &WWidget::hide);
  edit_->escapePressed().connect(text_, &WWidget::show);
  edit_->escapePressed().connect(this, &WInPlaceEdit::cancel);
  edit_->escapePressed().preventDefaultAction();

  setButtonsEnabled();
}

const WString& WInPlaceEdit::text() const
{
  return edit_->text();
}

void WInPlaceEdit::setText(const WString& text)
{
  if (text != WString::Empty)
    text_->setText(text);
  else
    text_->setText(emptyText_);

  edit_->setText(text);
}

void WInPlaceEdit::setEmptyText(const WString& emptyText)
{
  emptyText_ = emptyText;
}

const WString& WInPlaceEdit::emptyText()
{
  return emptyText_;
}

void WInPlaceEdit::save()
{
  edit_->hide();
  text_->show();

  if (edit_->text() != WString::Empty)
    text_->setText(edit_->text());
  else
    text_->setText(emptyText_);

  edit_->enable();

  valueChanged().emit(edit_->text());
}

void WInPlaceEdit::cancel()
{
  edit_->setText(text_->text());
}

void WInPlaceEdit::setButtonsEnabled(bool enabled)
{
  if (c1_.connected())
    c1_.disconnect();
  if (c2_.connected())
    c2_.disconnect();

  if (enabled) {
    save_ = new WPushButton(tr("Wt.WInPlaceEdit.Save"), impl_);
    cancel_ = new WPushButton(tr("Wt.WInPlaceEdit.Cancel"), impl_);
    save_->hide();
    cancel_->hide();

    text_->clicked().connect(save_,   &WWidget::show);
    text_->clicked().connect(cancel_, &WWidget::show);

    edit_->enterPressed() .connect(save_,   &WWidget::hide);
    edit_->enterPressed() .connect(cancel_, &WWidget::hide);
    edit_->escapePressed().connect(save_,   &WWidget::hide);
    edit_->escapePressed().connect(cancel_, &WWidget::hide);

    save_->clicked().connect(save_,   &WWidget::hide);
    save_->clicked().connect(cancel_, &WWidget::hide);
    save_->clicked().connect(edit_,   &WFormWidget::disable);
    save_->clicked().connect(this,    &WInPlaceEdit::save);
    
    cancel_->clicked().connect(save_,   &WWidget::hide);
    cancel_->clicked().connect(cancel_, &WWidget::hide);
    cancel_->clicked().connect(edit_,   &WWidget::hide);
    cancel_->clicked().connect(text_,   &WWidget::show);
    cancel_->clicked().connect(this,    &WInPlaceEdit::cancel);

  } else {
    delete save_;
    save_ = 0;
    delete cancel_;
    cancel_ = 0;
    c1_ = edit_->blurred().connect(edit_, &WFormWidget::disable);
    c2_ = edit_->blurred().connect(this,  &WInPlaceEdit::save);
  }
}

}
