/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractProxyModel"

namespace Wt {

#ifndef DOXYGEN_ONLY

WAbstractProxyModel::BaseItem::~BaseItem()
{ }

WAbstractProxyModel::WAbstractProxyModel(WObject *parent)
  : WAbstractItemModel(parent),
    sourceModel_(0)
{ }

void WAbstractProxyModel::setSourceModel(WAbstractItemModel *sourceModel)
{
  sourceModel_ = sourceModel;
}

boost::any WAbstractProxyModel::data(const WModelIndex& index, int role) const
{
  return sourceModel_->data(mapToSource(index), role);
}

bool WAbstractProxyModel::setData(const WModelIndex& index,
				  const boost::any& value, int role)
{
  return sourceModel_->setData(mapToSource(index), value, role);
}

bool WAbstractProxyModel::setItemData(const WModelIndex& index,
				      const DataMap& values)
{
  return sourceModel_->setItemData(mapToSource(index), values);
}

WFlags<ItemFlag> WAbstractProxyModel::flags(const WModelIndex& index) const
{
  return sourceModel_->flags(mapToSource(index));
}

bool WAbstractProxyModel::insertColumns(int column, int count,
					const WModelIndex& parent)
{
  return sourceModel_->insertColumns(column, count, parent);
}

bool WAbstractProxyModel::removeColumns(int column, int count,
					const WModelIndex& parent)
{
  return sourceModel_->removeColumns(column, count, parent);
}

std::string WAbstractProxyModel::mimeType() const
{
  return sourceModel_->mimeType();
}

std::vector<std::string> WAbstractProxyModel::acceptDropMimeTypes() const
{
  return sourceModel_->acceptDropMimeTypes();
}

void WAbstractProxyModel::dropEvent(const WDropEvent& e, DropAction action,
				    int row, int column,
				    const WModelIndex& parent)
{
  WModelIndex sourceParent = mapToSource(parent);

  int sourceRow = row;
  int sourceColumn = column;

  if (sourceRow != -1)
    sourceRow = mapToSource(index(row, 0, parent)).row();

  sourceModel_->dropEvent(e, action, sourceRow, sourceColumn, sourceParent);
}

void *WAbstractProxyModel::toRawIndex(const WModelIndex& index) const
{
  return sourceModel_->toRawIndex(mapToSource(index));
}

WModelIndex WAbstractProxyModel::fromRawIndex(void *rawIndex) const
{
  return mapFromSource(sourceModel_->fromRawIndex(rawIndex));
}

WFlags<HeaderFlag> WAbstractProxyModel::headerFlags(int section, Orientation orientation) const
{
  if (orientation == Wt::Horizontal) {
    section = mapToSource(index(0, section, Wt::WModelIndex())).column();
  } else {
    section = mapToSource(index(section, 0, Wt::WModelIndex())).row();
  }
  return sourceModel_->headerFlags(section, orientation);
}

boost::any WAbstractProxyModel::headerData(int section, Orientation orientation, int role) const
{
  if (orientation == Wt::Horizontal) {
    section = mapToSource(index(0, section, Wt::WModelIndex())).column();
  } else {
    section = mapToSource(index(section, 0, Wt::WModelIndex())).row();
  }
  return sourceModel_->headerData(section, orientation, role);
}

WModelIndex WAbstractProxyModel::createSourceIndex(int row, int column,
						   void *ptr) const
{
  return sourceModel_->createIndex(row, column, ptr);
}

void WAbstractProxyModel::shiftModelIndexes(const WModelIndex& sourceParent,
					    int start, int count,
					    ItemMap& items)
{
  /*
   * We must shift all indexes within sourceParent >= start with count
   * and delete items when count < 0.
   */
  std::vector<BaseItem *> shifted;
  std::vector<BaseItem *> erased;

  WModelIndex startIndex;
  if (sourceModel()->rowCount(sourceParent) == 0)
    startIndex = sourceParent;
  else if (start >= sourceModel()->rowCount(sourceParent))
    startIndex = sourceModel()->index(sourceModel()->rowCount(sourceParent)-1,0,
				     sourceParent);
  else
    startIndex = sourceModel()->index(start, 0, sourceParent);
  
  for (ItemMap::iterator it = items.lower_bound(startIndex);
       it != items.end();) {
#ifndef WT_TARGET_JAVA
    ItemMap::iterator n = it;
    ++n;
#endif
    WModelIndex i = it->first;
    if (i == sourceParent)
      continue;

    if (i.isValid()) {
      WModelIndex p = i.parent();
      if (p != sourceParent && !WModelIndex::isAncestor(p, sourceParent))
	break;

      if (p == sourceParent) {
	shifted.push_back(it->second);
      } else if (count < 0) {
	// delete indexes that are about to be deleted, if they are within
	// the range of deleted indexes
	do {
	  if (p.parent() == sourceParent
	      && p.row() >= start
	      && p.row() < start - count) {
	    erased.push_back(it->second);
	    break;
	  } else
	    p = p.parent();
	} while (p != sourceParent);
      }
    }

#ifndef WT_TARGET_JAVA
    it = n;
#endif
  }

  for (unsigned i = 0; i < erased.size(); ++i) {
    items.erase(erased[i]->sourceIndex_);
    delete erased[i];
  }

  for (unsigned i = 0; i < shifted.size(); ++i) {
    BaseItem *item = shifted[i];
    items.erase(item->sourceIndex_);
    if (item->sourceIndex_.row() + count >= start) {
      item->sourceIndex_ = sourceModel()->index
	(item->sourceIndex_.row() + count,
	 item->sourceIndex_.column(),
	 sourceParent);
    } else {
      delete item;
      shifted[i] = 0;
    }
  }

  for (unsigned i = 0; i < shifted.size(); ++i) {
    if (shifted[i])
      items[shifted[i]->sourceIndex_] = shifted[i];
  }
}

#endif // DOXYGEN_ONLY

}
