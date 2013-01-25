/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPushButton"
#include "Wt/WSplitButton"
#include "Wt/WToolBar"

namespace Wt {

WSplitButton::WSplitButton(WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  init(WString::Empty);
}

WSplitButton::WSplitButton(const WString& label, WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  init(label);
}

void WSplitButton::init(const WString& label)
{
  setImplementation(impl_ = new WToolBar());

  impl_->setInline(true);
  impl_->addButton(new WPushButton(label));
  impl_->addButton(new WPushButton());

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

void WSplitButton::setMenu(WPopupMenu *popupMenu)
{
  dropDownButton()->setMenu(popupMenu);
}

WPopupMenu *WSplitButton::menu() const
{
  return dropDownButton()->menu();
}

}
