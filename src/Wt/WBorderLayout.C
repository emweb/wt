/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBorderLayout"
#include "Wt/WLogger"
#include "Wt/WWidgetItem"

namespace Wt {

LOGGER("WBorderLayout");

WBorderLayout::WBorderLayout(WWidget *parent)
  : WLayout()
{
  grid_.columns_.insert(grid_.columns_.begin(), 3, Impl::Grid::Section(0));
  grid_.columns_[1].stretch_ = 1;

  grid_.rows_.insert(grid_.rows_.begin(), 3, Impl::Grid::Section(0));
  grid_.rows_[1].stretch_ = 1;

#ifndef WT_TARGET_JAVA
  grid_.items_.insert(grid_.items_.begin(), 3,
		      std::vector<Impl::Grid::Item>(3));
#else
  grid_.items_.insert(grid_.items_.begin(), 3, std::vector<Impl::Grid::Item>());
  for (unsigned i = 0; i < 3; ++i) {
    std::vector<Impl::Grid::Item>& items = grid_.items_[i];
    items.insert(items.begin(), 3, Impl::Grid::Item());
  }
#endif // WT_TARGET_JAVA

  grid_.items_[0][0].colSpan_ = 3;
  grid_.items_[2][0].colSpan_ = 3;

  if (parent)
    setLayoutInParent(parent);
}

WBorderLayout::~WBorderLayout()
{ }

void WBorderLayout::setSpacing(int size)
{
  grid_.horizontalSpacing_ = size;
  grid_.verticalSpacing_ = size;
}

void WBorderLayout::addItem(WLayoutItem *item)
{
  add(item, Center);
}

WLayoutItem *WBorderLayout::itemAt(int index) const
{
  int j = 0;
  for (int i = 0; i < 5; ++i) {
    WLayoutItem *it = itemAtPosition((Position)i).item_;
    if (it) {
      if (j == index)
	return it;
      else
	++j;
    }
  }

  return 0;
}

int WBorderLayout::count() const
{
  int j = 0;
  for (int i = 0; i < 5; ++i)
    if (itemAtPosition((Position)i).item_)
      ++j;

  return j;
}

void WBorderLayout::clear()
{
  for (int i = 0; i < 5; ++i) {
    Impl::Grid::Item &item = itemAtPosition((Position)i);
    clearLayoutItem(item.item_);
    item.item_ = 0;
  }
}

Impl::Grid::Item& WBorderLayout::itemAtPosition(Position position)
{
  switch (position) {
  case North: return grid_.items_[0][0];
  case East: return grid_.items_[1][2];
  case South: return grid_.items_[2][0];
  case West: return grid_.items_[1][0];
  case Center: return grid_.items_[1][1];
  default:
    LOG_ERROR("itemAtPosition(): invalid position:" << (int)position);
    return grid_.items_[1][1];
  }
}

const Impl::Grid::Item& WBorderLayout::itemAtPosition(Position position) const
{
  switch (position) {
  case North: return grid_.items_[0][0];
  case East: return grid_.items_[1][2];
  case South: return grid_.items_[2][0];
  case West: return grid_.items_[1][0];
  case Center: return grid_.items_[1][1];
  default:
    LOG_ERROR("itemAtPosition(): invalid position:" << (int)position);
    return grid_.items_[1][1];
  }
}

void WBorderLayout::addWidget(WWidget *w, Position position)
{
  add(new WWidgetItem(w), position);
}

WWidget *WBorderLayout::widgetAt(Position position) const
{
  WWidgetItem *item = dynamic_cast<WWidgetItem *>(itemAt(position));
  
  if (item)
    return item->widget();
  else
    return 0;
}

void WBorderLayout::add(WLayoutItem *item, Position position)
{
  if (itemAtPosition(position).item_) {
    LOG_ERROR("supports only one widget per position");
    return;
  }

  itemAtPosition(position).item_ = item;
  updateAddItem(item);
}

WLayoutItem *WBorderLayout::itemAt(Position position) const
{
  const Impl::Grid::Item& gridItem = itemAtPosition(position);

  return gridItem.item_;
}

void WBorderLayout::removeItem(WLayoutItem *item)
{
  for (int i = 0; i < 5; ++i) {
    Impl::Grid::Item& gridItem = itemAtPosition((Position)i);
    if (gridItem.item_ == item) {
      gridItem.item_ = 0;

      updateRemoveItem(item);

      break;
    }
  }
}

WBorderLayout::Position WBorderLayout::position(WLayoutItem *item) const
{
  for (int i = 0; i < 5; ++i) {
    if (itemAtPosition((Position)i).item_ == item) {
      return (Position)i;
    }
  }

  LOG_ERROR("position(): item not found");
  return Center;
}

}
