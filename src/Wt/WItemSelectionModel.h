// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WITEM_SELECTION_MODEL_H_
#define WITEM_SELECTION_MODEL_H_

#include <Wt/WObject.h>
#include <Wt/WModelIndex.h>
#include <Wt/WGlobal.h>

#include <string>

namespace Wt {

class WAbstractItemModel;
class WAbstractItemView;

/*! \class WItemSelectionModel Wt/WItemSelectionModel.h Wt/WItemSelectionModel.h
 *  \brief A class that represents a selection for a WAbstractItemView.
 *
 * This model is currently only used by WTreeView, and plays only
 * a role in drag & drop of an item selection.
 *
 * When an item selection is dragged from a view widget, the generated
 * drop events will have as source object (see WDropEvent::source())
 * this selection model.
 *
 * Although this class does not (yet) allow you to modify the
 * selection, note that manipulations to the model may modify the
 * selection (row insertions and removals may shift the selection, and
 * row deletions may shrink the selection).
 *
 * \note Currently this class cannot be shared between multiple views.
 *
 * \ingroup modelview
 *
 * \sa WTreeView, WTableView, WAbstractItemModel
 */
class WT_API WItemSelectionModel : public WObject
{
public:
  /*! \brief Returns the WAbstractItemModel.
   */
  std::shared_ptr<WAbstractItemModel> model() const { return model_; }

  /*! \brief Returns the set of selected items.
   *
   * The model indexes are returned as a set, topologically ordered (in
   * the order they appear in the view).
   *
   * When selection operates on rows (\link Wt::SelectionBehavior::Rows SelectionBehavior::Rows\endlink),
   * this method only returns the model index of first column's element of the 
   * selected rows.
   */
  WModelIndexSet selectedIndexes() const { return selection_; }

  /*! \brief Returns wheter an item is selected.
   *
   * When selection operates on rows (\link Wt::SelectionBehavior::Rows SelectionBehavior::Rows\endlink),
   * this method returns true for each element in a selected row.
   *
   * \sa selectedIndexes()
   */
  bool isSelected(const WModelIndex& index) const;

  /*! \brief Sets the selection behaviour.
   *
   * By default, the selection contains rows (\link Wt::SelectionBehavior::Rows
   * SelectionBehavior::Rows\endlink), in which case model indexes will always be
   * have column 0, but represent the whole row.
   *
   * Alternatively, you can allow selection for individual items
   * (\link Wt::SelectionBehavior::Items SelectionBehavior::Items\endlink).
   */
  void setSelectionBehavior(SelectionBehavior behavior);

  /*! \brief Returns the selection behaviour.
   *
   * \sa setSelectionBehavior()
   */
  SelectionBehavior selectionBehavior() const { return selectionBehavior_; }

  /*! \brief Returns the selection mime type.
   *
   * This should return the mime type for the current selection, or an emtpy
   * string if the selection cannot be dragged.
   *
   * The default implementation returns the mime type based on ItemDataRole::MimeType data
   * if all selected items indicate the same mime type, or the model mimeType()
   * otherwise.
   *
   * If one or more items indicate that they cannot be dragged, then an empty
   * string is returned.
   */
  virtual std::string mimeType();

private:
  WModelIndexSet selection_;
  std::shared_ptr<WAbstractItemModel> model_;
  SelectionBehavior selectionBehavior_;

  WItemSelectionModel();
  WItemSelectionModel(const std::shared_ptr<WAbstractItemModel>& model);

  void modelLayoutAboutToBeChanged();
  void modelLayoutChanged();

  friend class WAbstractItemView;
  friend class WTableView;
  friend class WTreeView;
};

}

#endif // WITEM_SELECTION_MODEL_H_
