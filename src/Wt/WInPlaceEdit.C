/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WInPlaceEdit.h"
#include "Wt/WCssDecorationStyle.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WPushButton.h"
#include "Wt/WText.h"
#include "Wt/WTheme.h"
#include "Wt/WLineEdit.h"

namespace Wt {

WInPlaceEdit::WInPlaceEdit()
{
  create();
}

WInPlaceEdit::WInPlaceEdit(const WString& text)
{
  create();
  setText(text);
}

WInPlaceEdit::WInPlaceEdit(bool buttons, const WString& text)
{
  create();
  setText(text);
  setButtonsEnabled(buttons);
}

void WInPlaceEdit::create()
{
  setImplementation(std::unique_ptr<WWidget>(impl_ = new WContainerWidget()));
  setInline(true);

  impl_->addWidget(std::unique_ptr<WWidget>
		   (text_ = new WText(WString::Empty, TextFormat::Plain)));
  text_->decorationStyle().setCursor(Cursor::Arrow);

  impl_->addWidget(std::unique_ptr<WWidget>(editing_ = new WContainerWidget()));
  editing_->setInline(true);
  editing_->hide();

  editing_->addWidget(std::unique_ptr<WWidget>(edit_ = new WLineEdit()));
  edit_->setTextSize(20);
  save_ = nullptr;
  cancel_ = nullptr;

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

  editing_->addWidget
    (std::unique_ptr<WWidget>(buttons_ = new WContainerWidget()));
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
    c2_.disconnect();

    buttons_->addWidget
      (std::unique_ptr<WWidget>
       (save_ = new WPushButton(tr("Wt.WInPlaceEdit.Save"))));
    buttons_->addWidget
      (std::unique_ptr<WWidget>
       (cancel_ = new WPushButton(tr("Wt.WInPlaceEdit.Cancel"))));

    save_->clicked().connect(edit_, &WFormWidget::disable);
    save_->clicked().connect(save_, &WFormWidget::disable);
    save_->clicked().connect(cancel_, &WFormWidget::disable);
    save_->clicked().connect(this, &WInPlaceEdit::save);
    
    cancel_->clicked().connect(editing_, &WWidget::hide);
    cancel_->clicked().connect(text_, &WWidget::show);
    cancel_->clicked().connect(this, &WInPlaceEdit::cancel);
  } else if (!enabled && save_) {
    save_->parent()->removeWidget(save_);
    cancel_->parent()->removeWidget(cancel_);
    save_ = nullptr;
    cancel_ = nullptr;

    c2_ = edit_->blurred().connect(this, &WInPlaceEdit::save);
  }
}

void WInPlaceEdit::render(WFlags<RenderFlag> flags)
{
  if (save_ && flags.test(RenderFlag::Full))
    wApp->theme()->apply(this, editing_, InPlaceEditing);

  WCompositeWidget::render(flags);
}
}
