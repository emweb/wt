/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>

#include <Wt/WContainerWidget>
#include <Wt/WMenu>
#include <Wt/WSubMenuItem>

namespace Wt {

WSubMenuItem::WSubMenuItem(const WString& text, WWidget *contents,
			   LoadPolicy policy)
  : WMenuItem(text, contents, policy)
{ }

void WSubMenuItem::setSubMenu(WMenu *subMenu)
{
  subMenu_ = subMenu;

  subMenu_->itemSelected().connect(this, &WSubMenuItem::subItemSelected);
}

void WSubMenuItem::subItemSelected()
{
  menu()->select(-1);
  renderSelected(true);
}

WWidget *WSubMenuItem::createItemWidget()
{
  if (subMenu_) {
    WContainerWidget *contents = new WContainerWidget();
    WWidget *anchor = WMenuItem::createItemWidget();
    contents->addWidget(anchor);
    contents->addWidget(subMenu_);

    return contents;
  } else
    return WMenuItem::createItemWidget();
}

void WSubMenuItem::updateItemWidget(WWidget *itemWidget)
{
  if (subMenu_) {
    WContainerWidget *contents = dynamic_cast<WContainerWidget *>(itemWidget);
    WWidget *anchor = contents->widget(0);
    WMenuItem::updateItemWidget(anchor);
  } else
    WMenuItem::updateItemWidget(itemWidget);
}

void WSubMenuItem::renderSelected(bool selected)
{
  WMenuItem::renderSelected(selected);
}

SignalBase& WSubMenuItem::activateSignal()
{
  if (subMenu_) {
    WContainerWidget *contents = dynamic_cast<WContainerWidget *>(itemWidget());
    WInteractWidget *wi = dynamic_cast<WInteractWidget *>
      (contents->widget(0)->webWidget());

    return wi->clicked();
  } else
    return WMenuItem::activateSignal();
}

std::string WSubMenuItem::pathComponent() const
{
  return WMenuItem::pathComponent() + "/";
}

void WSubMenuItem::setFromInternalPath(const std::string& path)
{
  WMenuItem::setFromInternalPath(path);

  if (subMenu_ && subMenu_->internalPathEnabled())
    subMenu_->internalPathChanged(path);
}

}
