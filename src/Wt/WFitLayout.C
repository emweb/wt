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
  : WLayout()
{ 
  grid_.columns_.insert(grid_.columns_.begin(), 1, Impl::Grid::Section(0));
  grid_.rows_.insert(grid_.rows_.begin(), 1, Impl::Grid::Section(0));

#ifndef WT_TARGET_JAVA
  grid_.items_.insert(grid_.items_.begin(), 1,
		      std::vector<Impl::Grid::Item>(1));
#else
  grid_.items_.insert(grid_.items_.begin(), 1, std::vector<Impl::Grid::Item>());
  for (unsigned i = 0; i < 1; ++i) {
    std::vector<Impl::Grid::Item>& items = grid_.items_[i];
    items.insert(items.begin(), 1, Impl::Grid::Item());
  }
#endif // WT_TARGET_JAVA

  if (parent)
    setLayoutInParent(parent);
}

WFitLayout::~WFitLayout()
{ }

void WFitLayout::addItem(WLayoutItem *item)
{
  if (grid_.items_[0][0].item_) {
    LOG_ERROR("addItem(): already have a widget");
    return;
  }

  grid_.items_[0][0].item_ = item;

  updateAddItem(item);
}

void WFitLayout::removeItem(WLayoutItem *item)
{
  if (item == grid_.items_[0][0].item_) {
    grid_.items_[0][0].item_ = 0;
    updateRemoveItem(item);
  }
}

WLayoutItem *WFitLayout::itemAt(int index) const
{
  return grid_.items_[0][0].item_;
}

void WFitLayout::clear()
{
  clearLayoutItem(grid_.items_[0][0].item_);
  grid_.items_[0][0].item_ = 0;
}

int WFitLayout::indexOf(WLayoutItem *item) const
{
  if (grid_.items_[0][0].item_ == item)
    return 0;
  else
    return -1;
}

int WFitLayout::count() const
{
  return grid_.items_[0][0].item_ ? 1 : 0;
}

}
