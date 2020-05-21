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

WAbstractItemModel::column_iterator::column_iterator(value_type idx, int column) : start_node_(idx), column_(column)
{
  begin();
}

WAbstractItemModel::column_iterator &WAbstractItemModel::column_iterator::operator++()
{
    /*
     * Algorithm description
     *
     * The algorithm traverses the tree structure of a selected index.
     * The column used for traversing is set in the constructor.
     *
     * With each increment, the iterator points to the next unvisited node
     * in the tree. For it's navigation around the tree structure, the algorithm uses
     * two variables: lead_node_ and follow_node_. The lead node is the current node at
     * which the itterator is pointing at. The follow node as the name suggest, allways
     * lag behing one step.
     * The initial condition is when the lead and the
     * follow node are equal, which are both set to the start node in the constructor.
     *
     *     DO {
     * (1)   IF(following node is one level up than lead node or the initial condition is met)
     *       {
     *         IF(lead node has children)
     *         {
     *           Go forward. Follow node is set to the lead node and lead node
     *           is set to it's first child. This will go on until a leaf is
     *           reached.
     *           This condition is also a sign that the node has not been
     *           visited before, so mark it as unvisited. This will break the while
     *           cycle and the itterator will point at the lead node.
     *         }
     *         ELSE(lead node is a leaf)
     *         {
     *           Go backwards. Set the lead node one level up and the follow node one
     *           level below, by switching them. The next iteration will fall in (2)
     *         }
     *       }
     * (2)   ELSE IF(follow node is one level down than lead node)
     *       {
     *         IF(lead node has more unvisited children)
     *         {
     *           Lead node is set to the next child and the follow node is set to
     *           the parent of that child(which is one level up). This condition is
     *           also a sign that the node has not been visited before, so mark it as
     *           unvisited. This will break the while cycle and the itterator will
     *           point at the lead node. Next iteration will fall in (1)
     *         }
     *         ELSE(the lead node has no more children to visit)
     *         {
     *           There is notihng more to traverse here. The only way is backwards.
     *
     *           IF(start node has not been reached)
     *           {
     *             We can go backward one step. Set the lead node one level up and
     *             the follow node one level below it. Next iteration will fall in (2)
     *           }
     *           ELSE(reached the start node)
     *           {
     *             Do not traverse beyond the start node. Set the iterator past the end
     *             and return from the method
     *           }
     *         }
     *       }
     *     }
     *     while(until a unvisited node is reached)
     */

  if( pastTheEnd() )
    return  *this;

  // If starting from a leaf node, set the past the end condition
  if( lead_node_ == follow_node_ && model_->rowCount(lead_node_) == 0 )
    return end();

  /*
   * A flag indicating that the node is visited for the first time
   * There are two situations when the node is considered as unvisited:
   * 1. When the iterator is going to the first child of the lead node
   * 2. When the iterator is going to the next child of the lead node
   */
  bool visited = true;

  // Traverse until a unvisited node is reached or past the end condition is met
  do {
    // Forward direction
    if ( model_->parent(lead_node_) == follow_node_ || lead_node_ == follow_node_ /* <-- Initial condition */) {
      // Node is not a leaf. Go forward to the first child
      if ( model_->rowCount(lead_node_) ) {
        follow_node_ = lead_node_;
        lead_node_   = model_->index(0, column_, lead_node_);  // lead_node_ becomes it's first child
        visited      = false;
      }
      // Reached a leaf. Go in backward direction one level up
      else
        std::swap(lead_node_, follow_node_);
    }
    // Backward direction
    else if ( model_->parent(follow_node_) == lead_node_ ) {
      // Has more unvisited children. Go to next child
      if ( follow_node_.row() + 1 < model_->rowCount(lead_node_) ) {
        lead_node_   = model_->index(follow_node_.row() + 1, column_, lead_node_);  // lead_node_ becomes the next child
        follow_node_ = model_->parent(follow_node_);
        visited      = false;
      }
      // Doesn't have more children. Go in backward direction one level up
      else {
        // When lead_node_ != start_node_ , can still go backward
        if ( lead_node_ != start_node_ ) {
          follow_node_ = lead_node_;
          lead_node_   = model_->parent(lead_node_);
        }
        // End of traversal. Can not go backward. Set past the end condition
        else
          return end();
      }
    }

  // The traversing will stop when a unvisited node is reached
  } while ( visited );

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
  if( pastTheEnd() )
    return start_node_;

  return lead_node_;
}

WAbstractItemModel::column_iterator::pointer WAbstractItemModel::column_iterator::operator->() const
{
  if( pastTheEnd() )
    return nullptr;

  return &lead_node_;
}

bool WAbstractItemModel::column_iterator::operator==(const WAbstractItemModel::column_iterator &other) const
{
  // An equality is considered true when both indexes are equal
  return lead_node_ == other.lead_node_;
}

bool WAbstractItemModel::column_iterator::operator!=(const WAbstractItemModel::column_iterator &other) const
{
  return !(*this == other);
}

WAbstractItemModel::column_iterator &WAbstractItemModel::column_iterator::begin()
{
  /*
   * Reset the iterator to point to the start node
   *
   * Begin condition is when all the members are equal to the start_node_
   */
  lead_node_   = start_node_;
  follow_node_ = start_node_;
  model_       = start_node_.model();
  return *this;
}

WAbstractItemModel::column_iterator &WAbstractItemModel::column_iterator::end()
{
  /*
   * Set to past the end condition
   *
   * Past the end condition is when every member is zeroed, except
   * the start_node_
   */
  follow_node_ = value_type();
  lead_node_   = value_type();
  model_       = nullptr;
  return *this;
}

bool WAbstractItemModel::column_iterator::pastTheEnd() const
{
  auto empty_node = value_type();
  return (follow_node_ == empty_node) && (follow_node_ == empty_node) && !model_;
}

}
