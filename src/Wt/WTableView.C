/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WTableView"

#include "Wt/WTable"
#include "Wt/WContainerWidget"
#include "Wt/WAbstractItemModel"
#include "Wt/WModelIndex"
#include "Wt/WAbstractItemDelegate"
#include "Wt/WApplication"
#include "Wt/WText"
#include "Wt/WItemDelegate"
#include "Wt/WEnvironment"
#include "Wt/WVBoxLayout"
#include "Wt/WVBoxLayout"

#include "JavaScriptLoader.h"
#include "Utils.h"
#include "EscapeOStream.h"

#ifndef WT_DEBUG_JS

#include "js/WTableView.min.js"
#endif

#include <iostream>
#include <math.h>

// TODO:
//  call updateModelIndex when shifting indexes

namespace Wt {

WTableView::WTableView(WContainerWidget *parent)
  : WAbstractItemView(parent),
    headers_(0),
    canvas_(0),
    table_(0),
    headerContainer_(0),
    contentsContainer_(0),
    plainTable_(0),
    dropEvent_(impl_, "dropEvent"),
    columnWidthChanged_(impl_, "columnResized"),
    scrolled_(impl_, "scrolled"),
    viewportLeft_(0),
    viewportWidth_(1000),
    viewportTop_(0),
    viewportHeight_(600)
{ 
  setSelectable(false);

  dropEvent_.connect(this, &WTableView::onDropEvent);

  setStyleClass("Wt-tableview");

  const char *CSS_RULES_NAME = "Wt::WTableView";
  
  WApplication *app = WApplication::instance();

  if (!app->styleSheet().isDefined(CSS_RULES_NAME)) {
    /* header */
    app->styleSheet().addRule
      (".Wt-tableview .Wt-headertable",
       "-moz-user-select: none;"
       "-khtml-user-select: none;"
       "user-select: none;"
       "overflow: hidden;"
       "width: 100%;", CSS_RULES_NAME);

    if (app->environment().agentIsIE())
      app->styleSheet().addRule
	(".Wt-tableview .Wt-header .Wt-label",
	 "zoom: 1;");

    /* resize handles */
    app->styleSheet().addRule
      (".Wt-tableview div.Wt-tv-rh",
       "float: right; width: 4px; cursor: col-resize;"
       "padding-left: 0px;");
    
    app->styleSheet().addRule
      (".Wt-tableview .Wt-header-el, .Wt-tableview .Wt-tv-c",
       "text-overflow: ellipsis;"
       "overflow: hidden;"
       "white-space: nowrap;"
       "padding: 0px;");

    app->styleSheet().addRule
      (".Wt-tableview .Wt-header .Wt-tv-c",
       "overflow: visible;"
       "padding-left: 6px;");

    app->styleSheet().addRule
      (".Wt-tableview .Wt-tv-contents .Wt-tv-c,"
       ".Wt-plaintable .Wt-tv-c",
       "padding: 0px 3px;");

    app->styleSheet().addRule
      (".Wt-tableview .Wt-tv-rh:hover",
       "background-color: #DDDDDD;");

    app->styleSheet().addRule
      (".Wt-tableview div.Wt-tv-rhc0",
       "float: left; width: 4px;");

    /* sort handles */
    app->styleSheet().addRule
      (".Wt-tableview .Wt-tv-sh", std::string() +
       "float: right; width: 16px; height: 16px; padding-bottom: 6px;"
       "cursor: pointer; cursor:hand;");

    app->styleSheet().addRule
      (".Wt-tableview .Wt-tv-sh-nrh", std::string() + 
       "float: right; width: 16px; height: 16px;"
       "cursor: pointer; cursor:hand;");

    /* borders: needed here for IE */
    app->styleSheet().addRule
      (".Wt-tableview .Wt-tv-br, "
       ".Wt-tableview .Wt-tv-contents .Wt-tv-c",
       "border-right: 1px solid white;");

    /* data item icons */
    app->styleSheet().addRule
      (".Wt-tableview .Wt-tv-contents img.icon, "
       ".Wt-tableview .Wt-tv-contents input.icon",
       "margin: 0px 3px 2px 0px; vertical-align: middle");
  }

  /* item drag & drop */
  app->styleSheet().addRule
    ("#" + id() + "dw",
     "width: 32px; height: 32px;"
     "background: url(" + WApplication::resourcesUrl() + "items-not-ok.gif);");

  app->styleSheet().addRule
    ("#" + id() + "dw.Wt-valid-drop",
     "width: 32px; height: 32px;"
     "background: url(" + WApplication::resourcesUrl() + "items-ok.gif);");

  if (app->environment().ajax()) {
    impl_->setPositionScheme(Relative);

    headers_ = new WContainerWidget();
    headers_->setStyleClass("Wt-headertable headerrh");

    table_ = new WContainerWidget();
    table_->setStyleClass("Wt-tv-contents");

    WVBoxLayout *layout = new WVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    headerContainer_ = new WContainerWidget();
    headerContainer_->setStyleClass("Wt-header headerrh cwidth");
    headerContainer_->setOverflow(WContainerWidget::OverflowHidden);
    headerContainer_->addWidget(headers_);

    contentsContainer_ = new WContainerWidget();
    contentsContainer_->setStyleClass("cwidth");
    contentsContainer_->setOverflow(WContainerWidget::OverflowAuto);
    contentsContainer_->setPositionScheme(Absolute);

    canvas_ = new WContainerWidget();
    canvas_->setStyleClass("Wt-spacer");
    canvas_->setPositionScheme(Relative);
    canvas_->clicked()      .connect(this, &WTableView::handleSingleClick);
    canvas_->doubleClicked().connect(this, &WTableView::handleDoubleClick);
    canvas_->mouseWentDown().connect(this, &WTableView::handleMouseWentDown); 
    canvas_->addWidget(table_);

    table_->setPositionScheme(Absolute);
    table_->resize(WLength(100, WLength::Percentage), WLength::Auto);

    contentsContainer_->addWidget(canvas_);

    scrolled_.connect(this, &WTableView::onViewportChange);

    layout->addWidget(headerContainer_);
    layout->addWidget(contentsContainer_, 1);

    impl_->setLayout(layout);

    app->addAutoJavaScript
      ("{var obj = $('#" + id() + "').data('obj');"
       "if (obj) obj.autoJavaScript();}");

    connectObjJS(canvas_->mouseWentDown(), "mouseDown");
  } else {
    plainTable_ = new WTable();
    plainTable_->setStyleClass("Wt-plaintable");
    plainTable_->setAttributeValue("style", "table-layout: fixed;");
    plainTable_->setHeaderCount(1);

    impl_->addWidget(plainTable_);

    resize(width(), height());
  }

  setRowHeight(rowHeight());

  updateTableBackground();
}

void WTableView::resize(const WLength& width, const WLength& height)
{
  if (ajaxMode()) {
    if (!height.isAuto()) {
      viewportHeight_
	= static_cast<int>(ceil((height.toPixels()
				 - headerHeight().toPixels())));
    }
  } else { // Plain HTML mode
    if (!plainTable_) // Not yet rendered
      return;

    plainTable_->resize(width, WLength::Auto);

    if (!height.isAuto()) {
      if (impl_->count() < 2)
	impl_->addWidget(createPageNavigationBar());
    }
  }

  computeRenderedArea();

  WCompositeWidget::resize(width, height);

  scheduleRerender(NeedAdjustViewPort);
}

WTableView::~WTableView()
{ 
  impl_->clear();
}

void WTableView::updateTableBackground()
{
  std::string backgroundImage;

  if (alternatingRowColors())
    backgroundImage = "/stripes/stripe-";
  else
    backgroundImage = "/no-stripes/no-stripe-";

  backgroundImage = WApplication::resourcesUrl()
    + "themes/" + WApplication::instance()->cssTheme()
    + backgroundImage 
    + boost::lexical_cast<std::string>(static_cast<int>(rowHeight().toPixels()))
    + "px.gif";

  if (ajaxMode())
    table_->decorationStyle().setBackgroundImage(backgroundImage);
  else
    // FIXME avoid background on header row ?
    plainTable_->decorationStyle().setBackgroundImage(backgroundImage);	
}

void WTableView::setModel(WAbstractItemModel* model)
{  
  WAbstractItemView::setModel(model);

  /* connect slots to new model */
  modelConnections_.push_back(model->columnsInserted().connect
			      (this, &WTableView::modelColumnsInserted));
  modelConnections_.push_back(model->columnsAboutToBeRemoved().connect
			     (this, &WTableView::modelColumnsAboutToBeRemoved));
  modelConnections_.push_back(model->rowsInserted().connect
			      (this, &WTableView::modelRowsInserted));
  modelConnections_.push_back(model->rowsAboutToBeRemoved().connect
			      (this, &WTableView::modelRowsAboutToBeRemoved));
  modelConnections_.push_back(model->rowsRemoved().connect
			      (this, &WTableView::modelRowsRemoved));
  modelConnections_.push_back(model->dataChanged().connect
			      (this, &WTableView::modelDataChanged));
  modelConnections_.push_back(model->headerDataChanged().connect
			      (this, &WTableView::modelHeaderDataChanged));
  modelConnections_.push_back(model->layoutAboutToBeChanged().connect
			      (this, &WTableView::modelLayoutAboutToBeChanged));
  modelConnections_.push_back(model->layoutChanged().connect
			      (this, &WTableView::modelLayoutChanged));
  modelConnections_.push_back(model->modelReset().connect
			      (this, &WTableView::modelReset));
}

WWidget* WTableView::renderWidget(WWidget* widget, const WModelIndex& index) 
{
  WAbstractItemDelegate *itemDelegate = this->itemDelegate(index.column());

  WFlags<ViewItemRenderFlag> renderFlags = 0;

  if (ajaxMode()) {
    if (   (selectionBehavior() == SelectItems && isSelected(index))
	   || (selectionBehavior() == SelectRows
	       && isSelected(model()->index(index.row(), 0, rootIndex()))))
      renderFlags |= RenderSelected;
  }

  if (isEditing(index)) {
    renderFlags |= RenderEditing;
    if (hasEditFocus(index))
      renderFlags |= RenderFocused;
  }

  if (!isValid(index)) {
    renderFlags |= RenderInvalid;
  }

  bool initial = !widget;

  widget = itemDelegate->update(widget, index, renderFlags);
  widget->setInline(false);
  widget->addStyleClass("Wt-tv-c");
  widget->resize(WLength::Auto, rowHeight());

  if (renderFlags & RenderEditing) {
    widget->setTabIndex(-1);
    setEditorWidget(index, widget);
  }

  if (initial) {
    /*
     * If we are re-creating an old editor, then reset its current edit
     * state (we do not actually check if it is an old editor, but we could
     * now with stateSaved)
     */
    if (renderFlags & RenderEditing) {
      boost::any state = editState(index);
      if (!state.empty())
	itemDelegate->setEditState(widget, state);
    }
  }

  return widget;
}

int WTableView::spannerCount(const Side side) const
{
  assert(ajaxMode());

  switch (side) {
  case Top: {
    return (int)(table_->offset(Top).toPixels() / rowHeight().toPixels());
  }
  case Bottom: {
    return (int)(model()->rowCount(rootIndex())
      - (table_->offset(Top).toPixels() + table_->height().toPixels())
		 / rowHeight().toPixels());
  }
  case Left:
    return firstColumn_;
  case Right:
    return columnCount() - (lastColumn_ + 1);
  default:
    assert(false);
    return -1;
  }
}

void WTableView::setSpannerCount(const Side side, const int count)
{
  assert(ajaxMode());

  switch (side) {
  case Top: {
    int size = model()->rowCount(rootIndex()) - count - spannerCount(Bottom);
    table_->setOffsets(count * rowHeight().toPixels(), Top);
    table_->resize(table_->width(), size * rowHeight().toPixels());
    break;
  }
  case Bottom: {
    int size = model()->rowCount(rootIndex()) - spannerCount(Top) - count;
    table_->resize(table_->width(), size * rowHeight().toPixels());
    break;
  }
  case Left: {
    int total = 0;
    for (int i = 0; i < count; i++)
      if (!columnInfo(i).hidden)
	total += (int)columnInfo(i).width.toPixels() + 7;
    table_->setOffsets(total, Left);
    firstColumn_ = count;
    break;
  }
  case Right:
    lastColumn_ = columnCount() - count - 1;
    break;
  default:
    assert(false);
  }
}

int WTableView::firstRow() const
{
  if (ajaxMode())
    return spannerCount(Top);
  else
    return renderedFirstRow_;
}

int WTableView::lastRow() const
{
  if (ajaxMode())
    return model()->rowCount(rootIndex()) - spannerCount(Bottom) - 1;
  else
    return renderedLastRow_;
}

int WTableView::firstColumn() const
{
  if (ajaxMode())
    return firstColumn_;
  else
    return 0;
}

int WTableView::lastColumn() const
{
  if (ajaxMode())
    return lastColumn_;
  else
    return model()->columnCount(rootIndex()) - 1;
}

void WTableView::addSection(const Side side,
			    const std::vector<WWidget *>& items)
{
  assert(ajaxMode());

  switch (side) {
  case Top:
    for (unsigned i = 0; i < items.size(); ++i) {
      ColumnWidget *w = columnContainer(i);
      w->insertWidget(0, items[i]);
    }

    setSpannerCount(side, spannerCount(side) - 1);
    break;
  case Bottom:
    for (unsigned i = 0; i < items.size(); ++i) {
      ColumnWidget *w = columnContainer(i);
      w->addWidget(items[i]);
    }

    setSpannerCount(side, spannerCount(side) - 1);
    break;
  case Left: {
    ColumnWidget *w = new ColumnWidget(this, firstColumn() - 1);
    for (unsigned i = 0; i < items.size(); ++i)
      w->addWidget(items[i]);

    if (!columnInfo(w->column()).hidden)
      table_->setOffsets(table_->offset(Left).toPixels()
			 - columnWidth(w->column()).toPixels() - 7, Left);
    else
      w->hide();

    --firstColumn_;
    break;
  }
  case Right: {
    ColumnWidget *w = new ColumnWidget(this, lastColumn() + 1);
    for (unsigned i = 0; i < items.size(); ++i)
      w->addWidget(items[i]);
    if (columnInfo(w->column()).hidden)
      w->hide();

    ++lastColumn_;
    break;
  }
  default:
    assert(false);
  }
}

void WTableView::deleteItem(int row, int col, WWidget *w)
{
  WModelIndex index = model()->index(row, col, rootIndex());

  if (isEditing(index)) {
    setEditState(index, itemDelegate(col)->editState(w));
    setEditorWidget(index, 0);
  }

  delete w;
}

void WTableView::removeSection(const Side side)
{
  assert(ajaxMode());

  int row = firstRow(), col = firstColumn();

  switch (side) {
  case Top:
    setSpannerCount(side, spannerCount(side) + 1);

    for (int i = 0; i < table_->count(); ++i) {
      ColumnWidget *w = columnContainer(i);
      deleteItem(row, col + i, w->widget(0));
    }
    break;
  case Bottom:
    row = lastRow();
    setSpannerCount(side, spannerCount(side) + 1);

    for (int i = 0; i < table_->count(); ++i) {
      ColumnWidget *w = columnContainer(i);
      deleteItem(row, col + i, w->widget(w->count() - 1));
    }
    break;
  case Left: {
    ColumnWidget *w = columnContainer(0);

    if (!columnInfo(w->column()).hidden)
      table_->setOffsets(table_->offset(Left).toPixels()
			 + columnWidth(w->column()).toPixels() + 7, Left);
    ++firstColumn_;

    for (int i = w->count() - 1; i >= 0; --i)
      deleteItem(row + i, col, w->widget(i));

    delete w;

    break;
  }
  case Right: {
    ColumnWidget *w = columnContainer(-1);
    col = w->column();

    --lastColumn_;

    for (int i = w->count() - 1; i >= 0; --i)
      deleteItem(row + i, col, w->widget(i));

    delete w;

    break;
  }
  default:
    break;
  }
}

void WTableView::renderTable(const int fr, const int lr,
			     const int fc, const int lc)
{
  assert(ajaxMode());

  if (fr > lastRow() || firstRow() > lr || 
      fc > lastColumn() || firstColumn() > lc)
    reset();

  int oldFirstRow = firstRow();
  int oldLastRow = lastRow();

  int topRowsToAdd = 0;
  int bottomRowsToAdd = 0;

  if (oldLastRow - oldFirstRow < 0) {
    topRowsToAdd = 0;
    setSpannerCount(Top, fr);
    setSpannerCount(Bottom, spannerCount(Bottom) - fr);
    bottomRowsToAdd = lr - fr + 1;
  } else {
    topRowsToAdd = firstRow() - fr;
    bottomRowsToAdd = lr - lastRow();
  }

  int oldFirstCol = firstColumn();
  int oldLastCol = lastColumn();

  int leftColsToAdd = 0;
  int rightColsToAdd = 0;
 
  if (oldLastCol - oldFirstCol < 0) {
    leftColsToAdd = 0;
    setSpannerCount(Left, fc);
    setSpannerCount(Right, spannerCount(Right) - fc);
    rightColsToAdd = lc - fc + 1;
  } else {
    leftColsToAdd = firstColumn() - fc;
    rightColsToAdd = lc - lastColumn();
  }

  // Remove columns
  for (int i = 0; i < -leftColsToAdd; ++i)
    removeSection(Left);
  for (int i = 0; i < -rightColsToAdd; ++i)
    removeSection(Right);

  // Remove rows
  for (int i = 0; i < -topRowsToAdd; ++i)
    removeSection(Top);
  for (int i = 0; i < -bottomRowsToAdd; ++i)
    removeSection(Bottom);

  // Add rows
  for (int i = 0; i < topRowsToAdd; i++) {
    int row = firstRow() - 1;

    std::vector<WWidget *> items;
    for (int j = firstColumn(); j <= lastColumn(); ++j)
      items.push_back(renderWidget(0, model()->index(row, j, rootIndex())));

    addSection(Top, items);
  }

  for (int i = 0; i < bottomRowsToAdd; ++i) {
    int row = lastRow() + 1;

    std::vector<WWidget *> items;
    for (int j = firstColumn(); j <= lastColumn(); ++j)
      items.push_back(renderWidget(0, model()->index(row, j, rootIndex())));

    addSection(Bottom, items);
  }

  // Add columns
  for (int i = 0; i < leftColsToAdd; ++i) {
    int col = firstColumn() - 1;

    std::vector<WWidget *> items;
    for (int j = firstRow(); j <= lastRow(); ++j)
      items.push_back(renderWidget(0, model()->index(j, col, rootIndex())));

    addSection(Left, items);
  }

  for (int i = 0; i < rightColsToAdd; ++i) {
    int col = lastColumn() + 1;

    std::vector<WWidget *> items;
    for (int j = firstRow(); j <= lastRow(); ++j)
      items.push_back(renderWidget(0, model()->index(j, col, rootIndex())));

    addSection(Right, items);
  }

  updateColumnOffsets();

  int scrollX1 = std::max(0, viewportLeft_ - viewportWidth_ / 2);
  int scrollX2 = viewportLeft_ + viewportWidth_ + viewportWidth_ / 2;
  int scrollY1 = std::max(0, viewportTop_ - viewportHeight_ / 2);
  int scrollY2 = viewportTop_ + viewportHeight_ + viewportHeight_ / 2;

  SStream s;

  s << "jQuery.data(" << jsRef() << ", 'obj').scrolled("
    << scrollX1 << ", " << scrollX2 << ", " << scrollY1 << ", " << scrollY2
    << ");";

  WApplication::instance()->doJavaScript(s.str());			
}

void WTableView::resetGeometry()
{
  if (ajaxMode()) {
    reset();
  } else { // plain HTML
    renderedLastRow_
      = std::min(model()->rowCount(rootIndex()) - 1,
		 renderedFirstRow_ + pageSize() - 1);
    renderedLastColumn_ = model()->columnCount(rootIndex()) - 1;
  }
}

double WTableView::canvasHeight() const
{
  return std::max(1.0,
		  model()->rowCount(rootIndex()) * rowHeight().toPixels());
}

void WTableView::reset()
{
  assert(ajaxMode());

  int total = 0;
  for (int i = 0; i < columnCount(); ++i)
    if (!columnInfo(i).hidden)
      total += (int)columnInfo(i).width.toPixels() + 7;

  headers_->resize(total, headers_->height());
  canvas_->resize(total, canvasHeight());

  computeRenderedArea();

  int renderedRows = lastRow() - firstRow() + 1;
  for (int i = 0; i < renderedRows; ++i)
    removeSection(Top);

  setSpannerCount(Top, 0);
  setSpannerCount(Left, 0);

  table_->clear();

  setSpannerCount(Bottom, model()->rowCount(rootIndex()));
  setSpannerCount(Right, columnCount());
}

void WTableView::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  const char *THIS_JS = "js/WTableView.js";

  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WTableView", wtjs1);
    app->setJavaScriptLoaded(THIS_JS);
  }

  app->doJavaScript("new " WT_CLASS ".WTableView("
		    + app->javaScriptClass() + "," + jsRef() + ","
		    + contentsContainer_->jsRef() + ","
		    + headerContainer_->jsRef() + ");");
}

void WTableView::render(WFlags<RenderFlag> flags)
{
  if (ajaxMode() && (flags & RenderFull))
    defineJavaScript();

  if (model())
    while (renderState_ != RenderOk) {
      RenderState s = renderState_;
      renderState_ = RenderOk;

      switch (s) {
      case NeedRerender:
	resetGeometry();
	rerenderHeader();
	rerenderData();
	break;
      case NeedRerenderHeader:
	rerenderHeader();
	break;
      case NeedRerenderData:
	rerenderData();
	break;
      case NeedAdjustViewPort:
	adjustToViewport();
	break;
      default:
	break;
      }
    }

  WAbstractItemView::render(flags);
}

void WTableView::rerenderData()
{
  if (ajaxMode()) {
    reset();

    renderTable(renderedFirstRow_, 
		renderedLastRow_, 
		renderedFirstColumn_, 
		renderedLastColumn_);
  } else {
    pageChanged().emit();

    while (plainTable_->rowCount() > 1)
      plainTable_->deleteRow(plainTable_->rowCount() - 1);

    for (int i = firstRow(); i <= lastRow(); ++i) {
      int renderedRow = i - firstRow();

      if (selectionBehavior() == SelectRows
	  && isSelected(model()->index(i, 0, rootIndex()))) {
	WTableRow *row = plainTable_->rowAt(renderedRow + 1);
	row->setStyleClass("Wt-selected");
      }

      for (int j = firstColumn(); j <= lastColumn(); ++j) {
	int renderedCol = j - firstColumn();

	WModelIndex index = model()->index(i, j, rootIndex());
	WWidget *w = renderWidget(0, index);
	WTableCell *cell = plainTable_->elementAt(renderedRow + 1, renderedCol);
	cell->addWidget(w);

	WInteractWidget *wi = dynamic_cast<WInteractWidget *>(w);
	if (wi && !isEditing(index))
	  clickedMapper_->mapConnect1(wi->clicked(), index);

	if (selectionBehavior() == SelectItems && isSelected(index))
	  cell->setStyleClass("Wt-selected");
      }
    }
  }
}

void WTableView::rerenderHeader()
{
  for (int i = 0; i < columnCount(); ++i) {
    WWidget *w = columnInfo(i).extraHeaderWidget;
    if (!w)
      columnInfo(i).extraHeaderWidget = createExtraHeaderWidget(i);
    else
      dynamic_cast<WContainerWidget *>(w->parent())->removeWidget(w);
  }

  WApplication *app = WApplication::instance();

  if (ajaxMode()) {
    headers_->clear();

    for (int i = 0; i < columnCount(); ++i) {
      WWidget *w = createHeaderWidget(app, i);
      w->setFloatSide(Left);
      headers_->addWidget(w);
      w->resize(columnInfo(i).width.toPixels() + 1, WLength::Auto);
      if (columnInfo(i).hidden)
	w->hide();
    }
  } else { // Plain HTML mode
    for (int i = 0; i < columnCount(); ++i) {
      WWidget *w = createHeaderWidget(app, i);
      WTableCell *cell = plainTable_->elementAt(0, i);
      cell->addWidget(w);
      w->resize(columnInfo(i).width.toPixels() + 1, WLength::Auto);
      cell->resize(columnInfo(i).width.toPixels() + 1, w->height());
      if (columnInfo(i).hidden)
	cell->hide();
    }
  }

  if (model())
    modelHeaderDataChanged(Horizontal, 0, columnCount() - 1);
}

void WTableView::setColumnHidden(int column, bool hidden)
{
  if (columnInfo(column).hidden != hidden) {
    WAbstractItemView::setColumnHidden(column, hidden);

    int delta = static_cast<int>(columnInfo(column).width.toPixels()) + 7;
    if (hidden)
      delta = -delta;

    headers_->resize(headers_->width().toPixels() + delta, headers_->height());
    canvas_->resize(canvas_->width().toPixels() + delta, canvas_->height());

    if (isColumnRendered(column))
      updateColumnOffsets();
    else
      if (column < firstColumn())
	setSpannerCount(Left, spannerCount(Left));

    WWidget *hc = headers_->widget(column);
    if (!ajaxMode())
      hc->parent()->setHidden(hidden);
    else
      hc->setHidden(hidden);
  }
}

void WTableView::setColumnWidth(int column, const WLength& width)
{
  int delta = (int)(width.toPixels() - columnInfo(column).width.toPixels());
  columnInfo(column).width = width;

  if (columnInfo(column).hidden)
    delta = 0;

  if (ajaxMode()) {
    headers_->resize(headers_->width().toPixels() + delta, headers_->height());
    canvas_->resize(canvas_->width().toPixels() + delta, canvas_->height());

    if (renderState_ >= NeedRerenderHeader)
      return;

    if (isColumnRendered(column))
      updateColumnOffsets();
    else
      if (column < firstColumn())
	setSpannerCount(Left, spannerCount(Left));
  }

  if (renderState_ >= NeedRerenderHeader)
    return;

  WWidget *hc = headers_->widget(column);
  hc->resize(width.toPixels() + 1, hc->height());
  if (!ajaxMode())
    hc->parent()->resize(width.toPixels() + 1, hc->height());
}

bool WTableView::isRowRendered(const int row)
{
  return row >= firstRow() && row <= lastRow();
}

bool WTableView::isColumnRendered(const int column)
{
  return column >= firstColumn() && column <= lastColumn();
}

WTableView::ColumnWidget::ColumnWidget(WTableView *view, int column)
  : column_(column)
{
  assert(view->ajaxMode());

  WTableView::ColumnInfo& ci = view->columnInfo(column);
  setStyleClass(ci.styleClass());
  setPositionScheme(Absolute);
  setOffsets(0, Top | Left);
  setOverflow(OverflowHidden);

  if (view->table_->count() == 0
      || column > view->columnContainer(-1)->column())
    view->table_->addWidget(this);
  else
    view->table_->insertWidget(0, this);
}

WTableView::ColumnWidget *WTableView::columnContainer(int renderedColumn) const
{
  assert(ajaxMode());

  if (table_->count() > 0) {
    if (renderedColumn < 0)
      return dynamic_cast<ColumnWidget *>(table_->widget(table_->count() - 1));
    else
      return dynamic_cast<ColumnWidget *>(table_->widget(renderedColumn));
  } else
    return 0;
}

void WTableView::updateColumnOffsets()
{
  assert(ajaxMode());

  int fc = firstColumn();
  int lc = lastColumn();

  int totalRendered = 0, total = 0;
  for (int i = 0; i < columnCount(); ++i) {
    ColumnInfo ci = columnInfo(i);

    if (i >= fc && i <= lc) {
      ColumnWidget *w = columnContainer(i - fc);

      w->setOffsets(totalRendered, Left);
      w->resize(ci.width.toPixels() + 7, WLength::Auto);

      if (!columnInfo(i).hidden)
	totalRendered += (int)columnInfo(i).width.toPixels() + 7;

      w->setHidden(columnInfo(i).hidden);
    }

    if (!columnInfo(i).hidden)
      total += (int)columnInfo(i).width.toPixels() + 7;
  }

  canvas_->resize(total, canvasHeight());
  headers_->resize(total, headers_->height());
}

void WTableView::setRowHeight(const WLength& rowHeight)
{
  int renderedRowCount = model() ? lastRow() - firstRow() + 1 : 0;

  WAbstractItemView::setRowHeight(rowHeight);

  if (ajaxMode()) {
    canvas_->setAttributeValue("style", "line-height: " + rowHeight.cssText());

    if (model()) {
      canvas_->resize(canvas_->width(), canvasHeight());
      table_->resize(table_->width(), renderedRowCount * rowHeight.toPixels());
    }
  } else // Plain HTML mode
    resize(width(), height());

  updateTableBackground();

  scheduleRerender(NeedRerenderData);
}

void WTableView::setHeaderHeight(const WLength& height, bool multiLine)
{
  WAbstractItemView::setHeaderHeight(height, multiLine);

  if (!ajaxMode())
    resize(this->width(), this->height());

  if (renderState_ >= NeedRerenderHeader)
    return;

  if (!WApplication::instance()->environment().agentIsIE())
    for (int i = 0; i < columnCount(); ++i)
      headerTextWidget(i)->setWordWrap(multiLine);
}

WWidget* WTableView::headerWidget(int column, bool contentsOnly)
{
  WWidget *result = 0;

  if (ajaxMode()) {
    if (headers_ && column < headers_->count())
      result = headers_->widget(column);
  } else
    if (plainTable_ && column < plainTable_->columnCount())
      result = plainTable_->elementAt(0, column)->widget(0);

  if (result && contentsOnly)
    return result->find("contents");
  else
    return result;
}

void WTableView::setColumnBorder(const WColor& color)
{
  // FIXME
}

void WTableView::shiftModelIndexes(int start, int count)
{
  WModelIndexSet& set = selectionModel()->selection_;
  
  std::vector<WModelIndex> toShift;
  std::vector<WModelIndex> toErase;

  for (WModelIndexSet::iterator it
	 = set.lower_bound(model()->index(start, 0, rootIndex()));
       it != set.end(); ++it) {

    if (count < 0) {
      if ((*it).row() < start - count) {
	toErase.push_back(*it);
	continue;
      }
    }

    toShift.push_back(*it);
    toErase.push_back(*it);
  }

  for (unsigned i = 0; i < toErase.size(); ++i)
    set.erase(toErase[i]);

  for (unsigned i = 0; i < toShift.size(); ++i) {
    WModelIndex newIndex = model()->index(toShift[i].row() + count,
					 toShift[i].column(),
					 toShift[i].parent());
    set.insert(newIndex);
  }
}

void WTableView::modelColumnsInserted(const WModelIndex& parent, 
				      int start, int end)
{
  if (parent != rootIndex())
    return;

  int count = end - start + 1;
  int width = 0;

  for (int i = start; i < start + count; ++i) {
    columns_.insert(columns_.begin() + i, createColumnInfo(i));
    width += (int)columnInfo(i).width.toPixels() + 7;
  }

  if (ajaxMode())
    canvas_->resize(canvas_->width().toPixels() + width, canvas_->height());

  if (renderState_ < NeedRerenderHeader)
    scheduleRerender(NeedRerenderHeader);

  if (start > lastColumn() || 
      renderState_ == NeedRerender || 
      renderState_ == NeedRerenderData)
    return;

  scheduleRerender(NeedRerenderData);
}

void WTableView::modelColumnsAboutToBeRemoved(const WModelIndex& parent, 
					      int start, int end)
{
  if (parent != rootIndex())
    return;

  for (int r = 0; r < model()->rowCount(); r++) {
    for (int c = start; c <= end; c++) {
      closeEditor(model()->index(r, c), false);
    }
  }

  int count = end - start + 1;
  int width = 0;

  for (int i = start; i < start + count; ++i)
    if (!columnInfo(i).hidden)
      width += (int)columnInfo(i).width.toPixels() + 7;

  columns_.erase(columns_.begin() + start, columns_.begin() + start + count);

  if (ajaxMode())
    canvas_->resize(canvas_->width().toPixels() - width, canvas_->height());

  if (start <= currentSortColumn_ && currentSortColumn_ <= end)
    currentSortColumn_ = -1;

  if (renderState_ < NeedRerenderHeader)
    scheduleRerender(NeedRerenderHeader);

  if (start > lastColumn() || 
      renderState_ == NeedRerender || 
      renderState_ == NeedRerenderData)
    return;

  scheduleRerender(NeedRerenderData);
}

void WTableView::modelRowsInserted(const WModelIndex& parent, 
				   int start, int end)
{
  if (parent != rootIndex())
    return;

  shiftModelIndexes(start, end - start + 1);

  if (ajaxMode()) {
    canvas_->resize(canvas_->width(), canvasHeight());
    scheduleRerender(NeedAdjustViewPort);
  }

  computeRenderedArea();

  if (start <= lastRow())
    scheduleRerender(NeedRerenderData);
}

void WTableView::modelRowsAboutToBeRemoved(const WModelIndex& parent,
					   int start, int end)
{
  if (parent != rootIndex())
    return;

  for (int c = 0; c < model()->columnCount(); c++) {
    for (int r = start; r <= end; r++) {
      closeEditor(model()->index(r, c), false);
    }
  }
}

void WTableView::modelRowsRemoved(const WModelIndex& parent, int start, int end)
{
  if (parent != rootIndex())
    return;

  shiftModelIndexes(start, -(end - start + 1));

  if (ajaxMode()) {
    canvas_->resize(canvas_->width(), canvasHeight());
    scheduleRerender(NeedAdjustViewPort);
  }

  computeRenderedArea();

  if (start <= lastRow())
    scheduleRerender(NeedRerenderData);
}


void WTableView::modelDataChanged(const WModelIndex& topLeft, 
				  const WModelIndex& bottomRight)
{
  if (topLeft.parent() != rootIndex())
    return;

  if (renderState_ < NeedRerenderData) {
    int row1 = std::max(topLeft.row(), firstRow());
    int row2 = std::min(bottomRight.row(), lastRow());
    int col1 = std::max(topLeft.column(), firstColumn());
    int col2 = std::min(bottomRight.column(), lastColumn());
    for (int i = row1; i <= row2; ++i)
      for (int j = col1; j <= col2; ++j) {
	int renderedRow = i - firstRow();
	int renderedColumn = j - firstColumn();

	WContainerWidget *parentWidget;
	WWidget *w;
	int wIndex;

	if (ajaxMode()) {
	  parentWidget = columnContainer(renderedColumn);
	  wIndex = renderedRow;
	} else {
	  parentWidget
	    = plainTable_->elementAt(renderedRow + 1, renderedColumn);
	  wIndex = 0;
	}

	w = parentWidget->widget(wIndex);

	WModelIndex index = model()->index(i, j, rootIndex());

	w = renderWidget(w, index);

	if (!w->parent()) {
	  parentWidget->insertWidget(wIndex, w);

	  if (!ajaxMode() && !isEditing(index)) {
	    WInteractWidget *wi = dynamic_cast<WInteractWidget *>(w);
	    if (wi)
	      clickedMapper_->mapConnect1(wi->clicked(), index);
	  }
	}
      }
  }
}
 
void WTableView::modelHeaderDataChanged(Orientation orientation, 
					int start, int end)
{
  if (renderState_ < NeedRerenderHeader) {
    if (orientation == Horizontal) {
      for (int i = start; i <= end; ++i) {
	WString label = asString(model()->headerData(i));
	headerTextWidget(i)->setText(label);
      }
    }
  }
}

void WTableView::onViewportChange(int left, int top, int width, int height)
{
  assert(ajaxMode());

  viewportLeft_ = left;
  viewportWidth_ = width;
  viewportTop_ = top;
  viewportHeight_ = height;

  computeRenderedArea();

  scheduleRerender(NeedAdjustViewPort);  
}

void WTableView::computeRenderedArea()
{
  if (ajaxMode()) {
    const int borderRows = 5;
    const int borderColumnPixels = 200;

    /* row range */
    int top = std::min(viewportTop_,
		       static_cast<int>(canvas_->height().toPixels()));
    int height = std::min(viewportHeight_,
			  static_cast<int>(canvas_->height().toPixels()));

    renderedFirstRow_ = static_cast<int>(top / rowHeight().toPixels());

    int renderedRows = static_cast<int>(height / rowHeight().toPixels() + 0.5);

    if (model())
      renderedLastRow_
	= std::min(renderedFirstRow_ + renderedRows * 2 + borderRows,
		   model()->rowCount(rootIndex()) - 1);
    renderedFirstRow_
      = std::max(renderedFirstRow_ - renderedRows - borderRows, 0);

    if (renderedFirstRow_ % 2 == 1)
      --renderedFirstRow_;

    /* column range */
    int left
      = std::max(0, viewportLeft_ - viewportWidth_ - borderColumnPixels);
    int right
      = std::min(static_cast<int>(canvas_->width().toPixels()),
		 viewportLeft_ + 2 * viewportWidth_ + borderColumnPixels);

    int total = 0;
    renderedFirstColumn_ = 0;
    renderedLastColumn_ = columnCount() - 1;
    for (int i = 0; i < columnCount(); i++) {
      int w = static_cast<int>(columnInfo(i).width.toPixels());

      if (total <= left && left < total + w)
	renderedFirstColumn_ = i;

      if (total <= right && right < total + w) {
	renderedLastColumn_ = i;
	break;
      }

      total += w + 7;
    }

    assert(renderedLastColumn_ == -1
	   || renderedFirstColumn_ <= renderedLastColumn_);
  } else { // Plain HTML
    renderedFirstColumn_ = 0;
    if (model()) {
      renderedLastColumn_ = model()->columnCount(rootIndex()) - 1;

      int cp = std::max(0, std::min(currentPage(), pageCount() - 1));
      setCurrentPage(cp);
    } else
      renderedFirstRow_ = renderedLastRow_ = 0;
  }
}

void WTableView::adjustToViewport()
{
  assert(ajaxMode());

  if (renderedFirstRow_ != firstRow() || 
      renderedLastRow_ != lastRow() ||
      renderedFirstColumn_ != firstColumn()||
      renderedLastColumn_ != lastColumn()) {
    renderTable(renderedFirstRow_, 
		renderedLastRow_, 
		renderedFirstColumn_, 
		renderedLastColumn_);
  }
}

void WTableView::setAlternatingRowColors(bool enable)
{
  WAbstractItemView::setAlternatingRowColors(enable);
  updateTableBackground();
}

void WTableView::handleSingleClick(const WMouseEvent& event)
{
  WModelIndex index = translateModelIndex(event);
  if (index.isValid())
    WAbstractItemView::handleClick(index, event);
}

void WTableView::handleDoubleClick(const WMouseEvent& event)
{
  WModelIndex index = translateModelIndex(event);
  if (index.isValid())
    WAbstractItemView::handleDoubleClick(index, event);
}

void WTableView::handleMouseWentDown(const WMouseEvent& event)
{
  WModelIndex index = translateModelIndex(event);
  if (index.isValid())
    WAbstractItemView::handleMouseDown(index, event);
}

void WTableView::modelLayoutChanged()
{
  WAbstractItemView::modelLayoutChanged();

  resetGeometry();
}

WModelIndex WTableView::translateModelIndex(const WMouseEvent& event)
{
  int row = (int)(event.widget().y / rowHeight().toPixels());
  int column = -1;

  int total = 0;
  for (int i = 0; i < columnCount(); i++) {
    if (!columnInfo(i).hidden)
      total += static_cast<int>(columnInfo(i).width.toPixels()) + 7;

    if (event.widget().x < total) {
      column = i;
      break;
    }
  }

  if (column >= 0 && row >= 0 && row < model()->rowCount(rootIndex()))
    return model()->index(row, column, rootIndex());
  else
    return WModelIndex();
}

bool WTableView::internalSelect(const WModelIndex& index, SelectionFlag option)
{
  if (selectionBehavior() == SelectRows && index.column() != 0)
    return internalSelect(model()->index(index.row(), 0, index.parent()),
			  option);

  if (WAbstractItemView::internalSelect(index, option)) {
    renderSelected(isSelected(index), index);
    return true;
  } else
    return false;
}

void WTableView::renderSelected(bool selected, const WModelIndex& index)
{
  if (selectionBehavior() == SelectRows) {
    if (isRowRendered(index.row())) {
      int renderedRow = index.row() - firstRow();

      if (ajaxMode()) {
	for (int i = 0; i < table_->count(); ++i) {
	  ColumnWidget *column = columnContainer(i);
	  WWidget *w = column->widget(renderedRow);
	  if (selected)
	    w->addStyleClass("Wt-selected");
	  else
	    w->removeStyleClass("Wt-selected");
	}
      } else {
	WTableRow *row = plainTable_->rowAt(renderedRow + 1);
	row->setStyleClass(selected ? "Wt-selected" : "");
      }
    }
  } else {
    if (isRowRendered(index.row()) && isColumnRendered(index.column())) {
      int renderedRow = index.row() - firstRow();
      int renderedCol = index.column() - firstColumn();

      WWidget *w = 0;
      if (ajaxMode()) {
	ColumnWidget *column = columnContainer(renderedCol);
	w = column->widget(renderedRow);
      } else {
	w = plainTable_->elementAt(renderedRow + 1, renderedCol);
      }

      if (selected)
	w->addStyleClass("Wt-selected");
      else
	w->removeStyleClass("Wt-selected");
    }
  }
}

void WTableView::selectRange(const WModelIndex& first, const WModelIndex& last)
{
  for (int c = first.column(); c <= last.column(); ++c)
    for (int r = first.row(); r <= last.row(); ++r)
      internalSelect(model()->index(r, c, rootIndex()), Select);
}

void WTableView::onDropEvent(int renderedRow, int columnId,
			     std::string sourceId, std::string mimeType,
			     WMouseEvent event)
{
  WDropEvent e(WApplication::instance()->decodeObject(sourceId), mimeType,
	       event);

  WModelIndex index = model()->index(firstRow() + renderedRow,
				    columnById(columnId), rootIndex());

  dropEvent(e, index);
}

void WTableView::setCurrentPage(int page)
{
  renderedFirstRow_ = page * pageSize();

  if (model())
    renderedLastRow_= std::min(renderedFirstRow_ + pageSize() - 1,
			       model()->rowCount(rootIndex()) - 1);
  else
    renderedLastRow_ = renderedFirstRow_;
 
  scheduleRerender(NeedRerenderData);
}

int WTableView::currentPage() const
{
  return renderedFirstRow_ / pageSize();
}

int WTableView::pageCount() const
{
  if (model()) {
    return (model()->rowCount(rootIndex()) - 1) / pageSize() + 1;
  } else
    return 1;
}

int WTableView::pageSize() const
{
  if (height().isAuto())
    return 10000;
  else {
    const int navigationBarHeight = 25; // set in wt.css

    return static_cast<int>
      ((height().toPixels() - headerHeight().toPixels() - navigationBarHeight)
       / rowHeight().toPixels());
  }
}

}
