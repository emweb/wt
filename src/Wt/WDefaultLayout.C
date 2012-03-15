/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WDefaultLayout"
#include "WebUtils.h"

namespace Wt {

WDefaultLayout::WDefaultLayout(WWidget *parent)
  : WLayout()
{ 
  if (parent)
    setLayoutInParent(parent);
}

WDefaultLayout::~WDefaultLayout()
{
  for (unsigned i = 0; i < items_.size(); ++i)
    delete items_[i];
}

void WDefaultLayout::addItem(WLayoutItem *item)
{
  items_.push_back(item);
  updateAddItem(item);
}

void WDefaultLayout::removeItem(WLayoutItem *item)
{
  int i = indexOf(item);

  if (i != -1) {
    items_.erase(items_.begin() + i);
    updateRemoveItem(item);
  }
}

WLayoutItem *WDefaultLayout::itemAt(int index) const
{
  return items_[index];
}

int WDefaultLayout::indexOf(WLayoutItem *item) const
{
  return Utils::indexOf(items_, item);
}

int WDefaultLayout::count() const
{
  return items_.size();
}

void WDefaultLayout::clear()
{
  for (unsigned i = 0; i < items_.size(); ++i)
    clearLayoutItem(items_[i]);
  items_.clear();
}

}
