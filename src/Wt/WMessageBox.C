/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WIcon.h"
#include "Wt/WImage.h"
#include "Wt/WMessageBox.h"
#include "Wt/WPushButton.h"
#include "Wt/WText.h"

namespace Wt {

StandardButton WMessageBox::order_[] = { 
  StandardButton::Ok,
  StandardButton::Yes, 
  StandardButton::YesAll, 
  StandardButton::Retry, 
  StandardButton::No,
  StandardButton::NoAll, 
  StandardButton::Abort, 
  StandardButton::Ignore, 
  StandardButton::Cancel 
};

const char *WMessageBox::buttonText_[]
  = { "Wt.WMessageBox.Ok", "Wt.WMessageBox.Yes", "Wt.WMessageBox.YesToAll",
      "Wt.WMessageBox.Retry", "Wt.WMessageBox.No", "Wt.WMessageBox.NoToAll",
      "Wt.WMessageBox.Abort", "Wt.WMessageBox.Ignore", "Wt.WMessageBox.Cancel"
    };

WMessageBox::WMessageBox()
  : icon_(Icon::None),
    result_(StandardButton::None),
    defaultButton_(nullptr),
    escapeButton_(nullptr)
{
  create();
}

WMessageBox::WMessageBox(const WString& caption, const WString& text,
			 Icon icon, WFlags<StandardButton> buttons)
  : WDialog(caption),
    icon_(Icon::None),
    defaultButton_(nullptr),
    escapeButton_(nullptr)
{
  create();

  setText(text);
  setIcon(icon);
  setStandardButtons(buttons);
}

void WMessageBox::create()
{
  std::unique_ptr<WIcon> icon(iconW_ = new WIcon());
  contents()->addWidget(std::move(icon));

  std::unique_ptr<WText> text(text_ = new WText());
  contents()->addWidget(std::move(text));

  contents()->addStyleClass("Wt-msgbox-body");

  rejectWhenEscapePressed();

  finished().connect(this, &WMessageBox::onFinished);
}

WPushButton *WMessageBox::addButton(StandardButton result)
{
  return addButton(standardButtonText(result), result);
}

WPushButton *WMessageBox::addButton(const WString& text, StandardButton result)
{
  WPushButton *b = new WPushButton(text);
  addButton(std::unique_ptr<WPushButton>(b), result);
  return b;
}

void WMessageBox::addButton(std::unique_ptr<WPushButton> button,
			    const StandardButton result)
{
  buttons_.push_back(Button());
  buttons_.back().button = button.get();
  buttons_.back().result = result;
  
  button->clicked().connect
    (this, std::bind(&WMessageBox::onButtonClick, this, result));

  if (button->isDefault())
    setDefaultButton(button.get());

  footer()->addWidget(std::move(button));
}

void WMessageBox::setDefaultButton(WPushButton *button)
{
  if (defaultButton_)
    defaultButton_->setDefault(false);
  defaultButton_ = button;
  if (defaultButton_)
    defaultButton_->setDefault(true);
}

void WMessageBox::setDefaultButton(StandardButton button)
{
  WPushButton *b = this->button(button);
  if (b)
    setDefaultButton(b);
}

void WMessageBox::setEscapeButton(WPushButton *button)
{
  escapeButton_ = button;
}

void WMessageBox::setEscapeButton(StandardButton button)
{
  WPushButton *b = this->button(button);
  if (b)
    setEscapeButton(b);
}

void WMessageBox::setText(const WString& text)
{
  text_->setText(text);
}

const WString& WMessageBox::text() const
{
  return text_->text();
}

void WMessageBox::setIcon(Icon icon)
{
  icon_ = icon;

  iconW_->toggleStyleClass("Wt-msgbox-icon", icon_ != Icon::None);
  text_->toggleStyleClass("Wt-msgbox-text", icon_ != Icon::None);
  iconW_->setSize(icon_ != Icon::None ? 2.5 : 1);

  switch (icon_) {
  case Icon::None:
    iconW_->setName(std::string());
    break;
  case Icon::Information:
    iconW_->setName("info");
    break;
  case Icon::Warning:
    iconW_->setName("warning");
    break;
  case Icon::Critical:
    iconW_->setName("exclamation");
    break;
  case Icon::Question:
    iconW_->setName("question");
  }
}

void WMessageBox::setStandardButtons(WFlags<StandardButton> buttons)
{
  buttons_.clear();
  footer()->clear();

  defaultButton_ = escapeButton_ = nullptr;

  for (int i = 0; i < 9; ++i)
    if (buttons.test(order_[i]))
      addButton(order_[i]);
}

WFlags<StandardButton> WMessageBox::standardButtons() const
{
  WFlags<StandardButton> result;

  for (unsigned i = 0; i < buttons_.size(); ++i)
    result |= buttons_[i].result;

  return result;
}

std::vector<WPushButton *> WMessageBox::buttons() const
{
  std::vector<WPushButton *> result;

  for (unsigned i = 0; i < buttons_.size(); ++i)
    result.push_back(buttons_[i].button);

  return result;
}

WPushButton *WMessageBox::button(StandardButton b)
{
  for (unsigned i = 0; i < buttons_.size(); ++i)
    if (buttons_[i].result == b)
      return buttons_[i].button;

  return nullptr;
}

void WMessageBox::onButtonClick(StandardButton b)
{
  result_ = b;
  buttonClicked_.emit(b);
}

void WMessageBox::onFinished()
{
  if (result() == DialogCode::Rejected) {
    if (escapeButton_) {
      for (unsigned i = 0; i < buttons_.size(); ++i) {
	if (buttons_[i].button == escapeButton_) {
	  onButtonClick(buttons_[i].result);
	  return;
	}
      }
    } else {
      if (buttons_.size() == 1) {
	onButtonClick(buttons_[0].result);
	return;
      } else {
	WPushButton *b = button(StandardButton::Cancel);
	if (b) {
	  onButtonClick(StandardButton::Cancel);
	  return;
	}
	b = button(StandardButton::No);
	if (b) {
	  onButtonClick(StandardButton::No);
	  return;
	}

	onButtonClick(StandardButton::None);
      }
    }
  }
}

void WMessageBox::setHidden(bool hidden, const WAnimation& animation)
{
  if (!hidden) {
    if (!defaultButton_) {
      for (unsigned i = 0; i < buttons_.size(); ++i) {
	if (buttons_[i].result == StandardButton::Ok || 
	    buttons_[i].result == StandardButton::Yes) {
	  buttons_[i].button->setDefault(true);	
	  break;
	}
      }
    }
  }

  WDialog::setHidden(hidden, animation);
}

StandardButton WMessageBox::show(const WString& caption,
				 const WString& text,
				 WFlags<StandardButton> buttons,
				 const WAnimation& animation)
{
  WMessageBox box(caption, text, Icon::Information, buttons);
  box.buttonClicked().connect(&box, &WMessageBox::accept);
  box.exec(animation);
  return box.buttonResult();
}

WString WMessageBox::standardButtonText(StandardButton button)
{
  for (unsigned i = 0; i < 9; ++i)
    if (order_[i] == button)
      return tr(buttonText_[i]);

  return WString::Empty;
}

}
