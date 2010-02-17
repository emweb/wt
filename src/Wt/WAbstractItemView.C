#include "Wt/WAbstractItemView"
#include "Wt/WItemSelectionModel"
#include "Wt/WItemDelegate"
#include "Wt/WAbstractItemModel"
#include "Wt/WEvent"
#include "Wt/WApplication"
#include "Wt/WText"
#include "Wt/WContainerWidget"
#include "Wt/WImage"
#include "Wt/WEnvironment"

#include "Utils.h"

namespace Wt {

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

  for (int i = columns_.size(); i < model_->columnCount(); ++i)
    columnInfo(i);

  configureModelDragDrop();

  scheduleRerender(NeedRerender);
  setHeaderHeight(headerLineHeight_, multiLineHeader_);
}

WAbstractItemView::WAbstractItemView(WContainerWidget *parent)
  : WCompositeWidget(parent),
    model_(0),
    itemDelegate_(new WItemDelegate(this)),
    selectionModel_(new WItemSelectionModel(0, this)),
    rowHeight_(20),
    headerLineHeight_(20),
    selectionMode_(NoSelection),
    sorting_(true),
    columnResize_(true),
    multiLineHeader_(false),
    nextColumnId_(1),
    clicked_(this),
    doubleClicked_(this),
    mouseWentDown_(this),
    mouseWentUp_(this),
    selectionChanged_(this),
    currentSortColumn_(-1),
    resizeHandleMDownJS_(this),
    resizeHandleMMovedJS_(this),
    resizeHandleMUpJS_(this),
    tieContentsHeaderScrollJS_(this),
    tieRowsScrollJS_(this),
    itemClickedJS_(this),
    itemDoubleClickedJS_(this),
    itemMouseDownJS_(this),
    itemMouseUpJS_(this),
    dragEnabled_(false),
    dropsEnabled_(false)
{
  clickedForSortMapper_ = new WSignalMapper<int>(this);
  clickedForSortMapper_->
    mapped().connect(SLOT(this,
			  WAbstractItemView::toggleSortColumn));
}

void WAbstractItemView::setColumnResizeEnabled(bool enabled)
{
  if (enabled != columnResize_) {
    columnResize_ = enabled;
    scheduleRerender(NeedRerenderHeader);
  }
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
}

void WAbstractItemView::setItemDelegateForColumn(int column,
					 WAbstractItemDelegate *delegate)
{
  columnInfo(column).itemDelegate_ = delegate;
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

void WAbstractItemView::modelReset()
{
  setModel(model_);
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

bool WAbstractItemView::isSelected(const WModelIndex& index) const
{
  return selectionModel_->isSelected(index);
}

void WAbstractItemView::select(const WModelIndex& index, SelectionFlag option)
{
  if (internalSelect(index, option))
    selectionChanged_.emit();
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

  if (columnResize_) {
    WContainerWidget *resizeHandle = new WContainerWidget();
    resizeHandle->setStyleClass("Wt-tv-rh headerrh");
    resizeHandle->mouseWentDown().connect(resizeHandleMDownJS_);
    resizeHandle->mouseWentUp().connect(resizeHandleMUpJS_);
    resizeHandle->mouseMoved().connect(resizeHandleMMovedJS_);

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

}
