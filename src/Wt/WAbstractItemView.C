/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractItemModel"
#include "Wt/WAbstractItemView"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WEvent"
#include "Wt/WImage"
#include "Wt/WItemSelectionModel"
#include "Wt/WItemDelegate"
#include "Wt/WPushButton"
#include "Wt/WText"

#include "SizeHandle.h"
#include "Utils.h"

#include <iostream>

namespace Wt {

class DefaultPagingBar : public WContainerWidget
{
public:
  DefaultPagingBar(WAbstractItemView *view)
    : view_(view)
  {
    setStyleClass("Wt-pagingbar");

    firstButton_ = new WPushButton(WString::fromUTF8("« First"), this);
    firstButton_->clicked().connect(this, &DefaultPagingBar::showFirstPage);

    prevButton_ = new WPushButton(WString::fromUTF8("‹ Previous"), this);
    prevButton_->clicked().connect(this, &DefaultPagingBar::showPreviousPage);

    current_ = new WText(this);

    nextButton_ = new WPushButton(WString::fromUTF8("Next ›"), this);
    nextButton_->clicked().connect(this, &DefaultPagingBar::showNextPage);

    lastButton_ = new WPushButton(WString::fromUTF8("Last »"), this);
    lastButton_->clicked().connect(this, &DefaultPagingBar::showLastPage);

    view_->pageChanged().connect(this, &DefaultPagingBar::update);

    update();
  }

private:
  WAbstractItemView *view_;
  WPushButton *prevButton_, *nextButton_, *firstButton_, *lastButton_;
  WText *current_;

  void update() {
    firstButton_->setDisabled(view_->currentPage() == 0);
    prevButton_->setDisabled(view_->currentPage() == 0);

    nextButton_->setDisabled(view_->currentPage() == view_->pageCount() - 1);
    lastButton_->setDisabled(view_->currentPage() == view_->pageCount() - 1);

    current_->setText(WString::fromUTF8("<b>{1}</b> from <b>{2}</b>").
		      arg(view_->currentPage() + 1).arg(view_->pageCount()));
  }

  void showFirstPage()
  {
    view_->setCurrentPage(0);
  }

  void showLastPage()
  {
    view_->setCurrentPage(view_->pageCount() - 1);
  }

  void showPreviousPage()
  {
    if (view_->currentPage() > 0)
      view_->setCurrentPage(view_->currentPage() - 1);
  }

  void showNextPage()
  {
    if (view_->currentPage() < view_->pageCount() - 1)
      view_->setCurrentPage(view_->currentPage() + 1);
  }
};

WAbstractItemView::ColumnInfo::ColumnInfo(const WAbstractItemView *view, 
					   int anId, 
					   int column)
  : id(anId),
    sortOrder(AscendingOrder),
    alignment(AlignLeft),
    headerAlignment(AlignLeft),
    extraHeaderWidget(0),
    sorting(view->sorting_),
    itemDelegate_(0)
{
  width = WLength(150);

  styleRule = new WCssTemplateRule("#" + view->id() + " ." + styleClass());
  styleRule->templateWidget()->resize(width.toPixels(), WLength::Auto);

  WApplication::instance()->styleSheet().addRule(styleRule);
}

std::string WAbstractItemView::ColumnInfo::styleClass() const
{
#ifndef WT_TARGET_JAVA
  char buf[40];
  buf[0] = 0;
  std::strcat(buf, "Wt-tv-c");
  Utils::itoa(id, buf + 7, 10);
  return buf;
#else
  return "Wt-tv-c" + boost::lexical_cast<std::string>(id);
#endif
}

WAbstractItemView::WAbstractItemView(WContainerWidget *parent)
  : WCompositeWidget(parent),
    impl_(new WContainerWidget()),
    renderState_(NeedRerender),
    currentSortColumn_(-1),
    dragEnabled_(false),
    dropsEnabled_(false),
    model_(0),
    itemDelegate_(0),
    selectionModel_(new WItemSelectionModel(0, this)),
    rowHeight_(20),
    headerLineHeight_(20),
    selectionMode_(NoSelection),
    sorting_(true),
    columnResize_(true),
    multiLineHeader_(false),
    columnWidthChanged_(impl_, "columnResized"),
    columnResized_(this),
    nextColumnId_(1),
    alternatingRowColors_(false),
    clicked_(this),
    doubleClicked_(this),
    mouseWentDown_(this),
    mouseWentUp_(this),
    selectionChanged_(this),
    pageChanged_(this),
    editTriggers_(DoubleClicked),
    editOptions_(SingleEditor)
{
  setImplementation(impl_);

  setItemDelegate(new WItemDelegate(this));

  WApplication *app = WApplication::instance();

  typedef WAbstractItemView Self;

  clickedForSortMapper_ = new WSignalMapper<int>(this);
  clickedForSortMapper_->mapped().connect(this, &Self::toggleSortColumn);

  clickedForCollapseMapper_ = new WSignalMapper<int>(this);
  clickedForCollapseMapper_->mapped().connect(this, &Self::collapseColumn);

  clickedForExpandMapper_ = new WSignalMapper<int>(this);
  clickedForExpandMapper_->mapped().connect(this, &Self::expandColumn);

  SizeHandle::loadJavaScript(app);

  if (!app->environment().ajax()) {
    clickedMapper_ = new WSignalMapper<WModelIndex, WMouseEvent>(this);
    clickedMapper_->mapped().connect(this, &Self::handleClick);

    columnResize_ = false;
  }

  bindObjJS(resizeHandleMDownJS_, "resizeHandleMDown");

  columnWidthChanged_.connect(this, &Self::updateColumnWidth);

  headerHeightRule_ = new WCssTemplateRule("#" + id() + " .headerrh");
  app->styleSheet().addRule(headerHeightRule_);
  setHeaderHeight(headerLineHeight_);
}

WAbstractItemView::~WAbstractItemView()
{
  delete headerHeightRule_;

  for (unsigned i = 0; i < columns_.size(); ++i)
    delete columns_[i].styleRule;
}

void WAbstractItemView::setModel(WAbstractItemModel *model)
{
  if (model_) {
    /* disconnect slots from previous model */
    for (unsigned i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();
    modelConnections_.clear();
  }

  model_ = model;
  WItemSelectionModel *oldSelectionModel = selectionModel_;
  selectionModel_ = new WItemSelectionModel(model, this);
  selectionModel_->setSelectionBehavior(oldSelectionModel->selectionBehavior());

  editedItems_.clear();

  configureModelDragDrop();

  setHeaderHeight(headerLineHeight_, multiLineHeader_);

  setRootIndex(WModelIndex());
}

void WAbstractItemView::setRootIndex(const WModelIndex& rootIndex)
{
  rootIndex_ = rootIndex;

  scheduleRerender(NeedRerender);

  // make sure columnCount() is correct
  while (static_cast<int>(columns_.size()) > model_->columnCount(rootIndex_)) {
    delete columns_.back().styleRule;
    columns_.erase(columns_.begin() + columns_.size() - 1);
  }

  columnInfo(model_->columnCount(rootIndex_) - 1);
}

void WAbstractItemView::setRowHeight(const WLength& rowHeight)
{
  rowHeight_ = rowHeight;
}

WLength WAbstractItemView::columnWidth(int column) const
{
  return columnInfo(column).width;
}

void WAbstractItemView::updateColumnWidth(int columnId, int width)
{
  int column = columnById(columnId);

  if (column >= 0) {
    columnInfo(column).width = width;
    columnResized_.emit(column, columnInfo(column).width);
  }
}

void WAbstractItemView::initDragDrop()
{
  WApplication *app = WApplication::instance();

  /* item drag & drop */
  app->styleSheet().addRule
    ("#" + id() + "dw",
     "width: 32px; height: 32px;"
     "background: url(" + WApplication::resourcesUrl() + "items-not-ok.gif);");

  app->styleSheet().addRule
    ("#" + id() + "dw.Wt-valid-drop",
     "width: 32px; height: 32px;"
     "background: url(" + WApplication::resourcesUrl() + "items-ok.gif);");

  selectionChanged_.connect(this, &WAbstractItemView::checkDragSelection);
}

void WAbstractItemView::setColumnResizeEnabled(bool enabled)
{
  if (enabled != columnResize_) {
    columnResize_ = enabled;
    scheduleRerender(NeedRerenderHeader);
  }
}

void WAbstractItemView::setColumnAlignment(int column, AlignmentFlag alignment)
{
  columnInfo(column).alignment = alignment;

  const char *align = 0;
  switch (alignment) {
  case AlignLeft: align = "left"; break;
  case AlignCenter: align = "center"; break;
  case AlignRight: align = "right"; break;
  case AlignJustify: align = "justify"; break;
  default:
    break;
  }

  if (align) {
    WWidget *w = columnInfo(column).styleRule->templateWidget();
    w->setAttributeValue("style", std::string("text-align: ") + align);
  }
}

AlignmentFlag WAbstractItemView::columnAlignment(int column) const
{
  return columnInfo(column).alignment;
}

void WAbstractItemView::setHeaderAlignment(int column, AlignmentFlag alignment)
{
  columnInfo(column).headerAlignment = alignment;

  if (renderState_ >= NeedRerenderHeader)
    return;

  WContainerWidget *wc = dynamic_cast<WContainerWidget *>(headerWidget(column));
  
  wc->setContentAlignment(alignment);
}

AlignmentFlag WAbstractItemView::headerAlignment(int column) const 
{
  return columnInfo(column).headerAlignment;
}

void WAbstractItemView::setAlternatingRowColors(bool enable)
{
  alternatingRowColors_ = enable;
}

void WAbstractItemView::setSelectionMode(SelectionMode mode)
{
  if (mode != selectionMode_) {
    clearSelection();
    selectionMode_ = mode;
  }
}

void WAbstractItemView::setSelectionBehavior(SelectionBehavior behavior)
{
  if (behavior != selectionBehavior()) {
    clearSelection();
    selectionModel_->setSelectionBehavior(behavior);
  }
}

SelectionBehavior WAbstractItemView::selectionBehavior() const
{
  return selectionModel_->selectionBehavior();
}

void WAbstractItemView::setItemDelegate(WAbstractItemDelegate *delegate)
{
  itemDelegate_ = delegate;
  itemDelegate_->closeEditor()
    .connect(this, &WAbstractItemView::closeEditorWidget);
}

void WAbstractItemView::setItemDelegateForColumn(int column,
					 WAbstractItemDelegate *delegate)
{
  columnInfo(column).itemDelegate_ = delegate;
  delegate->closeEditor()
    .connect(this, &WAbstractItemView::closeEditorWidget);
}

WAbstractItemDelegate *WAbstractItemView::itemDelegateForColumn(int column) const
{
  return columnInfo(column).itemDelegate_;
}

WAbstractItemDelegate *WAbstractItemView::itemDelegate(const WModelIndex& index) const
{
  return itemDelegate(index.column());
}

WAbstractItemDelegate *WAbstractItemView::itemDelegate(int column) const
{
  WAbstractItemDelegate *result = itemDelegateForColumn(column);

  return result ? result : itemDelegate_;
}

std::string repeat(const std::string& s, int times)
{
  std::string result;
  for (int i = 0; i < times; ++i) {
    result += s;
  }
  return result;
}

void WAbstractItemView::dropEvent(const WDropEvent& e, const WModelIndex& index)
{
  /*
   * Here, we only handle standard drag&drop actions between abstract
   * item models.
   */
  if (dropsEnabled_) {
    std::vector<std::string> acceptMimeTypes = model_->acceptDropMimeTypes();

    for (unsigned i = 0; i < acceptMimeTypes.size(); ++i)
      if (acceptMimeTypes[i] == e.mimeType()) {
	// we define internal by sharing the same selection model...
	// currently selection models cannot be shared
	bool internal = e.source() == selectionModel_;

	DropAction action = internal ? MoveAction : CopyAction;

	// TODO: (later) we need to interpret the event location
	// (e.mouseEvent().widget().y) For now we will implement to
	// add as a sibling before the index
	model_->dropEvent(e, action,
			  index.row(), index.column(), index.parent());

	setSelectedIndexes(WModelIndexSet());
	return;
      }
  }

  WCompositeWidget::dropEvent(e);
}

void WAbstractItemView::configureModelDragDrop()
{
  initDragDrop();

  if (!model_)
    return;

  if (dragEnabled_) {
    setAttributeValue("dmt", model_->mimeType());
    setAttributeValue("dsid",
		      WApplication::instance()->encodeObject(selectionModel_));

    checkDragSelection();
  }

  std::vector<std::string> acceptMimeTypes = model_->acceptDropMimeTypes();

  for (unsigned i = 0; i < acceptMimeTypes.size(); ++i)
    if (dropsEnabled_)
      acceptDrops(acceptMimeTypes[i], "Wt-drop-site");
    else
      stopAcceptDrops(acceptMimeTypes[i]);
}


void WAbstractItemView::setDropsEnabled(bool enable)
{
  if (dropsEnabled_ != enable) {
    dropsEnabled_ = enable;

    configureModelDragDrop();
  }
}

void WAbstractItemView::checkDragSelection()
{
  /*
   * Check whether the current selection can be drag and dropped
   */
  std::string dragMimeType = model_->mimeType();

  if (!dragMimeType.empty()) {
    const WModelIndexSet& selection = selectionModel_->selectedIndexes();

    bool dragOk = !selection.empty();

    for (WModelIndexSet::const_iterator i = selection.begin();
	 i != selection.end(); ++i)
      if (!((*i).flags() & ItemIsDragEnabled)) {
	dragOk = false;
	break;
      }

    if (dragOk)
      setAttributeValue("drag", "true");
    else
      setAttributeValue("drag", "false");
  }
}

WText *WAbstractItemView::headerSortIconWidget(int column)
{
  if (!columnInfo(column).sorting)
    return 0;

  return dynamic_cast<WText *>(headerWidget(column)->find("sort"));
}

WText *WAbstractItemView::headerTextWidget(int column)
{
  return dynamic_cast<WText *>(headerWidget(column)->find("text"));
}

WWidget *WAbstractItemView::createExtraHeaderWidget(int column)
{
  return 0;
}

WWidget *WAbstractItemView::extraHeaderWidget(int column)
{
  return columnInfo(column).extraHeaderWidget;
}

void WAbstractItemView::toggleSortColumn(int columnid)
{
  int column = columnById(columnid);

  if (column != currentSortColumn_)
    sortByColumn(column, columnInfo(column).sortOrder);
  else
    sortByColumn(column, columnInfo(column).sortOrder == AscendingOrder
		 ? DescendingOrder : AscendingOrder);
}

int WAbstractItemView::columnById(int columnid) const
{
  for (int i = 0; i < columnCount(); ++i)
    if (columnInfo(i).id == columnid)
      return i;

  return 0;
}

int WAbstractItemView::columnCount() const
{
  return columns_.size();
}

WAbstractItemView::ColumnInfo WAbstractItemView::createColumnInfo(int column)
  const
{
  return ColumnInfo(this, nextColumnId_++, column);
}

WAbstractItemView::ColumnInfo& WAbstractItemView::columnInfo(int column) const
{
  while (column >= (int)columns_.size())
    columns_.push_back(createColumnInfo(columns_.size()));

  return columns_[column];
}

void WAbstractItemView::sortByColumn(int column, SortOrder order)
{
  if (currentSortColumn_ != -1) {
    WText* t = headerSortIconWidget(currentSortColumn_);
    if (t)
      t->setStyleClass("Wt-tv-sh Wt-tv-sh-none");
  }

  currentSortColumn_ = column;
  columnInfo(column).sortOrder = order;

  if (renderState_ != NeedRerender) {
    WText* t = headerSortIconWidget(currentSortColumn_);
    if (t)
      t->setStyleClass(order == AscendingOrder
		       ? "Wt-tv-sh Wt-tv-sh-up" : "Wt-tv-sh Wt-tv-sh-down");
  }

  model_->sort(column, order);
}

void WAbstractItemView::setSortingEnabled(bool enabled)
{
  sorting_ = enabled;
  for (int i = 0; i < columnCount(); ++i)
    columnInfo(i).sorting = enabled;

  scheduleRerender(NeedRerenderHeader);
}

void WAbstractItemView::setSortingEnabled(int column, bool enabled)
{
  columnInfo(column).sorting = enabled;

  scheduleRerender(NeedRerenderHeader);
}

bool WAbstractItemView::isSortingEnabled(int column) const
{
  return columnInfo(column).sorting;
}

void WAbstractItemView::modelReset()
{
  setModel(model_);
}

bool WAbstractItemView::internalSelect(const WModelIndex& index,
				       SelectionFlag option)
{
  if (!(index.flags() & ItemIsSelectable) || selectionMode() == NoSelection)
    return false;

  if (option == ToggleSelect)
    option = isSelected(index) ? Deselect : Select;
  else if (option == ClearAndSelect) {
    clearSelection();
    option = Select;
  } else if (selectionMode() == SingleSelection && option == Select)
    clearSelection();

  /*
   * now option is either Select or Deselect and we only need to do
   * exactly that one thing
   */
  if (option == Select)
    selectionModel()->selection_.insert(index);
  else
    if (!selectionModel()->selection_.erase(index))
      return false;

  return true;
}

void WAbstractItemView::clearSelection()
{
  WModelIndexSet& nodes = selectionModel_->selection_;

  while (!nodes.empty()) {
    WModelIndex i = *nodes.begin();
    internalSelect(i, Deselect);
  }
}

void WAbstractItemView::setSelectedIndexes(const WModelIndexSet& indexes)
{
  if (indexes.empty() && selectionModel_->selection_.empty())
    return;

  clearSelection();

  for (WModelIndexSet::const_iterator i = indexes.begin();
       i != indexes.end(); ++i)
    internalSelect(*i, Select);

  selectionChanged_.emit();
}

void WAbstractItemView::extendSelection(const WModelIndex& index)
{
  if (selectionModel_->selection_.empty())
    internalSelect(index, Select);
  else {
    if (selectionBehavior() == SelectRows && index.column() != 0) {
      extendSelection(model_->index(index.row(), 0, index.parent()));
      return;
    }

    /*
     * Expand current selection. If index is within or below the
     * current selection, we select from the top item to index. If index
     * is above the current selection, select everything from the
     * bottom item to index.
     *
     * For a WTreeView, only indexes with expanded ancestors can be
     * part of the selection: this is asserted when collapsing a index.
     */
    WModelIndex top = Utils::first(selectionModel_->selection_);
    if (top < index) {
      clearSelection();
      selectRange(top, index);
    } else {
      WModelIndex bottom = Utils::last(selectionModel_->selection_);
      clearSelection();
      selectRange(index, bottom);
    }
  }

  selectionChanged_.emit();
}

bool WAbstractItemView::isSelected(const WModelIndex& index) const
{
  return selectionModel_->isSelected(index);
}

void WAbstractItemView::select(const WModelIndex& index, SelectionFlag option)
{
  if (internalSelect(index, option))
    selectionChanged_.emit();
}

void WAbstractItemView::selectionHandleClick(const WModelIndex& index,
					     WFlags<KeyboardModifier> modifiers)
{
  if (selectionMode_ == NoSelection)
    return;

  if (selectionMode_ == ExtendedSelection) {
    if (modifiers & ShiftModifier)
      extendSelection(index);
    else {
      if (!(modifiers & (ControlModifier | MetaModifier))) {
	//if (isSelected(index)) -> strange MacOS X behavor
	//  return;
	//else {
	select(index, ClearAndSelect);
	//}
      } else
	select(index, ToggleSelect);
    }
  } else
    select(index, Select);
}

WModelIndexSet WAbstractItemView::selectedIndexes() const
{
  return selectionModel_->selection_;
}

void WAbstractItemView::scheduleRerender(RenderState what)
{
  if (!isRendered())
    return;

  if ((what == NeedRerenderHeader && renderState_ == NeedRerenderData)
      || (what == NeedRerenderData && renderState_ == NeedRerenderHeader))
    renderState_ = NeedRerender;
  else
    renderState_ = std::max(what, renderState_);

  askRerender();
}

WWidget *WAbstractItemView::createHeaderWidget(WApplication *app, int column)
{
  static const char *OneLine = "<div>&nbsp;</div>";

  int headerLevel
    = model_ 
    ? static_cast<int>(asNumber(model_->headerData(column, Horizontal,
						   LevelRole))) : 0;

  int rightBorderLevel = headerLevel;
  if (model_ && column + 1 < columnCount()) {
    WFlags<HeaderFlag> flagsLeft = model_->headerFlags(column);
    WFlags<HeaderFlag> flagsRight = model_->headerFlags(column + 1);

    int rightHeaderLevel 
      = static_cast<int>(asNumber(model_->headerData(column + 1, Horizontal,
						     LevelRole)));

    if (flagsLeft & ColumnIsExpandedRight)
      rightBorderLevel = headerLevel + 1;
    else if (flagsRight & ColumnIsExpandedLeft)
      rightBorderLevel = rightHeaderLevel + 1;
    else
      rightBorderLevel = std::min(headerLevel, rightHeaderLevel);
  }

  ColumnInfo& info = columnInfo(column);
  WContainerWidget *w = new WContainerWidget();
  w->setObjectName("contents");

  if (info.sorting) {
    WText *sortIcon = new WText(w);
    sortIcon->setObjectName("sort");
    sortIcon->setInline(false);
    if (!columnResize_)
      sortIcon->setMargin(4, Right);
    sortIcon->setStyleClass("Wt-tv-sh Wt-tv-sh-none");
    clickedForSortMapper_->mapConnect(sortIcon->clicked(), info.id);

    if (currentSortColumn_ == column)
      sortIcon->setStyleClass(info.sortOrder == AscendingOrder
			      ? "Wt-tv-sh Wt-tv-sh-up"
			      : "Wt-tv-sh Wt-tv-sh-down");
  }

  if (model_->headerFlags(column)
      & (ColumnIsExpandedLeft | ColumnIsExpandedRight)) {
    WImage *collapseIcon = new WImage(w);
    collapseIcon->setFloatSide(Left);
    collapseIcon->setImageRef(WApplication::resourcesUrl() + "minus.gif");
    clickedForCollapseMapper_->mapConnect(collapseIcon->clicked(), info.id);
  } else if (model_->headerFlags(column) & ColumnIsCollapsed) {
    WImage *expandIcon = new WImage(w);
    expandIcon->setFloatSide(Left);
    expandIcon->setImageRef(WApplication::resourcesUrl() + "plus.gif");
    clickedForExpandMapper_->mapConnect(expandIcon->clicked(), info.id);
  }    

  WText *t = new WText("&nbsp;", w);
  t->setObjectName("text");
  t->setStyleClass("Wt-label");
  t->setInline(false);
  if (multiLineHeader_ || app->environment().agentIsIE())
    t->setWordWrap(true);
  else
    t->setWordWrap(false);

  WContainerWidget *result = new WContainerWidget();

  if (headerLevel) {
    WContainerWidget *spacer = new WContainerWidget(result);
    t = new WText(spacer);
    t->setInline(false);

    if (rightBorderLevel < headerLevel) {
      if (rightBorderLevel) {
	t->setText(repeat(OneLine, rightBorderLevel));
	spacer = new WContainerWidget(result);
	t = new WText(spacer);
	t->setInline(false);
      }
      t->setText(repeat(OneLine, headerLevel - rightBorderLevel));
      spacer->setStyleClass("Wt-tv-br");
    } else {
      t->setText(repeat(OneLine, headerLevel));
    }
  }

  w->setStyleClass(w->styleClass()
		   + (rightBorderLevel <= headerLevel ? " Wt-tv-br" : ""));

  result->addWidget(w);
  result->setStyleClass(info.styleClass() + " Wt-tv-c headerrh");
  result->setContentAlignment(info.headerAlignment);

  WWidget *extraW = columnInfo(column).extraHeaderWidget;
  if (extraW) {
    result->addWidget(extraW);
    extraW->setStyleClass(extraW->styleClass() + " Wt-tv-br");
  }

  if (columnResize_ && app->environment().ajax()) {
    WContainerWidget *resizeHandle = new WContainerWidget();
    resizeHandle->setStyleClass("Wt-tv-rh headerrh");
    resizeHandle->mouseWentDown().connect(resizeHandleMDownJS_);

    bool ie = wApp->environment().agentIsIE();
    WContainerWidget *parent = ie ? w
      : dynamic_cast<WContainerWidget *>(result->widget(0));
    parent->insertWidget(0, resizeHandle);

    if (ie) {
      parent->setAttributeValue("style", "zoom: 1");
      parent->resize(WLength::Auto, headerLineHeight_);
    }
  }

  WText *spacer = new WText();
  spacer->setInline(false);
  spacer->setStyleClass("Wt-tv-br headerrh");
  result->addWidget(spacer);

  return result;
}

void WAbstractItemView::setDragEnabled(bool enable)
{
  if (dragEnabled_ != enable) {
    dragEnabled_ = enable;

    if (enable) {
      dragWidget_ = new WText(headerContainer());
      dragWidget_->setId(id() + "dw");
      dragWidget_->setInline(false);
      dragWidget_->hide();
      setAttributeValue("dwid", dragWidget_->id());

      configureModelDragDrop();
    } else {
      // TODO disable
    }
  }
}

int WAbstractItemView::headerLevelCount() const
{
  int result = 0;

  if (model_)
    for (unsigned int i = 0; i < columns_.size(); ++i) {
      int l = static_cast<int>(asNumber(model_->headerData(i, Horizontal,
							   LevelRole)));
      result = std::max(result, l);			
    }

  return result + 1;
}


void WAbstractItemView::setHeaderHeight(const WLength& height, bool multiLine)
{
  headerLineHeight_ = height;
  multiLineHeader_ = multiLine;

  int lineCount = headerLevelCount();
  WLength headerHeight = headerLineHeight_ * lineCount;

  headerHeightRule_->templateWidget()->resize(WLength::Auto, headerHeight);
  if (!multiLineHeader_)
    headerHeightRule_->templateWidget()->setLineHeight(headerLineHeight_);
  else
    headerHeightRule_->templateWidget()->setLineHeight(WLength::Auto);
}

void WAbstractItemView::bindObjJS(JSlot& slot, const std::string& jsMethod)
{
  slot.setJavaScript
    ("function(obj, event) {"
     """jQuery.data(" + jsRef() + ", 'obj')." + jsMethod + "(obj, event);"
     "}");
}

void WAbstractItemView::convertToRaw(WModelIndexSet& set, 
				     std::vector<void *>& result)
{
  for (WModelIndexSet::const_iterator i = set.begin(); i != set.end(); ++i) {
    void *rawIndex = model_->toRawIndex(*i);
    if (rawIndex)
      result.push_back(rawIndex);
  }

  set.clear();
}

void WAbstractItemView::modelLayoutAboutToBeChanged()
{
  convertToRaw(selectionModel_->selection_, selectionRaw_);
  rawRootIndex_ = model_->toRawIndex(rootIndex_);
}

void WAbstractItemView::modelLayoutChanged()
{
  if (rawRootIndex_)
    rootIndex_ = model_->fromRawIndex(rawRootIndex_);
  else
    rootIndex_ = WModelIndex();

  editedItems_.clear();

  for (unsigned i = 0; i < selectionRaw_.size(); ++i) {
    WModelIndex index = model_->fromRawIndex(selectionRaw_[i]);
    if (index.isValid())
      selectionModel_->selection_.insert(index);
  }
  selectionRaw_.clear();

  scheduleRerender(NeedRerenderData);
}

WWidget *WAbstractItemView::createPageNavigationBar()
{
  return new DefaultPagingBar(this);
}

void WAbstractItemView::collapseColumn(int columnid)
{
  model_->collapseColumn(columnById(columnid));
  scheduleRerender(NeedRerenderHeader);
  setHeaderHeight(headerLineHeight_, multiLineHeader_);
}

void WAbstractItemView::expandColumn(int columnid)
{
  model_->expandColumn(columnById(columnid));
  scheduleRerender(NeedRerenderHeader);
  setHeaderHeight(headerLineHeight_, multiLineHeader_);
}

void WAbstractItemView::handleClick(const WModelIndex& index,
				    const WMouseEvent& event)
{
  bool doEdit = ((editTriggers() & SelectedClicked) && isSelected(index))
    || (editTriggers() & SingleClicked);

  selectionHandleClick(index, event.modifiers());

  if (doEdit)
    edit(index);

  clicked_.emit(index, event);
}

void WAbstractItemView::handleDoubleClick(const WModelIndex& index,
					  const WMouseEvent& event)
{
  bool doEdit = editTriggers() & DoubleClicked;
  if (doEdit)
    edit(index);

  doubleClicked_.emit(index, event);
}

void WAbstractItemView::handleMouseDown(const WModelIndex& index,
					const WMouseEvent& event)
{
  mouseWentDown_.emit(index, event);
}

void WAbstractItemView::setEditTriggers(WFlags<EditTrigger> editTriggers)
{
  editTriggers_ = editTriggers;
}

void WAbstractItemView::setEditOptions(WFlags<EditOption> editOptions)
{
  editOptions_ = editOptions;
}

void WAbstractItemView::edit(const WModelIndex& index)
{ 
  if (index.flags() & ItemIsEditable && !isEditing(index)) {
    if (editOptions_ & SingleEditor) {
      while (!editedItems_.empty())
	closeEditor(editedItems_.begin()->first, false);
    }

    editedItems_[index].widget = 0;

    modelDataChanged(index, index);
  }
}

void WAbstractItemView::closeEditorWidget(WWidget *editor, bool saveData)
{
  for (EditorMap::const_iterator i = editedItems_.begin();
       i != editedItems_.end(); ++i)
    if (i->second.widget == editor) {
      closeEditor(i->first, saveData);
      return;
    }
}

void WAbstractItemView::closeEditor(const WModelIndex& index, bool saveData)
{
  EditorMap::iterator i = editedItems_.find(index);

  if (i != editedItems_.end()) {
    Editor& editor = i->second;

    if (saveData || editOptions_ & SaveWhenClosed)
      saveEditedValue(index, editor);

    WModelIndex closed = index;
#ifndef WT_TARGET_JAVA
    editedItems_.erase(i);
#else
    editedItems_.erase(index);
#endif

    modelDataChanged(closed, closed);
  }
}

bool WAbstractItemView::isEditing(const WModelIndex& index) const
{
  return editedItems_.find(index) != editedItems_.end();
}

bool WAbstractItemView::isEditing() const
{
  return !editedItems_.empty();
}

void WAbstractItemView::saveEditedValue(const WModelIndex& index,
					Editor& editor)
{
  boost::any editState;
  WAbstractItemDelegate *delegate = itemDelegate(index);
    
  if (editor.widget)
    editState = delegate->editState(editor.widget);
  else
    editState = editor.editState;

  delegate->setModelData(editState, model(), index);
}

void WAbstractItemView::setEditState(const WModelIndex& index,
				     const boost::any& editState)
{
  editedItems_[index].editState = editState;
}

void WAbstractItemView::setEditorWidget(const WModelIndex& index,
					WWidget *editor)
{
  editedItems_[index].widget = editor;  
}

boost::any WAbstractItemView::editState(const WModelIndex& index) const
{
  EditorMap::const_iterator i = editedItems_.find(index);

  if (i != editedItems_.end())
    return i->second.editState;
  else
    return boost::any();
}

}
