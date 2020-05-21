/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WEvent.h"
#include "Wt/WModelIndex.h"
#include "Wt/WAbstractItemModel.h"
#include "Wt/WItemSelectionModel.h"
#include "Wt/WLogger.h"

#include "WebUtils.h"

#ifdef WT_WIN32
#define snprintf _snprintf
#endif

namespace {
  const char *DRAG_DROP_MIME_TYPE = "application/x-wabstractitemmodelselection";
}

namespace Wt {

LOGGER("WAbstractItemModel");

WAbstractItemModel::WAbstractItemModel()
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
  return ItemFlag::Selectable;
}

WFlags<HeaderFlag> WAbstractItemModel::headerFlags(int section,
						   Orientation orientation)
  const
{
  return None;
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
#ifndef WT_TARGET_JAVA
    for (int i = 0; i <= ItemDataRole::BarBrushColor; ++i)
#else
    for (int i = 0; i <= ItemDataRole::BarBrushColor.value(); ++i)
#endif
      result[ItemDataRole(i)] = data(index, ItemDataRole(i));
    result[ItemDataRole::User] = data(index, ItemDataRole::User);
  }

  return result;
}

cpp17::any WAbstractItemModel::data(int row, int column, ItemDataRole role,
				 const WModelIndex& parent) const
{
  return data(index(row, column, parent), role);
}

cpp17::any WAbstractItemModel::headerData(int section, Orientation orientation,
                                       ItemDataRole role) const
{
  if (role == ItemDataRole::Level)
    return cpp17::any((int)0);
  else
    return cpp17::any();
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
                                 const cpp17::any& value, ItemDataRole role)
{
  return false;
}

bool WAbstractItemModel::setHeaderData(int section, Orientation orientation,
                                       const cpp17::any& value, ItemDataRole role)
{
  return false;
}

bool WAbstractItemModel::setHeaderData(int section, const cpp17::any& value)
{
  return setHeaderData(section, Orientation::Horizontal, value);
}

bool WAbstractItemModel::setItemData(const WModelIndex& index,
				     const DataMap& values)
{
  bool result = true;

  for (DataMap::const_iterator i = values.begin(); i != values.end(); ++i)
    // if (i->first != ItemDataRole::Edit)
      if (!setData(index, i->second, i->first))
	result = false;

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

bool WAbstractItemModel::setData(int row, int column, const cpp17::any& value,
                                 ItemDataRole role, const WModelIndex& parent)
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
  return nullptr;
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
    destination->setData(dIndex, cpp17::any(), i->first);
  
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
    auto sourceModel = selectionModel->model();

    /*
     * (1) Insert new rows (or later: cells ?)
     */
    if (action == DropAction::Move || row == -1) {
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
      if (selectionModel->selectionBehavior() ==
	  SelectionBehavior::Rows) {
	WModelIndex sourceParent = sourceIndex.parent();

	for (int col = 0; col < sourceModel->columnCount(sourceParent); ++col) {
	  WModelIndex s = sourceModel->index(sourceIndex.row(), col,
					     sourceParent);
	  WModelIndex d = index(r, col, parent);
	  copyData(sourceModel.get(), s, this, d);
	}

	++r;
      } else {
	  
      }
    }

    /*
     * (3) Remove original data
     */
    if (action == DropAction::Move) {
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
                                          ItemDataRole role,
					  const cpp17::any& value,
					  int hits,
					  WFlags<MatchFlag> flags)
  const
{
  WModelIndexList result;

  const int rc = rowCount(start.parent());

  for (int i = 0; i < rc; ++i) {
    int row = start.row() + i;

    if (row >= rc) {
      if (!(flags & MatchFlag::Wrap))
	break;
      else
	row -= rc;
    }

    WModelIndex idx = index(row, start.column(), start.parent());
    cpp17::any v = data(idx, role);

    if (Impl::matchValue(v, value, flags)) {
      result.push_back(idx);
      if (hits != -1 && (int)result.size() == hits)
	break;
    }
  }

  return result;
}

WAbstractItemModel::column_iterator::column_iterator(value_type idx, int column) : model_(idx.model()), start_node_(idx), current_node_(idx), last_node_(idx), column_(column)
{
}

WAbstractItemModel::column_iterator &WAbstractItemModel::column_iterator::operator++()
{
  if( !current_node_.isValid() )
    return  *this;

  if( current_node_ == last_node_ && model_->rowCount(current_node_) == 0 )
    return end();

  // A flag indicating that the index is visited for the first time
  bool is_new = false;

  do {
    // Forward direction
    if ( model_->parent(current_node_) == last_node_ || /*Initial condition --> */ current_node_ == last_node_ ){
      // Node is not a leaf. Go forward to the first child
      if ( model_->rowCount(current_node_) ){
        last_node_ = current_node_;
        // current_node_ becomes it's first child
        current_node_ = model_->index(0, column_, current_node_);
        is_new        = true;
      }
      // Reached a leaf. Go in backward direction one level up
      else {
        last_node_    = current_node_;
        current_node_ = model_->parent(current_node_);
      }
    }
    // Backward direction
    else if ( model_->parent(last_node_) == current_node_ ){
        // Has more unvisited siblings. Go to next sibling
        if ( last_node_.row() + 1 < model_->rowCount(current_node_) ){
          // current_node_ becomes the next sibling
          current_node_ = model_->index(last_node_.row() + 1, column_, current_node_);
          last_node_    = model_->parent(last_node_);
          is_new        = true;
        }
        // Doesn't have more siblings. Go in backward direction one level up
        else {
          // When current_node_ != start_node_ , can still go backward
          if ( current_node_ != start_node_ ){
            last_node_    = current_node_;
            current_node_ = model_->parent(current_node_);
          }
          // Can not go backward. Set the end condition
          else
            return end();
        }
      }

      /* Two stop conditions: 1. End condition; 2. Reached an unvisited node
       * End condition is as soon as current_node_ becomes invalid
       * The traversing will continue as long as current_node_ is valid */
  } while ( current_node_.isValid() && !is_new );

  return *this;
}

WAbstractItemModel::column_iterator WAbstractItemModel::column_iterator::operator++(int)
{
  column_iterator retval = *this;
  ++(*this);
  return retval;
}

WAbstractItemModel::column_iterator::reference WAbstractItemModel::column_iterator::operator*() const
{
  if(!current_node_.isValid())
    return start_node_;

  return current_node_;
}

WAbstractItemModel::column_iterator::pointer WAbstractItemModel::column_iterator::operator->() const
{
  if(!current_node_.isValid())
      return nullptr;

  return &current_node_;
}

bool WAbstractItemModel::column_iterator::operator==(const WAbstractItemModel::column_iterator &other) const
{
  return (model_ == other.model_) &&
         (current_node_ == other.current_node_);
}

bool WAbstractItemModel::column_iterator::operator!=(const WAbstractItemModel::column_iterator &other) const
{
  return !(*this == other);
}

WAbstractItemModel::column_iterator &WAbstractItemModel::column_iterator::begin()
{
  current_node_ = start_node_;
  last_node_    = start_node_;
  model_        = start_node_.model();
  return *this;
}

WAbstractItemModel::column_iterator &WAbstractItemModel::column_iterator::end()
{
  last_node_    = value_type();
  current_node_ = value_type();
  model_        = nullptr;
  return *this;
}

}
