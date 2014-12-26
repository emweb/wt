/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WIcon"
#include "Wt/WImage"
#include "Wt/WMessageBox"
#include "Wt/WPushButton"
#include "Wt/WSignalMapper"
#include "Wt/WText"

namespace Wt {

StandardButton WMessageBox::order_[] = { Ok, Yes, YesAll, Retry, No,
					 NoAll, Abort, Ignore, Cancel };

const char *WMessageBox::buttonText_[]
  = { "Wt.WMessageBox.Ok", "Wt.WMessageBox.Yes", "Wt.WMessageBox.YesToAll",
      "Wt.WMessageBox.Retry", "Wt.WMessageBox.No", "Wt.WMessageBox.NoToAll",
      "Wt.WMessageBox.Abort", "Wt.WMessageBox.Ignore", "Wt.WMessageBox.Cancel"
    };

WMessageBox::WMessageBox(WObject *parent)
  : WDialog(parent),
    icon_(NoIcon),
    result_(NoButton),
    buttonClicked_(this),
    defaultButton_(0),
    escapeButton_(0)
{
  create();
}

WMessageBox::WMessageBox(const WString& caption, const WString& text,
			 Icon icon, WFlags<StandardButton> buttons,
			 WObject *parent)
  : WDialog(caption, parent),
    icon_(NoIcon),
    buttonClicked_(this),
    defaultButton_(0),
    escapeButton_(0)
{
  create();

  setText(text);
  setIcon(icon);
  setButtons(buttons);
}

void WMessageBox::create()
{
  iconW_ = new WIcon(contents());
  text_ = new WText(contents());
  contents()->addStyleClass("Wt-msgbox-body");

  buttonMapper_ = new WSignalMapper<StandardButton>(this);
  buttonMapper_->mapped().connect(this, &WMessageBox::onButtonClick);

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
  addButton(b, result);
  return b;
}

void WMessageBox::addButton(WPushButton *button, StandardButton result)
{
  buttons_.push_back(Button());
  buttons_.back().button = button;
  buttons_.back().result = result;

  footer()->addWidget(button);
  buttonMapper_->mapConnect(button->clicked(), result);

  if (button->isDefault())
    setDefaultButton(button);
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

  iconW_->toggleStyleClass("Wt-msgbox-icon", icon_ != NoIcon);
  text_->toggleStyleClass("Wt-msgbox-text", icon_ != NoIcon);
  iconW_->setSize(icon_ != NoIcon ? 2.5 : 1);

  switch (icon_) {
  case NoIcon:
    iconW_->setName(std::string());
    break;
  case Information:
    iconW_->setName("info");
    break;
  case Warning:
    iconW_->setName("warning");
    break;
  case Critical:
    iconW_->setName("exclamation");
    break;
  case Question:
    iconW_->setName("question");
  }
}

void WMessageBox::setButtons(WFlags<StandardButton> buttons)
{
  setStandardButtons(buttons);
}

void WMessageBox::setStandardButtons(WFlags<StandardButton> buttons)
{
  buttons_.clear();
  footer()->clear();

  defaultButton_ = escapeButton_ = 0;

  for (int i = 0; i < 9; ++i)
    if (buttons & order_[i])
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

  return 0;
}

void WMessageBox::onButtonClick(StandardButton b)
{
  result_ = b;
  buttonClicked_.emit(b);
}

void WMessageBox::onFinished()
{
  if (result() == Rejected) {
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
	WPushButton *b = button(Cancel);
	if (b) {
	  onButtonClick(Cancel);
	  return;
	}
	b = button(No);
	if (b) {
	  onButtonClick(No);
	  return;
	}

	onButtonClick(NoButton);
      }
    }
  }
}

void WMessageBox::setHidden(bool hidden, const WAnimation& animation)
{
  if (!hidden) {
    if (!defaultButton_) {
      for (unsigned i = 0; i < buttons_.size(); ++i) {
	if (buttons_[i].result == Ok || buttons_[i].result == Yes) {
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
  WMessageBox box(caption, text, Information, buttons);
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
