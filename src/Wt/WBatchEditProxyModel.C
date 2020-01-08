/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cassert>

#include "Wt/WBatchEditProxyModel.h"
#include "Wt/WException.h"

#include "WebUtils.h"

namespace {
  bool isAncestor(const Wt::WModelIndex& i1, const Wt::WModelIndex& i2) {
    if (!i1.isValid())
      return false;

    for (Wt::WModelIndex p = i1; p.isValid(); p = p.parent()) {
      if (p == i2)
	return true;
    }

    return !i2.isValid();
  }
}

namespace Wt {

bool WBatchEditProxyModel::Cell::operator< (const Cell& other) const
{
  if (row < other.row)
    return true;
  else if (row > other.row)
    return false;
  else
    return column < other.column;
}

WBatchEditProxyModel::Item::Item(const WModelIndex& sourceIndex)
  : BaseItem(sourceIndex),
    insertedParent_(nullptr)
{ }

WBatchEditProxyModel::Item::Item(Item *insertedParent)
  : BaseItem(WModelIndex()),
    insertedParent_(insertedParent)
{ }

WBatchEditProxyModel::Item::~Item()
{ 
  for (unsigned i = 0; i < insertedItems_.size(); ++i)
    delete insertedItems_[i];
}

WBatchEditProxyModel::WBatchEditProxyModel()
  : submitting_(false),
    dirtyIndicationRole_(-1)
{ }

WBatchEditProxyModel::~WBatchEditProxyModel()
{
  resetMappings();
}

void WBatchEditProxyModel
::setSourceModel(const std::shared_ptr<WAbstractItemModel>& model)
{
  for (unsigned i = 0; i < modelConnections_.size(); ++i)
    modelConnections_[i].disconnect();
  modelConnections_.clear();

  WAbstractProxyModel::setSourceModel(model);

  modelConnections_.push_back(sourceModel()->columnsAboutToBeInserted().connect
     (this, &WBatchEditProxyModel::sourceColumnsAboutToBeInserted));
  modelConnections_.push_back(sourceModel()->columnsInserted().connect
     (this, &WBatchEditProxyModel::sourceColumnsInserted));

  modelConnections_.push_back(sourceModel()->columnsAboutToBeRemoved().connect
     (this, &WBatchEditProxyModel::sourceColumnsAboutToBeRemoved));
  modelConnections_.push_back(sourceModel()->columnsRemoved().connect
     (this, &WBatchEditProxyModel::sourceColumnsRemoved));

  modelConnections_.push_back(sourceModel()->rowsAboutToBeInserted().connect
     (this, &WBatchEditProxyModel::sourceRowsAboutToBeInserted));
  modelConnections_.push_back(sourceModel()->rowsInserted().connect
     (this, &WBatchEditProxyModel::sourceRowsInserted));

  modelConnections_.push_back(sourceModel()->rowsAboutToBeRemoved().connect
     (this, &WBatchEditProxyModel::sourceRowsAboutToBeRemoved));
  modelConnections_.push_back(sourceModel()->rowsRemoved().connect
     (this, &WBatchEditProxyModel::sourceRowsRemoved));

  modelConnections_.push_back(sourceModel()->dataChanged().connect
     (this, &WBatchEditProxyModel::sourceDataChanged));
  modelConnections_.push_back(sourceModel()->headerDataChanged().connect
     (this, &WBatchEditProxyModel::sourceHeaderDataChanged));

  modelConnections_.push_back(sourceModel()->layoutAboutToBeChanged().connect
     (this, &WBatchEditProxyModel::sourceLayoutAboutToBeChanged));
  modelConnections_.push_back(sourceModel()->layoutChanged().connect
     (this, &WBatchEditProxyModel::sourceLayoutChanged));

  modelConnections_.push_back(sourceModel()->modelReset().connect
     (this, &WBatchEditProxyModel::sourceModelReset));

  resetMappings();
}

void WBatchEditProxyModel::setNewRowData(int column, const cpp17::any& data,
                                         ItemDataRole role)
{
  newRowData_[column][role] = data;
}

void WBatchEditProxyModel::setNewRowFlags(int column, WFlags<ItemFlag> flags)
{
  newRowFlags_[column] = flags;
}

void WBatchEditProxyModel::resetMappings()
{
  for (ItemMap::iterator i = mappedIndexes_.begin(); i != mappedIndexes_.end();
       ++i)
    delete i->second;

  mappedIndexes_.clear();
}

WModelIndex WBatchEditProxyModel::mapFromSource(const WModelIndex& sourceIndex)
  const
{
  if (sourceIndex.isValid()) {
    if (isRemoved(sourceIndex.parent()))
      return WModelIndex();

    WModelIndex sourceParent = sourceIndex.parent();

    Item *parentItem = itemFromSourceIndex(sourceParent);
    int row = adjustedProxyRow(parentItem, sourceIndex.row());
    int column = adjustedProxyColumn(parentItem, sourceIndex.column());

    if (row >= 0 && column >= 0)
      return createIndex(row, column, static_cast<void *>(parentItem));
    else
      return WModelIndex();      
  } else
    return WModelIndex();
}

bool WBatchEditProxyModel::isRemoved(const WModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid())
    return false;

  WModelIndex sourceParent = sourceIndex.parent();

  if (isRemoved(sourceParent))
    return true;
  else {
    Item *parentItem = itemFromSourceIndex(sourceParent);

    int row = adjustedProxyRow(parentItem, sourceIndex.row());
    if (row < 0)
      return true;

    int column = adjustedProxyColumn(parentItem, sourceIndex.column());
    return column < 0;
  }
}

WModelIndex WBatchEditProxyModel::mapToSource(const WModelIndex& proxyIndex)
  const
{
  if (proxyIndex.isValid()) {
    Item *parentItem = parentItemFromIndex(proxyIndex);
    int row = adjustedSourceRow(parentItem, proxyIndex.row());
    int column = adjustedSourceColumn(parentItem, proxyIndex.column());

    if (row >=0 && column >= 0)
      return sourceModel()->index(row, column, parentItem->sourceIndex_);
    else
      return WModelIndex();
  } else
    return WModelIndex();
}

int WBatchEditProxyModel::adjustedProxyRow(Item *item, int sourceRow) const
{
  return adjustedProxyIndex(sourceRow,
			    item->insertedRows_, item->removedRows_);
}

int WBatchEditProxyModel::adjustedProxyColumn(Item *item, int sourceColumn)
  const
{
  return adjustedProxyIndex(sourceColumn, item->insertedColumns_,
			    item->removedColumns_);
}

int WBatchEditProxyModel::adjustedProxyIndex(int sourceIndex,
					     const std::vector<int>& ins,
					     const std::vector<int>& rem) const
{
  /*
   * Cheap and obvious short-cut
   */
  if (ins.empty() && rem.empty())
    return sourceIndex;

  int insi = 0;
  int remi = 0;

  int proxyIndex = -1;

  /*
   * This could be optimized by considering the next element in ins and
   * and rem vectors and skipping directly to there
   */
  for (int si = 0; si <= sourceIndex; ++si) {
    ++proxyIndex;

    /*
     * First deal with removed rows/coluns
     */
    while (remi < static_cast<int>(rem.size()) && rem[remi] == proxyIndex) {
      // if was removed, doesn't matter if it was later re-inserted
      // return -1 - remi index
      if (si == sourceIndex)
	return -1 - remi;

      ++remi;
      ++si;
    }

    /*
     * When we are submitting inserted rows, we return the proxy row which
     * maps to the inserted row.
     */
    if (submitting_ && si == sourceIndex)
      return proxyIndex;

    while (insi < static_cast<int>(ins.size()) && ins[insi] == proxyIndex) {
      ++insi;
      ++proxyIndex;
    }
  }

  return proxyIndex;
}

int WBatchEditProxyModel::adjustedSourceRow(Item *item, int proxyRow) const
{
  return adjustedSourceIndex(proxyRow, item->insertedRows_, item->removedRows_);
}

int WBatchEditProxyModel::adjustedSourceColumn(Item *item, int proxyColumn)
  const
{
  return adjustedSourceIndex(proxyColumn, item->insertedColumns_,
			     item->removedColumns_);
}

int WBatchEditProxyModel::adjustedSourceIndex(int proxyIndex,
					      const std::vector<int>& ins,
					      const std::vector<int>& rem) const
{
  /*
   * Note a row/column can at the same time be removed and inserted !
   *
   * Therefore we process removals before (re-)insertions
   */

  // Suppose inserted:
  //  ins = [4, 5, 6, 8]
  //  then: adjustedSourceIndex(3) = 3
  //        adjustedSourceIndex(4) = -1
  //        adjustedSourceIndex(5) = -1
  //        adjustedSourceIndex(6) = -1
  //        adjustedSourceIndex(7) = 4
  //        adjustedSourceIndex(8) = -1
  //        adjustedSourceIndex(9) = 5

  unsigned inserted = Utils::lower_bound(ins, proxyIndex);

  if (inserted < ins.size() && ins[inserted] == proxyIndex)
    return -1;

  // suppose five indexs were removed:
  //   first 3 at index 2, [i.e. source model 2, 3, and 4]
  //    -> rem = [2, 2, 2]
  //   then 2 at index 3  [i.e. source model 6 and 7]
  //    -> rem = [2, 2, 2, 3, 3]
  //  then: adjustedSourceIndex(1) -> 1 + 0 = 1
  //        adjustedSourceIndex(2) -> 2 + 3 = 5
  //        adjustedSourceIndex(3) -> 3 + 5 = 8
  //
  // what if: first 2 at row 3 [i.e. source model 3 and 4]
  //           -> rem = [3, 3]
  //          then 2 at row 2 [i.e. source model 2 and 5]
  //           -> rem = [2, 2, 2, 2]
  //
  unsigned removed = Utils::upper_bound(rem, proxyIndex);

  // Together:
  //  then: adjustedSourceIndex(0) -> 0 ? = 0
  //        adjustedSourceIndex(1) -> 1 ? = 1
  //        adjustedSourceIndex(2) -> 2 ? = 5
  //        adjustedSourceIndex(3) -> 3 ? = 8
  //        adjustedSourceIndex(4) -> 4 ? = -1
  //        adjustedSourceIndex(5) -> 5 ? = -1
  //        adjustedSourceIndex(6) -> 6 ? = -1
  //        adjustedSourceIndex(7) -> 7 ? = 9 = 7 + 5 - 3

  return proxyIndex + removed - inserted;
}

WModelIndex WBatchEditProxyModel::index(int row, int column,
					const WModelIndex& parent) const
{
  Item *item = itemFromIndex(parent);
  return createIndex(row, column, static_cast<void *>(item));
}

WModelIndex WBatchEditProxyModel::parent(const WModelIndex& index) const
{
  if (index.isValid()) {
    Item *parentItem = parentItemFromIndex(index);

    return mapFromSource(parentItem->sourceIndex_);
  } else
    return WModelIndex();
}

WBatchEditProxyModel::Item *
WBatchEditProxyModel::parentItemFromIndex(const WModelIndex& index) const
{
  return static_cast<Item *>(index.internalPointer());
}

WBatchEditProxyModel::Item *
WBatchEditProxyModel::itemFromIndex(const WModelIndex& index,
				    bool autoCreate) const
{
  if (index.isValid()) {
    Item *parentItem = parentItemFromIndex(index);

    int row = adjustedSourceRow(parentItem, index.row());
    int column = adjustedSourceColumn(parentItem, index.column());

    if (row >= 0 && column >= 0) {
	WModelIndex sourceIndex
	= sourceModel()->index(row, column, parentItem->sourceIndex_);
      return itemFromSourceIndex(sourceIndex, autoCreate);
    } else {
      if (index.column() == 0)
	return itemFromInsertedRow(parentItem, index, autoCreate);
      else
	if (autoCreate)
	  throw WException("WBatchEditProxyModel does not support children in "
			   "column > 0");
	else
	  return nullptr;
    }
  } else
    return itemFromSourceIndex(WModelIndex(), autoCreate);
}

WBatchEditProxyModel::Item *
WBatchEditProxyModel::itemFromSourceIndex(const WModelIndex& sourceParent,
					  bool autoCreate)
  const
{
  if (isRemoved(sourceParent))
    return nullptr;

  ItemMap::const_iterator i = mappedIndexes_.find(sourceParent);
  if (i == mappedIndexes_.end()) {
    if (autoCreate) {
      Item *result = new Item(sourceParent);
      mappedIndexes_[sourceParent] = result;
      return result;
    } else
      return nullptr;
  } else
    return dynamic_cast<Item *>(i->second);
}

WBatchEditProxyModel::Item *
WBatchEditProxyModel::itemFromInsertedRow(Item *parentItem,
					  const WModelIndex& index,
					  bool autoCreate) const
{
  int i = Utils::indexOf(parentItem->insertedRows_, index.row());

  if (!parentItem->insertedItems_[i] && autoCreate) {
    Item *item = new Item(parentItem);
    parentItem->insertedItems_[i] = item;
  }

  return parentItem->insertedItems_[i];
}

int WBatchEditProxyModel::columnCount(const WModelIndex& parent) const
{
  Item *item = itemFromIndex(parent, false);

  if (item) {
    if (item->insertedParent_)
      return item->insertedColumns_.size();
    else
      return sourceModel()->columnCount(item->sourceIndex_)
	+ item->insertedColumns_.size() - item->removedColumns_.size();
  } else
    return sourceModel()->columnCount(mapToSource(parent));
}

int WBatchEditProxyModel::rowCount(const WModelIndex& parent) const
{
  Item *item = itemFromIndex(parent, false);

  if (item) {
    if (item->insertedParent_)
      return item->insertedRows_.size();
    else
      return sourceModel()->rowCount(item->sourceIndex_)
	+ item->insertedRows_.size() - item->removedRows_.size();
  } else
    return sourceModel()->rowCount(mapToSource(parent));
}

void WBatchEditProxyModel::sourceColumnsAboutToBeInserted
  (const WModelIndex& parent, int start, int end)
{
  if (isRemoved(parent))
    return;

  beginInsertColumns(mapFromSource(parent), start, end);
}

void WBatchEditProxyModel::sourceColumnsInserted(const WModelIndex& parent,
						 int start, int end)
{
  if (isRemoved(parent))
    return;

  WModelIndex pparent = mapFromSource(parent);
  Item *item = itemFromIndex(pparent);

  int count = end - start + 1;

  for (int i = 0; i < count; ++i) {
    int proxyColumn = adjustedProxyColumn(item, start + i);

    if (proxyColumn >= 0) {
      if (!submitting_) {
	beginInsertColumns(pparent, proxyColumn, proxyColumn);
	shiftColumns(item, proxyColumn, 1);
	endInsertColumns();
      } else {
	// The insert is being submitted. We do not need to shift
	// anything: the proxy indexes do not change
	int index = Utils::indexOf(item->insertedColumns_, proxyColumn);
	assert(index != -1);
	item->insertedColumns_.erase(item->insertedColumns_.begin() + index);
      }
    } else {
      // Since removed columns are processed first during submission,
      // it cannot be that the column is actually removed.
      assert(!submitting_);

      // The source model is inserting where we actually deleted. We
      // simply insert before the 'deleted' column.
      int remi = -proxyColumn - 1;
      proxyColumn = item->removedColumns_[remi];

      beginInsertColumns(pparent, proxyColumn, proxyColumn);
      shiftColumns(item, proxyColumn, 1);
      endInsertColumns();
    }
  }
}

void WBatchEditProxyModel::sourceColumnsAboutToBeRemoved
  (const WModelIndex& parent, int start, int end)
{ 
  if (isRemoved(parent))
    return;

  WModelIndex pparent = mapFromSource(parent);
  Item *item = itemFromIndex(pparent);

  int count = end - start + 1;

  for (int i = 0; i < count; ++i) {
    int proxyColumn = adjustedProxyColumn(item, start);

    if (proxyColumn >= 0) {
      beginRemoveColumns(pparent, proxyColumn, proxyColumn);

      shiftColumns(item, proxyColumn, -1);

      endRemoveColumns();
    } else {
      // Was removed. We do not need to shift anything: the 'proxy'
      // indexes do not change
      int remi = -proxyColumn - 1;
      item->removedColumns_.erase(item->removedColumns_.begin() + remi);
    }
  }
}

void WBatchEditProxyModel::sourceColumnsRemoved(const WModelIndex& parent,
						int start, int end)
{
  if (isRemoved(parent))
    return;

  endRemoveColumns();
}

void WBatchEditProxyModel::sourceRowsAboutToBeRemoved
(const WModelIndex& parent, int start, int end)
{
  if (isRemoved(parent))
    return;

  WModelIndex pparent = mapFromSource(parent);
  Item *item = itemFromIndex(pparent);

  int count = end - start + 1;

  for (int i = 0; i < count; ++i) {
    int proxyRow = adjustedProxyRow(item, start);

    if (proxyRow >= 0) {
      beginRemoveRows(pparent, proxyRow, proxyRow);

      deleteItemsUnder(item, proxyRow);

      shiftRows(item, proxyRow, -1);

      endRemoveRows();
    } else {
      // Was removed. We do not need to shift anything: the 'proxy'
      // indexes do not change and also items at indexes for which the
      // erased row is ancestor were already deleted
      int remi = -proxyRow - 1;
      item->removedRows_.erase(item->removedRows_.begin() + remi);
    }
  }
  
  startShiftModelIndexes(parent, start, -(end - start + 1), mappedIndexes_);
}

void WBatchEditProxyModel::deleteItemsUnder(Item *item, int row)
{
  WModelIndex sourceIndex = sourceModel()->index(row, 0, item->sourceIndex_);

  for (ItemMap::iterator i = mappedIndexes_.lower_bound(sourceIndex);
       i != mappedIndexes_.end();) {
    if (isAncestor(sourceIndex, i->first)) {
      delete i->second;
      Utils::eraseAndNext(mappedIndexes_, i);
    } else
      break;
  }
}

void WBatchEditProxyModel::sourceRowsRemoved(const WModelIndex& parent,
					     int start, int end)
{ 
  if (isRemoved(parent))
    return;
  
  endShiftModelIndexes(parent, start, -(end - start + 1), mappedIndexes_);
}

void WBatchEditProxyModel::sourceRowsAboutToBeInserted
(const WModelIndex& parent, int start, int end)
{ }

void WBatchEditProxyModel::sourceRowsInserted(const WModelIndex& parent,
					      int start, int end)
{
  if (isRemoved(parent))
    return;
  
  startShiftModelIndexes(parent, start, (end - start + 1), mappedIndexes_);

  WModelIndex pparent = mapFromSource(parent);
  Item *item = itemFromIndex(pparent);

  int count = end - start + 1;

  for (int i = 0; i < count; ++i) {
    int proxyRow = adjustedProxyRow(item, start + i);

    if (proxyRow >= 0) {
      if (!submitting_) {
	beginInsertRows(pparent, proxyRow, proxyRow);
	shiftRows(item, proxyRow, 1);
	endInsertRows();
      } else {
	// The insert is being submitted. We do not need to shift anything:
	// the 'proxy' indexes do not change
	int index = Utils::indexOf(item->insertedRows_, proxyRow);

	assert(index != -1);

	Item *child = item->insertedItems_[index];
	if (child) {
	  child->sourceIndex_ = sourceModel()->index(start + i, 0, parent);
	  child->insertedParent_ = nullptr;
	  mappedIndexes_[child->sourceIndex_] = child;
	}

	item->insertedItems_.erase(item->insertedItems_.begin() + index);
	item->insertedRows_.erase(item->insertedRows_.begin() + index);
      }
    } else {
      // Since removed rows are processed first, it cannot be that the
      // row is actually removed.
      assert(!submitting_);

      // The source model is inserting where we actually deleted. We simply
      // insert before the 'deleted' row.
      int remi = -proxyRow - 1;
      proxyRow = item->removedRows_[remi];

      beginInsertRows(pparent, proxyRow, proxyRow);
      shiftRows(item, proxyRow, 1);
      endInsertRows();
    }
  }
}

void WBatchEditProxyModel::sourceDataChanged(const WModelIndex& topLeft,
					     const WModelIndex& bottomRight)
{
  if (isRemoved(topLeft.parent()))
    return;

  for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
    for (int col = topLeft.column(); col <= bottomRight.column(); ++col) {
      WModelIndex l = sourceModel()->index(row, col, topLeft.parent());
      if (!isRemoved(l))
	dataChanged().emit(mapFromSource(l), mapFromSource(l));
    }
  }
}

void WBatchEditProxyModel::sourceHeaderDataChanged(Orientation orientation, 
						   int start, int end)
{
  if (orientation == Orientation::Vertical) {    
    Item *item = itemFromIndex(WModelIndex());
    for (int row = start; row <= end; ++row) {
      int proxyRow = adjustedProxyRow(item, row);
      if (proxyRow != -1)
	headerDataChanged().emit(orientation, proxyRow, proxyRow);
    }
  } else {
    // FIXME
    headerDataChanged().emit(orientation, start, end);
  }
}

void WBatchEditProxyModel::sourceLayoutAboutToBeChanged()
{
  // FIXME: what ?

  layoutAboutToBeChanged().emit();
  resetMappings();
}

void WBatchEditProxyModel::sourceLayoutChanged()
{
  layoutChanged().emit();
}

void WBatchEditProxyModel::sourceModelReset()
{
  resetMappings();
  reset();
}

cpp17::any WBatchEditProxyModel::data(const WModelIndex& index, ItemDataRole role) const
{
  Item *item = itemFromIndex(index.parent());

  ValueMap::const_iterator i
    = item->editedValues_.find(Cell(index.row(), index.column()));

  if (i != item->editedValues_.end()) {
    DataMap::const_iterator j = i->second.find(role);
    if (j != i->second.end())
      return indicateDirty(role, j->second);
    else
      return indicateDirty(role, cpp17::any());
  }

  WModelIndex sourceIndex = mapToSource(index);
  if (sourceIndex.isValid())
    return sourceModel()->data(sourceIndex, role);
  else
    return indicateDirty(role, cpp17::any());
}

void WBatchEditProxyModel::setDirtyIndication(ItemDataRole role, const cpp17::any& data)
{
  dirtyIndicationRole_ = role;
  dirtyIndicationData_ = data;
}

cpp17::any WBatchEditProxyModel::indicateDirty(ItemDataRole role, const cpp17::any& value) const
{
  if (role == dirtyIndicationRole_) {
    if (role == ItemDataRole::StyleClass) {
      WString s1 = asString(value);
      WString s2 = asString(dirtyIndicationData_);
      if (!s1.empty())
	s1 += " ";
      s1 += s2;
      return cpp17::any(s1);
    } else
      return dirtyIndicationData_;
  } else
    return value;
}

bool WBatchEditProxyModel::setData(const WModelIndex& index,
                                   const cpp17::any& value, ItemDataRole role)
{
  Item *item = itemFromIndex(index.parent());

  ValueMap::iterator i
    = item->editedValues_.find(Cell(index.row(), index.column()));

  if (i == item->editedValues_.end()) {
    WModelIndex sourceIndex = mapToSource(index);
    DataMap dataMap;

    if (sourceIndex.isValid())
      dataMap = sourceModel()->itemData(sourceIndex);

    dataMap[role] = value;

    if (role == ItemDataRole::Edit)
      dataMap[ItemDataRole::Display] = value;

    item->editedValues_[Cell(index.row(), index.column())] = dataMap;
  } else {
    i->second[role] = value;
    if (role == ItemDataRole::Edit)
      i->second[ItemDataRole::Display] = value;
  }

  dataChanged().emit(index, index);

  return true;
}

WFlags<ItemFlag> WBatchEditProxyModel::flags(const WModelIndex& index) const
{
  WModelIndex sourceIndex = mapToSource(index);

  if (sourceIndex.isValid())
    return sourceModel()->flags(sourceIndex);
  else {
    std::map<int, WFlags<ItemFlag> >::const_iterator i
      = newRowFlags_.find(index.column());
    if (i != newRowFlags_.end())
      return i->second;
    else
      return WAbstractProxyModel::flags(index);
  }
}

cpp17::any WBatchEditProxyModel::headerData(int section,
					    Orientation orientation,
                                            ItemDataRole role) const
{
  if (orientation == Orientation::Vertical)
    return cpp17::any(); // nobody cares
  else
    // FIXME
    return sourceModel()->headerData(section, orientation, role);
}

void WBatchEditProxyModel::shift(std::vector<int>& v, int index, int count)
{
  unsigned first = Utils::lower_bound(v, index);

  for (unsigned i = first; i < v.size(); ++i)
    v[i] += count;
}

void WBatchEditProxyModel::shiftRows(ValueMap& v, int row, int count)
{
  for (ValueMap::iterator i = v.begin(); i != v.end();) {
    if (i->first.row >= row) {
      Cell& c = const_cast<Cell&>(i->first);
      if (count < 0) {
	if (c.row >= row - count) {
	  c.row += count;
	  ++i;
	} else
	  Utils::eraseAndNext(v, i);
      } else {
	c.row += count;
	++i;
      }
    } else
      break;
  }
}

void WBatchEditProxyModel::shiftColumns(ValueMap& v, int column, int count)
{
  for (ValueMap::iterator i = v.begin(); i != v.end();) {
    if (i->first.column >= column) {
      Cell& c = const_cast<Cell&>(i->first);
      if (count < 0) {
	if (c.column >= column - count) {
	  c.column += count;
	  ++i;
	} else
	  Utils::eraseAndNext(v, i);
      } else {
	c.column += count;
	++i;
      }
    } else
      ++i;
  }
}

void WBatchEditProxyModel::shiftRows(Item *item, int row, int count)
{
  shift(item->insertedRows_, row, count);
  shift(item->removedRows_, row, count);
  shiftRows(item->editedValues_, row, count);
}

void WBatchEditProxyModel::shiftColumns(Item *item, int column, int count)
{
  shift(item->insertedColumns_, column, count);
  shift(item->removedColumns_, column, count);
  shiftColumns(item->editedValues_, column, count);
}

bool WBatchEditProxyModel::insertRows(int row, int count,
				      const WModelIndex& parent)
{
  if (columnCount(parent) == 0)
    insertColumns(0, 1, parent);

  beginInsertRows(parent, row, row + count - 1);

  Item *item = itemFromIndex(parent);

  shiftRows(item, row, count);

  insertIndexes(item, item->insertedRows_, &item->insertedItems_,
		row, count);

  for (int i = 0; i < count; ++i) {
    for (int j = 0; j < columnCount(parent); ++j) {
      DataMap data;
      std::map<int, DataMap>::const_iterator nri = newRowData_.find(j);
      if (nri != newRowData_.end())
	data = nri->second;
      item->editedValues_[Cell(row + i, j)] = data;
    }
  }

  endInsertRows();

  return true;
}

bool WBatchEditProxyModel::removeRows(int row, int count,
				      const WModelIndex& parent)
{
  beginRemoveRows(parent, row, row + count - 1);

  Item *item = itemFromIndex(parent);

  removeIndexes(item, item->insertedRows_, item->removedRows_,
		&item->insertedItems_, row, count);

  shiftRows(item->editedValues_, row, -count);

  endRemoveRows();

  return true;
}

bool WBatchEditProxyModel::insertColumns(int column, int count,
					 const WModelIndex& parent)
{
  beginInsertColumns(parent, column, column + count - 1);

  Item *item = itemFromIndex(parent);

  shiftColumns(item, column, count);

  insertIndexes(item, item->insertedColumns_, nullptr, column, count);

  endInsertColumns();

  return true;
}

bool WBatchEditProxyModel::removeColumns(int column, int count,
					 const WModelIndex& parent)
{
  beginRemoveColumns(parent, column, column + count - 1);

  Item *item = itemFromIndex(parent);

  removeIndexes(item, item->insertedColumns_, item->removedColumns_,
		nullptr, column, count);

  shiftColumns(item->editedValues_, column, count);

  endRemoveColumns();

  return true;
}

void WBatchEditProxyModel::insertIndexes(Item *item,
					 std::vector<int>& ins,
					 std::vector<Item *> *rowItems,
					 int index, int count)
{
  int insertIndex = Utils::lower_bound(ins, index);

  for (int i = 0; i < count; ++i) {
    ins.insert(ins.begin() + insertIndex + i, index + i);

    if (rowItems)
      rowItems->insert(rowItems->begin() + insertIndex + i, nullptr);
  }
}

void WBatchEditProxyModel::removeIndexes(Item *item,
					 std::vector<int>& ins,
					 std::vector<int>& rem,
					 std::vector<Item *>* rowItems,
					 int index, int count)
{
  /*
   * Example: rem contains [4, 8]
   *          ins contains [4, 5, 6]
   *   (a row/col was first removed at row/col 4, and later a new one
   *    was inserted at row/col 4, 5 and 6, and next a row/col removed at 8)
   *
   * Then: removeIndexes(3, 3):
   *   -> rem: [3, 3, 5]
   *   -> ins: [3]
   *
   *  iter 1:
   *   -> rem: [3, 3, 7]
   *   -> ins: [3, 4, 5]
   *  iter 2:
   *   -> rem: [3, 3, 6]
   *   -> ins: [3, 4]
   *  iter 3:
   *   -> rem: [3, 3, 5]
   *   -> ins: [3]
   */

  for (int i = 0; i < count; ++i) {
    /*
     * If inserted, then remove from the ins
     *              otherwise, add to rem
     * Shift inserted >= index with - 1,
     * Shift removed > index with -1
     */
    unsigned insi = Utils::lower_bound(ins, index);

    if (insi != ins.size() && ins[insi] == index) {
      ins.erase(ins.begin() + insi);

      if (rowItems) {
	delete (*rowItems)[insi];
	rowItems->erase(rowItems->begin() + insi);
      }
    } else {
      if (rowItems)
	deleteItemsUnder(item, index);

      rem.insert(rem.begin() + Utils::lower_bound(rem, index), index);
    }

    shift(ins, index, -1);
    shift(rem, index + 1, -1);
  }
}

void WBatchEditProxyModel::sort(int column, SortOrder order)
{
  sourceModel()->sort(column, order);
}

bool WBatchEditProxyModel::isDirty() const
{
  for (ItemMap::iterator i = mappedIndexes_.begin();
       i != mappedIndexes_.end(); ++i) {
    Item *item = dynamic_cast<Item *>(i->second);

    if (!item->removedColumns_.empty()
	|| !item->insertedColumns_.empty()
	|| !item->removedRows_.empty()
	|| !item->insertedRows_.empty()
	|| !item->editedValues_.empty())
      return true;
  }

  return false;
}

void WBatchEditProxyModel::commitAll()
{
  submitting_ = true;

  for (ItemMap::iterator i = mappedIndexes_.begin(); i != mappedIndexes_.end();
       ++i) {
    Item *item = dynamic_cast<Item *>(i->second);

    while (!item->removedColumns_.empty())
      sourceModel()->removeColumn(item->removedColumns_[0], item->sourceIndex_);

    while (!item->insertedColumns_.empty())
      sourceModel()->insertColumn(item->insertedColumns_[0],
				  item->sourceIndex_);

    while (!item->removedRows_.empty())
      sourceModel()->removeRow(item->removedRows_[0], item->sourceIndex_);

    while (!item->insertedRows_.empty())
      sourceModel()->insertRow(item->insertedRows_[0], item->sourceIndex_);

    for (ValueMap::iterator j = item->editedValues_.begin();
	 j != item->editedValues_.end();) {
      WModelIndex index = sourceModel()->index(j->first.row,
					       j->first.column,
					       item->sourceIndex_);
      DataMap data = j->second;

      Utils::eraseAndNext(item->editedValues_, j);
      sourceModel()->setItemData(index, data);
    }
  }

  submitting_ = false;
}

void WBatchEditProxyModel::revertAll()
{
  for (ItemMap::iterator i = mappedIndexes_.begin(); i != mappedIndexes_.end();
       ++i) {
    Item *item = dynamic_cast<Item *>(i->second);

    WModelIndex proxyIndex = mapFromSource(item->sourceIndex_);

    while (!item->insertedColumns_.empty())
      removeColumn(item->insertedColumns_[0], proxyIndex);

    while (!item->removedColumns_.empty()) {
      int column = item->removedColumns_[0];

      beginInsertColumns(proxyIndex, column, 1);
      item->removedColumns_.erase(item->removedColumns_.begin());
      shiftColumns(item, column, 1);
      endInsertColumns();
    }

    while (!item->insertedRows_.empty())
      removeRow(item->insertedRows_[0], proxyIndex);

    while (!item->removedRows_.empty()) {
      int row = item->removedRows_[0];

      beginInsertRows(proxyIndex, row, 1);
      item->removedRows_.erase(item->removedRows_.begin());
      shiftRows(item, row, 1);
      endInsertRows();
    }

    for (ValueMap::iterator j = item->editedValues_.begin();
	 j != item->editedValues_.end();) {
      Cell c = j->first;
      Utils::eraseAndNext(item->editedValues_, j);
      WModelIndex child = index(c.row, c.column, proxyIndex);
      dataChanged().emit(child, child);
    }
  }
}

}
