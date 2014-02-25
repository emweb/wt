/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WEvent"
#include "Wt/WModelIndex"
#include "Wt/WAbstractItemModel"
#include "Wt/WItemSelectionModel"
#include "Wt/WLogger"

#include "WebUtils.h"

#ifdef WT_WIN32
#define snprintf _snprintf
#endif

namespace {
  const char *DRAG_DROP_MIME_TYPE = "application/x-wabstractitemmodelselection";
}

namespace Wt {

LOGGER("WAbstractItemModel");

WAbstractItemModel::WAbstractItemModel(WObject *parent)
  : WObject(parent),
    columnsAboutToBeInserted_(this),
    columnsAboutToBeRemoved_(this),
    columnsInserted_(this),
    columnsRemoved_(this),
    rowsAboutToBeInserted_(this),
    rowsAboutToBeRemoved_(this),
    rowsInserted_(this),
    rowsRemoved_(this),
    dataChanged_(this),
    headerDataChanged_(this),
    layoutAboutToBeChanged_(this),
    layoutChanged_(this),
    modelReset_(this)
{ }

WAbstractItemModel::~WAbstractItemModel()
{ }

bool WAbstractItemModel::canFetchMore(const WModelIndex& parent) const
{
  return false;
}

void WAbstractItemModel::fetchMore(const WModelIndex& parent)
{ }

WFlags<ItemFlag> WAbstractItemModel::flags(const WModelIndex& index) const
{
  return ItemIsSelectable;
}

WFlags<HeaderFlag> WAbstractItemModel::headerFlags(int section,
						   Orientation orientation)
  const
{
  return 0;
}

bool WAbstractItemModel::hasChildren(const WModelIndex& index) const
{
  return rowCount(index) > 0 && columnCount(index) > 0;
}

bool WAbstractItemModel::hasIndex(int row, int column,
				  const WModelIndex& parent) const
{
  return (row >= 0
	  && column >= 0
	  && row < rowCount(parent)
	  && column < columnCount(parent));
}

WAbstractItemModel::DataMap
WAbstractItemModel::itemData(const WModelIndex& index) const
{
  DataMap result;

  if (index.isValid()) {
    for (int i = 0; i <= BarBrushColorRole; ++i)
      result[i] = data(index, i);
    result[UserRole] = data(index, UserRole);
  }

  return result;
}

boost::any WAbstractItemModel::data(int row, int column, int role,
				    const WModelIndex& parent) const
{
  return data(index(row, column, parent), role);
}

boost::any WAbstractItemModel::headerData(int section, Orientation orientation,
					  int role) const
{
  if (role == LevelRole)
    return 0;
  else
    return boost::any();
}

void WAbstractItemModel::sort(int column, SortOrder order)
{ }

void WAbstractItemModel::expandColumn(int column)
{ }

void WAbstractItemModel::collapseColumn(int column)
{ }

bool WAbstractItemModel::insertColumns(int column, int count,
				       const WModelIndex& parent)
{
  return false;
}

bool WAbstractItemModel::insertRows(int row, int count,
				    const WModelIndex& parent)
{
  return false;
}

bool WAbstractItemModel::removeColumns(int column, int count,
				       const WModelIndex& parent)
{
  return false;
}

bool WAbstractItemModel::removeRows(int row, int count,
				    const WModelIndex& parent)
{
  return false;
}

bool WAbstractItemModel::setData(const WModelIndex& index,
				 const boost::any& value, int role)
{
  return false;
}

bool WAbstractItemModel::setHeaderData(int section, Orientation orientation,
				       const boost::any& value, int role)
{
  return false;
}

bool WAbstractItemModel::setHeaderData(int section, const boost::any& value)
{
  return setHeaderData(section, Horizontal, value);
}

bool WAbstractItemModel::setItemData(const WModelIndex& index,
				     const DataMap& values)
{
  bool result = true;

  bool wasBlocked = dataChanged().isBlocked();
  dataChanged().setBlocked(true);

  for (DataMap::const_iterator i = values.begin(); i != values.end(); ++i)
    // if (i->first != EditRole)
      if (!setData(index, i->second, i->first))
	result = false;

  dataChanged().setBlocked(wasBlocked);
  dataChanged().emit(index, index);

  return result;
}

bool WAbstractItemModel::insertColumn(int column, const WModelIndex& parent)
{
  return insertColumns(column, 1, parent);
}

bool WAbstractItemModel::insertRow(int row, const WModelIndex& parent)
{
  return insertRows(row, 1, parent);
}

bool WAbstractItemModel::removeColumn(int column, const WModelIndex& parent)
{
  return removeColumns(column, 1, parent);
}

bool WAbstractItemModel::removeRow(int row, const WModelIndex& parent)
{
  return removeRows(row, 1, parent);
}

bool WAbstractItemModel::setData(int row, int column, const boost::any& value,
				 int role, const WModelIndex& parent)
{
  WModelIndex i = index(row, column, parent);

  if (i.isValid())
    return setData(i, value, role);
  else
    return false;
}

void WAbstractItemModel::reset()
{
  modelReset_.emit();
}

WModelIndex WAbstractItemModel::createIndex(int row, int column, void *ptr)
  const
{
  return WModelIndex(row, column, this, ptr);
}

WModelIndex WAbstractItemModel::createIndex(int row, int column, ::uint64_t id)
  const
{
  return WModelIndex(row, column, this, id);
}

void *WAbstractItemModel::toRawIndex(const WModelIndex& index) const
{
  return 0;
}

WModelIndex WAbstractItemModel::fromRawIndex(void *rawIndex) const
{
  return WModelIndex();
}

std::string WAbstractItemModel::mimeType() const
{
  return DRAG_DROP_MIME_TYPE;
}

std::vector<std::string> WAbstractItemModel::acceptDropMimeTypes() const
{
  std::vector<std::string> result;

  result.push_back(DRAG_DROP_MIME_TYPE);

  return result;
}

void WAbstractItemModel::copyData(const WAbstractItemModel *source,
				  const WModelIndex& sIndex,
				  WAbstractItemModel *destination,
				  const WModelIndex& dIndex)
{
  DataMap values = destination->itemData(dIndex);
  for (DataMap::const_iterator i = values.begin(); i != values.end(); ++i)
    destination->setData(dIndex, boost::any(), i->first);
  
  destination->setItemData(dIndex, source->itemData(sIndex));
}

void WAbstractItemModel::dropEvent(const WDropEvent& e, DropAction action,
				   int row, int column,
				   const WModelIndex& parent)
{
  // TODO: For now, we assumes selectionBehavior() == RowSelection !

  WItemSelectionModel *selectionModel
    = dynamic_cast<WItemSelectionModel *>(e.source());
  if (selectionModel) {
    WAbstractItemModel *sourceModel = selectionModel->model();

    /*
     * (1) Insert new rows (or later: cells ?)
     */
    if (action == MoveAction || row == -1) {
      if (row == -1)
	row = rowCount(parent);
      
      if (!insertRows(row, selectionModel->selectedIndexes().size(), parent)) {
	LOG_ERROR("dropEvent(): could not insertRows()");
	return;
      }
    }

    /*
     * (2) Copy data
     */
    WModelIndexSet selection = selectionModel->selectedIndexes();

    int r = row;
    for (WModelIndexSet::const_iterator i = selection.begin();
	 i != selection.end(); ++i) {
      WModelIndex sourceIndex = *i;
      if (selectionModel->selectionBehavior() == SelectRows) {
	WModelIndex sourceParent = sourceIndex.parent();

	for (int col = 0; col < sourceModel->columnCount(sourceParent); ++col) {
	  WModelIndex s = sourceModel->index(sourceIndex.row(), col,
					     sourceParent);
	  WModelIndex d = index(r, col, parent);
	  copyData(sourceModel, s, this, d);
	}

	++r;
      } else {
	  
      }
    }

    /*
     * (3) Remove original data
     */
    if (action == MoveAction) {
      while (!selectionModel->selectedIndexes().empty()) {
	WModelIndex i = Utils::last(selectionModel->selectedIndexes());

	if (!sourceModel->removeRow(i.row(), i.parent())) {
	  LOG_ERROR("dropEvent(): could not removeRows()");
	  return;
	}
      }
    }
  }
}

void WAbstractItemModel::beginInsertColumns(const WModelIndex& parent, 
					    int first, int last)
{
  first_ = first;
  last_ = last;
  parent_ = parent;

  columnsAboutToBeInserted().emit(parent_, first, last);
}

void WAbstractItemModel::endInsertColumns()
{
  columnsInserted().emit(parent_, first_, last_);
}

void WAbstractItemModel::beginInsertRows(const WModelIndex& parent,
					 int first, int last)
{
  first_ = first;
  last_ = last;
  parent_ = parent;

  rowsAboutToBeInserted().emit(parent, first, last);
}

void WAbstractItemModel::endInsertRows()
{
  rowsInserted().emit(parent_, first_, last_);
}

void WAbstractItemModel::beginRemoveColumns(const WModelIndex& parent,
					    int first, int last)
{
  first_ = first;
  last_ = last;
  parent_ = parent;

  columnsAboutToBeRemoved().emit(parent, first, last);
}

void WAbstractItemModel::endRemoveColumns()
{
  columnsRemoved().emit(parent_, first_, last_);
}

void WAbstractItemModel::beginRemoveRows(const WModelIndex& parent,
					 int first, int last)
{
  first_ = first;
  last_ = last;
  parent_ = parent;

  rowsAboutToBeRemoved().emit(parent, first, last);
}

void WAbstractItemModel::endRemoveRows()
{
  rowsRemoved().emit(parent_, first_, last_);
}

WModelIndexList WAbstractItemModel::match(const WModelIndex& start,
					  int role,
					  const boost::any& value,
					  int hits,
					  WFlags<MatchFlag> flags)
  const
{
  WModelIndexList result;

  const int rc = rowCount(start.parent());

  for (int i = 0; i < rc; ++i) {
    int row = start.row() + i;

    if (row >= rc) {
      if (!(flags & MatchWrap))
	break;
      else
	row -= rc;
    }

    WModelIndex idx = index(row, start.column(), start.parent());
    boost::any v = data(idx, role);

    if (Impl::matchValue(v, value, flags)) {
      result.push_back(idx);
      if (hits != -1 && (int)result.size() == hits)
	break;
    }
  }

  return result;
}

}
