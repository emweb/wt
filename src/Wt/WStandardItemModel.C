/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WStandardItem"
#include "Wt/WStandardItemModel"

#ifndef DOXYGEN_ONLY

namespace Wt {

WStandardItemModel::WStandardItemModel(WObject *parent)
  : WAbstractItemModel(parent),
    sortRole_(DisplayRole),
    itemChanged_(this)
{
  init();
}

WStandardItemModel::WStandardItemModel(int rows, int columns, WObject *parent)
  : WAbstractItemModel(parent),
    sortRole_(DisplayRole),
    itemChanged_(this)
{
  init();

  invisibleRootItem_->setColumnCount(columns);
  invisibleRootItem_->setRowCount(rows);
}

void WStandardItemModel::init()
{
  invisibleRootItem_ = new WStandardItem();
  invisibleRootItem_->model_ = this;

  itemPrototype_ = new WStandardItem();
}

WStandardItemModel::~WStandardItemModel()
{
  delete invisibleRootItem_;
  delete itemPrototype_;
}

void WStandardItemModel::clear()
{
  invisibleRootItem_->setRowCount(0);
  invisibleRootItem_->setColumnCount(0);

  columnHeaderData_.clear();
  rowHeaderData_.clear();
  columnHeaderFlags_.clear();
  rowHeaderFlags_.clear();

  reset();
}

WModelIndex WStandardItemModel::indexFromItem(const WStandardItem *item) const
{
  if (item == invisibleRootItem_)
    return WModelIndex();
  else
    return createIndex(item->row(), item->column(),
		       static_cast<void *>(item->parent()));
}

WStandardItem *WStandardItemModel::itemFromIndex(const WModelIndex& index) const
{
  return itemFromIndex(index, true);
}

WStandardItem *WStandardItemModel::itemFromIndex(const WModelIndex& index,
						 bool lazyCreate) const
{
  if (!index.isValid())
    return invisibleRootItem_;
  else
    if (index.model() != this)
      return 0;
    else {
      WStandardItem *parent
	= static_cast<WStandardItem *>(index.internalPointer());
      WStandardItem *c = parent->child(index.row(), index.column());

      if (lazyCreate && !c) {
	c = itemPrototype()->clone();
	parent->setChild(index.row(), index.column(), c);
      }
      
      return c;
    }
}


void WStandardItemModel::appendColumn(const std::vector<WStandardItem *>& items)
{
  insertColumn(columnCount(), items);
}

void WStandardItemModel::insertColumn(int column,
				      const std::vector<WStandardItem *>& items)
{
  invisibleRootItem_->insertColumn(column, items);
}

void WStandardItemModel::appendRow(const std::vector<WStandardItem *>& items)
{
  insertRow(rowCount(), items);
}

void WStandardItemModel::insertRow(int row,
				   const std::vector<WStandardItem *>& items)
{
  invisibleRootItem_->insertRow(row, items);
}

void WStandardItemModel::appendRow(WStandardItem *item)
{
  insertRow(rowCount(), item);
}

void WStandardItemModel::insertRow(int row, WStandardItem *item)
{
  invisibleRootItem_->insertRow(row, item);
}
  
WStandardItem *WStandardItemModel::item(int row, int column) const
{
  return invisibleRootItem_->child(row, column);
}

void WStandardItemModel::setItem(int row, int column, WStandardItem *item)
{
  invisibleRootItem_->setChild(row, column, item);
}

WStandardItem *WStandardItemModel::itemPrototype() const
{
  return itemPrototype_;
}

void WStandardItemModel::setItemPrototype(WStandardItem *item)
{
  delete itemPrototype_;
  itemPrototype_ = item;
}

std::vector<WStandardItem *> WStandardItemModel::takeColumn(int column)
{
  return invisibleRootItem_->takeColumn(column);
}

std::vector<WStandardItem *> WStandardItemModel::takeRow(int row)
{
  return invisibleRootItem_->takeRow(row);
}

WStandardItem *WStandardItemModel::takeItem(int row, int column)
{
  return invisibleRootItem_->takeChild(row, column);
}

WFlags<ItemFlag> WStandardItemModel::flags(const WModelIndex& index) const
{
  WStandardItem *item = itemFromIndex(index, false);

  return item ? item->flags() : WFlags<ItemFlag>(0);
}

WModelIndex WStandardItemModel::parent(const WModelIndex& index) const
{
  if (!index.isValid())
    return index;

  WStandardItem *parent = static_cast<WStandardItem *>(index.internalPointer());

  return indexFromItem(parent);
}

boost::any WStandardItemModel::data(const WModelIndex& index, int role) const
{
  WStandardItem *item = itemFromIndex(index, false);

  return item ? item->data(role) : boost::any();
}

boost::any WStandardItemModel::headerData(int section, Orientation orientation,
					  int role) const
{
  if (role == LevelRole)
    return 0;

  const HeaderData& d = (orientation == Horizontal)
    ? columnHeaderData_[section]
    : rowHeaderData_[section];

  HeaderData::const_iterator i = d.find(role);

  if (i != d.end()) {
    /*
     * Work around CLang bug, 'return i->second' would try to create
     * a boost::any<const boost::any> ... ?
     */
    boost::any result = i->second;
    return result;
  } else
    return boost::any();
}

WModelIndex WStandardItemModel::index(int row, int column,
				      const WModelIndex& parent) const
{
  WStandardItem *parentItem = itemFromIndex(parent, false);

  if (parentItem
      && row >= 0
      && column >= 0
      && row < parentItem->rowCount()
      && column < parentItem->columnCount())
    return createIndex(row, column, static_cast<void *>(parentItem));

  return WModelIndex();
}

int WStandardItemModel::columnCount(const WModelIndex& parent) const
{
  WStandardItem *parentItem = itemFromIndex(parent, false);

  return parentItem ? parentItem->columnCount() : 0;
}

int WStandardItemModel::rowCount(const WModelIndex& parent) const
{
  WStandardItem *parentItem = itemFromIndex(parent, false);

  return parentItem ? parentItem->rowCount() : 0;
}

bool WStandardItemModel::insertColumns(int column, int count,
				       const WModelIndex& parent)
{
  WStandardItem *parentItem = itemFromIndex(parent); // lazy create ok

  if (parentItem)
    parentItem->insertColumns(column, count);

  return parentItem;
}

bool WStandardItemModel::insertRows(int row, int count,
				    const WModelIndex& parent)
{
  WStandardItem *parentItem = itemFromIndex(parent); // lazy create ok

  if (parentItem)
    parentItem->insertRows(row, count);

  return parentItem;
}

bool WStandardItemModel::removeColumns(int column, int count,
				       const WModelIndex& parent)
{
  WStandardItem *parentItem = itemFromIndex(parent, false);

  if (parentItem)
    parentItem->removeColumns(column, count);

  return parentItem;
}

bool WStandardItemModel::removeRows(int row, int count,
				    const WModelIndex& parent)
{
  WStandardItem *parentItem = itemFromIndex(parent, false);

  if (parentItem)
    parentItem->removeRows(row, count);

  return parentItem;  
}

void WStandardItemModel::beginInsertColumns(const WModelIndex& parent,
					    int first, int last)
{
  WAbstractItemModel::beginInsertColumns(parent, first, last);

  insertHeaderData(columnHeaderData_, columnHeaderFlags_, itemFromIndex(parent),
		   first, last - first + 1);
}

void WStandardItemModel::beginInsertRows(const WModelIndex& parent,
					 int first, int last)
{
  WAbstractItemModel::beginInsertRows(parent, first, last);

  insertHeaderData(rowHeaderData_, rowHeaderFlags_, itemFromIndex(parent),
		   first, last - first + 1);
}

void WStandardItemModel::beginRemoveColumns(const WModelIndex& parent,
					    int first, int last)
{
  WAbstractItemModel::beginRemoveColumns(parent, first, last);

  removeHeaderData(columnHeaderData_, columnHeaderFlags_, itemFromIndex(parent),
		   first, last - first + 1);
}

void WStandardItemModel::beginRemoveRows(const WModelIndex& parent,
					 int first, int last)
{ 
  WAbstractItemModel::beginRemoveRows(parent, first, last);

  removeHeaderData(rowHeaderData_, rowHeaderFlags_, itemFromIndex(parent),
		   first, last - first + 1);
}

void WStandardItemModel::insertHeaderData(std::vector<HeaderData>& headerData,
					  std::vector<WFlags<HeaderFlag> >& fl,
					  WStandardItem *item, int index,
					  int count)
{
  if (item == invisibleRootItem_) {
    headerData.insert(headerData.begin() + index, count, HeaderData());
    fl.insert(fl.begin() + index, count, WFlags<HeaderFlag>());
  }
}

void WStandardItemModel::removeHeaderData(std::vector<HeaderData>& headerData,
					  std::vector<WFlags<HeaderFlag> >& fl,
					  WStandardItem *item, int index,
					  int count)
{
  if (item == invisibleRootItem_) {
    headerData.erase(headerData.begin() + index,
		     headerData.begin() + index + count);
    fl.erase(fl.begin() + index, fl.begin() + index + count);
  }
}

bool WStandardItemModel::setData(const WModelIndex& index,
				 const boost::any& value, int role)
{
  WStandardItem *item = itemFromIndex(index);

  if (item)
    item->setData(value, role);

  return item;
}

bool WStandardItemModel::setHeaderData(int section, Orientation orientation,
				       const boost::any& value, int role)
{
  std::vector<HeaderData>& header
    = (orientation == Horizontal) ? columnHeaderData_ : rowHeaderData_;

  HeaderData& d = header[section];

  if (role == EditRole)
    role = DisplayRole;

  d[role] = value;

  headerDataChanged().emit(orientation, section, section);

  return true;
}

void WStandardItemModel::setHeaderFlags(int section, Orientation orientation,
					WFlags<HeaderFlag> flags)
{
  std::vector<WFlags<HeaderFlag> >& fl
    = (orientation == Horizontal) ? columnHeaderFlags_ : rowHeaderFlags_;

  fl[section] = flags;
}

WFlags<HeaderFlag> WStandardItemModel::headerFlags(int section,
						   Orientation orientation)
  const
{
  const std::vector<WFlags<HeaderFlag> >& fl
    = (orientation == Horizontal) ? columnHeaderFlags_ : rowHeaderFlags_;
 
  return fl[section];
}

void *WStandardItemModel::toRawIndex(const WModelIndex& index) const
{
  return static_cast<void *>(itemFromIndex(index));
}

WModelIndex WStandardItemModel::fromRawIndex(void *rawIndex) const
{
  return indexFromItem(static_cast<WStandardItem *>(rawIndex));
}

void WStandardItemModel::setSortRole(int role)
{
  sortRole_ = role;
}

void WStandardItemModel::sort(int column, SortOrder order)
{
  invisibleRootItem_->sortChildren(column, order);
}

}

#endif // DOXYGEN_ONLY
