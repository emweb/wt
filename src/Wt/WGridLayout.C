/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WGridLayout"
#include "Wt/WWidgetItem"

namespace Wt {

  namespace Impl {

Grid::Section::Section(int stretch)
  : stretch_(stretch),
    resizable_(false)
{ }

Grid::Item::Item(WLayoutItem *item, WFlags<AlignmentFlag> alignment)
  : item_(item),
    rowSpan_(1),
    colSpan_(1),
    update_(true),
    alignment_(alignment)
{ }

Grid::Grid()
  : horizontalSpacing_(6),
    verticalSpacing_(6)
{ }

Grid::~Grid()
{
  for (unsigned i = 0; i < items_.size(); ++i)
    for (unsigned j = 0; j < items_[i].size(); ++j) {
      WLayoutItem *item = items_[i][j].item_;
      items_[i][j].item_ = 0;;
      delete item;
    }
}

void Grid::clear()
{
  rows_.clear();
  columns_.clear();
  items_.clear();
}

  }

WGridLayout::WGridLayout(WWidget *parent)
  : WLayout()
{
  if (parent)
    setLayoutInParent(parent);
}

void WGridLayout::addItem(WLayoutItem *item)
{
  addItem(item, 0, columnCount());
}

void WGridLayout::removeItem(WLayoutItem *item)
{
  int index = indexOf(item);

  if (index != -1) {
    int row = index / columnCount();
    int col = index % columnCount();

    grid_.items_[row][col].item_ = 0;

    updateRemoveItem(item);
  }
}

WLayoutItem *WGridLayout::itemAt(int index) const
{
  int row = index / columnCount();
  int col = index % columnCount();

  return grid_.items_[row][col].item_;
}

int WGridLayout::count() const
{
  return grid_.rows_.size() * grid_.columns_.size();
}

void WGridLayout::addItem(WLayoutItem *item, int row, int column,
			  int rowSpan, int columnSpan,
			  WFlags<AlignmentFlag> alignment)
{
  columnSpan = std::max(1, columnSpan);
  rowSpan = std::max(1, rowSpan);

  expand(row, column, rowSpan, columnSpan);

  Impl::Grid::Item& gridItem = grid_.items_[row][column];

  if (gridItem.item_) {
    WLayoutItem *oldItem = gridItem.item_;
    gridItem.item_ = 0;
    updateRemoveItem(oldItem);
  }

  gridItem.item_ = item;
  gridItem.rowSpan_ = rowSpan;
  gridItem.colSpan_ = columnSpan;
  gridItem.alignment_ = alignment;

  updateAddItem(item);
}

void WGridLayout::addLayout(WLayout *layout, int row, int column,
			    WFlags<AlignmentFlag> alignment)
{
  addItem(layout, row, column, 1, 1, alignment);
}

void WGridLayout::addLayout(WLayout *layout, int row, int column,
			    int rowSpan, int columnSpan,
			    WFlags<AlignmentFlag> alignment)
{
  addItem(layout, row, column, rowSpan, columnSpan, alignment);
}

void WGridLayout::addWidget(WWidget *widget, int row, int column,
			    WFlags<AlignmentFlag> alignment)
{
  addItem(new WWidgetItem(widget), row, column, 1, 1, alignment);
}

void WGridLayout::addWidget(WWidget *widget, int row, int column,
			    int rowSpan, int columnSpan,
			    WFlags<AlignmentFlag> alignment)
{
  addItem(new WWidgetItem(widget), row, column, rowSpan, columnSpan, alignment);
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

void WGridLayout::clear()
{
  unsigned c = count();
  for (unsigned i = 0; i < c; ++i) {
    WLayoutItem *item = itemAt(i);
    clearLayoutItem(item);
  }
  grid_.clear();
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
    for (int a_row = 0; a_row < rowCount(); ++a_row)
      grid_.items_[a_row].insert(grid_.items_[a_row].end(), extraColumns,
				 Impl::Grid::Item());
    grid_.columns_.insert(grid_.columns_.end(), extraColumns,
			  Impl::Grid::Section());
  }

  if (extraRows > 0) {
#ifndef WT_TARGET_JAVA
    grid_.items_.insert(grid_.items_.end(), extraRows,
			std::vector<Impl::Grid::Item>(newColumnCount));
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

}
