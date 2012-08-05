/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WFitLayout"
#include "Wt/WLogger"

namespace Wt {

LOGGER("WFitLayout");

WFitLayout::WFitLayout(WWidget *parent)
  : WLayout(),
    item_(0)
{ 
  if (parent)
    setLayoutInParent(parent);
}

WFitLayout::~WFitLayout()
{
  delete item_;
}

void WFitLayout::addItem(WLayoutItem *item)
{
  if (item_) {
    LOG_ERROR("addItem(): already have a widget");
    return;
  }

  item_ = item;
  updateAddItem(item);
}

void WFitLayout::removeItem(WLayoutItem *item)
{
  if (item == item_) {
    item_ = 0;
    updateRemoveItem(item);
  }
}

WLayoutItem *WFitLayout::itemAt(int index) const
{
  return item_;
}

void WFitLayout::clear()
{
  clearLayoutItem(item_);
  item_ = 0;
}

int WFitLayout::indexOf(WLayoutItem *item) const
{
  if (item_ == item)
    return 0;
  else
    return -1;
}

int WFitLayout::count() const
{
  return item_ ? 1 : 0;
}

}
