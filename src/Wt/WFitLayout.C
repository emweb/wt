/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WFitLayout"
#include "Wt/WWidgetItem"

#include "WtException.h"

namespace Wt {

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
  if (item_)
    throw WtException("WFitLayout supports only one widget");

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
