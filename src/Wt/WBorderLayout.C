/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBorderLayout.h"
#include "Wt/WLogger.h"
#include "Wt/WWidget.h"
#include "Wt/WWidgetItem.h"

#include "StdGridLayoutImpl2.h"

namespace Wt {

LOGGER("WBorderLayout");

WBorderLayout::WBorderLayout()
{
  grid_.columns_.insert(grid_.columns_.begin(), 3, Impl::Grid::Section(0));
  grid_.columns_[1].stretch_ = 1;

  grid_.rows_.insert(grid_.rows_.begin(), 3, Impl::Grid::Section(0));
  grid_.rows_[1].stretch_ = 1;

  for (unsigned j = 0; j < 3; ++j) {
    grid_.items_.push_back(std::vector<Impl::Grid::Item>());
    for (unsigned i = 0; i < 3; ++i) {
      auto& items = grid_.items_.back();
      items.push_back(Impl::Grid::Item());
    }
  }

  grid_.items_[0][0].colSpan_ = 3;
  grid_.items_[2][0].colSpan_ = 3;
}

WBorderLayout::~WBorderLayout()
{ }

void WBorderLayout::setSpacing(int size)
{
  grid_.horizontalSpacing_ = size;
  grid_.verticalSpacing_ = size;
}

void WBorderLayout::addItem(std::unique_ptr<WLayoutItem> item)
{
  add(std::move(item), LayoutPosition::Center);
}

WLayoutItem *WBorderLayout::itemAt(int index) const
{
  int j = 0;
  for (int i = 0; i < 5; ++i) {
    WLayoutItem *it = itemAtPosition((LayoutPosition)i).item_.get();
    if (it) {
      if (j == index)
	return it;
      else
	++j;
    }
  }

  return nullptr;
}

int WBorderLayout::count() const
{
  int j = 0;
  for (int i = 0; i < 5; ++i)
    if (itemAtPosition((LayoutPosition)i).item_)
      ++j;

  return j;
}

Impl::Grid::Item& WBorderLayout::itemAtPosition(LayoutPosition position)
{
  switch (position) {
  case LayoutPosition::North: return grid_.items_[0][0];
  case LayoutPosition::East: return grid_.items_[1][2];
  case LayoutPosition::South: return grid_.items_[2][0];
  case LayoutPosition::West: return grid_.items_[1][0];
  case LayoutPosition::Center: return grid_.items_[1][1];
  default:
    LOG_ERROR("itemAtPosition(): invalid position:" << (int)position);
    return grid_.items_[1][1];
  }
}

const Impl::Grid::Item& WBorderLayout::itemAtPosition(LayoutPosition position)
  const
{
  switch (position) {
  case LayoutPosition::North: return grid_.items_[0][0];
  case LayoutPosition::East: return grid_.items_[1][2];
  case LayoutPosition::South: return grid_.items_[2][0];
  case LayoutPosition::West: return grid_.items_[1][0];
  case LayoutPosition::Center: return grid_.items_[1][1];
  default:
    LOG_ERROR("itemAtPosition(): invalid position:" << (int)position);
    return grid_.items_[1][1];
  }
}

void WBorderLayout::addWidget(std::unique_ptr<WWidget> w,
			      LayoutPosition position)
{
  add(std::unique_ptr<WLayoutItem>(new WWidgetItem(std::move(w))), position);
}

WWidget *WBorderLayout::widgetAt(LayoutPosition position) const
{
  WWidgetItem *item = dynamic_cast<WWidgetItem *>(itemAt(position));
  
  if (item)
    return item->widget();
  else
    return nullptr;
}

void WBorderLayout::iterateWidgets(const HandleWidgetMethod& method) const
{
  for (unsigned r = 0; r < grid_.rows_.size(); ++r) {
    for (unsigned c = 0; c < grid_.columns_.size(); ++c) {
      WLayoutItem *item = grid_.items_[r][c].item_.get();
      if (item)
	item->iterateWidgets(method);
    }
  }
}

void WBorderLayout::add(std::unique_ptr<WLayoutItem> item,
			LayoutPosition position)
{
  auto& it = itemAtPosition(position);
  if (it.item_) {
    LOG_ERROR("supports only one widget per position");
    return;
  }

  it.item_ = std::move(item);
  itemAdded(it.item_.get());
}

WLayoutItem *WBorderLayout::itemAt(LayoutPosition position) const
{
  auto& gridItem = itemAtPosition(position);
  return gridItem.item_.get();
}

std::unique_ptr<WLayoutItem> WBorderLayout::removeItem(WLayoutItem *item)
{
  std::unique_ptr<WLayoutItem> result;

  for (int i = 0; i < 5; ++i) {
    Impl::Grid::Item& gridItem = itemAtPosition((LayoutPosition)i);
    if (gridItem.item_.get() == item) {
      result = std::move(gridItem.item_);
      itemRemoved(item);

      break;
    }
  }

  return result;
}

LayoutPosition WBorderLayout::position(WLayoutItem *item) const
{
  for (int i = 0; i < 5; ++i) {
    if (itemAtPosition((LayoutPosition)i).item_.get() == item) {
      return (LayoutPosition)i;
    }
  }

  LOG_ERROR("position(): item not found");
  return LayoutPosition::Center;
}

void WBorderLayout::setParentWidget(WWidget *parent)
{
  WLayout::setParentWidget(parent);

  if (parent)
    setImpl(std::unique_ptr<WLayoutImpl>
	    (new StdGridLayoutImpl2(this, grid_)));
}

}
