/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "CustomLayout.h"
#include "CustomLayoutImpl.h"

using namespace Wt;

CustomLayout::Item::Item(std::unique_ptr<WLayoutItem> item)
  : item_(std::move(item))
{ }

CustomLayout::CustomLayout()
{
  setImpl(std::make_unique<CustomLayoutImpl>(this));
}

void CustomLayout::addItem(std::unique_ptr<WLayoutItem> item)
{
  WLayoutItem *it = item.get();
  items_.push_back(std::move(item));
  itemAdded(it);
}

std::unique_ptr<WLayoutItem> CustomLayout::removeItem(WLayoutItem *item)
{
  for (int i = 0; i < items_.size(); ++i)
  {
    if (items_[i].item_.get() == item) {
      std::unique_ptr<WLayoutItem> res = std::move(items_[i].item_);
      items_.erase(items_.begin() + i);
      itemRemoved(item);
      return res;
    }
  }
  return std::unique_ptr<WLayoutItem>();
}

WLayoutItem *CustomLayout::itemAt(int index) const
{
  return items_[index].item_.get();
}

int CustomLayout::indexOf(WLayoutItem *item) const
{
  for (int i = 0; i < items_.size(); ++i)
  {
    if (items_[i].item_.get() == item) {
      return i;
    }
  }
  return -1;
}

int CustomLayout::count() const
{
  return items_.size();
}

void CustomLayout::iterateWidgets(const HandleWidgetMethod& method) const
{
  for (int i = 0; i < items_.size(); ++i)
  {
    items_[i].item_->iterateWidgets(method);
  }
}

bool CustomLayout::implementationIsFlexLayout() const
{
  return false;
}
