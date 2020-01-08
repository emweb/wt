/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractProxyModel.h"

namespace Wt {

#ifndef DOXYGEN_ONLY

WAbstractProxyModel::BaseItem::~BaseItem()
{ }

WAbstractProxyModel::WAbstractProxyModel()
  : sourceModel_(nullptr)
{ }

void WAbstractProxyModel
::setSourceModel(const std::shared_ptr<WAbstractItemModel>& sourceModel)
{
  sourceModel_ = sourceModel;
}

cpp17::any WAbstractProxyModel::data(const WModelIndex& index, ItemDataRole role) const
{
  return sourceModel_->data(mapToSource(index), role);
}

bool WAbstractProxyModel::setData(const WModelIndex& index,
                                  const cpp17::any& value, ItemDataRole role)
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

WFlags<HeaderFlag> WAbstractProxyModel::headerFlags(int section,
						    Orientation orientation) const
{
  if (orientation == Wt::Orientation::Horizontal) {
    section = mapToSource(index(0, section, Wt::WModelIndex())).column();
  } else {
    section = mapToSource(index(section, 0, Wt::WModelIndex())).row();
  }
  return sourceModel_->headerFlags(section, orientation);
}

cpp17::any WAbstractProxyModel::headerData(int section, Orientation orientation,
                                        ItemDataRole role) const
{
  if (orientation == Wt::Orientation::Horizontal) {
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

void WAbstractProxyModel::startShiftModelIndexes(const WModelIndex& sourceParent,
						 int start, int count,
						 ItemMap& items)
{
  /*
   * We must shift all indexes within sourceParent >= start with count
   * and delete items when count < 0.
   */
  std::vector<BaseItem *> erased;

  WModelIndex startIndex;
  if (sourceModel()->rowCount(sourceParent) == 0)
    startIndex = sourceParent;
  else if (start >= sourceModel()->rowCount(sourceParent))
    return;
  else
    startIndex = sourceModel()->index(start, 0, sourceParent);

#ifdef WT_TARGET_JAVA
  if (!startIndex.isValid())
    return;
#endif
  
  for (ItemMap::iterator it = items.lower_bound(startIndex);
       it != items.end();) {
#ifndef WT_TARGET_JAVA
    ItemMap::iterator n = it;
    ++n;
#endif
    WModelIndex i = it->first;
    if (i == sourceParent) {
#ifndef WT_TARGET_JAVA
      it = n;
#endif      
      continue;
    }

    if (i.isValid()) {
      WModelIndex p = i.parent();
      if (p != sourceParent && !WModelIndex::isAncestor(p, sourceParent))
	break;

      if (p == sourceParent) { /* Child of source parent: shift or remove */
	if (count < 0 &&
	    i.row() >= start &&
	    i.row() < start + (-count))
	  erased.push_back(it->second);
	else {
	  itemsToShift_.push_back(it->second);
	}
      } else if (count < 0) { /* Other descendent: remove if necessary */
	// delete indexes that are about to be deleted, if they are within
	// the range of deleted indexes
	do {
	  if (p.parent() == sourceParent
	      && p.row() >= start
	      && p.row() < start + (-count)) {
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

  for (unsigned i = 0; i < itemsToShift_.size(); ++i) {
    BaseItem *item = itemsToShift_[i];
    items.erase(item->sourceIndex_);
    item->sourceIndex_ = sourceModel()->index
      (item->sourceIndex_.row() + count,
       item->sourceIndex_.column(),
       sourceParent);
  }

  for (unsigned i = 0; i < erased.size(); ++i) {
    items.erase(erased[i]->sourceIndex_);
    delete erased[i];
  }

  if (count > 0)
    endShiftModelIndexes(sourceParent, start, count, items);
}
 
void WAbstractProxyModel::endShiftModelIndexes(const WModelIndex& sourceParent,
					       int start, int count,
					       ItemMap& items)
{
  for (unsigned i = 0; i < itemsToShift_.size(); ++i)
    items[itemsToShift_[i]->sourceIndex_] = itemsToShift_[i];

  itemsToShift_.clear();
}


#endif // DOXYGEN_ONLY

}
