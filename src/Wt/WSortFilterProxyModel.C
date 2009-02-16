/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <algorithm>
#include <boost/regex.hpp>
#include <iostream>

#include "Wt/WSortFilterProxyModel"

namespace Wt {

struct WSortFilterProxyRegExp
{
  boost::regex rx;
};

bool WSortFilterProxyModel::Compare::operator()(int sourceRow1,
						int sourceRow2) const
{
  if (model->sortOrder_ == AscendingOrder)
    return lessThan(sourceRow1, sourceRow2);
  else
    return lessThan(sourceRow2, sourceRow1);
}
  
bool WSortFilterProxyModel::Compare::lessThan(int sourceRow1,
					      int sourceRow2) const {

  if (model->sortKeyColumn_ == -1)
    return sourceRow1 < sourceRow2;

  WModelIndex lhs
    = model->sourceModel()->index(sourceRow1, model->sortKeyColumn_,
				  item->sourceIndex_);

  WModelIndex rhs
    = model->sourceModel()->index(sourceRow2, model->sortKeyColumn_,
				  item->sourceIndex_);

  return model->lessThan(lhs, rhs);
}

WSortFilterProxyModel::WSortFilterProxyModel(WObject *parent)
  : WAbstractProxyModel(parent),
    regex_(new WSortFilterProxyRegExp()),
    filterKeyColumn_(0),
    filterRole_(DisplayRole),
    sortKeyColumn_(-1),
    sortRole_(DisplayRole),
    sortOrder_(AscendingOrder),
    dynamic_(0)
{ }

WSortFilterProxyModel::~WSortFilterProxyModel()
{
  delete regex_;

  resetMappings();
}

void WSortFilterProxyModel::setSourceModel(WAbstractItemModel *model)
{
  if (sourceModel()) {
    for (unsigned i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();
    modelConnections_.clear();
  }

  WAbstractProxyModel::setSourceModel(model);

  modelConnections_.push_back(sourceModel()->columnsAboutToBeInserted.connect
     (SLOT(this, WSortFilterProxyModel::sourceColumnsAboutToBeInserted)));
  modelConnections_.push_back(sourceModel()->columnsInserted.connect
     (SLOT(this, WSortFilterProxyModel::sourceColumnsInserted)));

  modelConnections_.push_back(sourceModel()->columnsAboutToBeRemoved.connect
     (SLOT(this, WSortFilterProxyModel::sourceColumnsRemoved)));
  modelConnections_.push_back(sourceModel()->columnsRemoved.connect
     (SLOT(this, WSortFilterProxyModel::sourceColumnsRemoved)));

  modelConnections_.push_back(sourceModel()->rowsAboutToBeInserted.connect
     (SLOT(this, WSortFilterProxyModel::sourceRowsAboutToBeInserted)));
  modelConnections_.push_back(sourceModel()->rowsInserted.connect
     (SLOT(this, WSortFilterProxyModel::sourceRowsInserted)));

  modelConnections_.push_back(sourceModel()->rowsAboutToBeRemoved.connect
     (SLOT(this, WSortFilterProxyModel::sourceRowsAboutToBeRemoved)));
  modelConnections_.push_back(sourceModel()->rowsRemoved.connect
     (SLOT(this, WSortFilterProxyModel::sourceRowsRemoved)));

  modelConnections_.push_back(sourceModel()->dataChanged.connect
     (SLOT(this, WSortFilterProxyModel::sourceDataChanged)));
  modelConnections_.push_back(sourceModel()->headerDataChanged.connect
     (SLOT(this, WSortFilterProxyModel::sourceHeaderDataChanged)));

  modelConnections_.push_back(sourceModel()->layoutAboutToBeChanged.connect
     (SLOT(this, WSortFilterProxyModel::sourceLayoutAboutToBeChanged)));
  modelConnections_.push_back(sourceModel()->layoutChanged.connect
     (SLOT(this, WSortFilterProxyModel::sourceLayoutChanged)));

  resetMappings();
}

void WSortFilterProxyModel::setFilterKeyColumn(int column)
{
  filterKeyColumn_ = column;
}

void WSortFilterProxyModel::setFilterRole(int role)
{
  filterRole_ = role;
}

void WSortFilterProxyModel::setSortRole(int role)
{
  sortRole_ = role;
}

void WSortFilterProxyModel::setFilterRegExp(const WString& pattern)
{
  regex_->rx.assign(pattern.toUTF8().c_str());

  if (sourceModel() && dynamic_) {
    layoutAboutToBeChanged.emit();

    resetMappings();

    layoutChanged.emit();
  }
}

WString WSortFilterProxyModel::filterRegExp() const
{
  return WString::fromUTF8(regex_->rx.str());
}

void WSortFilterProxyModel::sort(int column, SortOrder order)
{
  sortKeyColumn_ = column;
  sortOrder_ = order;

  if (sourceModel()) {
    layoutAboutToBeChanged.emit();

    resetMappings();

    layoutChanged.emit();
  }
}

void WSortFilterProxyModel::setDynamicSortFilter(bool enable)
{
  dynamic_ = enable;
}

void WSortFilterProxyModel::resetMappings()
{
  for (ItemMap::iterator i = mappedIndexes_.begin();
       i != mappedIndexes_.end(); ++i)
    delete i->second;

  mappedIndexes_.clear();
}

WModelIndex WSortFilterProxyModel::mapFromSource(const WModelIndex& sourceIndex)
  const
{
  if (sourceIndex.isValid()) {
    WModelIndex sourceParent = sourceIndex.parent();

    Item *item = itemFromSourceIndex(sourceParent);

    int row = item->sourceRowMap_[sourceIndex.row()];
    if (row != -1)
      return createIndex(row, sourceIndex.column(),
			 static_cast<void *>(item));
    else
      return WModelIndex();
  } else
    return WModelIndex();
}

WModelIndex WSortFilterProxyModel::mapToSource(const WModelIndex& proxyIndex)
  const
{
  if (proxyIndex.isValid()) {
    Item *parentItem = parentItemFromIndex(proxyIndex);
    return sourceModel()->index(parentItem->proxyRowMap_[proxyIndex.row()],
				proxyIndex.column(),
				parentItem->sourceIndex_);
  } else
    return WModelIndex();
}

WModelIndex WSortFilterProxyModel::index(int row, int column,
					 const WModelIndex& parent) const
{
  Item *item = itemFromIndex(parent);

  return createIndex(row, column, static_cast<void *>(item));
}

WModelIndex WSortFilterProxyModel::parent(const WModelIndex& index) const
{
  if (index.isValid()) {
    Item *parentItem = parentItemFromIndex(index);

    return mapFromSource(parentItem->sourceIndex_);
  } else
    return WModelIndex();
}

WSortFilterProxyModel::Item *
WSortFilterProxyModel::parentItemFromIndex(const WModelIndex& index) const
{
  return static_cast<Item *>(index.internalPointer());
}

WSortFilterProxyModel::Item *
WSortFilterProxyModel::itemFromIndex(const WModelIndex& index) const
{
  if (index.isValid()) {
    Item *parentItem = parentItemFromIndex(index);

    WModelIndex sourceIndex
      = sourceModel()->index(parentItem->proxyRowMap_[index.row()],
			     index.column(), parentItem->sourceIndex_);
    return itemFromSourceIndex(sourceIndex);
  } else
    return itemFromSourceIndex(WModelIndex());
}

WSortFilterProxyModel::Item *
WSortFilterProxyModel::itemFromSourceIndex(const WModelIndex& sourceParent)
  const
{
  ItemMap::const_iterator i = mappedIndexes_.find(sourceParent);
  if (i == mappedIndexes_.end()) {
    Item *result = new Item(sourceParent);
    mappedIndexes_[sourceParent] = result;
    updateItem(result);
    return result;
  } else
    return i->second;
}

void WSortFilterProxyModel::updateItem(Item *item) const
{
  int sourceRowCount = sourceModel()->rowCount(item->sourceIndex_);
  item->sourceRowMap_.resize(sourceRowCount);
  item->proxyRowMap_.clear();

  /*
   * Filter...
   */
  for (int i = 0; i < sourceRowCount; ++i) {
    if (filterAcceptRow(i, item->sourceIndex_)) {
      item->sourceRowMap_[i] = item->proxyRowMap_.size();
      item->proxyRowMap_.push_back(i);
    } else
      item->sourceRowMap_[i] = -1;
  }

  /*
   * Sort...
   */
  if (sortKeyColumn_ != -1) {
    std::stable_sort(item->proxyRowMap_.begin(), item->proxyRowMap_.end(),
		     Compare(this, item));

    rebuildSourceRowMap(item);
  }
}

void WSortFilterProxyModel::rebuildSourceRowMap(Item *item) const
{
  for (unsigned i = 0; i < item->proxyRowMap_.size(); ++i)
    item->sourceRowMap_[item->proxyRowMap_[i]] = i;
}

int WSortFilterProxyModel::changedMappedRow(int sourceRow,
					    int currentMappedRow,
					    Item *item) const
{
  /*
   * Filter...
   */
  bool acceptRow = filterAcceptRow(sourceRow, item->sourceIndex_);

  if (!acceptRow)
    return -1;
  else {
    int newRow = std::lower_bound(item->proxyRowMap_.begin(),
				  item->proxyRowMap_.end(), sourceRow,
				  Compare(this, item))
      - item->proxyRowMap_.begin();

    return newRow;
  }
}

bool WSortFilterProxyModel::filterAcceptRow(int sourceRow,
					    const WModelIndex& sourceParent)
  const
{
  if (!regex_->rx.empty()) {
    WString s = asString(sourceModel()->data(sourceRow, filterKeyColumn_,
					     filterRole_, sourceParent));

    bool result = boost::regex_match(s.toUTF8(), regex_->rx);

    return result;
  } else
    return true;
}

bool WSortFilterProxyModel::lessThan(const WModelIndex& lhs,
				     const WModelIndex& rhs) const
{
  return Wt::lessThan(lhs.data(sortRole_), rhs.data(sortRole_));
}

int WSortFilterProxyModel::columnCount(const WModelIndex& parent) const
{
  return sourceModel()->columnCount(mapToSource(parent));
}

int WSortFilterProxyModel::rowCount(const WModelIndex& parent) const
{
  Item *item = itemFromIndex(parent);

  return item->proxyRowMap_.size();
}

void WSortFilterProxyModel::sourceColumnsAboutToBeInserted
  (const WModelIndex& parent, int start, int end)
{ 
  beginInsertColumns(parent, start, end);
}

void WSortFilterProxyModel::sourceColumnsInserted(const WModelIndex& parent,
						  int start, int end)
{
  endInsertColumns();
}

void WSortFilterProxyModel::sourceColumnsAboutToBeRemoved
  (const WModelIndex& parent, int start, int end)
{ 
  beginRemoveColumns(parent, start, end);
}

void WSortFilterProxyModel::sourceColumnsRemoved(const WModelIndex& parent,
						 int start, int end)
{ 
  endRemoveColumns();
}

void WSortFilterProxyModel::sourceRowsAboutToBeInserted
  (const WModelIndex& parent, int start, int end)
{ 
  // TODO
}

void WSortFilterProxyModel::sourceRowsInserted(const WModelIndex& parent,
					       int start, int end)
{ 
  // TODO
}

void WSortFilterProxyModel::sourceRowsAboutToBeRemoved
(const WModelIndex& parent, int start, int end)
{
  // TODO
}

void WSortFilterProxyModel::sourceRowsRemoved(const WModelIndex& parent,
					      int start, int end)
{ 
  // TODO
}

void WSortFilterProxyModel::sourceDataChanged(const WModelIndex& topLeft,
					      const WModelIndex& bottomRight)
{
  bool refilter
    = dynamic_ && (filterKeyColumn_ >= topLeft.column() 
		   && filterKeyColumn_ <= bottomRight.column());

  bool resort
    = dynamic_ && (sortKeyColumn_ >= topLeft.column() 
		   && sortKeyColumn_ <= bottomRight.column());

  WModelIndex parent = mapFromSource(topLeft.parent());
  Item *item = itemFromIndex(parent);

  for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
    int oldMappedRow = item->sourceRowMap_[row];
    bool propagateDataChange = oldMappedRow != -1;

    if (refilter || resort) {
      item->proxyRowMap_.erase(item->proxyRowMap_.begin() + oldMappedRow);
      int newMappedRow = changedMappedRow(row, oldMappedRow, item);
      item->proxyRowMap_.insert(item->proxyRowMap_.begin() + oldMappedRow, row);

      if (newMappedRow != oldMappedRow) {
	if (oldMappedRow != -1) {
	  beginRemoveRows(parent, oldMappedRow, oldMappedRow);
	  item->proxyRowMap_.erase
	    (item->proxyRowMap_.begin() + oldMappedRow);
	  rebuildSourceRowMap(item);
	  endRemoveRows();
	}

	if (newMappedRow != -1) {
	  beginInsertRows(parent, newMappedRow, newMappedRow);
	  item->proxyRowMap_.insert
	    (item->proxyRowMap_.begin() + newMappedRow, row);
	  rebuildSourceRowMap(item);
	  endInsertRows();
	}

	propagateDataChange = false;
      }
    }

    if (propagateDataChange) {
      WModelIndex l = sourceModel()->index(row, topLeft.column(),
					   topLeft.parent());
      WModelIndex r = sourceModel()->index(row, bottomRight.column(),
					   topLeft.parent());

      dataChanged.emit(mapFromSource(l), mapFromSource(r));
    }
  }
}

void WSortFilterProxyModel::sourceHeaderDataChanged(Orientation orientation, 
						    int start, int end)
{
  if (orientation == Vertical) {
    Item *item = itemFromIndex(WModelIndex());
    for (int row = start; row <= end; ++row) {
      int mappedRow = item->sourceRowMap_[row];
      if (mappedRow != -1)
	headerDataChanged.emit(orientation, mappedRow, mappedRow);
    }
  } else
    headerDataChanged.emit(orientation, start, end);
}

void WSortFilterProxyModel::sourceLayoutAboutToBeChanged()
{ 
  layoutAboutToBeChanged.emit();
  resetMappings();
}

void WSortFilterProxyModel::sourceLayoutChanged()
{
  layoutChanged.emit();
}

}
