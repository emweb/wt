/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WTableView"

#include "Wt/WAbstractItemDelegate"
#include "Wt/WApplication"
#include "Wt/WAbstractItemModel"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WGridLayout"
#include "Wt/WModelIndex"
#include "Wt/WStringStream"
#include "Wt/WTable"
#include "Wt/WTheme"

#ifndef WT_DEBUG_JS

#include "js/WTableView.min.js"
#endif

#define UNKNOWN_VIEWPORT_HEIGHT 800
#define CONTENTS_VIEWPORT_HEIGHT -1

#include <cmath>
#include <math.h>

#if defined(_MSC_VER) && (_MSC_VER < 1800)
namespace {
  double round(double x)
  {
    return floor(x + 0.5);
  }
}
#endif

namespace Wt {

LOGGER("WTableView");

WTableView::WTableView(WContainerWidget *parent)
  : WAbstractItemView(parent),
    headers_(0),
    canvas_(0),
    table_(0),
    headerContainer_(0),
    contentsContainer_(0),
    headerColumnsCanvas_(0),
    headerColumnsTable_(0),
    headerColumnsHeaderContainer_(0),
    headerColumnsContainer_(0),
    plainTable_(0),
    dropEvent_(impl_, "dropEvent"),
    scrolled_(impl_, "scrolled"),
    firstColumn_(-1),
    lastColumn_(-1),
    viewportLeft_(0),
    viewportWidth_(1000),
    viewportTop_(0),
    viewportHeight_(UNKNOWN_VIEWPORT_HEIGHT),
    scrollToRow_(-1),
    scrollToHint_(EnsureVisible),
    columnResizeConnected_(false)
{
  setSelectable(false);

  setStyleClass("Wt-itemview Wt-tableview");

  WApplication *app = WApplication::instance();

  if (app->environment().ajax()) {
    impl_->setPositionScheme(Relative);

    headers_ = new WContainerWidget();
    headers_->setStyleClass("Wt-headerdiv headerrh");

    table_ = new WContainerWidget();
    table_->setStyleClass("Wt-tv-contents");
    table_->setPositionScheme(Absolute);
    table_->setWidth(WLength(100, WLength::Percentage));

    WGridLayout *layout = new WGridLayout();
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    headerContainer_ = new WContainerWidget();
    headerContainer_->setStyleClass("Wt-header headerrh");
    headerContainer_->setOverflow(WContainerWidget::OverflowHidden);
    headerContainer_->addWidget(headers_);

    canvas_ = new WContainerWidget();
    canvas_->setStyleClass("Wt-spacer");
    canvas_->setPositionScheme(Relative);
    canvas_->clicked()
      .connect(boost::bind(&WTableView::handleSingleClick, this, false, _1));
    canvas_->clicked().connect("function(o, e) { "
			       "$(document).trigger('click', e);"
			       "}");
    canvas_->clicked().preventPropagation();
    canvas_->mouseWentDown()
      .connect(boost::bind(&WTableView::handleMouseWentDown, this, false, _1)); 
    canvas_->mouseWentDown().preventPropagation();
    canvas_->mouseWentDown().connect("function(o, e) { "
				     "$(document).trigger('mousedown', e);"
				     "}");
    canvas_->mouseWentUp()
      .connect(boost::bind(&WTableView::handleMouseWentUp, this, false, _1)); 
    canvas_->mouseWentUp().preventPropagation();
    canvas_->addWidget(table_);

    contentsContainer_ = new WContainerWidget();
    contentsContainer_->setOverflow(WContainerWidget::OverflowAuto);
    contentsContainer_->setPositionScheme(Absolute);
    contentsContainer_->addWidget(canvas_);

    contentsContainer_->clicked()
      .connect(boost::bind(&WTableView::handleRootSingleClick, this, 0, _1));
    contentsContainer_->mouseWentUp()
      .connect(boost::bind(&WTableView::handleRootMouseWentUp, this, 0, _1));

    headerColumnsHeaderContainer_ = new WContainerWidget();
    headerColumnsHeaderContainer_->setStyleClass("Wt-header Wt-headerdiv "
						 "headerrh");
    headerColumnsHeaderContainer_->hide();

    headerColumnsTable_ = new WContainerWidget();
    headerColumnsTable_->setStyleClass("Wt-tv-contents");
    headerColumnsTable_->setPositionScheme(Absolute);
    headerColumnsTable_->setWidth(WLength(100, WLength::Percentage));

    headerColumnsCanvas_ = new WContainerWidget();
    headerColumnsCanvas_->setPositionScheme(Relative);
    headerColumnsCanvas_->clicked()
      .connect(boost::bind(&WTableView::handleSingleClick, this, true, _1));
    headerColumnsCanvas_->mouseWentDown()
      .connect(boost::bind(&WTableView::handleMouseWentDown, this, true, _1)); 
    headerColumnsCanvas_->mouseWentUp()
      .connect(boost::bind(&WTableView::handleMouseWentUp, this, true, _1)); 
    headerColumnsCanvas_->addWidget(headerColumnsTable_);

    headerColumnsContainer_ = new WContainerWidget();
    headerColumnsContainer_->setPositionScheme(Absolute);
    headerColumnsContainer_->setOverflow(WContainerWidget::OverflowHidden);
    headerColumnsContainer_->addWidget(headerColumnsCanvas_);
    headerColumnsContainer_->hide();

    headerColumnsContainer_->clicked()
      .connect(boost::bind(&WTableView::handleRootSingleClick, this, 0, _1));
    headerColumnsContainer_->mouseWentUp()
      .connect(boost::bind(&WTableView::handleRootMouseWentUp, this, 0, _1)); 

    layout->addWidget(headerColumnsHeaderContainer_, 0, 0);
    layout->addWidget(headerContainer_, 0, 1);
    layout->addWidget(headerColumnsContainer_, 1, 0);
    layout->addWidget(contentsContainer_, 1, 1);

    for (int i = 0; i < layout->count(); ++i)
      layout->itemAt(i)->widget()->addStyleClass("tcontainer");

    layout->setRowStretch(1, 1);
    layout->setColumnStretch(1, 1);

    impl_->setLayout(layout);
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
    if (height.unit() == WLength::Percentage) {
      LOG_ERROR("resize(): height cannot be a Percentage");
      return;
    }

    if (!height.isAuto()) {
      viewportHeight_
	= static_cast<int>(std::ceil((height.toPixels()
				      - headerHeight().toPixels())));
      if (scrollToRow_ != -1) {
	WModelIndex index = model()->index(scrollToRow_, 0, rootIndex());
	scrollToRow_ = -1;
	scrollTo(index, scrollToHint_);
      }
    } else
      viewportHeight_ = UNKNOWN_VIEWPORT_HEIGHT;
  } else { // Plain HTML mode
    if (!plainTable_) // Not yet rendered
      return;

    plainTable_->setWidth(width);

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
  if (ajaxMode()) {
    WApplication::instance()->theme()->apply(this, table_,
					     TableViewRowContainerRole);
    WApplication::instance()->theme()->apply(this, headerColumnsTable_,
					     TableViewRowContainerRole);
  } else
    // FIXME avoid background on header row ?
    WApplication::instance()->theme()->apply(this, plainTable_,
					     TableViewRowContainerRole);
}

void WTableView::setModel(WAbstractItemModel* model)
{  
  WAbstractItemView::setModel(model);

  typedef WTableView Self;

  /* connect slots to new model */
  modelConnections_.push_back(model->columnsInserted().connect
			      (this, &Self::modelColumnsInserted));
  modelConnections_.push_back(model->columnsAboutToBeRemoved().connect
			      (this, &Self::modelColumnsAboutToBeRemoved));
  modelConnections_.push_back(model->rowsInserted().connect
			      (this, &Self::modelRowsInserted));
  modelConnections_.push_back(model->rowsAboutToBeRemoved().connect
			      (this, &Self::modelRowsAboutToBeRemoved));
  modelConnections_.push_back(model->rowsRemoved().connect
			      (this, &Self::modelRowsRemoved));
  modelConnections_.push_back(model->dataChanged().connect
			      (this, &Self::modelDataChanged));
  modelConnections_.push_back(model->headerDataChanged().connect
			      (this, &Self::modelHeaderDataChanged));
  modelConnections_.push_back(model->layoutAboutToBeChanged().connect
			      (this, &Self::modelLayoutAboutToBeChanged));
  modelConnections_.push_back(model->layoutChanged().connect
			      (this, &Self::modelLayoutChanged));
  modelConnections_.push_back(model->modelReset().connect
			      (this, &Self::modelReset));

  firstColumn_ = lastColumn_ = -1;
  adjustSize();
}

WWidget* WTableView::renderWidget(WWidget* widget, const WModelIndex& index) 
{
  WAbstractItemDelegate *itemDelegate = this->itemDelegate(index.column());

  WFlags<ViewItemRenderFlag> renderFlags = 0;

  if (ajaxMode()) {
    if (isSelected(index))
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
  widget->setHeight(rowHeight());

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
    return firstColumn_; // headers are included
  case Right:
    return columnCount() - (lastColumn_ + 1);
  default:
    assert(false);
    return -1;
  }
}

void WTableView::setRenderedHeight(double th)
{
  table_->setHeight(th);
  headerColumnsTable_->setHeight(th);
  for (int i = 0; i < renderedColumnsCount(); ++i) {
    ColumnWidget *w = columnContainer(i);
    w->setHeight(th);
  }
}

void WTableView::setSpannerCount(const Side side, const int count)
{
  assert(ajaxMode());

  switch (side) {
  case Top: {
    int size = model()->rowCount(rootIndex()) - count - spannerCount(Bottom);

    double to = count * rowHeight().toPixels();
    table_->setOffsets(to, Top);
    headerColumnsTable_->setOffsets(to, Top);

    double th = size * rowHeight().toPixels();
    setRenderedHeight(th);
    break;
  }
  case Bottom: {
    int size = model()->rowCount(rootIndex()) - spannerCount(Top) - count;
    double th = size * rowHeight().toPixels();
    setRenderedHeight(th);
    break;
  }
  case Left: {
    int total = 0;
    for (int i = rowHeaderCount(); i < count; i++)
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
    return columnCount() - 1;
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
  persistEditor(model()->index(row, col, rootIndex()));
  delete w;
}

void WTableView::removeSection(const Side side)
{
  assert(ajaxMode());

  int row = firstRow(), col = firstColumn();

  switch (side) {
  case Top:
    setSpannerCount(side, spannerCount(side) + 1);

    for (int i = 0; i < renderedColumnsCount(); ++i) {
      ColumnWidget *w = columnContainer(i);
      deleteItem(row, col + i, w->widget(0));
    }
    break;
  case Bottom:
    row = lastRow();
    setSpannerCount(side, spannerCount(side) + 1);

    for (int i = 0; i < renderedColumnsCount(); ++i) {
      ColumnWidget *w = columnContainer(i);
      deleteItem(row, col + i, w->widget(w->count() - 1));
    }
    break;
  case Left: {
    ColumnWidget *w = columnContainer(rowHeaderCount());

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
    setSpannerCount(Bottom, model()->rowCount(rootIndex()) - fr);
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
    setSpannerCount(Right, columnCount() - fc);
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
    for (int j = 0; j < rowHeaderCount(); ++j)
      items.push_back(renderWidget(0, model()->index(row, j, rootIndex())));
    for (int j = firstColumn(); j <= lastColumn(); ++j)
      items.push_back(renderWidget(0, model()->index(row, j, rootIndex())));

    addSection(Top, items);
  }

  for (int i = 0; i < bottomRowsToAdd; ++i) {
    int row = lastRow() + 1;

    std::vector<WWidget *> items;
    for (int j = 0; j < rowHeaderCount(); ++j)
      items.push_back(renderWidget(0, model()->index(row, j, rootIndex())));
    for (int j = firstColumn(); j <= lastColumn(); ++j)
      items.push_back(renderWidget(0, model()->index(row, j, rootIndex())));

    addSection(Bottom, items);
  }

  // Add columns
  for (int i = 0; i < leftColsToAdd; ++i) {
    int col = firstColumn() - 1;

    std::vector<WWidget *> items;
    int nfr = firstRow(), nlr = lastRow();
    for (int j = nfr; j <= nlr; ++j)
      items.push_back(renderWidget(0, model()->index(j, col, rootIndex())));

    addSection(Left, items);
  }

  for (int i = 0; i < rightColsToAdd; ++i) {
    int col = lastColumn() + 1;

    std::vector<WWidget *> items;
    int nfr = firstRow(), nlr = lastRow();
    for (int j = nfr; j <= nlr; ++j)
      items.push_back(renderWidget(0, model()->index(j, col, rootIndex())));

    addSection(Right, items);
  }

  updateColumnOffsets();

  // assert(lastRow() == lr && firstRow() == fr);

  int scrollX1 = std::max(0, viewportLeft_ - viewportWidth_ / 2);
  int scrollX2 = viewportLeft_ + viewportWidth_ / 2;
  int scrollY1 = std::max(0, viewportTop_ - viewportHeight_ / 2);
  int scrollY2 = viewportTop_ + viewportHeight_ / 2;

  WStringStream s;

  s << "jQuery.data(" << jsRef() << ", 'obj').scrolled("
    << scrollX1 << ", " << scrollX2 << ", " << scrollY1 << ", " << scrollY2
    << ");";

  doJavaScript(s.str());			
}

void WTableView::setHidden(bool hidden, const WAnimation& animation)
{
  bool change = isHidden() != hidden;

  WAbstractItemView::setHidden(hidden, animation);

  if (change && !hidden) {
    /*
     * IE9 reset the scroll position to (0, 0) when display changes from
     * 'none' to ''
     */
    WApplication *app = WApplication::instance();
    if (app->environment().ajax() && isRendered()
	&& app->environment().agentIsIE()
	&& !app->environment().agentIsIElt(9)) {
      WStringStream s;
      s << "jQuery.data(" << jsRef() << ", 'obj').resetScroll();";
      doJavaScript(s.str());
    }
  }
}

void WTableView::resetGeometry()
{
  if (ajaxMode()) {
    reset();
  } else { // plain HTML
    renderedLastRow_
      = std::min(model()->rowCount(rootIndex()) - 1,
		 renderedFirstRow_ + pageSize() - 1);
    renderedLastColumn_ = columnCount() - 1;
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

  headers_->setWidth(total);
  canvas_->resize(total, canvasHeight());
  headerColumnsCanvas_->setHeight(canvasHeight());

  computeRenderedArea();

  int renderedRows = lastRow() - firstRow() + 1;
  for (int i = 0; i < renderedRows; ++i)
    removeSection(Top);

  setSpannerCount(Top, 0);
  setSpannerCount(Left, rowHeaderCount());

  table_->clear();

  setSpannerCount(Bottom, model()->rowCount(rootIndex()));
  setSpannerCount(Right, columnCount() - rowHeaderCount());

  headerColumnsTable_->clear();

  for (int i = 0; i < rowHeaderCount(); ++i)
    new ColumnWidget(this, i);
}

void WTableView::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WTableView.js", "WTableView", wtjs1);

  setJavaScriptMember(" WTableView", "new " WT_CLASS ".WTableView("
		      + app->javaScriptClass() + "," + jsRef() + ","
		      + contentsContainer_->jsRef() + ","
		      + headerContainer_->jsRef() + ","
		      + headerColumnsContainer_->jsRef() + ",'"
		      + WApplication::instance()->theme()->activeClass()
		      + "');");

  if (!dropEvent_.isConnected())
    dropEvent_.connect(this, &WTableView::onDropEvent);

  if (!scrolled_.isConnected())
    scrolled_.connect(this, &WTableView::onViewportChange);

  if (!columnResizeConnected_) {
    columnResized().connect(this, &WTableView::onColumnResize);
    columnResizeConnected_ = true;
  }

  if (viewportTop_ != 0) {
    WStringStream s;
    s << "function(o, w, h) {"
      <<   "if (!o.scrollTopSet) {"
      <<     "o.scrollTop = " << viewportTop_ << ";"
      <<     "o.onscroll();"
      <<     "o.scrollTopSet = true;"
      <<   "}"
      << "}";
    contentsContainer_->setJavaScriptMember(WT_RESIZE_JS, s.str());
  }

  if (canvas_) {
    app->addAutoJavaScript
      ("{var obj = $('#" + id() + "').data('obj');"
       "if (obj) obj.autoJavaScript();}");
  
    connectObjJS(canvas_->mouseWentDown(), "mouseDown");
    connectObjJS(canvas_->mouseWentUp(), "mouseUp");

    /* Two-lines needed for WT_PORT */
    EventSignalBase& ccScrolled = contentsContainer_->scrolled();
    connectObjJS(ccScrolled, "onContentsContainerScroll");

    EventSignalBase& cKeyDown = canvas_->keyWentDown();
    connectObjJS(cKeyDown, "onkeydown");
  }
}

void WTableView::render(WFlags<RenderFlag> flags)
{
  if (ajaxMode()) {
    if (flags & RenderFull)
      defineJavaScript();

    if (!canvas_->doubleClicked().isConnected()
	&& (editTriggers() & DoubleClicked || doubleClicked().isConnected())) {
      canvas_->doubleClicked()
	.connect(boost::bind(&WTableView::handleDblClick, this, false, _1));
      canvas_->doubleClicked().preventPropagation();
      headerColumnsCanvas_->doubleClicked()
	.connect(boost::bind(&WTableView::handleDblClick, this, true, _1));
      headerColumnsCanvas_->doubleClicked().preventPropagation();

      contentsContainer_->doubleClicked()
	.connect(boost::bind(&WTableView::handleRootDoubleClick, this, 0, _1));
      headerColumnsContainer_->doubleClicked()
	.connect(boost::bind(&WTableView::handleRootDoubleClick, this, 0, _1));
    }
  }

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
      case NeedUpdateModelIndexes:
	updateModelIndexes();
      case NeedAdjustViewPort:
	adjustToViewport();
	break;
      default:
	break;
      }
    }

  WAbstractItemView::render(flags);
}

void WTableView::updateModelIndexes()
{
  int row1 = firstRow();
  int row2 = lastRow();
  int col1 = firstColumn();
  int col2 = lastColumn();

  for (int i = row1; i <= row2; ++i) {
    int renderedRow = i - firstRow();

    int rhc = ajaxMode() ? rowHeaderCount() : 0;

    for (int j = 0; j < rhc; ++j) {
      int renderedColumn = j;

      WModelIndex index = model()->index(i, j, rootIndex());
      updateModelIndex(index, renderedRow, renderedColumn);
    }

    for (int j = col1; j <= col2; ++j) {
      int renderedColumn = rhc + j - firstColumn();

      WModelIndex index = model()->index(i, j, rootIndex());
      updateModelIndex(index, renderedRow, renderedColumn);
    }
  }
}

void WTableView::updateModelIndex(const WModelIndex& index,
				  int renderedRow, int renderedColumn)
{
  WContainerWidget *parentWidget;
  int wIndex;

  if (ajaxMode()) {
    parentWidget = columnContainer(renderedColumn);
    wIndex = renderedRow;
  } else {
    parentWidget = plainTable_->elementAt(renderedRow + 1, renderedColumn);
    wIndex = 0;
  }

  WAbstractItemDelegate *itemDelegate = this->itemDelegate(index.column());
  WWidget *widget = parentWidget->widget(wIndex);
  itemDelegate->updateModelIndex(widget, index);
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

      std::string cl = WApplication::instance()->theme()->activeClass();

      if (selectionBehavior() == SelectRows
	  && isSelected(model()->index(i, 0, rootIndex()))) {
	WTableRow *row = plainTable_->rowAt(renderedRow + 1);
	row->setStyleClass(cl);
      }

      for (int j = firstColumn(); j <= lastColumn(); ++j) {
	int renderedCol = j - firstColumn();

	WModelIndex index = model()->index(i, j, rootIndex());
	WWidget *w = renderWidget(0, index);
	WTableCell *cell = plainTable_->elementAt(renderedRow + 1,
						  renderedCol);
	if (columnInfo(j).hidden)
	  cell->hide();

	cell->addWidget(w);

	WInteractWidget *wi = dynamic_cast<WInteractWidget *>(w);
	if (wi && !isEditing(index))
	  clickedMapper_->mapConnect1(wi->clicked(), index);

	if (selectionBehavior() == SelectItems && isSelected(index))
	  cell->setStyleClass(cl);
      }
    }
  }
}

void WTableView::rerenderHeader()
{
  saveExtraHeaderWidgets();

  if (ajaxMode()) {
    headers_->clear();
    headerColumnsHeaderContainer_->clear();

    for (int i = 0; i < columnCount(); ++i) {
      WWidget *w = createHeaderWidget(i);
      w->setFloatSide(Left);
      if (i < rowHeaderCount())
	headerColumnsHeaderContainer_->addWidget(w);
      else
	headers_->addWidget(w);
      w->setWidth(columnInfo(i).width.toPixels() + 1);
      if (columnInfo(i).hidden)
	w->hide();
    }
  } else { // Plain HTML mode
    for (int i = 0; i < columnCount(); ++i) {
      WWidget *w = createHeaderWidget(i);
      WTableCell *cell = plainTable_->elementAt(0, i);
      cell->clear();
      cell->setStyleClass("headerrh");
      cell->addWidget(w);
      w->setWidth(columnInfo(i).width.toPixels() + 1);
      cell->resize(columnInfo(i).width.toPixels() + 1, w->height());
      if (columnInfo(i).hidden)
	cell->hide();
    }
  }
}

void WTableView::setColumnHidden(int column, bool hidden)
{
  if (columnInfo(column).hidden != hidden) {
    WAbstractItemView::setColumnHidden(column, hidden);

    int delta = static_cast<int>(columnInfo(column).width.toPixels()) + 7;
    if (hidden)
      delta = -delta;

    if (ajaxMode()) {
      headers_->setWidth(headers_->width().toPixels() + delta);
      canvas_->setWidth(canvas_->width().toPixels() + delta);

      if (isColumnRendered(column))
	updateColumnOffsets();
      else
	if (column < firstColumn())
	  setSpannerCount(Left, spannerCount(Left));

      if (renderState_ >= NeedRerenderHeader)
	return;

      WWidget *hc = headerWidget(column, false);
      hc->setHidden(hidden);
    } else {
      if (renderState_ < NeedRerenderData) {
	for (int i = 0; i < plainTable_->rowCount(); ++i)
	  plainTable_->elementAt(i, column)->setHidden(hidden);
      }
    }
  }
}

void WTableView::setColumnWidth(int column, const WLength& width)
{
  WLength rWidth = WLength(round(width.value()), width.unit());
  double delta = rWidth.toPixels() - columnInfo(column).width.toPixels();
  columnInfo(column).width = rWidth;

  if (columnInfo(column).hidden)
    delta = 0;

  if (ajaxMode()) {
    headers_->setWidth(headers_->width().toPixels() + delta);
    canvas_->setWidth(canvas_->width().toPixels() + delta);

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

  WWidget *hc;
  if (column < rowHeaderCount())
    hc = headerColumnsHeaderContainer_->widget(column);
  else
    hc = headers_->widget(column - rowHeaderCount());

  hc->setWidth(0);
  hc->setWidth(rWidth.toPixels() + 1);
  if (!ajaxMode())
    hc->parent()->resize(rWidth.toPixels() + 1, hc->height());
}

bool WTableView::isRowRendered(const int row) const
{
  return row >= firstRow() && row <= lastRow();
}

bool WTableView::isColumnRendered(const int column) const
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
  setHeight(view->table_->height());

  if (column >= view->rowHeaderCount()) {
    if (view->table_->count() == 0
	|| column > view->columnContainer(-1)->column())
      view->table_->addWidget(this);
    else
      view->table_->insertWidget(0, this);
  } else
    view->headerColumnsTable_->insertWidget(column, this);
}

WTableView::ColumnWidget *WTableView::columnContainer(int renderedColumn) const
{
  assert(ajaxMode());

  if (renderedColumn < rowHeaderCount() && renderedColumn >= 0)
    return dynamic_cast<ColumnWidget *>
      (headerColumnsTable_->widget(renderedColumn));
  else if (table_->count() > 0) {
    // -1 is last column
    if (renderedColumn < 0)
      return dynamic_cast<ColumnWidget *>(table_->widget(table_->count() - 1));
    else
      return dynamic_cast<ColumnWidget *>
        (table_->widget(renderedColumn - rowHeaderCount()));
  } else
    return 0;
}

void WTableView::updateColumnOffsets()
{
  assert(ajaxMode());

  int totalRendered = 0;
  for (int i = 0; i < rowHeaderCount(); ++i) {
    ColumnInfo ci = columnInfo(i);

    ColumnWidget *w = columnContainer(i);
    w->setOffsets(0, Left);
    w->setOffsets(totalRendered, Left);
    w->setWidth(0);
    w->setWidth(ci.width.toPixels() + 7);

    if (!columnInfo(i).hidden)
      totalRendered += (int)ci.width.toPixels() + 7;

    w->setHidden(ci.hidden);
  }

  headerColumnsContainer_->setWidth(totalRendered);
  headerColumnsCanvas_->setWidth(totalRendered);
  headerColumnsTable_->setWidth(totalRendered);
  headerColumnsHeaderContainer_->setWidth(totalRendered);

  headerColumnsContainer_->setHidden(totalRendered == 0);
  headerColumnsHeaderContainer_->setHidden(totalRendered == 0);

  int fc = firstColumn();
  int lc = lastColumn();

  totalRendered = 0;
  int total = 0;
  for (int i = rowHeaderCount(); i < columnCount(); ++i) {
    ColumnInfo ci = columnInfo(i);

    if (i >= fc && i <= lc) {
      ColumnWidget *w = columnContainer(rowHeaderCount() + i - fc);

      w->setOffsets(0, Left);
      w->setOffsets(totalRendered, Left);
      w->setWidth(0);
      w->setWidth(ci.width.toPixels() + 7);

      if (!columnInfo(i).hidden)
	totalRendered += (int)ci.width.toPixels() + 7;

      w->setHidden(ci.hidden);
    }

    if (!columnInfo(i).hidden)
      total += (int)columnInfo(i).width.toPixels() + 7;
  }

  double ch = canvasHeight();
  canvas_->resize(total, ch);
  headerColumnsCanvas_->setHeight(ch);
  headers_->setWidth(total);
  table_->setWidth(totalRendered);
}

void WTableView::setRowHeight(const WLength& rowHeight)
{
  int renderedRowCount = model() ? lastRow() - firstRow() + 1 : 0;

  // Avoid floating point error which might lead to incorrect viewport calculation
  WLength len = WLength((int)rowHeight.toPixels()); 

  WAbstractItemView::setRowHeight(len);

  if (ajaxMode()) {
    canvas_->setLineHeight(len);
    headerColumnsCanvas_->setLineHeight(len);

    if (model()) {
      double ch = canvasHeight();
      canvas_->resize(canvas_->width(), ch);
      headerColumnsCanvas_->setHeight(ch);
      double th = renderedRowCount * len.toPixels();
      setRenderedHeight(th);
    }
  } else { // Plain HTML mode
    plainTable_->setLineHeight(len);
    resize(width(), height());
  }

  updateTableBackground();

  scheduleRerender(NeedRerenderData);
}

void WTableView::setHeaderHeight(const WLength& height)
{
  WAbstractItemView::setHeaderHeight(height);

  if (!ajaxMode())
    resize(this->width(), this->height());
}

WWidget* WTableView::headerWidget(int column, bool contentsOnly)
{
  WWidget *result = 0;

  if (ajaxMode()) {
    if (headers_) {
      if (column < rowHeaderCount()) {
	if (column < headerColumnsHeaderContainer_->count())
	  result = headerColumnsHeaderContainer_->widget(column);
      } else if (column - rowHeaderCount() < headers_->count())
	result = headers_->widget(column - rowHeaderCount());
    }
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

void WTableView::shiftModelIndexRows(int start, int count)
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

  shiftEditorRows(rootIndex(), start, count, true);

  if (!toErase.empty())
    selectionChanged().emit();
}

void WTableView::shiftModelIndexColumns(int start, int count)
{
  WModelIndexSet& set = selectionModel()->selection_;
  
  std::vector<WModelIndex> toShift;
  std::vector<WModelIndex> toErase;

  for (WModelIndexSet::iterator it = set.begin(); it != set.end(); ++it) {
    if (count < 0) {
      if ((*it).column() < start - count) {
	toErase.push_back(*it);
	continue;
      }
    }

    if ((*it).column() >= start) {
      toShift.push_back(*it);
      toErase.push_back(*it);
    }
  }

  for (unsigned i = 0; i < toErase.size(); ++i)
    set.erase(toErase[i]);

  for (unsigned i = 0; i < toShift.size(); ++i) {
    WModelIndex newIndex = model()->index(toShift[i].row(),
					  toShift[i].column() + count,
					  toShift[i].parent());
    set.insert(newIndex);
  }

  shiftEditorColumns(rootIndex(), start, count, true);

  if (!toShift.empty() || !toErase.empty())
    selectionChanged().emit();
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

  shiftModelIndexColumns(start, end - start + 1);

  if (ajaxMode())
    canvas_->setWidth(canvas_->width().toPixels() + width);

  if (renderState_ < NeedRerenderHeader)
    scheduleRerender(NeedRerenderHeader);

  if (start > (lastColumn() + 1) || 
      renderState_ == NeedRerender || 
      renderState_ == NeedRerenderData)
    return;

  scheduleRerender(NeedRerenderData);
  adjustSize();
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

  shiftModelIndexColumns(start, -(end - start + 1));

  int count = end - start + 1;
  int width = 0;

  for (int i = start; i < start + count; ++i)
    if (!columnInfo(i).hidden)
      width += (int)columnInfo(i).width.toPixels() + 7;

  for (int i=start; i<start+count; i++)
    delete columns_[i].styleRule;
  columns_.erase(columns_.begin() + start, columns_.begin() + start + count);

  if (ajaxMode())
    canvas_->setWidth(canvas_->width().toPixels() - width);

  if (start <= currentSortColumn_ && currentSortColumn_ <= end)
    currentSortColumn_ = -1;

  if (renderState_ < NeedRerenderHeader)
    scheduleRerender(NeedRerenderHeader);

  if (start > lastColumn() || 
      renderState_ == NeedRerender || 
      renderState_ == NeedRerenderData)
    return;

  resetGeometry();

  scheduleRerender(NeedRerenderData);
  adjustSize();
}

void WTableView::modelRowsInserted(const WModelIndex& parent, 
				   int start, int end)
{
  if (parent != rootIndex())
    return;

  int count = end - start + 1;
  shiftModelIndexRows(start, count);

  computeRenderedArea();

  if (ajaxMode()) {
    canvas_->setHeight(canvasHeight());
    headerColumnsCanvas_->setHeight(canvasHeight());
    scheduleRerender(NeedAdjustViewPort);

    if (start < firstRow())
      setSpannerCount(Top, spannerCount(Top) + count);
    else if (start <= lastRow())
      scheduleRerender(NeedRerenderData);
  } else if (start <= lastRow())
    scheduleRerender(NeedRerenderData);

  adjustSize();
}

void WTableView::modelRowsAboutToBeRemoved(const WModelIndex& parent,
					   int start, int end)
{
  if (parent != rootIndex())
    return;

  for (int c = 0; c < columnCount(); c++) {
    for (int r = start; r <= end; r++) {
      closeEditor(model()->index(r, c), false);
    }
  }

  shiftModelIndexRows(start, -(end - start + 1));  
}

void WTableView::modelRowsRemoved(const WModelIndex& parent, int start, int end)
{
  if (parent != rootIndex())
    return;

  if (ajaxMode()) {
    canvas_->setHeight(canvasHeight());
    headerColumnsCanvas_->setHeight(canvasHeight());
    scheduleRerender(NeedAdjustViewPort);

    if (start >= firstRow() && start <= lastRow()) {
      int toRemove = std::min(lastRow(), end) - start + 1;
      int first = start - firstRow();
      
      for (int i = 0; i < renderedColumnsCount(); ++i) {
	ColumnWidget *column = columnContainer(i);
	for (int j = 0; j < toRemove; ++j)
	  delete column->widget(first);
      }

      setSpannerCount(Bottom, spannerCount(Bottom) + toRemove);
    }
  }

  if (start <= lastRow())
    scheduleRerender(NeedUpdateModelIndexes);

  computeRenderedArea();
  adjustSize();
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

    for (int i = row1; i <= row2; ++i) {
      int renderedRow = i - firstRow();

      int rhc = ajaxMode() ? rowHeaderCount() : 0;

      for (int j = topLeft.column(); j < rhc; ++j) {
	int renderedColumn = j;

	WModelIndex index = model()->index(i, j, rootIndex());
	updateItem(index, renderedRow, renderedColumn);
      }

      for (int j = col1; j <= col2; ++j) {
	int renderedColumn = rhc + j - firstColumn();

	WModelIndex index = model()->index(i, j, rootIndex());
	updateItem(index, renderedRow, renderedColumn);
      }
    }
  }
}

void WTableView::updateItem(const WModelIndex& index,
			    int renderedRow, int renderedColumn)
{
  WContainerWidget *parentWidget;
  int wIndex;

  if (ajaxMode()) {
    parentWidget = columnContainer(renderedColumn);
    wIndex = renderedRow;
  } else {
    parentWidget = plainTable_->elementAt(renderedRow + 1, renderedColumn);
    wIndex = 0;
  }

  WWidget *current = parentWidget->widget(wIndex);

  WWidget *w = renderWidget(current, index);

  if (!w->parent()) {
    delete current;
    parentWidget->insertWidget(wIndex, w);

    if (!ajaxMode() && !isEditing(index)) {
      WInteractWidget *wi = dynamic_cast<WInteractWidget *>(w);
      if (wi)
	clickedMapper_->mapConnect1(wi->clicked(), index);
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

  if (scrollToRow_ != -1) {
    WModelIndex index = model()->index(scrollToRow_, 0, rootIndex());
    scrollToRow_ = -1;
    scrollTo(index, scrollToHint_);
  }

  computeRenderedArea();

  scheduleRerender(NeedAdjustViewPort);  
}

void WTableView::onColumnResize()
{
  computeRenderedArea();

  scheduleRerender(NeedAdjustViewPort);
}

void WTableView::computeRenderedArea()
{
  if (ajaxMode()) {
    const int borderRows = 5;
    const int borderColumnPixels = 200;

    int modelHeight = 0;
    if (model())
      modelHeight = model()->rowCount(rootIndex());

    if (viewportHeight_ != -1) {
      /* row range */
      int top = std::min(viewportTop_,
			 static_cast<int>(canvas_->height().toPixels()));

      int height = std::min(viewportHeight_,
			    static_cast<int>(canvas_->height().toPixels()));

      int renderedRows = static_cast<int>(height / rowHeight().toPixels()
					  + 0.5);

      renderedFirstRow_ = static_cast<int>(top / rowHeight().toPixels());

      renderedLastRow_
	= std::min(renderedFirstRow_ + renderedRows * 2 + borderRows,
		   modelHeight - 1);
      renderedFirstRow_
	= std::max(renderedFirstRow_ - renderedRows - borderRows, 0);
    } else {
      renderedFirstRow_ = 0;
      renderedLastRow_ = modelHeight - 1;
    }

    if (renderedFirstRow_ % 2 == 1)
      --renderedFirstRow_;

    /* column range */
    int left
      = std::max(0, viewportLeft_ - viewportWidth_ - borderColumnPixels);
    int right
      = std::min(static_cast<int>(canvas_->width().toPixels()),
		 viewportLeft_ + 2 * viewportWidth_ + borderColumnPixels);

    int total = 0;
    renderedFirstColumn_ = rowHeaderCount();
    renderedLastColumn_ = columnCount() - 1;
    for (int i = rowHeaderCount(); i < columnCount(); i++) {
      if (columnInfo(i).hidden)
	continue;

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
      renderedLastColumn_ = columnCount() - 1;

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

void WTableView::handleSingleClick(bool headerColumns, const WMouseEvent& event)
{
  WModelIndex index = translateModelIndex(headerColumns, event);
  handleClick(index, event);
}

void WTableView::handleDblClick(bool headerColumns, const WMouseEvent& event)
{
  WModelIndex index = translateModelIndex(headerColumns, event);
  handleDoubleClick(index, event);
}

void WTableView::handleMouseWentDown(bool headerColumns,
				     const WMouseEvent& event)
{
  WModelIndex index = translateModelIndex(headerColumns, event);
  handleMouseDown(index, event);
}

void WTableView::handleMouseWentUp(bool headerColumns, const WMouseEvent& event)
{
  WModelIndex index = translateModelIndex(headerColumns, event);
  handleMouseUp(index, event);
}

void WTableView::handleRootSingleClick(int u, const WMouseEvent& event)
{
  handleClick(WModelIndex(), event);
}

void WTableView::handleRootDoubleClick(int u, const WMouseEvent& event)
{
  handleDoubleClick(WModelIndex(), event);
}

void WTableView::handleRootMouseWentDown(int u, const WMouseEvent& event)
{
  handleMouseDown(WModelIndex(), event);
}

void WTableView::handleRootMouseWentUp(int u, const WMouseEvent& event)
{
  handleMouseUp(WModelIndex(), event);
}

void WTableView::modelLayoutChanged()
{
  WAbstractItemView::modelLayoutChanged();

  resetGeometry();
}

WModelIndex WTableView::modelIndexAt(WWidget *widget) const
{
  for (WWidget *w = widget; w; w = w->parent()) {
    if (w->hasStyleClass("Wt-tv-c")) {
      ColumnWidget *column = dynamic_cast<ColumnWidget *>(w->parent());

      if (!column)
	return WModelIndex();

      int row = firstRow() + column->indexOf(w);
      int col = column->column();

      return model()->index(row, col, rootIndex());
    }
  }

  return WModelIndex();
}

WModelIndex WTableView::translateModelIndex(bool headerColumns,
					    const WMouseEvent& event)
{
  int row = (int)(event.widget().y / rowHeight().toPixels());
  int column = -1;

  int total = 0;

  if (headerColumns) {
    for (int i = 0; i < rowHeaderCount(); ++i) {
      if (!columnInfo(i).hidden)
	total += static_cast<int>(columnInfo(i).width.toPixels()) + 7;

      if (event.widget().x < total) {
	column = i;
	break;
      }
    }
  } else {
    for (int i = rowHeaderCount(); i < columnCount(); i++) {
      if (!columnInfo(i).hidden)
	total += static_cast<int>(columnInfo(i).width.toPixels()) + 7;

      if (event.widget().x < total) {
	column = i;
	break;
      }
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

int WTableView::renderedColumnsCount() const
{
  assert(ajaxMode());

  return headerColumnsTable_->count() + table_->count();
}

WWidget *WTableView::itemWidget(const WModelIndex& index) const
{
  if (index.column() < rowHeaderCount() ||
      (isRowRendered(index.row()) && isColumnRendered(index.column())))
  {
    int renderedRow = index.row() - firstRow();
    int renderedCol;

    if (index.column() < rowHeaderCount())
      renderedCol = index.column();
    else
      renderedCol = rowHeaderCount() + index.column() - firstColumn();

    if (ajaxMode()) {
      ColumnWidget *column = columnContainer(renderedCol);
      return column->widget(renderedRow);
    } else {
      return plainTable_->elementAt(renderedRow + 1, renderedCol);
    }
  } else
    return 0;
}

void WTableView::renderSelected(bool selected, const WModelIndex& index)
{
  std::string cl = WApplication::instance()->theme()->activeClass();

  if (selectionBehavior() == SelectRows) {
    if (isRowRendered(index.row())) {
      int renderedRow = index.row() - firstRow();

      if (ajaxMode()) {
	for (int i = 0; i < renderedColumnsCount(); ++i) {
	  ColumnWidget *column = columnContainer(i);
	  WWidget *w = column->widget(renderedRow);
	  w->toggleStyleClass(cl, selected);
	}
      } else {
	WTableRow *row = plainTable_->rowAt(renderedRow + 1);
	row->toggleStyleClass(cl, selected);
      }
    }
  } else {
    WWidget *w = itemWidget(index);
    if (w)
      w->toggleStyleClass(cl, selected);
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

    int pageSize = static_cast<int>
      ((height().toPixels() - headerHeight().toPixels() - navigationBarHeight)
       / rowHeight().toPixels());
    if (pageSize <= 0)
      pageSize = 1;

    return pageSize;
  }
}

void WTableView::scrollTo(const WModelIndex& index, ScrollHint hint)
{
  if (index.parent() == rootIndex()) {
    if (ajaxMode()) {
      int rh = static_cast<int>(rowHeight().toPixels());
      int rowY = index.row() * rh;

      if (viewportHeight_ != UNKNOWN_VIEWPORT_HEIGHT) {
	if (hint == EnsureVisible) {
	  if (viewportTop_ + viewportHeight_ < rowY)
	    hint = PositionAtTop;
	  else if (rowY < viewportTop_)
	   hint = PositionAtBottom;
	}

	switch (hint) {
	case PositionAtTop:
	  viewportTop_ = rowY; break;
	case PositionAtBottom:
	  viewportTop_ = rowY - viewportHeight_ + rh; break;
	case PositionAtCenter:
	  viewportTop_ = rowY - (viewportHeight_ - rh)/2; break;
	default:
	  break;
	}

	viewportTop_ = std::max(0, viewportTop_);

	if (hint != EnsureVisible) {
	  computeRenderedArea();

	  scheduleRerender(NeedAdjustViewPort);
	}
      } else {
	scrollToRow_ = index.row();
	scrollToHint_ = hint;
      }

      if (isRendered()) {
	WStringStream s;

	s << "jQuery.data("
	  << jsRef() << ", 'obj').setScrollToPending();"
	  << "setTimeout(function() { jQuery.data("
	  << jsRef() << ", 'obj').scrollTo(-1, "
	  << rowY << "," << (int)hint << "); }, 0);";

	doJavaScript(s.str());
      }
    } else
      setCurrentPage(index.row() / pageSize());
  }
}

void WTableView::scrollTo(int x, int y)
{
  if (ajaxMode()) {
    if (isRendered()) {
      WStringStream s;

      s << "jQuery.data(" << jsRef() << ", 'obj').scrollToPx(" << x << ", "
        << y << ");";

      doJavaScript(s.str());
    }
  }
}

void WTableView::setOverflow(WContainerWidget::Overflow overflow, WFlags< Orientation > orientation){
  if (contentsContainer_)
    contentsContainer_->setOverflow(overflow, orientation);
}

void WTableView::setRowHeaderCount(int count)
{
  WAbstractItemView::setRowHeaderCount(count);

  if (ajaxMode()) {
    int total = 0;
    for (int i = 0; i < count; i++)
      if (!columnInfo(i).hidden)
	total += (int)columnInfo(i).width.toPixels() + 7;

    headerColumnsContainer_->setWidth(total);
    headerColumnsContainer_->setHidden(count == 0);
    headerColumnsHeaderContainer_->setHidden(count == 0);
  }
}

EventSignal<WScrollEvent>& WTableView::scrolled(){
  if (wApp->environment().ajax() && contentsContainer_ != 0)
    return contentsContainer_->scrolled();

  throw WException("Scrolled signal existes only with ajax.");
}
}
