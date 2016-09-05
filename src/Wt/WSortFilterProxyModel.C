/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WSortFilterProxyModel"
#include "Wt/WRegExp"

#include "WebUtils.h"

namespace Wt {

#ifndef DOXYGEN_ONLY
#ifndef WT_TARGET_JAVA
bool WSortFilterProxyModel::Compare::operator()(int sourceRow1,
						int sourceRow2) const
{
  if (model->sortOrder_ == AscendingOrder)
    return lessThan(sourceRow1, sourceRow2);
  else
    return lessThan(sourceRow2, sourceRow1);
}

bool WSortFilterProxyModel::Compare::lessThan(int sourceRow1, int sourceRow2)
  const
{
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
#endif // WT_TARGET_JAVA

int WSortFilterProxyModel::Compare::compare(int sourceRow1, int sourceRow2)
  const
{
  int factor = (model->sortOrder_ == AscendingOrder) ? 1 : -1;

  if (model->sortKeyColumn_ == -1)
    return factor * (sourceRow1 - sourceRow2);

  WModelIndex lhs
    = model->sourceModel()->index(sourceRow1, model->sortKeyColumn_,
				  item->sourceIndex_);

  WModelIndex rhs
    = model->sourceModel()->index(sourceRow2, model->sortKeyColumn_,
				  item->sourceIndex_);

  return factor * model->compare(lhs, rhs);
}

#endif // DOXYGEN_ONLY

WSortFilterProxyModel::Item::~Item()
{ }

WSortFilterProxyModel::WSortFilterProxyModel(WObject *parent)
  : WAbstractProxyModel(parent),
    regex_(0),
    filterKeyColumn_(0),
    filterRole_(DisplayRole),
    sortKeyColumn_(-1),
    sortRole_(DisplayRole),
    sortOrder_(AscendingOrder),
    dynamic_(false),
    inserting_(false),
    mappedRootItem_(0)
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

  modelConnections_.push_back(sourceModel()->columnsAboutToBeInserted().connect
     (this, &WSortFilterProxyModel::sourceColumnsAboutToBeInserted));
  modelConnections_.push_back(sourceModel()->columnsInserted().connect
     (this, &WSortFilterProxyModel::sourceColumnsInserted));

  modelConnections_.push_back(sourceModel()->columnsAboutToBeRemoved().connect
     (this, &WSortFilterProxyModel::sourceColumnsAboutToBeRemoved));
  modelConnections_.push_back(sourceModel()->columnsRemoved().connect
     (this, &WSortFilterProxyModel::sourceColumnsRemoved));

  modelConnections_.push_back(sourceModel()->rowsAboutToBeInserted().connect
     (this, &WSortFilterProxyModel::sourceRowsAboutToBeInserted));
  modelConnections_.push_back(sourceModel()->rowsInserted().connect
     (this, &WSortFilterProxyModel::sourceRowsInserted));

  modelConnections_.push_back(sourceModel()->rowsAboutToBeRemoved().connect
     (this, &WSortFilterProxyModel::sourceRowsAboutToBeRemoved));
  modelConnections_.push_back(sourceModel()->rowsRemoved().connect
     (this, &WSortFilterProxyModel::sourceRowsRemoved));

  modelConnections_.push_back(sourceModel()->dataChanged().connect
     (this, &WSortFilterProxyModel::sourceDataChanged));
  modelConnections_.push_back(sourceModel()->headerDataChanged().connect
     (this, &WSortFilterProxyModel::sourceHeaderDataChanged));

  modelConnections_.push_back(sourceModel()->layoutAboutToBeChanged().connect
     (this, &WSortFilterProxyModel::sourceLayoutAboutToBeChanged));
  modelConnections_.push_back(sourceModel()->layoutChanged().connect
     (this, &WSortFilterProxyModel::sourceLayoutChanged));

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

void WSortFilterProxyModel::setFilterRegExp(const WT_USTRING& pattern)
{
  if (!regex_)
    regex_ = new WRegExp(pattern);
  else
    regex_->setPattern(pattern, regex_->flags());

  invalidate();
}

WT_USTRING WSortFilterProxyModel::filterRegExp() const
{
  return regex_ ? regex_->pattern() : WT_USTRING();
}

void WSortFilterProxyModel::setFilterFlags(WFlags<RegExpFlag> flags)
{
  if (!regex_)
    regex_ = new WRegExp(".*");

  regex_->setPattern(regex_->pattern(), flags);
}

WFlags<RegExpFlag> WSortFilterProxyModel::filterFlags() const
{
  if (regex_)
    return regex_->flags();
  else 
#ifndef WT_TARGET_JAVA
    return WFlags<RegExpFlag>();
#else
    return (int)0;
#endif
}

void WSortFilterProxyModel::sort(int column, SortOrder order)
{
  sortKeyColumn_ = column;
  sortOrder_ = order;

  invalidate();
}

void WSortFilterProxyModel::invalidate()
{
  if (sourceModel()) {
    layoutAboutToBeChanged().emit();
    resetMappings();
    layoutChanged().emit();
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

  delete mappedRootItem_;
  mappedRootItem_ = 0;
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
  if (!sourceParent.isValid()) {
    if (!mappedRootItem_) {
      Item *result = new Item(sourceParent);
      mappedRootItem_ = result;
      updateItem(result);
    }
    return mappedRootItem_;
  }

  ItemMap::const_iterator i = mappedIndexes_.find(sourceParent);
  if (i == mappedIndexes_.end()) {
    Item *result = new Item(sourceParent);
    mappedIndexes_[sourceParent] = result;
    updateItem(result);
    return result;
  } else
    return dynamic_cast<Item *>(i->second);
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
    Utils::stable_sort(item->proxyRowMap_, Compare(this, item));

    rebuildSourceRowMap(item);
  }
}

void WSortFilterProxyModel::rebuildSourceRowMap(Item *item) const
{
  for (unsigned i = 0; i < item->sourceRowMap_.size(); ++i)
    item->sourceRowMap_[i] = -1;

  for (unsigned i = 0; i < item->proxyRowMap_.size(); ++i)
    item->sourceRowMap_[item->proxyRowMap_[i]] = i;
}

int WSortFilterProxyModel::mappedInsertionPoint(int sourceRow, Item *item) const
{
  /*
   * Filter...
   */
  bool acceptRow = filterAcceptRow(sourceRow, item->sourceIndex_);

  if (!acceptRow)
    return -1;
  else
    return Utils::insertion_point(item->proxyRowMap_ , sourceRow,
				  Compare(this, item));
}

bool WSortFilterProxyModel::filterAcceptRow(int sourceRow,
					    const WModelIndex& sourceParent)
  const
{
  if (regex_) {
    WString s = asString(sourceModel()
			 ->index(sourceRow, filterKeyColumn_, sourceParent)
			 .data(filterRole_));
    bool result = regex_->exactMatch(s);

    return result;
  } else
    return true;
}

#ifndef WT_TARGET_JAVA
bool WSortFilterProxyModel::lessThan(const WModelIndex& lhs,
				     const WModelIndex& rhs) const
{
  return WSortFilterProxyModel::compare(lhs, rhs) < 0;
}
#endif // WT_TARGET_JAVA


int WSortFilterProxyModel::compare(const WModelIndex& lhs,
				   const WModelIndex& rhs) const
{
  return Wt::Impl::compare(lhs.data(sortRole_), rhs.data(sortRole_));
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

bool WSortFilterProxyModel::setHeaderData(int section, Orientation orientation,
					  const boost::any& value, int role)
{
  if (orientation == Vertical)
    section = mapToSource(index(section, 0)).row();

  return sourceModel()->setHeaderData(section, orientation, value, role);
}

boost::any WSortFilterProxyModel::headerData(int section,
					     Orientation orientation, int role)
  const
{
  if (orientation == Vertical)
    section = mapToSource(index(section, 0)).row();

  return sourceModel()->headerData(section, orientation, role);
}

WFlags<HeaderFlag> WSortFilterProxyModel::headerFlags(int section,
						      Orientation orientation)
    const
{
  if (orientation == Vertical)
    section = mapToSource(index(section, 0)).row();

  return sourceModel()->headerFlags(section, orientation);
}

void WSortFilterProxyModel::sourceColumnsAboutToBeInserted
  (const WModelIndex& parent, int start, int end)
{
  beginInsertColumns(mapFromSource(parent), start, end);
}

void WSortFilterProxyModel::sourceColumnsInserted(const WModelIndex& parent,
						  int start, int end)
{
  endInsertColumns();
}

void WSortFilterProxyModel::sourceColumnsAboutToBeRemoved
  (const WModelIndex& parent, int start, int end)
{ 
  beginRemoveColumns(mapFromSource(parent), start, end);
}

void WSortFilterProxyModel::sourceColumnsRemoved(const WModelIndex& parent,
						 int start, int end)
{ 
  endRemoveColumns();
}

void WSortFilterProxyModel::sourceRowsAboutToBeInserted
  (const WModelIndex& parent, int start, int end)
{
  if (inserting_)
    return;

  /*
   * Make sure the item starts in a known state, otherwise if the item
   * does not yet have a sourceRowMap, it will be created taking into
   * account the already updated number of source rows, in
   * sourceRowsInserted().
   *
   * BTW. one might wonder if a user of the proxy model is interested
   * at all in changes to a node which he has not yet 'opened' ..., but
   * strictly spoken we are obliged to propagate these changes !
   */
  WModelIndex pparent = mapFromSource(parent);
  // distinguish between invalid parent being root item or being filtered out
  if (parent.isValid() && !pparent.isValid())
    return;
  itemFromIndex(pparent);
}

void WSortFilterProxyModel::sourceRowsInserted(const WModelIndex& parent,
					       int start, int end)
{
  shiftModelIndexes(parent, end + 1, (end - start + 1), mappedIndexes_);

  if (inserting_)
    return;

  int count = end - start + 1;

  WModelIndex pparent = mapFromSource(parent);
  // distinguish between invalid parent being root item or being filtered out
  if (parent.isValid() && !pparent.isValid())
    return;
  Item *item = itemFromIndex(pparent);

  // Shift existing entries in proxyRowMap, and reserve place in sourceRowMap
  // After this step, existing rows are okay again.
  for (unsigned i = 0; i < item->proxyRowMap_.size(); ++i) {
    if (item->proxyRowMap_[i] >= start)
      item->proxyRowMap_[i] += count;
  }

  item->sourceRowMap_.insert(item->sourceRowMap_.begin() + start, count, -1);

  if (!dynamic_)
    return;

  for (int row = start; row <= end; ++row) {
    int newMappedRow = mappedInsertionPoint(row, item);
    if (newMappedRow != -1) {
      beginInsertRows(pparent, newMappedRow, newMappedRow);
      item->proxyRowMap_.insert
	(item->proxyRowMap_.begin() + newMappedRow, row);
      rebuildSourceRowMap(item); // insertion may have shifted some
      endInsertRows();
    } else
      item->sourceRowMap_[row] = -1;
  }
}

void WSortFilterProxyModel::sourceRowsAboutToBeRemoved
(const WModelIndex& parent, int start, int end)
{
  WModelIndex pparent = mapFromSource(parent);
  // distinguish between invalid parent being root item or being filtered out
  if (parent.isValid() && !pparent.isValid())
    return;
  Item *item = itemFromIndex(pparent);

  for (int row = start; row <= end; ++row) {
    int mappedRow = item->sourceRowMap_[row];

    if (mappedRow != -1) {
      beginRemoveRows(pparent, mappedRow, mappedRow);
      item->proxyRowMap_.erase(item->proxyRowMap_.begin() + mappedRow);
      rebuildSourceRowMap(item); // erase may have shifted some
      endRemoveRows();
    }
  }
}

void WSortFilterProxyModel::sourceRowsRemoved(const WModelIndex& parent,
					      int start, int end)
{
  int count = end - start + 1;

  shiftModelIndexes(parent, start, -count, mappedIndexes_);

  WModelIndex pparent = mapFromSource(parent);
  // distinguish between invalid parent being root item or being filtered out
  if (parent.isValid() && !pparent.isValid())
    return;
  Item *item = itemFromIndex(pparent);

  // Shift existing entries in proxyRowMap, and remove entries in sourceRowMap
  for (unsigned i = 0; i < item->proxyRowMap_.size(); ++i) {
    if (item->proxyRowMap_[i] >= start)
      item->proxyRowMap_[i] -= count;
  }

  item->sourceRowMap_.erase(item->sourceRowMap_.begin() + start,
			    item->sourceRowMap_.begin() + start + count);
}

void WSortFilterProxyModel::sourceDataChanged(const WModelIndex& topLeft,
					      const WModelIndex& bottomRight)
{
  if (!topLeft.isValid() || !bottomRight.isValid())
    return;

  bool refilter
    = dynamic_ && (filterKeyColumn_ >= topLeft.column() 
		   && filterKeyColumn_ <= bottomRight.column());

  bool resort
    = dynamic_ && (sortKeyColumn_ >= topLeft.column() 
		   && sortKeyColumn_ <= bottomRight.column());

  WModelIndex parent = mapFromSource(topLeft.parent());
  // distinguish between invalid parent being root item or being filtered out
  if (topLeft.parent().isValid() && !parent.isValid())
    return;
  Item *item = itemFromIndex(parent);

  for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
    int oldMappedRow = item->sourceRowMap_[row];
    bool propagateDataChange = oldMappedRow != -1;

    if (refilter || resort) {
      // Determine new insertion point: erase it temporarily for this
      if (oldMappedRow != -1)
	item->proxyRowMap_.erase(item->proxyRowMap_.begin() + oldMappedRow);
      int newMappedRow = mappedInsertionPoint(row, item);
      if (oldMappedRow != -1)
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

      dataChanged().emit(mapFromSource(l), mapFromSource(r));
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
	headerDataChanged().emit(orientation, mappedRow, mappedRow);
    }
  } else
    headerDataChanged().emit(orientation, start, end);
}

void WSortFilterProxyModel::sourceLayoutAboutToBeChanged()
{ 
  layoutAboutToBeChanged().emit();
  resetMappings();
}

void WSortFilterProxyModel::sourceLayoutChanged()
{
  layoutChanged().emit();
}

bool WSortFilterProxyModel::insertRows(int row, int count,
				       const WModelIndex& parent)
{
  int sourceRow;

  int currentCount = rowCount(parent);
  if (row < currentCount)
    sourceRow = mapToSource(index(row, 0, parent)).row();
  else
    sourceRow = sourceModel()->rowCount(mapToSource(parent));

  inserting_ = true;
  bool result
    = sourceModel()->insertRows(sourceRow, count, mapToSource(parent));
  inserting_ = false;

  if (!result)
    return false;

  Item *item = itemFromIndex(parent);

  beginInsertRows(parent, row, row);
  item->proxyRowMap_.push_back(sourceRow);
  item->sourceRowMap_.insert(item->sourceRowMap_.begin() + sourceRow, row);
  endInsertRows();

  return true;
}

bool WSortFilterProxyModel::removeRows(int row, int count,
				       const WModelIndex& parent)
{
  for (int i = 0; i < count; ++i) {
    int sourceRow = mapToSource(index(row, 0, parent)).row();
    if (!sourceModel()->removeRows(sourceRow, 1, mapToSource(parent)))
      return false;
  }

  return true;
}

}
