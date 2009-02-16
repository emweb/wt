/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBorderLayout"
#include "Wt/WWidgetItem"

#include "WtException.h"

namespace Wt {

WBorderLayout::WBorderLayout(WWidget *parent)
  : WLayout()
{
  grid_.columns_.insert(grid_.columns_.begin(), 3, Impl::Grid::Column(false));
  grid_.columns_[1].stretch_ = 1;

  grid_.rows_.insert(grid_.rows_.begin(), 3, Impl::Grid::Row(false));
  grid_.rows_[1].stretch_ = 1;

  grid_.items_.insert(grid_.items_.begin(), 3,
		      std::vector<Impl::Grid::Item>(3));

  grid_.items_[0][0].colSpan_ = 3;
  grid_.items_[2][0].colSpan_ = 3;

  if (parent)
    setLayoutInParent(parent);
}

WBorderLayout::~WBorderLayout()
{ }

void WBorderLayout::addItem(WLayoutItem *item)
{
  add(item, Center);
}

WLayoutItem *WBorderLayout::itemAt(int index) const
{
  int j = 0;
  for (int i = 0; i < 5; ++i) {
    WLayoutItem *it = itemAtPosition((Position)i).item_;
    if (it)
      if (j == index)
	return it;
      else
	++j;
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

Impl::Grid::Item& WBorderLayout::itemAtPosition(Position position)
{
  switch (position) {
  case North: return grid_.items_[0][0];
  case East: return grid_.items_[1][0];
  case South: return grid_.items_[2][0];
  case West: return grid_.items_[1][2];
  case Center: return grid_.items_[1][1];
  default:
    throw WtException("WBorderLayout::itemAtPosition(): invalid position");
  }
}

const Impl::Grid::Item& WBorderLayout::itemAtPosition(Position position) const
{
  switch (position) {
  case North: return grid_.items_[0][0];
  case East: return grid_.items_[1][0];
  case South: return grid_.items_[2][0];
  case West: return grid_.items_[1][2];
  case Center: return grid_.items_[1][1];
  default:
    throw WtException("WBorderLayout::itemAtPosition(): invalid position");
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
  if (itemAtPosition(position).item_)
    throw WtException("WBorderLayout supports only one widget per position");

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
      updateRemoveItem(item);
      gridItem.item_ = 0;

      break;
    }
  }
}

WBorderLayout::Position WBorderLayout::position(WLayoutItem *item) const
{
  for (int i = 0; i < 5; ++i) {
    if (itemAtPosition((Position)i).item_ == item) {
      return static_cast<WBorderLayout::Position>(i);
    }
  }

  throw WtException("WBorderLayout::position(): invalid item");
}

}
