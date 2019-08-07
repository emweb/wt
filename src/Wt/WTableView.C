/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WTableView.h"

#include "Wt/WAbstractItemDelegate.h"
#include "Wt/WApplication.h"
#include "Wt/WAbstractItemModel.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WGridLayout.h"
#include "Wt/WLogger.h"
#include "Wt/WModelIndex.h"
#include "Wt/WStringStream.h"
#include "Wt/WTable.h"
#include "Wt/WTheme.h"

#include "WebUtils.h"

#ifndef WT_DEBUG_JS

#include "js/WTableView.min.js"
#endif

#define UNKNOWN_VIEWPORT_HEIGHT 800
#define CONTENTS_VIEWPORT_HEIGHT -1

#include <algorithm>
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

WTableView::WTableView()
  : headers_(nullptr),
    canvas_(nullptr),
    table_(nullptr),
    headerContainer_(nullptr),
    contentsContainer_(nullptr),
    headerColumnsCanvas_(nullptr),
    headerColumnsTable_(nullptr),
    headerColumnsHeaderContainer_(nullptr),
    headerColumnsContainer_(nullptr),
    plainTable_(nullptr),
    dropEvent_(impl_, "dropEvent"),
    scrolled_(impl_, "scrolled"),
    itemTouchSelectEvent_(impl_, "itemTouchSelectEvent"),
    firstColumn_(-1),
    lastColumn_(-1),
    viewportLeft_(0),
    viewportWidth_(1000),
    viewportTop_(0),
    viewportHeight_(UNKNOWN_VIEWPORT_HEIGHT),
    scrollToRow_(-1),
    scrollToHint_(ScrollHint::EnsureVisible),
    columnResizeConnected_(false)
{
  preloadMargin_[0] = preloadMargin_[1] = preloadMargin_[2] = preloadMargin_[3] = WLength();

  setSelectable(false);

  setStyleClass("Wt-itemview Wt-tableview");

  setup();
}

void WTableView::setup()
{
  impl_->clear();

  WApplication *app = WApplication::instance();

  if (app->environment().ajax()) {
    impl_->setPositionScheme(PositionScheme::Relative);

    headers_ = new WContainerWidget();
    headers_->setStyleClass("Wt-headerdiv headerrh");

    table_ = new WContainerWidget();
    table_->setStyleClass("Wt-tv-contents");
    table_->setPositionScheme(PositionScheme::Absolute);
    table_->setWidth(WLength(100, LengthUnit::Percentage));

    std::unique_ptr<WGridLayout> layout(new WGridLayout());
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    headerContainer_ = new WContainerWidget();
    headerContainer_->setStyleClass("Wt-header headerrh");
    headerContainer_->setOverflow(Overflow::Hidden);
    headerContainer_->addWidget(std::unique_ptr<WWidget>(headers_));

    canvas_ = new WContainerWidget();
    canvas_->setStyleClass("Wt-spacer");
    canvas_->setPositionScheme(PositionScheme::Relative);
    canvas_->clicked().connect
      (this, std::bind(&WTableView::handleSingleClick, this, false,
		       std::placeholders::_1));

    canvas_->clicked().connect
      ("function(o, e) { "
       """$(document).trigger($.event.fix(e));"
       "}");

    canvas_->clicked().preventPropagation();
    canvas_->mouseWentDown().connect
      (this, std::bind(&WTableView::handleMouseWentDown, this, false,
		       std::placeholders::_1)); 
    canvas_->mouseWentDown().preventPropagation();
    canvas_->mouseWentDown().connect("function(o, e) { "
                                     "$(document).trigger($.event.fix(e));"
                                     "}");
    canvas_->mouseWentUp().connect
      (this, std::bind(&WTableView::handleMouseWentUp, this, false,
		       std::placeholders::_1));
    canvas_->mouseWentUp().preventPropagation();
    canvas_->mouseWentUp().connect("function(o, e) { "
                                     "$(document).trigger($.event.fix(e));"
                                     "}");
    canvas_->addWidget(std::unique_ptr<WWidget>(table_));

    contentsContainer_ = new WContainerWidget();
    contentsContainer_->setOverflow(Overflow::Auto);
    contentsContainer_->setPositionScheme(PositionScheme::Absolute);
    contentsContainer_->addWidget(std::unique_ptr<WWidget>(canvas_));

    contentsContainer_->clicked().connect
      (this, std::bind(&WTableView::handleRootSingleClick, this, 0,
		       std::placeholders::_1));
    contentsContainer_->mouseWentUp().connect
      (this, std::bind(&WTableView::handleRootMouseWentUp, this, 0,
		       std::placeholders::_1));

    headerColumnsHeaderContainer_ = new WContainerWidget();
    headerColumnsHeaderContainer_->setStyleClass("Wt-header Wt-headerdiv "
						 "headerrh");
    headerColumnsHeaderContainer_->hide();

    headerColumnsTable_ = new WContainerWidget();
    headerColumnsTable_->setStyleClass("Wt-tv-contents");
    headerColumnsTable_->setPositionScheme(PositionScheme::Absolute);
    headerColumnsTable_->setWidth(WLength(100, LengthUnit::Percentage));

    headerColumnsCanvas_ = new WContainerWidget();
    headerColumnsCanvas_->setPositionScheme(PositionScheme::Relative);
    headerColumnsCanvas_->clicked().preventPropagation();
    headerColumnsCanvas_->clicked().connect
      (this, std::bind(&WTableView::handleSingleClick, this, true,
		       std::placeholders::_1));
    headerColumnsCanvas_->clicked().connect("function(o, e) { "
                               "$(document).trigger($.event.fix(e));"
                               "}");
    headerColumnsCanvas_->mouseWentDown().preventPropagation();
    headerColumnsCanvas_->mouseWentDown().connect
      (this, std::bind(&WTableView::handleMouseWentDown, this, true,
		       std::placeholders::_1)); 
    headerColumnsCanvas_->mouseWentDown().connect("function(o, e) { "
                                     "$(document).trigger($.event.fix(e));"
                                     "}");
    headerColumnsCanvas_->mouseWentUp().preventPropagation();
    headerColumnsCanvas_->mouseWentUp().connect
      (this, std::bind(&WTableView::handleMouseWentUp, this, true,
		       std::placeholders::_1)); 
    headerColumnsCanvas_->mouseWentUp().connect("function(o, e) { "
                                     "$(document).trigger($.event.fix(e));"
                                     "}");
    headerColumnsCanvas_->addWidget
      (std::unique_ptr<WWidget>(headerColumnsTable_));

    headerColumnsContainer_ = new WContainerWidget();
    headerColumnsContainer_->setPositionScheme(PositionScheme::Absolute);
    headerColumnsContainer_->setOverflow(Overflow::Hidden);
    headerColumnsContainer_->addWidget
      (std::unique_ptr<WWidget>(headerColumnsCanvas_));
    headerColumnsContainer_->hide();

    headerColumnsContainer_->clicked().connect
      (this, std::bind(&WTableView::handleRootSingleClick, this, 0,
		       std::placeholders::_1));
    headerColumnsContainer_->mouseWentUp().connect
      (this, std::bind(&WTableView::handleRootMouseWentUp, this, 0,
		       std::placeholders::_1)); 

    layout->addWidget(std::unique_ptr<WWidget>(headerColumnsHeaderContainer_),
		      0, 0);
    layout->addWidget(std::unique_ptr<WWidget>(headerContainer_), 0, 1);
    layout->addWidget(std::unique_ptr<WWidget>(headerColumnsContainer_), 1, 0);
    layout->addWidget(std::unique_ptr<WWidget>(contentsContainer_), 1, 1);

    for (int i = 0; i < layout->count(); ++i)
      layout->itemAt(i)->widget()->addStyleClass("tcontainer");

    layout->setRowStretch(1, 1);
    layout->setColumnStretch(1, 1);

    impl_->setLayout(std::move(layout));
  } else {
    plainTable_ = new WTable();
    plainTable_->setStyleClass("Wt-plaintable");
    plainTable_->setAttributeValue("style", "table-layout: fixed;");
    plainTable_->setHeaderCount(1);

    impl_->addWidget(std::unique_ptr<WWidget>(plainTable_));

    resize(width(), height());
  }

  setRowHeight(rowHeight());

  updateTableBackground();
}

void WTableView::enableAjax()
{
  plainTable_ = 0;
  setup();
  defineJavaScript();
  scheduleRerender(RenderState::NeedRerenderHeader);
  WAbstractItemView::enableAjax();
}

void WTableView::resize(const WLength& width, const WLength& height)
{
  if (ajaxMode()) {
    if (height.unit() == LengthUnit::Percentage) {
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

  scheduleRerender(RenderState::NeedAdjustViewPort);
}

WTableView::~WTableView()
{ 
  impl_->clear();
}

void WTableView::updateTableBackground()
{
  if (ajaxMode()) {
    WApplication::instance()->theme()->apply
      (this, table_, TableViewRowContainer);
    WApplication::instance()->theme()->apply
      (this, headerColumnsTable_, TableViewRowContainer);
  } else
    // FIXME avoid background on header row ?
    WApplication::instance()->theme()->apply
      (this, plainTable_, TableViewRowContainer);
}

void WTableView::setModel(const std::shared_ptr<WAbstractItemModel>& model)
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

std::unique_ptr<WWidget> WTableView::renderWidget(WWidget* widget, const WModelIndex& index) 
{
  auto itemDelegate = this->itemDelegate(index.column());

  WFlags<ViewItemRenderFlag> renderFlags = None;

  if (ajaxMode()) {
    if (isSelected(index))
      renderFlags |= ViewItemRenderFlag::Selected;
  }

  if (isEditing(index)) {
    renderFlags |= ViewItemRenderFlag::Editing;
    if (hasEditFocus(index))
      renderFlags |= ViewItemRenderFlag::Focused;
  }

  if (!isValid(index)) {
    renderFlags |= ViewItemRenderFlag::Invalid;
  }

  bool initial = !widget;

  std::unique_ptr<WWidget> wAfter = itemDelegate->update(widget, index, renderFlags);
  if (wAfter)
    widget = wAfter.get();
  widget->setInline(false);
  widget->addStyleClass("Wt-tv-c");
  widget->setHeight(rowHeight());

  if (renderFlags.test(ViewItemRenderFlag::Editing)) {
    widget->setTabIndex(-1);
    setEditorWidget(index, widget);
  }

  if (initial) {
    /*
     * If we are re-creating an old editor, then reset its current edit
     * state (we do not actually check if it is an old editor, but we could
     * now with stateSaved)
     */
    if (renderFlags.test(ViewItemRenderFlag::Editing)) {
      cpp17::any state = editState(index);
      if (cpp17::any_has_value(state))
	itemDelegate->setEditState(widget, index, state);
    }
  }

  return wAfter;
}

int WTableView::spannerCount(const Side side) const
{
  assert(ajaxMode());

  switch (side) {
  case Side::Top: {
    return (int)(table_->offset(Side::Top).toPixels() / rowHeight().toPixels());
  }
  case Side::Bottom: {
    return (int)(model()->rowCount(rootIndex())
      - (table_->offset(Side::Top).toPixels() + table_->height().toPixels())
		 / rowHeight().toPixels());
  }
  case Side::Left:
    return firstColumn_; // headers are included
  case Side::Right:
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
  case Side::Top: {
    int size = model()->rowCount(rootIndex()) - count - spannerCount(Side::Bottom);

    double to = count * rowHeight().toPixels();
    table_->setOffsets(to, Side::Top);
    headerColumnsTable_->setOffsets(to, Side::Top);

    double th = size * rowHeight().toPixels();
    setRenderedHeight(th);
    break;
  }
  case Side::Bottom: {
    int size = model()->rowCount(rootIndex()) - spannerCount(Side::Top) - count;
    double th = size * rowHeight().toPixels();
    setRenderedHeight(th);
    break;
  }
  case Side::Left: {
    int total = 0;
    for (int i = rowHeaderCount(); i < count; i++)
      if (!columnInfo(i).hidden)
	total += (int)columnInfo(i).width.toPixels() + 7;
    table_->setOffsets(total, Side::Left);
    firstColumn_ = count;
    break;
  }
  case Side::Right:
    lastColumn_ = columnCount() - count - 1;
    break;
  default:
    assert(false);
  }
}

int WTableView::firstRow() const
{
  if (ajaxMode())
    return spannerCount(Side::Top);
  else
    return renderedFirstRow_;
}

int WTableView::lastRow() const
{
  if (ajaxMode())
    return model()->rowCount(rootIndex()) - spannerCount(Side::Bottom) - 1;
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

void WTableView::addSection(const Side side)
{
  assert(ajaxMode());

  switch (side) {
  case Side::Top:
    setSpannerCount(side, spannerCount(side) - 1);
    break;
  case Side::Bottom:
    setSpannerCount(side, spannerCount(side) - 1);
    break;
  case Side::Left: {
    ColumnWidget *w = new ColumnWidget(this, firstColumn() - 1);

    if (!columnInfo(w->column()).hidden)
      table_->setOffsets(table_->offset(Side::Left).toPixels()
			 - columnWidth(w->column()).toPixels() - 7, Side::Left);
    else
      w->hide();

    --firstColumn_;
    break;
  }
  case Side::Right: {
    ColumnWidget *w = new ColumnWidget(this, lastColumn() + 1);

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
  w->removeFromParent();
}

void WTableView::removeSection(const Side side)
{
  assert(ajaxMode());

  int row = firstRow(), col = firstColumn();

  switch (side) {
  case Side::Top:
    setSpannerCount(side, spannerCount(side) + 1);

    for (int i = 0; i < renderedColumnsCount(); ++i) {
      ColumnWidget *w = columnContainer(i);
      deleteItem(row, col + i, w->widget(0));
    }
    break;
  case Side::Bottom:
    row = lastRow();
    setSpannerCount(side, spannerCount(side) + 1);

    for (int i = 0; i < renderedColumnsCount(); ++i) {
      ColumnWidget *w = columnContainer(i);
      deleteItem(row, col + i, w->widget(w->count() - 1));
    }
    break;
  case Side::Left: {
    ColumnWidget *w = columnContainer(rowHeaderCount());

    if (!columnInfo(w->column()).hidden)
      table_->setOffsets(table_->offset(Side::Left).toPixels()
			 + columnWidth(w->column()).toPixels() + 7, Side::Left);
    ++firstColumn_;

    for (int i = w->count() - 1; i >= 0; --i)
      deleteItem(row + i, col, w->widget(i));

    w->removeFromParent();

    break;
  }
  case Side::Right: {
    ColumnWidget *w = columnContainer(-1);
    col = w->column();

    --lastColumn_;

    for (int i = w->count() - 1; i >= 0; --i)
      deleteItem(row + i, col, w->widget(i));

    w->removeFromParent();

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
    setSpannerCount(Side::Top, fr);
    setSpannerCount(Side::Bottom, model()->rowCount(rootIndex()) - fr);
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
    setSpannerCount(Side::Left, fc);
    setSpannerCount(Side::Right, columnCount() - fc);
    rightColsToAdd = lc - fc + 1;
  } else {
    leftColsToAdd = firstColumn() - fc;
    rightColsToAdd = lc - lastColumn();
  }

  // Remove columns
  for (int i = 0; i < -leftColsToAdd; ++i)
    removeSection(Side::Left);
  for (int i = 0; i < -rightColsToAdd; ++i)
    removeSection(Side::Right);

  // Remove rows
  for (int i = 0; i < -topRowsToAdd; ++i)
    removeSection(Side::Top);
  for (int i = 0; i < -bottomRowsToAdd; ++i)
    removeSection(Side::Bottom);

  // Add (empty) columns
  for (int i = 0; i < leftColsToAdd; ++i)
    addSection(Side::Left);
  for (int i = 0; i < rightColsToAdd; ++i)
    addSection(Side::Right);

  // Add new top rows
  for (int i = 0; i < topRowsToAdd; ++i) {
    int row = fr + i;
    for (int col = 0; col < rowHeaderCount(); ++col) {
      ColumnWidget *w = columnContainer(col);
      w->insertWidget(i, renderWidget(nullptr, model()->index(row, col, rootIndex())));
    }
    for (int col = fc; col <= lc; ++col) {
      ColumnWidget *w = columnContainer(col - fc + rowHeaderCount());
      w->insertWidget(i, renderWidget(nullptr, model()->index(row, col, rootIndex())));
    }
    addSection(Side::Top);
  }
  // Populate new columns of existing rows
  if (oldLastRow != -1 &&
      (leftColsToAdd > 0 ||
       rightColsToAdd > 0)) {
    for (int row = std::max(oldFirstRow, fr); row <= std::min(oldLastRow, lr); ++row) {
      // Populate left columns
      for (int j = 0; j < leftColsToAdd; ++j) {
        int col = fc + j;
        int renderCol = rowHeaderCount() + j;
        ColumnWidget *w = columnContainer(renderCol);
        w->addWidget(renderWidget(nullptr, model()->index(row, col, rootIndex())));
      }
      // Populate right columns
      for (int j = 0; j < rightColsToAdd; ++j) {
        int col = lc - rightColsToAdd + 1 + j;
        ColumnWidget *w = columnContainer(col - fc + rowHeaderCount());
        w->addWidget(renderWidget(nullptr, model()->index(row, col, rootIndex())));
      }
    }
  }
  // Add new bottom rows
  for (int i = 0; i < bottomRowsToAdd; ++i) {
    int row = oldLastRow == -1 ? fr + i : oldLastRow + 1 + i;
    for (int col = 0; col < rowHeaderCount(); ++col) {
      ColumnWidget *w = columnContainer(col);
      w->addWidget(renderWidget(nullptr, model()->index(row, col, rootIndex())));
    }
    for (int col = fc; col <= lc; ++col) {
      ColumnWidget *w = columnContainer(col - fc + rowHeaderCount());
      w->addWidget(renderWidget(nullptr, model()->index(row, col, rootIndex())));
    }
    addSection(Side::Bottom);
  }

  updateColumnOffsets();

  assert(lastRow() == lr && firstRow() == fr);
  assert(lastColumn() == lc && firstColumn() == fc);

  const double marginTop = (preloadMargin(Side::Top).isAuto() ? viewportHeight_ : preloadMargin(Side::Top).toPixels()) / 2;
  const double marginBottom = (preloadMargin(Side::Bottom).isAuto() ? viewportHeight_ : preloadMargin(Side::Bottom).toPixels()) / 2;
  const double marginLeft = (preloadMargin(Side::Left).isAuto() ? viewportWidth_ : preloadMargin(Side::Left).toPixels()) / 2;
  const double marginRight = (preloadMargin(Side::Right).isAuto() ? viewportWidth_ : preloadMargin(Side::Right).toPixels()) / 2;

  const double scrollX1 = round(std::max(0.0, viewportLeft_ - marginLeft));
  const double scrollX2 = round(viewportLeft_ + marginRight);
  const double scrollY1 = round(std::max(0.0, viewportTop_ - marginTop));
  const double scrollY2 = round(viewportTop_ + marginBottom);

  WStringStream s;

  char buf[30];

  s << jsRef() << ".wtObj.scrolled(";
  s << Utils::round_js_str(scrollX1, 3, buf) << ", ";
  s << Utils::round_js_str(scrollX2, 3, buf) << ", ";
  s << Utils::round_js_str(scrollY1, 3, buf) << ", ";
  s << Utils::round_js_str(scrollY2, 3, buf) << ");";

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
      s << jsRef() << ".wtObj.resetScroll();";
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
    removeSection(Side::Top);

  setSpannerCount(Side::Top, 0);
  setSpannerCount(Side::Left, rowHeaderCount());

  table_->clear();

  setSpannerCount(Side::Bottom, model()->rowCount(rootIndex()));
  setSpannerCount(Side::Right, columnCount() - rowHeaderCount());

  headerColumnsTable_->clear();

  for (int i = 0; i < rowHeaderCount(); ++i)
    new ColumnWidget(this, i);
}

void WTableView::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WTableView.js", "WTableView", wtjs1);

  WStringStream s;
  s << "new " WT_CLASS ".WTableView("
    << app->javaScriptClass() << ',' << jsRef() << ','
    << contentsContainer_->jsRef() << ','
    << viewportTop_ << ','
    << headerContainer_->jsRef() << ','
    << headerColumnsContainer_->jsRef() << ",'"
    << WApplication::instance()->theme()->activeClass()
    << "');";
  setJavaScriptMember(" WTableView", s.str());

  if (!dropEvent_.isConnected())
    dropEvent_.connect(this, &WTableView::onDropEvent);

  if (!scrolled_.isConnected())
    scrolled_.connect(this, &WTableView::onViewportChange);

  if (!itemTouchSelectEvent_.isConnected())
    itemTouchSelectEvent_.connect(this, &WTableView::handleTouchSelected);

  if (!columnResizeConnected_) {
    columnResized().connect(this, &WTableView::onColumnResize);
    columnResizeConnected_ = true;
  }

  if (canvas_) {
    app->addAutoJavaScript
      ("{var obj = " + jsRef() + ";"
       "if (obj && obj.wtObj) obj.wtObj.autoJavaScript();}");
  
    connectObjJS(canvas_->mouseWentDown(), "mouseDown");
    connectObjJS(canvas_->mouseWentUp(), "mouseUp");

#ifdef WT_CNOR
    // workaround because cnor is a bit dumb and does not understand that it
    // can convert EventSignal<TouchEvent>& to EventSignalBase&
    EventSignalBase& a = canvas_->touchStarted();
#endif

    connectObjJS(canvas_->touchStarted(), "touchStart");
    connectObjJS(canvas_->touchMoved(), "touchMove");
    connectObjJS(canvas_->touchEnded(), "touchEnd");

    /* Two-lines needed for WT_PORT */
    EventSignalBase& ccScrolled = contentsContainer_->scrolled();
    connectObjJS(ccScrolled, "onContentsContainerScroll");

    EventSignalBase& cKeyDown = canvas_->keyWentDown();
    connectObjJS(cKeyDown, "onkeydown");
  }
}

void WTableView::render(WFlags<RenderFlag> flags)
{
  if (flags.test(RenderFlag::Full) && !ajaxMode() &&
      Wt::WApplication::instance()->environment().ajax()) {
    // Was not rendered when Ajax was enabled, issue #5470
    plainTable_ = 0;
    setup();
  }

  if (ajaxMode()) {
    if (flags.test(RenderFlag::Full))
      defineJavaScript();

    if (!canvas_->doubleClicked().isConnected()
	&& (editTriggers().test(EditTrigger::DoubleClicked) || 
	    doubleClicked().isConnected())) {
      canvas_->doubleClicked().connect
	(this, std::bind(&WTableView::handleDblClick, this, false,
			 std::placeholders::_1));
      canvas_->doubleClicked().preventPropagation();
      headerColumnsCanvas_->doubleClicked().connect
	(this, std::bind(&WTableView::handleDblClick, this, true,
			 std::placeholders::_1));
      headerColumnsCanvas_->doubleClicked().preventPropagation();

      contentsContainer_->doubleClicked().connect
	(this, std::bind(&WTableView::handleRootDoubleClick, this, 0,
			 std::placeholders::_1));
      headerColumnsContainer_->doubleClicked().connect
	(this, std::bind(&WTableView::handleRootDoubleClick, this, 0,
			 std::placeholders::_1));
    }

    if (!touchStartConnection_.isConnected()
        && touchStarted().isConnected()) {
      touchStartConnection_ = canvas_->touchStarted()
	.connect(this, &WTableView::handleTouchStarted);
    }

    if (!touchMoveConnection_.isConnected()
        && touchMoved().isConnected()) {
      touchMoveConnection_ = canvas_->touchMoved()
        .connect(this, &WTableView::handleTouchMoved);
    }

    if (!touchEndConnection_.isConnected()
        && touchEnded().isConnected()) {
      touchEndConnection_ = canvas_->touchEnded()
	.connect(this, &WTableView::handleTouchEnded);
    }
  }

  if (model())
    while (renderState_ != RenderState::RenderOk) {
      RenderState s = renderState_;
      renderState_ = RenderState::RenderOk;

      switch (s) {
      case RenderState::NeedRerender:
	resetGeometry();
	rerenderHeader();
	rerenderData();
	break;
      case RenderState::NeedRerenderHeader:
	rerenderHeader();
	break;
      case RenderState::NeedRerenderData:
	rerenderData();
	break;
      case RenderState::NeedUpdateModelIndexes:
	updateModelIndexes();
        /* fallthrough */
      case RenderState::NeedAdjustViewPort:
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

  auto itemDelegate = this->itemDelegate(index.column());
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
      plainTable_->removeRow(plainTable_->rowCount() - 1);

    for (int i = firstRow(); i <= lastRow(); ++i) {
      int renderedRow = i - firstRow();

      std::string cl = WApplication::instance()->theme()->activeClass();

      if (selectionBehavior() == SelectionBehavior::Rows
	  && isSelected(model()->index(i, 0, rootIndex()))) {
	WTableRow *row = plainTable_->rowAt(renderedRow + 1);
	row->setStyleClass(cl);
      }

      for (int j = firstColumn(); j <= lastColumn(); ++j) {
	int renderedCol = j - firstColumn();

	const WModelIndex index = model()->index(i, j, rootIndex());
	std::unique_ptr<WWidget> w = renderWidget(nullptr, index);
	WTableCell *cell = plainTable_->elementAt
	  (renderedRow + 1, renderedCol);
	if (columnInfo(j).hidden)
	  cell->hide();

	WInteractWidget *wi = dynamic_cast<WInteractWidget *>(w.get());
	if (wi && !isEditing(index))
	  wi->clicked().connect
	    (this, std::bind(&WTableView::handleClick, this, index,
			     std::placeholders::_1));

	if (selectionBehavior() == SelectionBehavior::Items &&
	    isSelected(index))
	  cell->setStyleClass(cl);

	cell->addWidget(std::move(w));
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
      std::unique_ptr<WWidget> w = createHeaderWidget(i);
      w->setFloatSide(Side::Left);
      w->setWidth(columnInfo(i).width.toPixels() + 1);
      if (columnInfo(i).hidden)
	w->hide();
      if (i < rowHeaderCount())
	headerColumnsHeaderContainer_->addWidget(std::move(w));
      else
	headers_->addWidget(std::move(w));
    }
  } else { // Plain HTML mode
    for (int i = 0; i < columnCount(); ++i) {
      std::unique_ptr<WWidget> w = createHeaderWidget(i);
      WTableCell *cell = plainTable_->elementAt(0, i);
      cell->clear();
      cell->setStyleClass("headerrh");
      w->setWidth(columnInfo(i).width.toPixels() + 1);
      cell->resize(columnInfo(i).width.toPixels() + 1, w->height());
      if (columnInfo(i).hidden)
	cell->hide();
      cell->addWidget(std::move(w));
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
	  setSpannerCount(Side::Left, spannerCount(Side::Left));

      if (static_cast<unsigned int>(renderState_) >=
	  static_cast<unsigned int>(RenderState::NeedRerenderHeader))
	return;

      WWidget *hc = headerWidget(column, false);
      hc->setHidden(hidden);
    } else {
      if (static_cast<unsigned int>(renderState_) <
	  static_cast<unsigned int>(RenderState::NeedRerenderData)) {
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

    if (static_cast<unsigned int>(renderState_) >= 
	static_cast<unsigned int>(RenderState::NeedRerenderHeader))
      return;

    if (isColumnRendered(column))
      updateColumnOffsets();
    else
      if (column < firstColumn())
	setSpannerCount(Side::Left, spannerCount(Side::Left));
  }

  if (static_cast<unsigned int>(renderState_) >=
      static_cast<unsigned int>(RenderState::NeedRerenderHeader))
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
  setPositionScheme(PositionScheme::Absolute);
  setOffsets(0, Side::Top | Side::Left);
  setOverflow(Overflow::Hidden);
  setHeight(view->table_->height());

  std::unique_ptr<WWidget> self(this);

  if (column >= view->rowHeaderCount()) {
    if (view->table_->count() == 0
	|| column > view->columnContainer(-1)->column())
      view->table_->addWidget(std::move(self));
    else
      view->table_->insertWidget(0, std::move(self));
  } else
    view->headerColumnsTable_->insertWidget(column, std::move(self));
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
    return nullptr;
}

void WTableView::updateColumnOffsets()
{
  assert(ajaxMode());

  int totalRendered = 0;
  for (int i = 0; i < rowHeaderCount(); ++i) {
    ColumnInfo ci = columnInfo(i);

    ColumnWidget *w = columnContainer(i);
    w->setOffsets(0, Side::Left);
    w->setOffsets(totalRendered, Side::Left);
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

      w->setOffsets(0, Side::Left);
      w->setOffsets(totalRendered, Side::Left);
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

  scheduleRerender(RenderState::NeedRerenderData);
}

void WTableView::setHeaderHeight(const WLength& height)
{
  WAbstractItemView::setHeaderHeight(height);

  if (!ajaxMode())
    resize(this->width(), this->height());
}

WWidget* WTableView::headerWidget(int column, bool contentsOnly)
{
  WWidget *result = nullptr;

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

  if (static_cast<unsigned int>(renderState_) < 
      static_cast<unsigned int>(RenderState::NeedRerenderHeader))
    scheduleRerender(RenderState::NeedRerenderHeader);

  if (start > (lastColumn() + 1) || 
      renderState_ == RenderState::NeedRerender || 
      renderState_ == RenderState::NeedRerenderData)
    return;

  scheduleRerender(RenderState::NeedRerenderData);
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

  WApplication *app = WApplication::instance();
  for (int i = start; i< start + count; ++i)
    app->styleSheet().removeRule(columns_[i].styleRule.get());
  columns_.erase(columns_.begin() + start, columns_.begin() + start + count);

  if (ajaxMode())
    canvas_->setWidth(canvas_->width().toPixels() - width);

  if (start <= currentSortColumn_ && currentSortColumn_ <= end)
    currentSortColumn_ = -1;

  if (static_cast<unsigned int>(renderState_) <
      static_cast<unsigned int>(RenderState::NeedRerenderHeader))
    scheduleRerender(RenderState::NeedRerenderHeader);

  if (start > lastColumn() || 
      renderState_ == RenderState::NeedRerender || 
      renderState_ == RenderState::NeedRerenderData)
    return;

  resetGeometry();

  scheduleRerender(RenderState::NeedRerenderData);
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
    scheduleRerender(RenderState::NeedAdjustViewPort);

    if (start < firstRow())
      setSpannerCount(Side::Top, spannerCount(Side::Top) + count);
    else if (start <= lastRow())
      scheduleRerender(RenderState::NeedRerenderData);
  } else if (start <= lastRow())
    scheduleRerender(RenderState::NeedRerenderData);

  adjustSize();
}

namespace {

int calcOverlap(int start1, int end1,
		int start2, int end2)
{
  int s = std::max(start1, start2);
  int e = std::min(end1, end2);
  return std::max(0, e - s);
}

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

  int overlapTop = calcOverlap(0, spannerCount(Side::Top),
			       start, end + 1);
  int overlapMiddle = calcOverlap(firstRow(), lastRow() + 1,
				  start, end + 1);

  if (overlapMiddle > 0) {
    int first = std::max(0, start - firstRow());
  
    for (int i = 0; i < renderedColumnsCount(); ++i) {
      ColumnWidget *column = columnContainer(i);
      for (int j = 0; j < overlapMiddle; ++j)
        column->widget(first)->removeFromParent();
    }

    setSpannerCount(Side::Bottom, spannerCount(Side::Bottom) + overlapMiddle);
  }

  if (overlapTop > 0) {
    setSpannerCount(Side::Top, spannerCount(Side::Top) - overlapTop);
    setSpannerCount(Side::Bottom, spannerCount(Side::Bottom) + overlapTop);
  }
}

void WTableView::modelRowsRemoved(const WModelIndex& parent,
				  int start, int end)
{
  if (parent != rootIndex())
    return;

  if (ajaxMode()) {
    canvas_->setHeight(canvasHeight());
    headerColumnsCanvas_->setHeight(canvasHeight());
    scheduleRerender(RenderState::NeedAdjustViewPort);
  }

  scheduleRerender(RenderState::NeedUpdateModelIndexes);

  computeRenderedArea();
  adjustSize();
}

void WTableView::modelDataChanged(const WModelIndex& topLeft, 
				  const WModelIndex& bottomRight)
{
  if (topLeft.parent() != rootIndex())
    return;

  if (static_cast<unsigned int>(renderState_) <
      static_cast<unsigned int>(RenderState::NeedRerenderData)) {
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

  std::unique_ptr<WWidget> wAfter = renderWidget(current, index);
  WWidget *w = nullptr;
  if (wAfter)
    w = wAfter.get();
  else
    w = current;

  if (wAfter) {
    parentWidget->removeWidget(current); // current may or may not be dangling at this point
    parentWidget->insertWidget(wIndex, std::move(wAfter));

    if (!ajaxMode() && !isEditing(index)) {
      WInteractWidget *wi = dynamic_cast<WInteractWidget *>(w);
      if (wi)
	wi->clicked().connect
	  (this, std::bind(&WTableView::handleClick, this, index,
			   std::placeholders::_1));
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

  scheduleRerender(RenderState::NeedAdjustViewPort);  
}

void WTableView::onColumnResize()
{
  computeRenderedArea();

  scheduleRerender(RenderState::NeedAdjustViewPort);
}

void WTableView::computeRenderedArea()
{
  if (ajaxMode()) {
    const int borderRows = 5;

    int modelHeight = 0;
    if (model())
      modelHeight = model()->rowCount(rootIndex());

    if (viewportHeight_ != -1) {
      /* row range */
      const int top = std::min(viewportTop_,
			 static_cast<int>(canvas_->height().toPixels()));

      const int height = std::min(viewportHeight_,
			    static_cast<int>(canvas_->height().toPixels()));

      const double renderedRows = height / rowHeight().toPixels();

      const double renderedRowsAbove = preloadMargin(Side::Top).isAuto() ? renderedRows + borderRows :
                                                                           preloadMargin(Side::Top).toPixels() / rowHeight().toPixels();

      const double renderedRowsBelow = preloadMargin(Side::Bottom).isAuto() ? renderedRows + borderRows :
                                                                              preloadMargin(Side::Bottom).toPixels() / rowHeight().toPixels();

      renderedFirstRow_ = static_cast<int>(std::floor(top / rowHeight().toPixels()));

      renderedLastRow_
        = static_cast<int>(std::ceil(std::min(renderedFirstRow_ + renderedRows + renderedRowsBelow, modelHeight - 1.0)));
      renderedFirstRow_
        = static_cast<int>(std::floor(std::max(renderedFirstRow_ - renderedRowsAbove, 0.0)));
    } else {
      renderedFirstRow_ = 0;
      renderedLastRow_ = modelHeight - 1;
    }

    if (renderedFirstRow_ % 2 == 1)
      --renderedFirstRow_;

    const int borderColumnPixels = 200;
    const double marginLeft = preloadMargin(Side::Left).isAuto() ? viewportWidth_ + borderColumnPixels : preloadMargin(Side::Left).toPixels();
    const double marginRight = preloadMargin(Side::Right).isAuto() ? viewportWidth_ + borderColumnPixels : preloadMargin(Side::Right).toPixels();

    /* column range */
    int left
      = static_cast<int>(std::floor(std::max(0.0, viewportLeft_ - marginLeft)));
    int right
      = static_cast<int>(std::ceil(std::min(std::max(canvas_->width().toPixels(),
                                            viewportWidth_ * 1.0), // When a column was made wider, and the
                                                                   // canvas is narrower than the viewport,
                                                                   // the size of the canvas will not have
                                                                   // been updated yet, so we use the viewport
                                                                   // width instead.
                                   viewportLeft_ + viewportWidth_ + marginRight)));

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

  computeRenderedArea();

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

void WTableView::handleTouchSelected(const WTouchEvent& event)
{
  std::vector<WModelIndex> indices;
  for(std::size_t i = 0; i < event.touches().size(); i++){
    indices.push_back(translateModelIndex(event.touches()[i]));
  }
  handleTouchSelect(indices, event);
}

void WTableView::handleTouchStarted(const WTouchEvent& event)
{
  std::vector<WModelIndex> indices;
  const std::vector<Touch> &touches = event.changedTouches();
  for(std::size_t i = 0; i < touches.size(); i++){
    indices.push_back(translateModelIndex(touches[i]));
  }
  handleTouchStart(indices, event);
}

void WTableView::handleTouchMoved(const WTouchEvent& event)
{
  std::vector<WModelIndex> indices;
  const std::vector<Touch> &touches = event.changedTouches();
  for(std::size_t i = 0; i < touches.size(); i++){
    indices.push_back(translateModelIndex(touches[i]));
  }
  handleTouchMove(indices, event);
}

void WTableView::handleTouchEnded(const WTouchEvent& event)
{
  std::vector<WModelIndex> indices;
  const std::vector<Touch> &touches = event.changedTouches();
  for (std::size_t i = 0; i < touches.size(); i++) {
    indices.push_back(translateModelIndex(touches[i]));
  }
  handleTouchEnd(indices, event);
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

WModelIndex WTableView::translateModelIndex(const Touch& touch)
{
  int row = (int)(touch.widget().y / rowHeight().toPixels());
  int column = -1;

  int total = 0;

  for (int i = rowHeaderCount(); i < columnCount(); i++) {
    if (!columnInfo(i).hidden)
      total += static_cast<int>(columnInfo(i).width.toPixels()) + 7;

    if (touch.widget().x < total) {
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
  if (selectionBehavior() == SelectionBehavior::Rows &&
      index.column() != 0)
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
  if ((index.column() < rowHeaderCount() || isColumnRendered(index.column())) &&
      isRowRendered(index.row()))
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
    return nullptr;
}

void WTableView::renderSelected(bool selected, const WModelIndex& index)
{
  std::string cl = WApplication::instance()->theme()->activeClass();

  if (selectionBehavior() == SelectionBehavior::Rows) {
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
      internalSelect(model()->index(r, c, rootIndex()),
		     SelectionFlag::Select);
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
 
  scheduleRerender(RenderState::NeedRerenderData);
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
	if (hint == ScrollHint::EnsureVisible) {
	  if (viewportTop_ + viewportHeight_ < rowY)
	    hint = ScrollHint::PositionAtTop;
	  else if (rowY < viewportTop_)
	   hint = ScrollHint::PositionAtBottom;
	}

	switch (hint) {
	case ScrollHint::PositionAtTop:
	  viewportTop_ = rowY; break;
	case ScrollHint::PositionAtBottom:
	  viewportTop_ = rowY - viewportHeight_ + rh; break;
	case ScrollHint::PositionAtCenter:
	  viewportTop_ = rowY - (viewportHeight_ - rh)/2; break;
	default:
	  break;
	}

	viewportTop_ = std::max(0, viewportTop_);

	if (hint != ScrollHint::EnsureVisible) {
	  computeRenderedArea();

	  scheduleRerender(RenderState::NeedAdjustViewPort);
	}
      } else {
	scrollToRow_ = index.row();
	scrollToHint_ = hint;
      }

      if (isRendered()) {
	WStringStream s;

	s << jsRef() << ".wtObj.setScrollToPending();"
	  << "setTimeout(function() {"
	  << jsRef() << ".wtObj.scrollTo(-1, "
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

      s << jsRef() << ".wtObj.scrollToPx(" << x << ", "
        << y << ");";

      doJavaScript(s.str());
    }
  }
}

void WTableView::setOverflow(Overflow overflow, 
			     WFlags< Orientation > orientation)
{
  if (contentsContainer_)
    contentsContainer_->setOverflow(overflow, orientation);
}

void WTableView::setPreloadMargin(const WLength &margin, WFlags<Side> side)
{
  if (side.test(Side::Top)) {
    preloadMargin_[0] = margin;
  }
  if (side.test(Side::Right)) {
    preloadMargin_[1] = margin;
  }
  if (side.test(Side::Bottom)) {
    preloadMargin_[2] = margin;
  }
  if (side.test(Side::Left)) {
    preloadMargin_[3] = margin;
  }

  computeRenderedArea();

  scheduleRerender(RenderState::NeedAdjustViewPort);
}

WLength WTableView::preloadMargin(Side side) const
{
  switch (side) {
  case Side::Top:
    return preloadMargin_[0];
  case Side::Right:
    return preloadMargin_[1];
  case Side::Bottom:
    return preloadMargin_[2];
  case Side::Left:
    return preloadMargin_[3];
  default:
    return WLength();
  }
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
  if (wApp->environment().ajax() && contentsContainer_ != nullptr)
    return contentsContainer_->scrolled();

  throw WException("Scrolled signal existes only with ajax.");
}
}
