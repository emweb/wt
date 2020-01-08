/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPopupMenu.h"
#include "Wt/WPushButton.h"
#include "Wt/WSplitButton.h"
#include "Wt/WToolBar.h"

namespace Wt {

WSplitButton::WSplitButton()
{
  init(WString::Empty);
}

WSplitButton::WSplitButton(const WString& label)
{
  init(label);
}

void WSplitButton::init(const WString& label)
{
  setImplementation(std::unique_ptr<WToolBar>(impl_ = new WToolBar()));

  impl_->setInline(true);
  impl_->addButton(std::unique_ptr<WPushButton>(new WPushButton(label)));
  impl_->addButton(std::unique_ptr<WPushButton>(new WPushButton()));

  dropDownButton()->setStyleClass("dropdown-toggle");
}

WPushButton *WSplitButton::actionButton() const
{
  return dynamic_cast<WPushButton *>(impl_->widget(0));
}

WPushButton *WSplitButton::dropDownButton() const
{
  return dynamic_cast<WPushButton *>(impl_->widget(1));
}

void WSplitButton::setMenu(std::unique_ptr<WPopupMenu> popupMenu)
{
  dropDownButton()->setMenu(std::move(popupMenu));
}

WPopupMenu *WSplitButton::menu() const
{
  return dropDownButton()->menu();
}

}
