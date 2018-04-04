// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WTABLEVIEW_H_
#define WT_WTABLEVIEW_H_

#include <Wt/WAbstractItemView.h>
#include <Wt/WContainerWidget.h>

namespace Wt {

  class WContainerWidget;
  class WModelIndex;

/*! \class WTableView Wt/WTableView.h Wt/WTableView.h
 *  \brief An MVC View widget for tabular data.
 *
 * The view displays data from a WAbstractItemModel in a table. It
 * provides incremental rendering, without excessive use of client- or
 * serverside resources.
 *
 * The rendering (and editing) of items is handled by a
 * WAbstractItemDelegate, by default it uses WItemDelegate which
 * renders data of all predefined roles (see also Wt::ItemDataRole),
 * including text, icons, checkboxes, and tooltips.
 *
 * The view provides virtual scrolling in both horizontal and vertical
 * directions, and can therefore be used to display large data models
 * (with large number of columns and rows).
 *
 * When the view is updated, it will read the data from the model
 * row per row, starting at the top visible row. If \c (r1,c1) and \c (r2,c2)
 * are two model indexes of visible table cells, and \c r1 \c < \c r2 or
 * \c r1 \c == \c r2 and \c c1 \c < \c c2, then the data for the first
 * model index is read before the second. Keep this into account when
 * implementing a custom WAbstractItemModel if you want to optimize
 * performance.
 *
 * The view may support editing of items, if the model indicates
 * support (see the Wt::ItemFlag::Editable flag). You can define triggers
 * that initiate editing of an item using setEditTriggers(). The
 * actual editing is provided by the item delegate (you can set an
 * appropriate delegate for one column using
 * setItemDelegateForColumn()). Using setEditOptions() you can
 * customize if and how the view deals with multiple editors.
 *
 * By default, all columns are given a width of 150px. Column widths
 * of all columns can be set through the API method setColumnWidth(),
 * and also by the user using handles provided in the header.
 *
 * If the model supports sorting (WAbstractItemModel::sort()), such as
 * the WStandardItemModel, then you can enable sorting buttons in the
 * header, using setSortingEnabled().
 *
 * You can allow selection on row or item level (using
 * setSelectionBehavior()), and selection of single or multiple items
 * (using setSelectionMode()), and listen for changes in the selection
 * using the selectionChanged() signal.
 *
 * You may enable drag & drop support for this view, with awareness
 * of the items in the model. When enabling dragging (see
 * setDragEnabled()), the current selection may be dragged, but only
 * when all items in the selection indicate support for dragging
 * (controlled by the \link Wt::ItemFlag::DragEnabled
 * ItemFlag::DragEnabled\endlink flag), and if the model indicates a
 * mime-type (controlled by WAbstractItemModel::mimeType()). Likewise,
 * by enabling support for dropping (see setDropsEnabled()), the view
 * may receive a drop event on a particular item, at least if the item
 * indicates support for drops (controlled by the \link
 * Wt::ItemFlag::DropEnabled ItemFlag::DropEnabled\endlink flag).
 *
 * You may also react to mouse click events on any item, by connecting
 * to one of the clicked() or doubleClicked() signals.
 *
 * If a WTableView is not constrained in height (either by
 * a layout manager or by setHeight()), then it will grow according
 * to the size of the model.
 *
 * \ingroup modelview
 */
class WT_API WTableView : public WAbstractItemView
{
public:
  /*! \brief Constructor
   */
  WTableView();

  virtual ~WTableView();

  virtual WWidget *itemWidget(const WModelIndex& index) const override;
  virtual void setModel(const std::shared_ptr<WAbstractItemModel>& model)
    override;

  virtual void setColumnWidth(int column, const WLength& width) override;
  virtual void setAlternatingRowColors(bool enable) override;
  virtual void setRowHeight(const WLength& rowHeight) override;
  virtual void setHeaderHeight(const WLength& height) override;
#ifndef WT_CNOR
  using WAbstractItemView::setHeaderHeight;
#endif
  virtual void resize(const WLength& width, const WLength& height) override;
  virtual void setColumnHidden(int column, bool hidden) override;
  virtual void setRowHeaderCount(int count) override;

  virtual int pageCount() const override;
  virtual int pageSize() const override;
  virtual int currentPage() const override;
  virtual void setCurrentPage(int page) override;

  virtual void scrollTo(const WModelIndex& index,
			ScrollHint hint = ScrollHint::EnsureVisible) override;

  /*! \brief Scrolls the view x px left and y px top.
   */
  void scrollTo(int x, int y);

  /*! \brief set css overflow
   */
  void setOverflow(Overflow overflow,
		   WFlags<Orientation> orientation 
		   = (Orientation::Horizontal | Orientation::Vertical));

  virtual void setHidden(bool hidden,
			 const WAnimation& animation = WAnimation()) override;

  /*! \brief Returns the model index corresponding to a widget.
   *
   * This returns the model index for the item that is or contains the
   * given widget.
   */
  WModelIndex modelIndexAt(WWidget *widget) const;

  virtual EventSignal<WScrollEvent>& scrolled() override;

 protected:
  virtual void render(WFlags<RenderFlag> flags) override;

  /*! \brief Called when rows or columns are inserted/removed.
   *
   * Override this method when you want to adjust the table's size when
   * columns or rows are inserted or removed. The method is also called when
   * the model is reset. The default implementation does nothing.
   */
  virtual void adjustSize() {}

  virtual void enableAjax() override;

private:
  class ColumnWidget : public WContainerWidget
  {
  public:
    ColumnWidget(WTableView *view, int column);
    int column() const { return column_; }

  private:
    int column_;
  };

  /* For Ajax implementation */
  WContainerWidget *headers_, *canvas_, *table_;
  WContainerWidget *headerContainer_, *contentsContainer_;
  WContainerWidget *headerColumnsCanvas_, *headerColumnsTable_;
  WContainerWidget *headerColumnsHeaderContainer_, *headerColumnsContainer_;

  /* For plain HTML implementation */
  WTable *plainTable_;

  JSignal<int, int, std::string, std::string, WMouseEvent> dropEvent_;
  JSignal<int, int, int, int> scrolled_;
  JSignal<WTouchEvent> itemTouchSelectEvent_;

  Signals::connection touchStartConnection_;
  Signals::connection touchMoveConnection_;
  Signals::connection touchEndConnection_;

  /* Ajax only: First and last columns rendered (this somewhat
   * redundant with the state contained in the widgets, but because
   * columns are variable width, we cache these values as well). The
   * first and last rows rendered can be derived from widget
   * properties. */
  int firstColumn_, lastColumn_;

  /* Current size of the viewport */
  int viewportLeft_, viewportWidth_, viewportTop_, viewportHeight_;

  /* Desired rendered area */
  int renderedFirstRow_, renderedLastRow_,
    renderedFirstColumn_, renderedLastColumn_;

  /* Scroll to to process after viewport height is known */
  int scrollToRow_;
  ScrollHint scrollToHint_;
  bool columnResizeConnected_;

  void updateTableBackground();

  ColumnWidget *columnContainer(int renderedColumn) const;

  void modelColumnsInserted(const WModelIndex& parent, int start, int end);
  void modelColumnsAboutToBeRemoved(const WModelIndex& parent,
				    int start, int end);
  void modelRowsInserted(const WModelIndex& parent, int start, int end);
  void modelRowsAboutToBeRemoved(const WModelIndex& parent, int start, int end);
  void modelRowsRemoved(const WModelIndex& parent, int start, int end);
  virtual void modelDataChanged(const WModelIndex& topLeft,
				const WModelIndex& bottomRight) override;

  virtual void modelLayoutChanged() override;

  std::unique_ptr<WWidget> renderWidget(WWidget* w, const WModelIndex& index);

  int spannerCount(const Side side) const;
  void setSpannerCount(const Side side, const int count);

  void renderTable(const int firstRow, const int lastRow, 
		   const int firstColumn, const int lastColumn);
  void addSection(const Side side);
  void removeSection(const Side side);
  int firstRow() const;
  int lastRow() const;
  int firstColumn() const;
  int lastColumn() const;

  void setup();
  void reset();
  void rerenderHeader();
  void rerenderData();
  void adjustToViewport();
  void computeRenderedArea();

  virtual WContainerWidget* headerContainer() override { 
    return headerContainer_;
  }

  virtual WWidget *headerWidget(int column, bool contentsOnly = true) override;

  void onViewportChange(int left, int top, int width, int height);
  void onColumnResize();
  void resetGeometry();
  
  void handleSingleClick(bool headerColumns, const WMouseEvent& event);
  void handleDblClick(bool headerColumns, const WMouseEvent& event);
  void handleMouseWentDown(bool headerColumns, const WMouseEvent& event);
  void handleMouseWentUp(bool headerColumns, const WMouseEvent& event);
  void handleTouchSelected(const WTouchEvent& event);
  void handleTouchStarted(const WTouchEvent& event);
  void handleTouchMoved(const WTouchEvent& event);
  void handleTouchEnded(const WTouchEvent& event);
  WModelIndex translateModelIndex(bool headerColumns, const WMouseEvent& event);
  WModelIndex translateModelIndex(const Touch& touch);

  void handleRootSingleClick(int u, const WMouseEvent& event);
  void handleRootDoubleClick(int u, const WMouseEvent& event);
  void handleRootMouseWentDown(int u, const WMouseEvent& event);
  void handleRootMouseWentUp(int u, const WMouseEvent& event);

  void updateItem(const WModelIndex& index,
		  int renderedRow, int renderedColumn);

  virtual bool internalSelect(const WModelIndex& index, SelectionFlag option)
    override;
  virtual void selectRange(const WModelIndex& first, const WModelIndex& last)
    override;
  void shiftModelIndexRows(int start, int count);
  void shiftModelIndexColumns(int start, int count);
  void renderSelected(bool selected, const WModelIndex& index);
  int renderedColumnsCount() const;

  void defineJavaScript();

  bool isRowRendered(const int row) const;
  bool isColumnRendered(const int column) const;
  void updateColumnOffsets();
  void updateModelIndexes();
  void updateModelIndex(const WModelIndex& index,
			int renderedRow, int renderedColumn);

  void onDropEvent(int renderedRow, int columnId,
		   std::string sourceId, std::string mimeType,
		   WMouseEvent event);

  void deleteItem(int row, int col, WWidget *widget);

  bool ajaxMode() const { return table_ != nullptr; }
  double canvasHeight() const;
  void setRenderedHeight(double th);
};

}

#endif // WT_WTABLEVIEW_H
