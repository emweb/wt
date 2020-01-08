// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WABSTRACTITEMVIEW_H_
#define WT_WABSTRACTITEMVIEW_H_

#include <Wt/WCompositeWidget.h>
#include <Wt/WModelIndex.h>
#include <Wt/WItemSelectionModel.h>
#include <Wt/WJavaScriptSlot.h>
#include <Wt/WValidator.h>

namespace Wt {

/*! \brief Enumeration that specifies the user action that triggers editing.
 *
 * \sa setEditTriggers(), edit()
 */
enum class EditTrigger {
  None = 0x0,  //!< Do not allow user to initiate editing
  SingleClicked = 0x1,  //!< Edit an item when clicked
  DoubleClicked = 0x2,  //!< Edit an item when double clicked
  SelectedClicked = 0x4 //!< Edit a selected item that is clicked again
};

/*! \brief Enumeration that specifies editing options.
 *
 * \sa setEditOptions()
 */
enum class EditOption {
  SingleEditor = 0x1,    //!< Never show more than one active editor
  MultipleEditors = 0x2, //!< Allow multiple editors at the same time
  SaveWhenClosed = 0x4,  //!< Always save the current edit value when closing
  LeaveEditorsOpen = 0x8 //!< Editors can only be closed using closeEditor()
};

/*! \brief Enumeration that specifies a scrolling option.
 *
 * \sa scrollTo()
 */
enum class ScrollHint {
  EnsureVisible,         //!< Scrolls minimally to make it visible
  PositionAtTop,         //!< Positions the item at the top of the viewport
  PositionAtBottom,      //!< Positions the item at the bottom of the viewport
  PositionAtCenter       //!< Positions the item at the center of the viewport
};

  class WAbstractItemDelegate;
  class WAbstractItemModel;
  class WApplication;
  class WCssTemplateRule;

/*! \class WAbstractItemView Wt/WAbstractItemView.h Wt/WAbstractItemView.h
 *  \brief An abstract base class for item Views.
 *
 * See WTableView or WTreeView for a description.
 *
 *
 * <h3>i18n</h3>
 *
 * The strings used in this class can be translated by overriding
 * the default values for the following localization keys:
 * - Wt.WAbstractItemView.PageIOfN: <b>{1}</b> of <b>{2}</b>
 * - Wt.WAbstractItemView.PageBar.First: &\#xc2ab; First
 * - Wt.WAbstractItemView.PageBar.Previous: &\#xe280b9; Previous
 * - Wt.WAbstractItemView.PageBar.Next: Next &\#xe280ba;
 * - Wt.WAbstractItemView.PageBar.Last: Last &\#xc2bb;
 *
 * \ingroup modelview
 */
class WT_API WAbstractItemView : public WCompositeWidget
{
public:
  virtual ~WAbstractItemView() override;

  /*! \brief Sets the model.
   *
   * The View will display data of the given \p model and changes in
   * the model are reflected by the View.
   *
   * The initial model is \c 0.
   *
   * \sa setRootIndex()
   */
  virtual void setModel(const std::shared_ptr<WAbstractItemModel>& model);

  /*! \brief Returns the model.
   *
   * \sa setModel()
   */
  std::shared_ptr<WAbstractItemModel> model() const { return model_; }

  /*! \brief Sets the root index.
   *
   * The root index is the model index that is considered the root
   * node. This node itself is not rendered, but its children are.
   *
   * \if cpp
   * The default value is an invalid model index, corresponding to the model's
   * root node.
   * \endif
   * \if java
   * The default value is \c null, corresponding to the model's root node.
   * \endif
   *
   * \sa setModel()
   */
  void setRootIndex(const WModelIndex& rootIndex);

  /*! \brief Returns the root index.
   *
   * \sa setRootIndex()
   */
  const WModelIndex& rootIndex() const { return rootIndex_; }

  /*! \brief Sets the default item delegate.
   *
   * The previous delegate is not deleted. This item delegate is for
   * all columns for which no specific item delegate is set.
   *
   * The default item delegate is a WItemDelegate.
   *
   * \sa setItemDelegateForColumn()
   */
  void setItemDelegate(const std::shared_ptr<WAbstractItemDelegate>& delegate);

  /*! \brief Returns the default item delegate.
   *
   * \sa setItemDelegate()
   */
  std::shared_ptr<WAbstractItemDelegate> itemDelegate() const {
    return itemDelegate_;
  }

  /*! \brief Sets the delegate for a column.
   *
   * A delegate previously set (if any) is not deleted.
   *
   * \sa setItemDelegate()
   */
  void setItemDelegateForColumn
    (int column, const std::shared_ptr<WAbstractItemDelegate>& delegate);
  
  /*! \brief Returns the delegate that was set for a column.
   *
   * Returns \c 0 if no delegate was set for the column.
   *
   * \sa setItemDelegateForColumn()
   */
  std::shared_ptr<WAbstractItemDelegate> itemDelegateForColumn(int column)
    const;

  /*! \brief Returns the delegate for rendering an item.
   *
   * \sa setItemDelegateForColumn(), setItemDelegate()
   */
  std::shared_ptr<WAbstractItemDelegate> itemDelegate(const WModelIndex& index)
    const;

  /*! \brief Returns the delegate for a column.
   *
   * Returns either the delegate that was set for the column, or the default
   * delegate.
   */
  std::shared_ptr<WAbstractItemDelegate> itemDelegate(int column) const;

  /*! \brief Returns the widget that renders an item.
   *
   * This returns the widget that renders the given item. This may return 0
   * if the item is currently not rendered.
   *
   * This widget has been created by an item delegate, and usually an item
   * delegate is involved when updating it.
   */
  virtual WWidget *itemWidget(const WModelIndex& index) const = 0;

  /*! \brief Sets the header item delegate.
   *
   * This item delegate is used for rendering items in the header.
   *
   * The previous delegate is not deleted. This item delegate is for
   * all columns for which no specific item delegate is set.
   *
   * The default item delegate is a WItemDelegate.
   */
  void setHeaderItemDelegate(const std::shared_ptr<WAbstractItemDelegate>&
			     delegate);

  /*! \brief Returns the header item delegate.
   *
   * \sa setHeaderItemDelegate()
   */
  std::shared_ptr<WAbstractItemDelegate> headerItemDelegate() const;

  /*! \brief Sets the content alignment for a column.
   *
   * The default value is Wt::AlignmentFlag::Left.
   *
   * \sa setHeaderAlignment()
   */
  virtual void setColumnAlignment(int column, AlignmentFlag alignment);

  /*! \brief Returns the content alignment for a column.
   *
   * \sa setColumnAlignment()
   */
  virtual AlignmentFlag columnAlignment(int column) const;

  /*! \brief Sets the header alignment for a column.
   *
   * The default alignemnt is horizontally left, and vertically centered.
   * (Wt::AlignmentFlag::Left | Wt::AlignmentFlag::Middle).
   *
   * Valid options for horizontal alignment are Wt::AlignmentFlag::Left,
   * Wt::AlignmentFlag::Center or Wt::AlignmentFlag::Right.
   *
   * Valid options for vertical alignment are Wt::AlignmentFlag::Middle or
   * Wt::AlignmentFlag::Top. In the latter case, other contents may be added
   * below the label in createExtraHeaderWidget().
   *
   * \sa setColumnAlignment()
   */
  virtual void setHeaderAlignment(int column, WFlags<AlignmentFlag> alignment);

  /*! \brief Returns the horizontal header alignment for a column.
   *
   * \sa setHeaderAlignment()
   */
  AlignmentFlag horizontalHeaderAlignment(int column) const;

  /*! \brief Returns the vertical header alignment for a column.
   *
   * \sa setHeaderAlignment()
   */
  AlignmentFlag verticalHeaderAlignment(int column) const;

  /*! \brief Configures header text wrapping.
   *
   * This setting only affects a multiline header, and the default
   * value is \c true. When set to \c false, the header itself will not
   * wrap (as with a vertically centered header), and thus extra widgets
   * will not shift down when there is a long header label.
   */
  void setHeaderWordWrap(int column, bool enabled);

  bool headerWordWrap(int column) const;

  /*! \brief Sets if alternating row colors are to be used.
   *
   * Configure whether rows get alternating background colors, defined by the
   * current CSS theme.
   *
   * The default value is \c false.
   */
  virtual void setAlternatingRowColors(bool enable);

  /*! \brief Returns whether alternating row colors are used.
   *
   * When enabled, rows are displayed in alternating row colors, according
   * to the current theme's definition.
   *
   * \sa setAlternatingRowColors()
   */
  virtual bool alternatingRowColors() const { return alternatingRowColors_; }
  
  /*! \brief Sorts the data according to a column.
   *
   * Sorts the data according to data in column \p column and sort
   * order \p order.
   *
   * The default sorting column is -1: the model is unsorted.
   *
   * \sa WAbstractItemModel::sort()
   */
  void sortByColumn(int column, SortOrder order);

  /*! \brief Returns the current sorting columm.
   *
   * \sa sortByColumn(), sortOrder()
   */
  int sortColumn() const;

  /*! \brief Returns the current sorting order.
   *
   * \sa sortByColumn(), sortColumn()
   */
  SortOrder sortOrder() const;

  /*! \brief Enables or disables sorting for all columns.
   *
   * Enable or disable sorting by the user on all columns.
   *
   * Sorting is enabled by default.
   *
   * \sa WAbstractItemModel::sort()
   */
  void setSortingEnabled(bool enabled);

  /*! \brief Enables or disables sorting for a single column.
   *
   * Enable or disable sorting by the user for a specific column.
   *
   * Sorting is enabled by default.
   *
   * \sa WAbstractItemModel::sort()
   */
  void setSortingEnabled(int column, bool enabled);

  /*! \brief Returns whether sorting is enabled.
   *
   * \sa setSortingEnabled()
   */
  bool isSortingEnabled() const { return sorting_; }

  /*! \brief Returns whether sorting is enabled for a single column.
   *
   * \sa setSortingEnabled()
   */
  bool isSortingEnabled(int column) const;

  /*! \brief Enables interactive column resizing
   *
   * Enable or disable column resize handles for interactive resizing of
   * the columns.
   *
   * Column resizing is enabled by default when JavaScript is available.
   *
   * \sa setColumnResizeEnabled()
   */
  void setColumnResizeEnabled(bool enabled);

  /*! \brief Returns whether column resizing is enabled.
   *
   * \sa setColumnResizeEnabled()
   */
  bool isColumnResizeEnabled() const { return columnResize_; }

  /*! \brief Changes the selection behaviour.
   *
   * The selection behavior indicates whether whole rows or individual
   * items can be selected. It is a property of the selectionModel().
   *
   * By default, selection operates on rows (SelectionBehavior::Rows),
   * in which case model indexes will always be
   * in the first column (column \c 0).
   *
   * Alternatively, you can allow selection for individual items
   * (SelectionBehavior::Items)
   *
   * \sa WItemSelectionModel::setSelectionBehavior(), setSelectionMode()
   */
  void setSelectionBehavior(SelectionBehavior behavior);

  /*! \brief Returns the selection behaviour.
   *
   * \sa setSelectionBehavior()
   */
  SelectionBehavior selectionBehavior() const;

  /*! \brief Sets the selection mode.
   *
   * By default selection is disabled (SelectionMode::None).
   *
   * \sa setSelectionBehavior()
   */
  void setSelectionMode(SelectionMode mode);

  /*! \brief Returns the selection mode.
   *
   * \sa setSelectionMode()
   */
  SelectionMode selectionMode() const { return selectionMode_; }

  /*! \brief Returns the selection model.
   *
   * The selection model keeps track of the currently selected items.
   */
  WItemSelectionModel *selectionModel() const { return selectionModel_.get(); }

  /*! \brief Sets the selected items
   *
   * Replaces the current selection with \p indexes.
   * 
   * When selection operates on rows (SelectionBehavior::Rows), 
   * it is sufficient to pass the first element in a row (column \c 0 )
   * to select the entire row.
   *
   * \sa select(), selectionModel()
   */
  void setSelectedIndexes(const WModelIndexSet& indexes);

  /*! \brief Clears the selection.
   *
   * \sa setSelectedIndexes()
   */
  void clearSelection();

  /*! \brief Selects a single item.
   *
   * \sa setSelectedIndexes(), selectionModel()
   */
  void select(const WModelIndex& index,
	      SelectionFlag option = SelectionFlag::Select);

  /*! \brief Returns wheter an item is selected.
   *
   * When selection operates on rows (SelectionBehavior::Rows),
   * this method returns true for each element in a selected row.
   *
   * This is a convenience method for:
   * \code
   * selectionModel()->isSelected(index)
   * \endcode
   *
   * \sa selectedIndexes(), select(), selectionModel()
   */
  bool isSelected(const WModelIndex& index) const;

  /*! \brief Returns the set of selected items.
   *
   * The model indexes are returned as a set, topologically ordered (in
   * the order they appear in the view).
   *
   * When selection operates on rows (SelectionBehavior::Rows),
   * this method only returns the model index of first column's element of the 
   * selected rows.
   *
   * This is a convenience method for:
   * \code
   * selectionModel()->selectedIndexes()
   * \endcode
   *
   * \sa setSelectedIndexes()
   */
  WModelIndexSet selectedIndexes() const;
  
  /*! \brief Enables the selection to be dragged (drag & drop).
   *
   * To enable dragging of the selection, you first need to enable
   * selection using setSelectionMode().
   *
   * Whether an individual item may be dragged is controlled by the
   * item's ItemFlag::DragEnabled flag. The selection can be dragged
   * only if all items currently selected can be dragged.
   *
   * \sa setDropsEnabled() 
   */
  void setDragEnabled(bool enable);

  /*! \brief Enables drop operations (drag & drop).
   *
   * When drop is enabled, the tree view will indicate that something
   * may be dropped when the mime-type of the dragged object is
   * compatible with one of the model's accepted drop mime-types (see
   * WAbstractItemModel::acceptDropMimeTypes()) or this widget's
   * accepted drop mime-types (see WWidget::acceptDrops()), and the
   * target item has drop enabled (which is controlled by the item's
   * ItemFlag::DropEnabled flag).
   *
   * Drop events must be handled in dropEvent().
   *
   * \sa setDragEnabled(), dropEvent()
   */
  void setDropsEnabled(bool enable);

  /*! \brief Sets the row height.
   *
   * The view renders all rows with a same height. This method
   * configures this row height.
   *
   * The default value is 20 pixels.
   *
   * \note The height must be specified in LengthUnit::Pixel units.
   *
   * \sa setColumnWidth()
   */
  virtual void setRowHeight(const WLength& rowHeight);

  /*! \brief Returns the row height.
   */
  const WLength& rowHeight() const { return rowHeight_; }

  /*! \brief Sets the column width.
   *
   * The default column width is 150 pixels.
   *
   * \note The width must be specified in LengthUnit::Pixel units.
   *
   * \note The actual space occupied by each column is the column width
   *       augmented by 7 pixels for internal padding and a border.
   */
  virtual void setColumnWidth(int column, const WLength& width) = 0;
 
  /*! \brief Returns the column width.
   *
   * \sa setColumnWidth()
   */
  WLength columnWidth(int column) const;

  /*! \brief Changes the visibility of a column.
   *
   * \sa isColumnHidden()
   */
  virtual void setColumnHidden(int column, bool hide);

  /*! \brief Returns if a column is hidden.
   *
   * \sa setColumnHidden()
   */
  bool isColumnHidden(int column) const;

  /*! \brief Hides a column.
   *
   * \sa showColumn(), setColumnHidden()
   */
  void hideColumn(int column);

  /*! \brief Shows a column.
   *
   * \sa hideColumn(), setColumnHidden()
   */
  void showColumn(int column);

  /*! \brief Sets the header height.
   *
   * The default value is 20 pixels.
   *
   * \note The height must be specified in LengthUnit::Pixel units.
   */
  virtual void setHeaderHeight(const WLength& height);

  /*! \brief Returns the header height.
   *
   * \sa setHeaderHeight()
   */
  const WLength& headerHeight() const { return headerLineHeight_; } ;

  /*! \brief Returns the number of pages.
   *
   * When Ajax/JavaScript is not available, the view will use a paging
   * navigation bar to allow scrolling through the data. This returns the
   * number of pages currently shown.
   *
   * \sa createPageNavigationBar(), pageChanged()
   */
  virtual int pageCount() const = 0;

  /*! \brief Returns the page size.
   *
   * When Ajax/JavaScript is not available, the view will use a paging
   * navigation bar to allow scrolling through the data. This returns the
   * number of items per page.
   *
   * \sa createPageNavigationBar(), pageChanged()
   */
  virtual int pageSize() const = 0;

  /*! \brief Returns the current page.
   *
   * When Ajax/JavaScript is not available, the view will use a paging
   * navigation bar to allow scrolling through the data. This returns the
   * current page (between 0 and pageCount() - 1).
   *
   * \sa createPageNavigationBar(), pageChanged()
   */
  virtual int currentPage() const = 0;

  /*! \brief Sets the current page.
   *
   * When Ajax/JavaScript is not available, the view will use a paging
   * navigation bar to allow scrolling through the data. This method can
   * be used to change the current page.
   *
   * \sa createPageNavigationBar(), pageChanged()
   */
  virtual void setCurrentPage(int page) = 0;

  /*! \brief Scrolls the view to an item.
   *
   * Scrolls the view to ensure that the item which represents the
   * provided \p index is visible. A \p hint may indicate how the item
   * should appear in the viewport (if possible).
   *
   * \note Currently only implemented to scroll to the correct row, not
   *       taking into account the column.
   */
  virtual void scrollTo(const WModelIndex& index,
			ScrollHint hint = ScrollHint::EnsureVisible) = 0;

  /*! \brief Configures what actions should trigger editing.
   *
   * The default value is DoubleClicked.
   *
   * \sa edit()
   */
  void setEditTriggers(WFlags<EditTrigger> editTriggers);

  /*! \brief Returns the editing triggers.
   *
   * \sa setEditTriggers()
   */
  WFlags<EditTrigger> editTriggers() const { return editTriggers_; }

  /*! \brief Configures editing options.
   *
   * The default value is SingleEditor;
   */
  void setEditOptions(WFlags<EditOption> options);

  /*! \brief Returns the editing options.
   *
   * \sa setEditOptions()
   */
  WFlags<EditOption> editOptions() const { return editOptions_; }

  /*! \brief Opens an editor for the given index.
   *
   * Unless multiple editors are enabled, any other open editor is closed
   * first.
   *
   * \sa setEditTriggers(), setEditOptions(), closeEditor()
   */
  void edit(const WModelIndex& index);

  /*! \brief Closes the editor for the given index.
   *
   * If \p saveData is true, then the currently edited value is saved first
   * to the model.
   *
   * \sa edit()
   */
  void closeEditor(const WModelIndex& index, bool saveData = true);

  /*! \brief Closes all open editors.
   *
   * If \p saveData is true, then the currently edited values are saved
   * to the model before closing the editor.
   *
   * \sa closeEditor()
   */
  void closeEditors(bool saveData = true);

  /*! \brief Validates the editor for the given index.
   * 
   * Validation is done by invoking WAbstractItemDelegate::validate().
   */
  ValidationState validateEditor(const WModelIndex& index);

  /*! \brief Validates all editors.
   * 
   * \sa validateEditor().
   */
  ValidationState validateEditors();

  /*! \brief Returns whether an editor is open for a given index.
   *
   * \sa edit()
   */
  bool isEditing(const WModelIndex& index) const;
  
  /*! \brief Returns whether an editor's state is valid.
   */
  bool isValid(const WModelIndex& index) const;

  bool isEditing() const;

  /*! \brief %Signal emitted when clicked.
   *
   * When the event happened over an item, the first argument
   * indicates the item that was clicked on.

   * \sa doubleClicked()
   */
  Signal<WModelIndex, WMouseEvent>& clicked() { return clicked_; }

  /*! \brief %Signal emitted when double clicked.
   *
   * When the event happened over an item, the first argument
   * indicates the item that was double clicked on.
   *
   * \sa clicked()
   */
  Signal<WModelIndex, WMouseEvent>& doubleClicked() { return doubleClicked_; }

  /*! \brief %Signal emitted when a mouse button is pressed down.
   *
   * This signal is emitted only when 'over' an item (the model index is
   * passed as first argument is never invalid).
   *
   * \sa mouseWentUp()
   */
  Signal<WModelIndex, WMouseEvent>& mouseWentDown() { return mouseWentDown_; }

  /*! \brief %Signal emitted when the mouse button is released.
   *
   * When the event happened over an item, the first argument
   * indicates the item where the mouse went up.
   *
   * \sa mouseWentDown()
   */
  Signal<WModelIndex, WMouseEvent>& mouseWentUp() { return mouseWentUp_; }

  /*! \brief %Signal emitted when a finger is placed on the screen.
   *
   * When the event happened over an item, the first argument
   * indicates the item that was touched.
   *
   * \deprecated Use touchStarted() instead.
   */
  Signal<WModelIndex, WTouchEvent>& touchStart() { return touchStart_; }

  /*! \brief %Signal emitted when one or more fingers are placed on the screen.
   *
   * When the event happened over an item, the first argument
   * indicates the items that were touched. The indices in the model index
   * vector match the indices in the \link WTouchEvent::changedTouches() changedTouches() of the WTouchEvent\endlink.
   */
  Signal<std::vector<WModelIndex>, WTouchEvent>& touchStarted() { return touchStarted_; }

  /*! \brief %Signal emitted when one or more fingers are moved on the screen.
   *
   * When the event happened over an item, the first argument
   * indicates the items that were touched. The indices in the model index
   * vector match the indices in the \link WTouchEvent::changedTouches() changedTouches() of the WTouchEvent\endlink.
   */
  Signal<std::vector<WModelIndex>, WTouchEvent>& touchMoved() { return touchMoved_; }

  /*! \brief %Signal emitted when one or more fingers are removed from the screen.
   *
   * When the event happened over an item, the first argument
   * indicates the items where the touch ended. The indices in the model index
   * vector match the indices in the \link WTouchEvent::changedTouches() changedTouches() of the WTouchEvent\endlink.
   *
   * \note When JavaScript is disabled, the signal will never fire.
   */
  Signal<std::vector<WModelIndex>, WTouchEvent>& touchEnded() { return touchEnded_; }

  /*! \brief %Signal emitted when the selection is changed.
   *
   * \sa select(), setSelectionMode(), setSelectionBehavior()
   */
  Signal<>& selectionChanged() { return selectionChanged_; }

  /*! \brief %Signal emitted when page information was updated.
   *
   * When Ajax/JavaScript is not available, the view will use a paging
   * navigation bar to allow scrolling through the data. This signal
   * is emitted when page-related information changed (e.g. the
   * current page was changed, or the number of rows was changed).
   *
   * \sa createPageNavigationBar()
   */
  Signal<>& pageChanged() { return pageChanged_; }

  /*! \brief Returns the signal emitted when a column is resized by the user.
   *
   * The arguments of the signal are: the column index and the new
   * width of the column.
   */
  Signal<int, WLength>& columnResized() { return columnResized_; }

  /*! \brief Returns whether the view is sortable.
   *
   * When enabeld the view can be sorted by clicking on the header.
   */
  bool sortEnabled() {return sortEnabled_;}

  /*! \brief Alow to sort.
   *
   * When enabeld the view can be sorted by clicking on the header.
   */
  void setHeaderClickSortEnabled(bool enabled) {sortEnabled_ = enabled;}

  /*! \brief %Signal emitted when a header item is clicked.
   *
   * The argument that is passed is the column number.
   *
   * \sa clicked(), headerDblClicked()
   */
  Signal<int, WMouseEvent> &headerClicked() { return headerClicked_; }

  /*! \brief %Signal emitted when a header item is double clicked.
   *
   * The argument that is passed is the column number.
   *
   * \sa doubleClicked(), headerClicked()
   */
  Signal<int, WMouseEvent> &headerDoubleClicked() { return headerDblClicked_; }

  /*! \brief %Signal emitted when a mouse button is pressed on a header item
   *
   * The argument that is passed is the column number.
   *
   * \sa headerMouseWentDownUp()
   */
  Signal<int, WMouseEvent>& headerMouseWentDown() { return headerMouseWentDown_; }

  /*! \brief %Signal emitted when a mouse button is released on a header item
   *
   * The argument that is passed is the column number.
   *
   * \sa headerMouseWentDown()
   */
  Signal<int, WMouseEvent>& headerMouseWentUp() { return headerMouseWentUp_; }

  /*! \brief %Signal emitted when scrolling.
   *
   *  \note Works only if ajax is available.
   */
  virtual EventSignal<WScrollEvent>& scrolled() = 0;

  /*! \brief Configures the number of columns that are used as row
   *         headers.
   *
   * An item view does not use the vertical header data from the model
   * in any way, but instead you can configure data in the first
   * column(s) to be used as a row headers.
   *
   * These columns will not scroll horizontally together with the rest
   * of the model.
   *
   * The default value is 0.
   *
   * \note Currently, this property must be set before any other settings of
   *       the view and only a value of 0 or 1 is supported.
   */
  virtual void setRowHeaderCount(int count);

  /*! \brief Returns the number of columns that are used as row headers.
   *
   * \sa setRowHeaderCount()
   */
  int rowHeaderCount() const { return rowHeaderCount_; }
  
  /*! \brief Event signal emitted when a keyboard key is pushed down.
   *
   * The keyWentDown signal is the first signal emitted when a key is
   * pressed (before the keyPressed signal). Unlike keyPressed()
   * however it is also emitted for modifier keys (such as "shift",
   * "control", ...) or keyboard navigation keys that do not have a
   * corresponding character.
   *
   *
   * \sa keyPressed(), keyWentUp()
   */
  EventSignal<WKeyEvent>& keyWentDown();

  /*! \brief Event signal emitted when a "character" was entered.
   *
   * The keyPressed signal is emitted when a key is pressed, and a
   * character is entered. Unlike keyWentDown(), it is emitted only
   * for key presses that result in a character being entered, and
   * thus not for modifier keys or keyboard navigation keys.
   *
   * \sa keyWentDown()
   */
  EventSignal<WKeyEvent>& keyPressed();
    
  /*! \brief Event signal emitted when a keyboard key is released.
   *
   * This is the counter-part of the keyWentDown() event. Every
   * key-down has its corresponding key-up.
   *
   * \sa keyWentDown()
   */
  EventSignal<WKeyEvent>& keyWentUp();
 
protected:
  /*! \brief Creates a new item view.
   */
  WAbstractItemView();

  /*! \brief Handles a drop event (drag & drop).
   *
   * The \p event object contains details about the drop
   * operation, identifying the source (which provides the data) and
   * the mime-type of the data. The drop was received on the
   * \p target item.
   *
   * The drop event can be handled either by the view itself, or by
   * the model. The default implementation checks if the mime-type is
   * accepted by the model, and if so passes the drop event to the
   * model. If the source is the view's own selection model, then the
   * drop event will be handled as a DropAction::Move, otherwise the
   * drop event will be handled as a DropAction::Copy.
   *
   * \sa WAbstractItemModel::dropEvent()
   */
  virtual void dropEvent(const WDropEvent& event, const WModelIndex& target);

  using WWidget::dropEvent;

  /*! \brief Create an extra widget in the header.
   *
   * You may reimplement this method to provide an extra widget to be placed
   * below the header label. The extra widget will be visible only if
   * a multi-line header is configured using setHeaderHeight().
   *
   * The widget is created only once, but this method may be called
   * repeatedly for a column for which prior calls returned \c 0
   * (i.e. each time the header is rerendered).
   *
   * The default implementation returns \c 0.
   *
   * \sa setHeaderHeight(), extraHeaderWidget()
   */
  virtual std::unique_ptr<WWidget> createExtraHeaderWidget(int column);

  /*! \brief Returns the extra header widget.
   *
   * Returns the widget previously created using createExtraHeaderWidget()
   *
   * \sa createExtraHeaderWidget()
   */
  WWidget *extraHeaderWidget(int column);

  /*! \brief Returns a page navigation widget.
   *
   * When Ajax/JavaScript is not available, the view will use a paging
   * navigation bar to allow scrolling through the data, created by this
   * method. The default implementation displays a simple navigation bar
   * with (First, Prevous, Next, Last) buttons and a page counter.
   *
   * You may want to reimplement this method to provide a custom page
   * navigation bar. You can use the currentPage(), pageCount(), and
   * setCurrentPage() methods to set or get the page information, and
   * listen to the pageChanged() signal to react to changes.
   */
  virtual std::unique_ptr<WWidget> createPageNavigationBar();

protected:
  struct ColumnInfo {
    observing_ptr<WCssTemplateRule> styleRule;
    int id;
    SortOrder sortOrder;
    AlignmentFlag alignment;
    AlignmentFlag headerHAlignment, headerVAlignment;
    bool headerWordWrap;
    WLength width;
    observing_ptr<WWidget> extraHeaderWidget;
    bool sorting, hidden;
    std::shared_ptr<WAbstractItemDelegate> itemDelegate_;

    std::string styleClass() const;

    ColumnInfo(const WAbstractItemView *view, int id);
  };

  enum class RenderState {
    RenderOk = 0,
    NeedAdjustViewPort = 1,
    NeedUpdateModelIndexes = 2,
    NeedRerenderData = 3,
    NeedRerenderHeader = 4,
    NeedRerender = 5
  };

  WContainerWidget *impl_;
  RenderState renderState_;
  std::vector<Wt::Signals::connection> modelConnections_;

  mutable std::vector<ColumnInfo> columns_;
  int currentSortColumn_;

  bool dragEnabled_, dropsEnabled_;
  std::unique_ptr<WWidget> uDragWidget_;
  observing_ptr<WWidget> dragWidget_;

  virtual void scheduleRerender(RenderState what);

  virtual void modelDataChanged(const WModelIndex& topLeft,
				const WModelIndex& bottomRight) = 0;
  virtual void modelLayoutAboutToBeChanged();
  virtual void modelLayoutChanged();
  void modelHeaderDataChanged(Orientation orientation, int start, int end);
  void modelReset();

  ColumnInfo& columnInfo(int column) const;
  int columnById(int columnid) const;

  int columnCount() const;
  int visibleColumnCount() const;
  int visibleColumnIndex(int modelColumn) const;
  int modelColumnIndex(int visibleColumn) const;

  virtual ColumnInfo createColumnInfo(int column) const;

  void saveExtraHeaderWidgets();
  std::unique_ptr<WWidget> createHeaderWidget(int column);
  WText *headerTextWidget(int column);

  /*! \brief Handles a click event.
   *
   * This processes the event for internal purposes (such as
   * selection or editing) and emits the clicked() signal.
   *
   * You may want to override this signal to override the built-in
   * selection or editing behaviour.
   */
  virtual void handleClick(const WModelIndex& index,
			   const WMouseEvent& event);

  /*! \brief Handles a double click event.
   *
   * This processes the event for internal purposes (such as
   * editing) and emits the doubleClicked() signal.
   *
   * You may want to override this signal to override the built-in
   * editing behaviour.
   */
  virtual void handleDoubleClick(const WModelIndex& index,
				 const WMouseEvent& event);

  /*! \brief Handles a mouse down event.
   *
   * This emits the mouseWentDown() signal.
   */
  virtual void handleMouseDown(const WModelIndex& index,
			       const WMouseEvent& event);

  /*! \brief Handles a mouse up event.
   *
   * This emits the mouseWentUp() signal.
   */
  virtual void handleMouseUp(const WModelIndex& index,
			     const WMouseEvent& event);

  /*! \brief Handles a touch select event.
   */
  virtual void handleTouchSelect(const std::vector<WModelIndex>& indices,
				 const WTouchEvent& event);

  /*! \brief Handles a touch started event.
   */
  virtual void handleTouchStart(const std::vector<WModelIndex>& indices,
				const WTouchEvent& event);

  /*! \brief Handles a touch moved event.
   */
  virtual void handleTouchMove(const std::vector<WModelIndex>& indices,
                               const WTouchEvent& event);

  /*! \brief Handles a touch ended event.
   */
  virtual void handleTouchEnd(const std::vector<WModelIndex>& indices,
			      const WTouchEvent& event);

  virtual bool internalSelect(const WModelIndex& index, SelectionFlag option);

  virtual void enableAjax() override;

  void setEditState(const WModelIndex& index, const cpp17::any& editState);
  cpp17::any editState(const WModelIndex& index) const;
  bool hasEditFocus(const WModelIndex& index) const;

  void setEditorWidget(const WModelIndex& index, WWidget *editor);

  void bindObjJS(JSlot& slot, const std::string& jsMethod);
  void connectObjJS(EventSignalBase& s, const std::string& jsMethod);
  bool shiftEditorRows(const WModelIndex& parent, int start, int count,
		       bool persistWhenShifted);
  bool shiftEditorColumns(const WModelIndex& parent, int start, int count,
			  bool persistWhenShifted);
  void persistEditor(const WModelIndex& index);

private:
  struct Editor {
    Editor() :
      widget(nullptr),
      stateSaved(false),
      valid(false) {
      editState = cpp17::any();
    }
    observing_ptr<WWidget> widget;
    cpp17::any editState;
    bool stateSaved;
    bool valid;
  };

  std::shared_ptr<WAbstractItemModel> model_, headerModel_;
  WModelIndex rootIndex_;
  std::shared_ptr<WAbstractItemDelegate> itemDelegate_, headerItemDelegate_;
  std::unique_ptr<WItemSelectionModel> selectionModel_;
  WLength rowHeight_, headerLineHeight_;
  SelectionMode selectionMode_;
  bool sorting_, columnResize_;
  AlignmentFlag defaultHeaderVAlignment_;
  bool defaultHeaderWordWrap_;
  int rowHeaderCount_;
  WString computedDragMimeType_;
  bool sortEnabled_;
  WModelIndex delayedClearAndSelectIndex_;

  JSignal<int, int> columnWidthChanged_;
  Signal<int, WLength> columnResized_;

  observing_ptr<WCssTemplateRule> headerHeightRule_;

  mutable int nextColumnId_;
  
  bool alternatingRowColors_;

  JSlot resizeHandleMDownJS_;

  typedef std::map<WModelIndex, Editor> EditorMap;
  EditorMap editedItems_;

  Signal<int, WMouseEvent> headerClicked_;
  Signal<int, WMouseEvent> headerDblClicked_;
  Signal<int, WMouseEvent> headerMouseWentDown_;
  Signal<int, WMouseEvent> headerMouseWentUp_;

  Signal<WModelIndex, WMouseEvent> clicked_;
  Signal<WModelIndex, WMouseEvent> doubleClicked_;
  Signal<WModelIndex, WMouseEvent> mouseWentDown_;
  Signal<WModelIndex, WMouseEvent> mouseWentUp_;
  Signal<WModelIndex, WTouchEvent> touchStart_;
  Signal<std::vector<WModelIndex>, WTouchEvent> touchStarted_;
  Signal<std::vector<WModelIndex>, WTouchEvent> touchMoved_;
  Signal<std::vector<WModelIndex>, WTouchEvent> touchEnded_;
  Signal<> selectionChanged_;
  Signal<> pageChanged_;

  WFlags<EditTrigger> editTriggers_;
  WFlags<EditOption> editOptions_;

  bool touchRegistered_;

  void closeEditorWidget(WWidget *editor, bool saveData);
  void saveEditedValue(const WModelIndex& index, Editor& editor);
  void persistEditor(const WModelIndex& index, Editor& editor);

  virtual WWidget *headerWidget(int column, bool contentsOnly = true) = 0;
  virtual WText *headerSortIconWidget(int column);

  void selectionHandleClick(const WModelIndex& index,
			    WFlags<KeyboardModifier> modifiers);
  void selectionHandleTouch(const std::vector<WModelIndex>& indices, const WTouchEvent& event);
  void extendSelection(const WModelIndex& index);
  void extendSelection(const std::vector<WModelIndex>& index);
  virtual void selectRange(const WModelIndex& first, const WModelIndex& last)
    = 0;

  void checkDragSelection();
  void configureModelDragDrop();

  void handleHeaderMouseDown(int columnid, WMouseEvent event);
  void handleHeaderMouseUp(int columnid, WMouseEvent event);
  void handleHeaderClicked(int columnid, WMouseEvent event);
  void handleHeaderDblClicked(int columnid, WMouseEvent event);
  void toggleSortColumn(int columnid);
  void updateColumnWidth(int columnId, int width);

  virtual WContainerWidget* headerContainer() = 0;

  int headerLevel(int column) const;
  int headerLevelCount() const;

  void expandColumn(int columnid);
  void collapseColumn(int columnid);

  void initDragDrop();
};

W_DECLARE_OPERATORS_FOR_FLAGS(EditOption)
W_DECLARE_OPERATORS_FOR_FLAGS(EditTrigger)

}

#endif // WT_WABSTRACTITEMVIEW_H_
