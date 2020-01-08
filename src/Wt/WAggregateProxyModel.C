/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iostream>

#include "Wt/WAggregateProxyModel.h"
#include "Wt/WException.h"

namespace {
  bool contains2(int a1, int a2, int b1, int b2) {
    return b1 >= a1 && b1 <= a2 && b2 >= a1 && b2 <= a2;
  }

  bool overlaps(int a1, int a2, int b1, int b2) {
    return !((a2 < b1) || (a1 > b2));
  }

  std::string nestingError(int pa, int a1, int a2, int pb, int b1, int b2) {
    std::stringstream msg;

    msg
      << "WAggregateProxyModel: aggregates must strictly nest: ["
      << pa << ": " << a1 << " - " << a2 << "] overlaps partially with ["
      << pb << ": " << b1 << " - " << b2 << "]";
    
    return msg.str();
  }
}

namespace Wt {

WAggregateProxyModel::Aggregate::Aggregate()
  : parentSrc_(-1),
    firstChildSrc_(-1),
    lastChildSrc_(-1),
    level_(0),
    collapsed_(false)
{ }

WAggregateProxyModel::Aggregate::Aggregate(int parentColumn,
					   int firstColumn, int lastColumn)
  : parentSrc_(parentColumn),
    firstChildSrc_(firstColumn),
    lastChildSrc_(lastColumn),
    level_(0),
    collapsed_(false)
{
  if (parentSrc_ != firstChildSrc_ - 1 && parentSrc_ != lastChildSrc_ + 1)
    throw WException("WAggregateProxyModel::addAggregate: parent column "
		     "must border children range");
}

bool WAggregateProxyModel::Aggregate::contains(const Aggregate& other) const
{
  int pa = parentSrc_, a1 = firstChildSrc_, a2 = lastChildSrc_,
    pb = other.parentSrc_, b1 = other.firstChildSrc_, b2 = other.lastChildSrc_;

  if (pb >= a1 && pb <= a2) {
    if (!::contains2(a1, a2, b1, b2))
      throw WException(nestingError(pa, a1, a2, pb, b1, b2));

    return true;
  } else {
    if (::overlaps(a1, a2, b1, b2))
      throw WException(nestingError(pa, a1, a2, pb, b1, b2));

    return false;
  }
}

WAggregateProxyModel::Aggregate *
WAggregateProxyModel::Aggregate::add(const Aggregate& toAdd)
{
  for (unsigned int i = 0; i < nestedAggregates_.size(); ++i) {
    Aggregate& a = nestedAggregates_[i];

    if (a.contains(toAdd))
      return a.add(toAdd);

    if (toAdd.before(a)) {
      nestedAggregates_.insert(nestedAggregates_.begin() + i, toAdd);
      nestedAggregates_[i].level_ = level_ + 1;
      return &nestedAggregates_[i];
    }
  }

  nestedAggregates_.push_back(toAdd);
  nestedAggregates_.back().level_ = level_ + 1;
  return &nestedAggregates_.back();
}

WAggregateProxyModel::Aggregate *
WAggregateProxyModel::Aggregate::findAggregate(int parentColumn)
{
  if (parentSrc_ == parentColumn)
    return this;
  else if (parentSrc_ != -1 && parentColumn > lastChildSrc_)
    return nullptr;
  else {
    for (unsigned int i = 0; i < nestedAggregates_.size(); ++i) {
      Aggregate& a = nestedAggregates_[i];

      Aggregate *result = a.findAggregate(parentColumn);
      if (result)
	return result;
    }
  }

  return nullptr;
}

const WAggregateProxyModel::Aggregate *
WAggregateProxyModel::Aggregate::findAggregate(int parentColumn) const
{
  return const_cast<Aggregate *>(this)->findAggregate(parentColumn);
}

const WAggregateProxyModel::Aggregate *
WAggregateProxyModel::Aggregate::findEnclosingAggregate(int column) const
{
  for (unsigned int i = 0; i < nestedAggregates_.size(); ++i) {
    const Aggregate& a = nestedAggregates_[i];

    if (a.after(column))
      return this;

    if (a.contains(column))
      return a.findEnclosingAggregate(column);
  }

  return this;
}

int WAggregateProxyModel::Aggregate::mapFromSource(int sourceColumn) const
{
  int collapsedCount = 0;

  for (unsigned i = 0; i < nestedAggregates_.size(); ++i) {
    const Aggregate& a = nestedAggregates_[i];

    if (a.after(sourceColumn))
      return sourceColumn - collapsedCount;
    else if (a.contains(sourceColumn))
      if (a.collapsed_)
	return -1;
      else
	return a.mapFromSource(sourceColumn) - collapsedCount;
    else // a < sourceColumn
      collapsedCount += a.collapsedCount();
  }

  return sourceColumn - collapsedCount;
}

int WAggregateProxyModel::Aggregate::mapToSource(int column) const
{
  int sourceColumn = column;

  for (unsigned i = 0; i < nestedAggregates_.size(); ++i) {
    const Aggregate& a = nestedAggregates_[i];

    if (a.after(sourceColumn))
      return sourceColumn;
    else if (!a.collapsed_ && a.contains(sourceColumn))
      return a.mapToSource(sourceColumn);
    else
      sourceColumn += a.collapsedCount();
  }

  return sourceColumn;
}

bool WAggregateProxyModel::Aggregate::before(const Aggregate& other) const
{
  return lastChildSrc_ < other.firstChildSrc_;
}

bool WAggregateProxyModel::Aggregate::after(int column) const
{
  return firstChildSrc_ > column;
}

bool WAggregateProxyModel::Aggregate::before(int column) const
{
  return lastChildSrc_ < column;
}

bool WAggregateProxyModel::Aggregate::contains(int sourceColumn) const
{
  return firstChildSrc_ <= sourceColumn && sourceColumn <= lastChildSrc_;
}

int WAggregateProxyModel::Aggregate::collapsedCount() const
{
  if (collapsed_)
    return lastChildSrc_ - firstChildSrc_ + 1;
  else {
    int result = 0;

    for (unsigned i = 0; i < nestedAggregates_.size(); ++i) {
      const Aggregate& a = nestedAggregates_[i];

      result += a.collapsedCount();
    }

    return result;
  }
}

int WAggregateProxyModel::Aggregate::firstVisibleNotBefore(int column) const
{
  if (collapsed_)
    return lastChildSrc_ + 1;
  else {
    for (unsigned i = 0; i < nestedAggregates_.size(); ++i) {
      const Aggregate& a = nestedAggregates_[i];

      if (a.after(column))
	return column;
      else if (a.before(column))
	continue;
      else
	column = a.firstVisibleNotBefore(column);
    }

    return column;
  }
}

int WAggregateProxyModel::Aggregate::lastVisibleNotAfter(int column) const
{
  if (collapsed_)
    return firstChildSrc_ - 1;
  else {
    for (int i = nestedAggregates_.size() - 1; i >= 0; --i) {
      const Aggregate& a = nestedAggregates_[i];

      if (a.before(column))
	return column;
      else if (a.after(column))
	continue;
      else
	column = a.lastVisibleNotAfter(column);
    }

    return column;
  }
}

WAggregateProxyModel::WAggregateProxyModel()
  : topLevel_()
{ }

WAggregateProxyModel::~WAggregateProxyModel()
{ }

void WAggregateProxyModel
::setSourceModel(const std::shared_ptr<WAbstractItemModel>& model)
{
  for (unsigned i = 0; i < modelConnections_.size(); ++i)
    modelConnections_[i].disconnect();
  modelConnections_.clear();

  WAbstractProxyModel::setSourceModel(model);

  modelConnections_.push_back(sourceModel()->columnsAboutToBeInserted().connect
     (this, &WAggregateProxyModel::sourceColumnsAboutToBeInserted));
  modelConnections_.push_back(sourceModel()->columnsInserted().connect
     (this, &WAggregateProxyModel::sourceColumnsInserted));

  modelConnections_.push_back(sourceModel()->columnsAboutToBeRemoved().connect
     (this, &WAggregateProxyModel::sourceColumnsAboutToBeRemoved));
  modelConnections_.push_back(sourceModel()->columnsRemoved().connect
     (this, &WAggregateProxyModel::sourceColumnsRemoved));

  modelConnections_.push_back(sourceModel()->rowsAboutToBeInserted().connect
     (this, &WAggregateProxyModel::sourceRowsAboutToBeInserted));
  modelConnections_.push_back(sourceModel()->rowsInserted().connect
     (this, &WAggregateProxyModel::sourceRowsInserted));

  modelConnections_.push_back(sourceModel()->rowsAboutToBeRemoved().connect
     (this, &WAggregateProxyModel::sourceRowsAboutToBeRemoved));
  modelConnections_.push_back(sourceModel()->rowsRemoved().connect
     (this, &WAggregateProxyModel::sourceRowsRemoved));

  modelConnections_.push_back(sourceModel()->dataChanged().connect
     (this, &WAggregateProxyModel::sourceDataChanged));
  modelConnections_.push_back(sourceModel()->headerDataChanged().connect
     (this, &WAggregateProxyModel::sourceHeaderDataChanged));

  modelConnections_.push_back(sourceModel()->layoutAboutToBeChanged().connect
     (this, &WAggregateProxyModel::sourceLayoutAboutToBeChanged));
  modelConnections_.push_back(sourceModel()->layoutChanged().connect
     (this, &WAggregateProxyModel::sourceLayoutChanged));

  modelConnections_.push_back(sourceModel()->modelReset().connect
     (this, &WAggregateProxyModel::sourceModelReset));

  topLevel_ = Aggregate();
}

void WAggregateProxyModel::addAggregate(int parentColumn,
					int firstColumn, int lastColumn)
{
  Aggregate *added
    = topLevel_.add(Aggregate(parentColumn, firstColumn, lastColumn));

  collapse(*added);
}

void WAggregateProxyModel::propagateBeginRemove(const WModelIndex& proxyIndex,
						int start, int end)
{
  // should be beginRemoveColumns(), but endRemoveColumns() calls cannot
  // be nested
  columnsAboutToBeRemoved().emit(proxyIndex, start, end);

  unsigned int rc = rowCount(proxyIndex);
  for (unsigned i = 0; i < rc; ++i)
    propagateBeginRemove(index(i, 0, proxyIndex), start, end);
}

void WAggregateProxyModel::propagateEndRemove(const WModelIndex& proxyIndex,
					      int start, int end)
{
  // should be endRemoveColumns(), but endRemoveColumns() calls cannot
  // be nested
  columnsRemoved().emit(proxyIndex, start, end);

  unsigned int rc = rowCount(proxyIndex);
  for (unsigned i = 0; i < rc; ++i)
    propagateEndRemove(index(i, 0, proxyIndex), start, end);
}

void WAggregateProxyModel::propagateBeginInsert(const WModelIndex& proxyIndex,
						int start, int end)
{
  // should be beginInsertColumns(), but endInsertColumns() calls cannot
  // be nested
  columnsAboutToBeInserted().emit(proxyIndex, start, end);

  unsigned int rc = rowCount(proxyIndex);
  for (unsigned i = 0; i < rc; ++i)
    propagateBeginInsert(index(i, 0, proxyIndex), start, end);
}

void WAggregateProxyModel::propagateEndInsert(const WModelIndex& proxyIndex,
					      int start, int end)
{
  // should be endInsertColumns(), but endInsertColumns() calls cannot
  // be nested
  columnsInserted().emit(proxyIndex, start, end);

  unsigned int rc = rowCount(proxyIndex);
  for (unsigned i = 0; i < rc; ++i)
    propagateEndInsert(index(i, 0, proxyIndex), start, end);
}

void WAggregateProxyModel::expandColumn(int column)
{
  int sourceColumn = topLevel_.mapToSource(column);
  Aggregate *ag = topLevel_.findAggregate(sourceColumn);

  if (ag)
    expand(*ag);
}

void WAggregateProxyModel::collapseColumn(int column)
{
  int sourceColumn = topLevel_.mapToSource(column);
  Aggregate *ag = topLevel_.findAggregate(sourceColumn);

  if (ag)
    collapse(*ag);
}

void WAggregateProxyModel::expand(Aggregate& aggregate)
{
  int c = topLevel_.mapFromSource(aggregate.parentSrc_);
  if (c >= 0) {
    aggregate.collapsed_ = false;
    int c1 = topLevel_.mapFromSource(firstVisibleSourceNotBefore
				     (aggregate.firstChildSrc_));
    int c2 = topLevel_.mapFromSource(lastVisibleSourceNotAfter
				     (aggregate.lastChildSrc_));
    aggregate.collapsed_ = true;

    propagateBeginInsert(WModelIndex(), c1, c2);
    aggregate.collapsed_ = false;
    propagateEndInsert(WModelIndex(), c1, c2);
  } else
    aggregate.collapsed_ = false;
}

void WAggregateProxyModel::collapse(Aggregate& aggregate)
{
  int c = topLevel_.mapFromSource(aggregate.parentSrc_);
  if (c >= 0) {
    int c1 = topLevel_.mapFromSource(firstVisibleSourceNotBefore
				     (aggregate.firstChildSrc_));
    int c2 = topLevel_.mapFromSource(lastVisibleSourceNotAfter
				     (aggregate.lastChildSrc_));

    propagateBeginRemove(WModelIndex(), c1, c2);
    aggregate.collapsed_ = true;
    propagateEndRemove(WModelIndex(), c1, c2);
  } else
    aggregate.collapsed_ = true;
}

WModelIndex WAggregateProxyModel::mapFromSource(const WModelIndex& sourceIndex)
  const
{
  if (sourceIndex.isValid()) {
    int column = topLevel_.mapFromSource(sourceIndex.column());
    if (column >= 0) {
      int row = sourceIndex.row();

      return createIndex(row, column, sourceIndex.internalPointer());
    } else
      return WModelIndex();
  } else
    return WModelIndex();
}

WModelIndex WAggregateProxyModel::mapToSource(const WModelIndex& proxyIndex)
  const
{
  if (proxyIndex.isValid()) {
    int column = topLevel_.mapToSource(proxyIndex.column());
    int row = proxyIndex.row();

    return createSourceIndex(row, column, proxyIndex.internalPointer());
  } else
    return WModelIndex();
}

WModelIndex WAggregateProxyModel::index(int row, int column,
					const WModelIndex& parent) const
{
  WModelIndex sourceParent = mapToSource(parent);
  int sourceRow = row;
  int sourceColumn = topLevel_.mapToSource(column);

  WModelIndex sourceIndex
    = sourceModel()->index(sourceRow, sourceColumn, sourceParent);

  return createIndex(row, column,
		     sourceIndex.isValid() ? sourceIndex.internalPointer() : 
		     nullptr);
}

WModelIndex WAggregateProxyModel::parent(const WModelIndex& index) const
{
  if (index.isValid())
    return mapFromSource(mapToSource(index).parent());
  else
    return WModelIndex();
}

int WAggregateProxyModel::columnCount(const WModelIndex& parent) const
{
  int c = sourceModel()->columnCount(mapToSource(parent));
  if (c > 0) {
    c = lastVisibleSourceNotAfter(c - 1);
    return topLevel_.mapFromSource(c) + 1;
  } else
    return 0;
}

int WAggregateProxyModel::rowCount(const WModelIndex& parent) const
{
  return sourceModel()->rowCount(mapToSource(parent));
}

void WAggregateProxyModel::sort(int column, Wt::SortOrder order)
{
  sourceModel()->sort(topLevel_.mapToSource(column), order);
}

cpp17::any WAggregateProxyModel::headerData(int section,
                                     Orientation orientation, ItemDataRole role) const
{
  if (orientation == Orientation::Horizontal) {
    section = topLevel_.mapToSource(section);
    if (role == ItemDataRole::Level) {
      const Aggregate *agg = topLevel_.findEnclosingAggregate(section);
      return cpp17::any(agg->level_);
    } else
      return sourceModel()->headerData(section, orientation, role);
  } else
    return sourceModel()->headerData(section, orientation, role);
}

bool WAggregateProxyModel::setHeaderData(int section, Orientation orientation,
                                         const cpp17::any& value, ItemDataRole role)
{
  if (orientation == Orientation::Horizontal)
    section = topLevel_.mapToSource(section);

  return sourceModel()->setHeaderData(section, orientation, value, role);
}

WFlags<HeaderFlag> WAggregateProxyModel::headerFlags(int section,
						     Orientation orientation)
  const
{
  if (orientation == Orientation::Horizontal) {
    int srcColumn = topLevel_.mapToSource(section);

    WFlags<HeaderFlag> result
      = sourceModel()->headerFlags(srcColumn, orientation);

    const Aggregate *agg = topLevel_.findAggregate(srcColumn);
    if (agg) {
      if (agg->collapsed_)
	return result | HeaderFlag::ColumnIsCollapsed;
      else
	if (agg->parentSrc_ == agg->lastChildSrc_ + 1)
	  return result | HeaderFlag::ColumnIsExpandedLeft;
	else // agg->parentSrc_ == firstChildSrc_ - 1
	  return result | HeaderFlag::ColumnIsExpandedRight;
    } else
      return result;
  } else
    return sourceModel()->headerFlags(section, orientation);
}

void WAggregateProxyModel::sourceColumnsAboutToBeInserted
  (const WModelIndex& parent, int start, int end)
{
  throw WException("WAggregateProxyModel does not support "
		   "source model column insertion");
}

void WAggregateProxyModel::sourceColumnsInserted(const WModelIndex& parent,
						 int start, int end)
{
  throw WException("WAggregateProxyModel does not support "
		   "source model column insertion");
}

void WAggregateProxyModel::sourceColumnsAboutToBeRemoved
  (const WModelIndex& parent, int start, int end)
{
  throw WException("WAggregateProxyModel does not support "
		   "source model column removal");
}

void WAggregateProxyModel::sourceColumnsRemoved(const WModelIndex& parent,
						int start, int end)
{
  throw WException("WAggregateProxyModel does not support "
		   "source model column removal");
}

void WAggregateProxyModel::sourceRowsAboutToBeInserted
  (const WModelIndex& parent, int start, int end)
{
  WModelIndex proxyParent = mapFromSource(parent);

  if (proxyParent.isValid() || !parent.isValid())
    beginInsertRows(proxyParent, start, end);
}

void WAggregateProxyModel::sourceRowsInserted(const WModelIndex& parent,
					      int start, int end)
{
  WModelIndex proxyParent = mapFromSource(parent);

  if (proxyParent.isValid() || !parent.isValid())
    endInsertRows();
}

void WAggregateProxyModel::sourceRowsAboutToBeRemoved
(const WModelIndex& parent, int start, int end)
{
  WModelIndex proxyParent = mapFromSource(parent);

  if (proxyParent.isValid() || !parent.isValid())
    beginRemoveRows(proxyParent, start, end);
}

void WAggregateProxyModel::sourceRowsRemoved(const WModelIndex& parent,
					      int start, int end)
{ 
  WModelIndex proxyParent = mapFromSource(parent);

  if (proxyParent.isValid() || !parent.isValid())
    endRemoveRows();
}

int WAggregateProxyModel::firstVisibleSourceNotBefore(int column) const
{
  return topLevel_.firstVisibleNotBefore(column);
}

int WAggregateProxyModel::lastVisibleSourceNotAfter(int column) const
{
  return topLevel_.lastVisibleNotAfter(column);
}

void WAggregateProxyModel::sourceDataChanged(const WModelIndex& topLeft,
					     const WModelIndex& bottomRight)
{
  int l = firstVisibleSourceNotBefore(topLeft.column());
  int r = lastVisibleSourceNotAfter(bottomRight.column());

  if (r >= l) {
    WModelIndex tl = mapFromSource(sourceModel()->index(topLeft.row(),
							l,
							topLeft.parent()));
    WModelIndex br = mapFromSource(sourceModel()->index(bottomRight.row(),
							r,
							bottomRight.parent()));
    dataChanged().emit(tl, br);
  }
}

void WAggregateProxyModel::sourceHeaderDataChanged(Orientation orientation, 
						   int start, int end)
{
  if (orientation == Orientation::Vertical) {
    headerDataChanged().emit(orientation, start, end);
  } else {
    int l = firstVisibleSourceNotBefore(start);
    int r = lastVisibleSourceNotAfter(end);

    if (r >= l) {
      l = topLevel_.mapFromSource(l);
      r = topLevel_.mapFromSource(r);

      headerDataChanged().emit(orientation, l, r);
    }
  }
}

void WAggregateProxyModel::sourceLayoutAboutToBeChanged()
{ 
  layoutAboutToBeChanged().emit();
}

void WAggregateProxyModel::sourceLayoutChanged()
{
  layoutChanged().emit();
}

void WAggregateProxyModel::sourceModelReset()
{
  topLevel_ = Aggregate();
  reset();
}

}
