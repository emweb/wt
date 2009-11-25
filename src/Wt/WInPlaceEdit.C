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
  text_->clicked().connect(SLOT(text_,   WWidget::hide));
  text_->clicked().connect(SLOT(edit_,   WWidget::show));
  text_->clicked().connect(SLOT(edit_,   WFormWidget::setFocus));

  edit_->enterPressed().connect(SLOT(edit_,   WFormWidget::disable));
  edit_->enterPressed().connect(SLOT(this,    WInPlaceEdit::save));
  edit_->enterPressed().setPreventDefault(true);

  edit_->escapePressed().connect(SLOT(edit_, WWidget::hide));
  edit_->escapePressed().connect(SLOT(text_, WWidget::show));
  edit_->escapePressed().connect(SLOT(this, WInPlaceEdit::cancel));
  edit_->escapePressed().setPreventDefault(true);

  setButtonsEnabled();
}

const WString& WInPlaceEdit::text() const
{
  return text_->text();
}

void WInPlaceEdit::setText(const WString& text)
{
  text_->setText(text);
  edit_->setText(text);
}

void WInPlaceEdit::save()
{
  edit_->hide();
  text_->show();
  text_->setText(edit_->text());
  edit_->enable();

  valueChanged().emit(edit_->text());
}

void WInPlaceEdit::cancel()
{
  edit_->setText(text_->text());
}

void WInPlaceEdit::setButtonsEnabled(bool enabled)
{
  if (c1_.connected()) c1_.disconnect();
  if (c2_.connected()) c2_.disconnect();
  if (enabled) {
    save_ = new WPushButton("Save", impl_);
    cancel_ = new WPushButton("Cancel", impl_);
    save_->hide();
    cancel_->hide();

    text_->clicked().connect(SLOT(save_,   WWidget::show));
    text_->clicked().connect(SLOT(cancel_, WWidget::show));
    
    edit_->enterPressed().connect(SLOT(save_,    WWidget::hide));
    edit_->enterPressed().connect(SLOT(cancel_,  WWidget::hide));
    edit_->escapePressed().connect(SLOT(save_,   WWidget::hide));
    edit_->escapePressed().connect(SLOT(cancel_, WWidget::hide));

    save_->clicked().connect(SLOT(save_,   WWidget::hide));
    save_->clicked().connect(SLOT(cancel_, WWidget::hide));
    save_->clicked().connect(SLOT(edit_,   WFormWidget::disable));
    save_->clicked().connect(SLOT(this,    WInPlaceEdit::save));
    
    cancel_->clicked().connect(SLOT(save_,   WWidget::hide));
    cancel_->clicked().connect(SLOT(cancel_, WWidget::hide));
    cancel_->clicked().connect(SLOT(edit_,   WWidget::hide));
    cancel_->clicked().connect(SLOT(text_,   WWidget::show));
    cancel_->clicked().connect(SLOT(this,    WInPlaceEdit::cancel));
   
  } else {
    delete save_;
    save_ = 0;
    delete cancel_;
    cancel_ = 0;
    c1_ = edit_->blurred().connect(SLOT(edit_, WFormWidget::disable));
    c2_ = edit_->blurred().connect(SLOT(this,  WInPlaceEdit::save));
  }
}

}
