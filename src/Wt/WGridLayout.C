/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WGridLayout.h"
#include "Wt/WWidgetItem.h"

#include "StdGridLayoutImpl2.h"

#include <algorithm>

namespace Wt {

  namespace Impl {

Grid::Section::Section(int stretch)
  : stretch_(stretch),
    resizable_(false)
{ }

Grid::Item::Item(std::unique_ptr<WLayoutItem> item,
		 WFlags<AlignmentFlag> alignment)
  : item_(std::move(item)),
    rowSpan_(1),
    colSpan_(1),
    update_(true),
    alignment_(alignment)
{ }

Grid::Item::~Item()
{ }

Grid::Grid()
  : horizontalSpacing_(6),
    verticalSpacing_(6)
{ }

Grid::~Grid()
{
}

void Grid::clear()
{
  rows_.clear();
  columns_.clear();
  items_.clear();
}

  }

WGridLayout::WGridLayout()
{ }

WGridLayout::~WGridLayout()
{ }

void WGridLayout::addItem(std::unique_ptr<WLayoutItem> item)
{
  addItem(std::move(item), 0, columnCount());
}

std::unique_ptr<WLayoutItem> WGridLayout::removeItem(WLayoutItem *item)
{
  int index = indexOf(item);

  std::unique_ptr<WLayoutItem> result;

  if (index != -1) {
    int row = index / columnCount();
    int col = index % columnCount();

    result = std::move(grid_.items_[row][col].item_);
    itemRemoved(item);
  }

  return result;
}

WLayoutItem *WGridLayout::itemAt(int index) const
{
  int row = index / columnCount();
  int col = index % columnCount();

  return grid_.items_[row][col].item_.get();
}

void WGridLayout::iterateWidgets(const HandleWidgetMethod& method) const
{
  for (unsigned r = 0; r < grid_.rows_.size(); ++r) {
    for (unsigned c = 0; c < grid_.columns_.size(); ++c) {
      WLayoutItem *item = grid_.items_[r][c].item_.get();
      if (item)
	item->iterateWidgets(method);
    }
  }
}

int WGridLayout::count() const
{
  return grid_.rows_.size() * grid_.columns_.size();
}

void WGridLayout::addItem(std::unique_ptr<WLayoutItem> item,
			  int row, int column,
			  int rowSpan, int columnSpan,
			  WFlags<AlignmentFlag> alignment)
{
  columnSpan = std::max(1, columnSpan);
  rowSpan = std::max(1, rowSpan);

  expand(row, column, rowSpan, columnSpan);

  Impl::Grid::Item& gridItem = grid_.items_[row][column];

  if (gridItem.item_) {
    auto oldItem = std::move(gridItem.item_);
    itemRemoved(oldItem.get());
  }

  gridItem.item_ = std::move(item);
  gridItem.rowSpan_ = rowSpan;
  gridItem.colSpan_ = columnSpan;
  gridItem.alignment_ = alignment;

  itemAdded(gridItem.item_.get());
}

void WGridLayout::addLayout(std::unique_ptr<WLayout> layout,
			    int row, int column,
			    WFlags<AlignmentFlag> alignment)
{
  addItem(std::move(layout), row, column, 1, 1, alignment);
}

void WGridLayout::addLayout(std::unique_ptr<WLayout> layout,
			    int row, int column,
			    int rowSpan, int columnSpan,
			    WFlags<AlignmentFlag> alignment)
{
  addItem(std::move(layout), row, column, rowSpan, columnSpan, alignment);
}

void WGridLayout::addWidget(std::unique_ptr<WWidget> widget,
			    int row, int column,
			    WFlags<AlignmentFlag> alignment)
{
  addItem(std::unique_ptr<WLayoutItem>(new WWidgetItem(std::move(widget))),
	  row, column, 1, 1, alignment);
}

void WGridLayout::addWidget(std::unique_ptr<WWidget> widget,
			    int row, int column,
			    int rowSpan, int columnSpan,
			    WFlags<AlignmentFlag> alignment)
{
  addItem(std::unique_ptr<WLayoutItem>(new WWidgetItem(std::move(widget))),
	  row, column, rowSpan, columnSpan, alignment);
}

void WGridLayout::setHorizontalSpacing(int size)
{
  grid_.horizontalSpacing_ = size;

  update();
}

void WGridLayout::setVerticalSpacing(int size)
{
  grid_.verticalSpacing_ = size;

  update();
}

int WGridLayout::columnCount() const
{
  return grid_.columns_.size();
}

int WGridLayout::rowCount() const
{
  return grid_.rows_.size();
}

void WGridLayout::setColumnStretch(int column, int stretch)
{
  expand(0, column, 0, 1);
  grid_.columns_[column].stretch_ = stretch;

  update();
}

int WGridLayout::columnStretch(int column) const
{
  return grid_.columns_[column].stretch_;
}

void WGridLayout::setRowStretch(int row, int stretch)
{
  expand(row, 0, 1, 0);
  grid_.rows_[row].stretch_ = stretch;

  update();
}

int WGridLayout::rowStretch(int row) const
{
  return grid_.rows_[row].stretch_;
}

void WGridLayout::setRowResizable(int row, bool enabled,
				  const WLength& initialSize)
{
  expand(row, 0, 1, 0);
  grid_.rows_[row].resizable_ = enabled;
  grid_.rows_[row].initialSize_ = initialSize;

  update();
}

bool WGridLayout::rowIsResizable(int row) const
{
  return grid_.rows_[row].resizable_;
}

void WGridLayout::setColumnResizable(int column, bool enabled,
				     const WLength& initialSize)
{
  expand(0, column, 0, 1);
  grid_.columns_[column].resizable_ = enabled;
  grid_.columns_[column].initialSize_ = initialSize;

  update();
}

bool WGridLayout::columnIsResizable(int column) const
{
  return grid_.columns_[column].resizable_;
}

void WGridLayout::expand(int row, int column, int rowSpan, int columnSpan)
{
  int newRowCount = std::max(rowCount(), row + rowSpan);
  int newColumnCount = std::max(columnCount(), column + columnSpan);

  int extraRows = newRowCount - rowCount();
  int extraColumns = newColumnCount - columnCount();

  if (extraColumns > 0) {
    for (int a_row = 0; a_row < rowCount(); ++a_row) {
      for (int i = 0; i < extraColumns; ++i)
	grid_.items_[a_row].push_back(Impl::Grid::Item());
    }

    grid_.columns_.insert(grid_.columns_.end(), extraColumns,
			  Impl::Grid::Section());
  }

  if (extraRows > 0) {
#ifndef WT_TARGET_JAVA
    for (int i = 0; i < extraRows; ++i) {
      std::vector<Impl::Grid::Item> row;
      for (int a_col = 0; a_col < columnCount(); ++a_col)
	row.push_back(Impl::Grid::Item());
      grid_.items_.push_back(std::move(row));
    }
#else
    grid_.items_.insert(grid_.items_.end(), extraRows,
			std::vector<Impl::Grid::Item>());
    for (int i = 0; i < extraRows; ++i) {
      std::vector<Impl::Grid::Item>& items
	= grid_.items_[grid_.items_.size() - extraRows + i];
      items.insert(items.end(), newColumnCount, Impl::Grid::Item());
    }
#endif // WT_TARGET_JAVA
    grid_.rows_.insert(grid_.rows_.end(), extraRows, Impl::Grid::Section());
  }
}

void WGridLayout::setParentWidget(WWidget *parent)
{
  WLayout::setParentWidget(parent);

  if (parent)
    setImpl(std::unique_ptr<WLayoutImpl>
	    (new StdGridLayoutImpl2(this, grid_)));
}


}
