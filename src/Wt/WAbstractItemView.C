/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractItemModel.h"
#include "Wt/WAbstractItemView.h"
#include "Wt/WAbstractTableModel.h"
#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WEnvironment.h"
#include "Wt/WEvent.h"
#include "Wt/WImage.h"
#include "Wt/WItemSelectionModel.h"
#include "Wt/WItemDelegate.h"
#include "Wt/WLogger.h"
#include "Wt/WPushButton.h"
#include "Wt/WText.h"
#include "Wt/WTheme.h"

#include "SizeHandle.h"
#include "WebUtils.h"

namespace Wt {

LOGGER("WAbstractItemView");

class DefaultPagingBar : public WContainerWidget
{
public:
  DefaultPagingBar(WAbstractItemView *view)
    : view_(view)
  {
    view_->addStyleClass("Wt-itemview-paged");
    setStyleClass("Wt-pagingbar");

    firstButton_ = addWidget(
          cpp14::make_unique<WPushButton>(
            tr("Wt.WAbstractItemView.PageBar.First")));
    firstButton_->clicked().connect(this, &DefaultPagingBar::showFirstPage);

    prevButton_ = addWidget(
          cpp14::make_unique<WPushButton>(
            tr("Wt.WAbstractItemView.PageBar.Previous")));
    prevButton_->clicked().connect(this, &DefaultPagingBar::showPreviousPage);

    current_ = addWidget(cpp14::make_unique<WText>());

    nextButton_ = addWidget(
          cpp14::make_unique<WPushButton>(
            tr("Wt.WAbstractItemView.PageBar.Next")));
    nextButton_->clicked().connect(this, &DefaultPagingBar::showNextPage);

    lastButton_ = addWidget(
          cpp14::make_unique<WPushButton>(
            tr("Wt.WAbstractItemView.PageBar.Last")));
    lastButton_->clicked().connect(this, &DefaultPagingBar::showLastPage);

    view_->pageChanged().connect(this, &DefaultPagingBar::update);

    update();
  }

private:
  WAbstractItemView *view_;
  observing_ptr<WPushButton> prevButton_, nextButton_,
    firstButton_, lastButton_;
  observing_ptr<WText> current_;

  void update() {
    firstButton_->setDisabled(view_->currentPage() == 0);
    prevButton_->setDisabled(view_->currentPage() == 0);

    nextButton_->setDisabled(view_->currentPage() == view_->pageCount() - 1);
    lastButton_->setDisabled(view_->currentPage() == view_->pageCount() - 1);

    current_->setText(WString::tr("Wt.WAbstractItemView.PageIOfN").
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

class HeaderProxyModel : public WAbstractTableModel
{
public:
  HeaderProxyModel(const std::shared_ptr<WAbstractItemModel>& model)
    : model_(model)
  { }

  virtual int columnCount(const WModelIndex& parent = WModelIndex()) const
    override
  {
    return model_->columnCount();
  }

  virtual int rowCount(const WModelIndex& parent = WModelIndex()) const
    override
  {
    return 1;
  }

  virtual cpp17::any data(const WModelIndex& index,
                       ItemDataRole role = ItemDataRole::Display) const override
  {
    return model_->headerData(index.column(), Orientation::Horizontal, role);
  }

  virtual bool setData(const WModelIndex& index, const cpp17::any& value,
                       ItemDataRole role = ItemDataRole::Edit) override
  {
    return model_->setHeaderData(index.column(), Orientation::Horizontal,
				 value, role);
  }
  
  virtual WFlags<ItemFlag> flags(const WModelIndex& index) const override
  {
    WFlags<HeaderFlag> headerFlags
      = model_->headerFlags(index.column(), Orientation::Horizontal);

    WFlags<ItemFlag> result;

    if (headerFlags.test(HeaderFlag::UserCheckable))
      result |= ItemFlag::UserCheckable;
    if (headerFlags.test(HeaderFlag::Tristate))
      result |= ItemFlag::Tristate;
    if (headerFlags.test(HeaderFlag::XHTMLText))
      result |= ItemFlag::XHTMLText;

    return result;
  }

private:
  std::shared_ptr<WAbstractItemModel> model_;
};

WAbstractItemView::ColumnInfo::ColumnInfo(const WAbstractItemView *view, 
					  int anId)
  : id(anId),
    sortOrder(SortOrder::Ascending),
    alignment(AlignmentFlag::Left),
    headerHAlignment(AlignmentFlag::Left),
    headerVAlignment(view->defaultHeaderVAlignment_),
    headerWordWrap(view->defaultHeaderWordWrap_),
    extraHeaderWidget(nullptr),
    sorting(view->sorting_),
    hidden(false),
    itemDelegate_(nullptr)
{
  width = WLength(150);

  std::unique_ptr<WCssTemplateRule> r
    (new WCssTemplateRule("#" + view->id() + " ." + styleClass()));
  r->templateWidget()->resize(width.toPixels(), WLength::Auto);
  styleRule = r.get();

  WApplication::instance()->styleSheet().addRule(std::move(r));
}

int WAbstractItemView::visibleColumnIndex(int modelColumn) const
{
  if (columns_[modelColumn].hidden)
    return -1;

  int j = 0;

  for (int i = 0; i < modelColumn; ++i)
    if (!columns_[i].hidden)
      ++j;

  return j;
}

int WAbstractItemView::modelColumnIndex(int visibleColumn) const
{
  int j = -1;

  for (int i = 0; i <= visibleColumn; ++i) {
    ++j;
    while (static_cast<unsigned>(j) < columns_.size() && columns_[j].hidden)
      ++j;

    if (static_cast<unsigned>(j) >= columns_.size())
      return -1;
  }

  return j;
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
  return "Wt-tv-c" + std::to_string(id);
#endif
}

WAbstractItemView::WAbstractItemView()
  : WCompositeWidget(std::unique_ptr<WWidget>(new WContainerWidget())),
    renderState_(RenderState::NeedRerender),
    currentSortColumn_(-1),
    dragEnabled_(false),
    dropsEnabled_(false),
    selectionModel_(new WItemSelectionModel()),
    rowHeight_(20),
    headerLineHeight_(20),
    selectionMode_(SelectionMode::None),
    sorting_(true),
    columnResize_(true),
    defaultHeaderVAlignment_(AlignmentFlag::Middle),
    defaultHeaderWordWrap_(true),
    rowHeaderCount_(0),
    sortEnabled_(true),
    columnWidthChanged_(implementation(), "columnResized"),
    nextColumnId_(1),
    alternatingRowColors_(false),
    editTriggers_(EditTrigger::DoubleClicked),
    editOptions_(EditOption::SingleEditor),
    touchRegistered_(false)
{
  impl_ = dynamic_cast<WContainerWidget *>(implementation());

  auto d = std::make_shared<WItemDelegate>();
  setItemDelegate(d);
  setHeaderItemDelegate(d);

  WApplication *app = WApplication::instance();

  SizeHandle::loadJavaScript(app);

  if (!app->environment().ajax())
    columnResize_ = false;

  bindObjJS(resizeHandleMDownJS_, "resizeHandleMDown");

  headerHeightRule_ = new WCssTemplateRule("#" + id() + " .headerrh");
  app->styleSheet().addRule(std::unique_ptr<WCssRule>(headerHeightRule_.get()));
  setHeaderHeight(headerLineHeight_);
}

WAbstractItemView::~WAbstractItemView()
{
  WApplication *app = WApplication::instance();

  app->styleSheet().removeRule(headerHeightRule_.get());

  for (unsigned i = 0; i < columns_.size(); ++i)
    app->styleSheet().removeRule(columns_[i].styleRule.get());
}

void WAbstractItemView
::setModel(const std::shared_ptr<WAbstractItemModel>& model)
{
  if (!columnWidthChanged_.isConnected())
    columnWidthChanged_.connect(this, &WAbstractItemView::updateColumnWidth);

  bool isReset = model_.get();

  /* disconnect slots from previous model */
  for (unsigned i = 0; i < modelConnections_.size(); ++i)
    modelConnections_[i].disconnect();
  modelConnections_.clear();

  model_ = model;
  headerModel_.reset(new HeaderProxyModel(model_));

  auto oldSelectionModel = std::move(selectionModel_);
  selectionModel_.reset(new WItemSelectionModel(model));
  selectionModel_->setSelectionBehavior(oldSelectionModel->selectionBehavior());

  delayedClearAndSelectIndex_ = WModelIndex();

  editedItems_.clear();

  if (!isReset)
    initDragDrop();

  configureModelDragDrop();

  setRootIndex(WModelIndex());

  setHeaderHeight(headerLineHeight_);
}

void WAbstractItemView::setRootIndex(const WModelIndex& rootIndex)
{
  rootIndex_ = rootIndex;

  scheduleRerender(RenderState::NeedRerender);

  unsigned modelColumnCount = model_->columnCount(rootIndex_);

  WApplication *app = WApplication::instance();

  while (columns_.size() > modelColumnCount) {
    int i = columns_.size() - 1;
    app->styleSheet().removeRule(columns_[i].styleRule.get());
    columns_.erase(columns_.begin() + i);
  }

  while (columns_.size() < modelColumnCount)
    columns_.push_back(createColumnInfo(columns_.size()));
}

void WAbstractItemView::setRowHeight(const WLength& rowHeight)
{
  rowHeight_ = rowHeight;
}

void WAbstractItemView::setRowHeaderCount(int count)
{
  rowHeaderCount_ = count;
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

    WWidget *w = headerWidget(column, 0);
    if (w)
      w->scheduleRender(RepaintFlag::SizeAffected); // for layout
  }
}

void WAbstractItemView::setColumnHidden(int column, bool hidden)
{
  columnInfo(column).hidden = hidden;
}

bool WAbstractItemView::isColumnHidden(int column) const
{
  return columnInfo(column).hidden;
}

void WAbstractItemView::hideColumn(int column)
{
  setColumnHidden(column, true);
}

void WAbstractItemView::showColumn(int column)
{
  setColumnHidden(column, false);
}

void WAbstractItemView::initDragDrop()
{
  /* item drag & drop */
  addCssRule
    ("#" + id() + "dw",
     "width: 32px; height: 32px;"
     "background: url(" + WApplication::resourcesUrl() + "items-not-ok.gif);");

  addCssRule
    ("#" + id() + "dw.Wt-valid-drop",
     "width: 32px; height: 32px;"
     "background: url(" + WApplication::resourcesUrl() + "items-ok.gif);");

  selectionChanged_.connect(this, &WAbstractItemView::checkDragSelection);
}

void WAbstractItemView::setColumnResizeEnabled(bool enabled)
{
  if (enabled != columnResize_) {
    columnResize_ = enabled;
    scheduleRerender(RenderState::NeedRerenderHeader);
  }
}

void WAbstractItemView::setColumnAlignment(int column, AlignmentFlag alignment)
{
  columnInfo(column).alignment = alignment;

  WApplication *app = WApplication::instance();

  const char *align = nullptr;
  switch (alignment) {
  case AlignmentFlag::Left:
    align = app->layoutDirection() == LayoutDirection::LeftToRight 
      ? "left" : "right";
    break;
  case AlignmentFlag::Center: align = "center"; break;
  case AlignmentFlag::Right:
    align = app->layoutDirection() == LayoutDirection::LeftToRight 
      ? "right" : "left";
    break;
  case AlignmentFlag::Justify: align = "justify"; break;
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

void WAbstractItemView::setHeaderAlignment(int column,
					   WFlags<AlignmentFlag> alignment)
{
  columnInfo(column).headerHAlignment = alignment & AlignHorizontalMask;

  if (alignment.test(AlignVerticalMask))
    columnInfo(column).headerVAlignment = alignment & AlignVerticalMask;

  if (columnInfo(column).hidden || 
      static_cast<unsigned int>(renderState_) >=
      static_cast<unsigned int>(RenderState::NeedRerenderHeader))
    return;

  WContainerWidget *wc = dynamic_cast<WContainerWidget *>(headerWidget(column));
  
  wc->setContentAlignment(alignment);

  if (columnInfo(column).headerVAlignment == AlignmentFlag::Middle)
    wc->setLineHeight(headerLineHeight_);
  else
    wc->setLineHeight(WLength::Auto);
}

void WAbstractItemView::setHeaderWordWrap(int column, bool enabled)
{
  columnInfo(column).headerWordWrap = enabled;

  if (columnInfo(column).hidden || 
      static_cast<unsigned int>(renderState_) >= 
      static_cast<unsigned int>(RenderState::NeedRerenderHeader))
    return;

  if (columnInfo(column).headerVAlignment == AlignmentFlag::Top) {
    WContainerWidget *wc
      = dynamic_cast<WContainerWidget *>(headerWidget(column));
    wc->toggleStyleClass("Wt-wwrap", enabled);
  }
}

AlignmentFlag WAbstractItemView::horizontalHeaderAlignment(int column) const
{
  return columnInfo(column).headerHAlignment;
}

AlignmentFlag WAbstractItemView::verticalHeaderAlignment(int column) const
{
  return columnInfo(column).headerVAlignment;
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

void WAbstractItemView
::setItemDelegate(const std::shared_ptr<WAbstractItemDelegate>& delegate)
{
  itemDelegate_ = delegate;
  itemDelegate_->closeEditor()
    .connect(this, &WAbstractItemView::closeEditorWidget);
}

void WAbstractItemView
::setItemDelegateForColumn(int column,
			const std::shared_ptr<WAbstractItemDelegate>& delegate)
{
  columnInfo(column).itemDelegate_ = delegate;
  delegate->closeEditor()
    .connect(this, &WAbstractItemView::closeEditorWidget);
}

std::shared_ptr<WAbstractItemDelegate> WAbstractItemView
::itemDelegateForColumn(int column)
  const
{
  return columnInfo(column).itemDelegate_;
}

std::shared_ptr<WAbstractItemDelegate> WAbstractItemView
::itemDelegate(const WModelIndex& index)
  const
{
  return itemDelegate(index.column());
}

std::shared_ptr<WAbstractItemDelegate> WAbstractItemView
::itemDelegate(int column) const
{
  auto result = itemDelegateForColumn(column);

  return result ? result : itemDelegate_;
}

void WAbstractItemView::
setHeaderItemDelegate(const std::shared_ptr<WAbstractItemDelegate>& delegate)
{
  headerItemDelegate_ = delegate;
}

std::shared_ptr<WAbstractItemDelegate> WAbstractItemView::headerItemDelegate()
  const
{
  return headerItemDelegate_;
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
	bool internal = e.source() == selectionModel_.get();

	DropAction action = internal ? 
	  DropAction::Move : DropAction::Copy;

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
    setAttributeValue
      ("dsid",
       WApplication::instance()->encodeObject(selectionModel_.get()));

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
  computedDragMimeType_ = selectionModel_->mimeType();

  setAttributeValue("dmt", computedDragMimeType_);

  if (!computedDragMimeType_.empty())
    setAttributeValue("drag", "true");
  else
    setAttributeValue("drag", "false");
}

WText *WAbstractItemView::headerSortIconWidget(int column)
{
  if (!columnInfo(column).sorting)
    return nullptr;

  WWidget *hw = headerWidget(column);
  if (hw)
    return dynamic_cast<WText *>(hw->find("sort"));
  else
    return nullptr;
}

std::unique_ptr<WWidget> WAbstractItemView::createExtraHeaderWidget(int column)
{
  return std::unique_ptr<WWidget>();
}

WWidget *WAbstractItemView::extraHeaderWidget(int column)
{
  return columnInfo(column).extraHeaderWidget.get();
}

void WAbstractItemView::handleHeaderClicked(int columnid, WMouseEvent event)
{
  int column = columnById(columnid);
  ColumnInfo& info = columnInfo(column);

  if (sortEnabled_ && info.sorting)
    toggleSortColumn(columnid);

  headerClicked_.emit(column, event);
}

void WAbstractItemView::handleHeaderDblClicked(int columnid, WMouseEvent event)
{
  headerDblClicked_.emit(columnById(columnid), event);
}

void WAbstractItemView::handleHeaderMouseDown(int columnid, WMouseEvent event)
{
  headerMouseWentDown_.emit(columnById(columnid), event);
}

void WAbstractItemView::handleHeaderMouseUp(int columnid, WMouseEvent event)
{
  headerMouseWentUp_.emit(columnById(columnid), event);
}

void WAbstractItemView::toggleSortColumn(int columnid)
{
  int column = columnById(columnid);

  if (column != currentSortColumn_)
    sortByColumn(column, columnInfo(column).sortOrder);
  else
    sortByColumn(column, 
		 columnInfo(column).sortOrder == SortOrder::Ascending
		 ? SortOrder::Descending : SortOrder::Ascending);
}

int WAbstractItemView::sortColumn() const
{
  return currentSortColumn_;
}

SortOrder WAbstractItemView::sortOrder() const
{
  if (currentSortColumn_ >= 0
      && currentSortColumn_ < static_cast<int>(columns_.size()))
    return columns_[currentSortColumn_].sortOrder;
  else
    return SortOrder::Ascending;
}

int WAbstractItemView::columnById(int columnid) const
{
  for (unsigned i = 0; i < columns_.size(); ++i)
    if (columnInfo(i).id == columnid)
      return i;

  return 0;
}

int WAbstractItemView::columnCount() const
{
  return columns_.size();
}

int WAbstractItemView::visibleColumnCount() const
{
  int result = 0;

  for (unsigned i = 0; i < columns_.size(); ++i)
    if (!columns_[i].hidden)
      ++result;

  return result;
}

WAbstractItemView::ColumnInfo WAbstractItemView::createColumnInfo(int column)
  const
{
  return ColumnInfo(this, nextColumnId_++);
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

  if (renderState_ != RenderState::NeedRerender) {
    WText* t = headerSortIconWidget(currentSortColumn_);
    if (t)
      t->setStyleClass(order == SortOrder::Ascending
		       ? "Wt-tv-sh Wt-tv-sh-up" : "Wt-tv-sh Wt-tv-sh-down");
  }

  model_->sort(column, order);
}

void WAbstractItemView::setSortingEnabled(bool enabled)
{
  sorting_ = enabled;
  for (unsigned i = 0; i < columns_.size(); ++i)
    columnInfo(i).sorting = enabled;

  scheduleRerender(RenderState::NeedRerenderHeader);
}

void WAbstractItemView::setSortingEnabled(int column, bool enabled)
{
  columnInfo(column).sorting = enabled;

  scheduleRerender(RenderState::NeedRerenderHeader);
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
  if (!(index.flags() & ItemFlag::Selectable) || 
      selectionMode() == SelectionMode::None)
    return false;

  if (option == SelectionFlag::ToggleSelect)
    option = isSelected(index) 
      ? SelectionFlag::Deselect : SelectionFlag::Select;

  if (selectionMode() == SelectionMode::Single &&
      option == SelectionFlag::Select)
    option = SelectionFlag::ClearAndSelect;

  if ((option == SelectionFlag::ClearAndSelect || 
       option == SelectionFlag::Select) &&
      selectionModel()->selection_.size() == 1 &&
      isSelected(index))
    return false;
  else if (option == SelectionFlag::Deselect && !isSelected(index))
    return false;

  if (option == SelectionFlag::ClearAndSelect) {
    clearSelection();
    option = SelectionFlag::Select;
  }

  /*
   * now option is either SelectionFlag::Select or SelectionFlag::Deselect and we only need to do
   * exactly that one thing
   */
  if (option == SelectionFlag::Select)
    selectionModel()->selection_.insert(index);
  else
    selectionModel()->selection_.erase(index);

  return true;
}

void WAbstractItemView::clearSelection()
{
  WModelIndexSet& nodes = selectionModel_->selection_;

  while (!nodes.empty()) {
    WModelIndex i = *nodes.begin();
    internalSelect(i, SelectionFlag::Deselect);
  }
}

void WAbstractItemView::setSelectedIndexes(const WModelIndexSet& indexes)
{
  if (indexes.empty() && selectionModel_->selection_.empty())
    return;

  clearSelection();

  for (WModelIndexSet::const_iterator i = indexes.begin();
       i != indexes.end(); ++i)
    internalSelect(*i, SelectionFlag::Select);

  selectionChanged_.emit();
}

void WAbstractItemView::extendSelection(const WModelIndex& index)
{
  if (selectionModel_->selection_.empty())
    internalSelect(index, SelectionFlag::Select);
  else {
    if (selectionBehavior() == SelectionBehavior::Rows &&
	index.column() != 0) {
      extendSelection(model_->index(index.row(), 0, index.parent()));
      return;
    }
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
  selectionChanged_.emit();
}


void WAbstractItemView::extendSelection(const std::vector<WModelIndex>& indices)
{
  const WModelIndex &firstIndex = indices[0];
  const WModelIndex &secondIndex = indices[indices.size()-1];
  if (indices.size() > 1) {
    if(firstIndex.row() > secondIndex.row())
      selectRange(secondIndex, firstIndex);
    else
      selectRange(firstIndex, secondIndex);
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
  if (selectionMode_ == SelectionMode::None)
    return;

  if (selectionMode_ == SelectionMode::Extended) {
    if (modifiers.test(KeyboardModifier::Shift))
      extendSelection(index);
    else {
      if (!(modifiers & (KeyboardModifier::Control | 
			 KeyboardModifier::Meta))) {
	if (!dragEnabled_)
	  select(index, SelectionFlag::ClearAndSelect);
	else {
	  if (!isSelected(index))
	    select(index, SelectionFlag::ClearAndSelect);
	  else
	    delayedClearAndSelectIndex_ = index;
	}
      } else
	select(index, SelectionFlag::ToggleSelect);
    }
  } else {
    if (!(modifiers & (KeyboardModifier::Control | 
		      KeyboardModifier::Meta)).empty() &&
	isSelected(index)) {
      clearSelection();
      selectionChanged_.emit();
    } else
      select(index, SelectionFlag::Select);
  }
}

void WAbstractItemView::selectionHandleTouch(const std::vector<WModelIndex>& indices,
					     const WTouchEvent& event)
{
  if (selectionMode_ == SelectionMode::None)
    return;

  const WModelIndex &index = indices[0];

  if (selectionMode_ == SelectionMode::Extended) {
    if (event.touches().size() > 1)
      extendSelection(indices);
    else {
      select(index, SelectionFlag::ToggleSelect);
    }
  } else {
    if (isSelected(index)) {
      clearSelection();
      selectionChanged_.emit();
    } else
      select(index, SelectionFlag::ClearAndSelect);
  }
}

WModelIndexSet WAbstractItemView::selectedIndexes() const
{
  return selectionModel_->selection_;
}

void WAbstractItemView::scheduleRerender(RenderState what)
{
  if ((what == RenderState::NeedRerenderHeader && 
       renderState_ == RenderState::NeedRerenderData)
      || (what == RenderState::NeedRerenderData && 
	  renderState_ == RenderState::NeedRerenderHeader))
    renderState_ = RenderState::NeedRerender;
  else
    renderState_ = std::max(what, renderState_);

  if (!isRendered())
    return;

  scheduleRender();
}

void WAbstractItemView::modelHeaderDataChanged(Orientation orientation,
					       int start, int end)
{
  if (static_cast<unsigned int>(renderState_) < 
      static_cast<unsigned int>(RenderState::NeedRerenderHeader)) {
    if (orientation == Orientation::Horizontal) {
      for (int i = start; i <= end; ++i) {
	WContainerWidget *hw
	  = dynamic_cast<WContainerWidget *>(headerWidget(i, true));
	WWidget *tw = hw->widget(hw->count() - 1);
	headerItemDelegate_->update(tw, headerModel_->index(0, i), None);
	tw->setInline(false);
	tw->addStyleClass("Wt-label");

        WWidget *h = headerWidget(i, false);
        ColumnInfo& info = columnInfo(i);
        h->setStyleClass(info.styleClass() + " Wt-tv-c headerrh");
        WT_USTRING sc = asString(headerModel_->index(0, i)
				 .data(ItemDataRole::StyleClass));
        if (!sc.empty())
          h->addStyleClass(sc);
      }
    }
  }
}

int WAbstractItemView::headerLevel(int column) const
{
  cpp17::any d = model_->headerData(column, Orientation::Horizontal,
				 ItemDataRole::Level);

  if (cpp17::any_has_value(d))
    return static_cast<int>(asNumber(d));
  else
    return 0;
}

void WAbstractItemView::saveExtraHeaderWidgets()
{
  for (int i = 0; i < columnCount(); ++i) {
    WWidget *w = columnInfo(i).extraHeaderWidget.get();
    if (w && w->parent()) {
      w->parent()->removeWidget(w).release();
    }
  }
}

std::unique_ptr<WWidget> WAbstractItemView::createHeaderWidget(int column)
{
  /****
   * result:
   * +------------------------------+
   * | +----------------------+     +
   * | | +------------------+ |     |
   * | | | contents         | |     |
   * | | +------------------+ | +-+ |
   * | | +------------------+ | | | |
   * | | | extra widget     | | | <------ resize handle with border right and
   * | | | ....             | | | | |     margin-top for right-border-level
   * | | +------------------+ | | | |
   * | +----------------------+ +-+ |
   * +------------------------------+    
   */

  ColumnInfo& info = columnInfo(column);

  /* Contents */

  std::unique_ptr<WContainerWidget> contents(new WContainerWidget());
  contents->setObjectName("contents");

  if (info.sorting) {
    std::unique_ptr<WText> sortIcon(new WText());
    sortIcon->setObjectName("sort");
    sortIcon->setInline(false);
    sortIcon->setStyleClass("Wt-tv-sh Wt-tv-sh-none");
    if (currentSortColumn_ == column)
      sortIcon->setStyleClass(info.sortOrder == SortOrder::Ascending
			      ? "Wt-tv-sh Wt-tv-sh-up"
			      : "Wt-tv-sh Wt-tv-sh-down");
    contents->addWidget(std::move(sortIcon));
  }

  if (!(model_->headerFlags(column) & (HeaderFlag::ColumnIsExpandedLeft | 
				     HeaderFlag::ColumnIsExpandedRight)).empty()) {
    std::unique_ptr<WImage> collapseIcon(new WImage());
    collapseIcon->setFloatSide(Side::Left);
    collapseIcon
      ->setImageLink(WLink(WApplication::relativeResourcesUrl() + "minus.gif"));
    collapseIcon->clicked().connect
      (this, std::bind(&WAbstractItemView::collapseColumn, this, info.id));
    contents->addWidget(std::move(collapseIcon));
  } else if (model_->headerFlags(column).test(HeaderFlag::ColumnIsCollapsed)) {
    std::unique_ptr<WImage> expandIcon(new WImage());
    expandIcon->setFloatSide(Side::Left);
    expandIcon
      ->setImageLink(WLink(WApplication::relativeResourcesUrl() + "plus.gif"));
    expandIcon->clicked().connect
      (this, std::bind(&WAbstractItemView::expandColumn, this, info.id));
    contents->addWidget(std::move(expandIcon));
  }    

  WModelIndex index = headerModel_->index(0, column);
  std::unique_ptr<WWidget> i(headerItemDelegate_->update(nullptr, index, None));
  i->setInline(false);
  i->addStyleClass("Wt-label");
  contents->addWidget(std::move(i));

  int headerLevel = model_ ? this->headerLevel(column) : 0;

  contents->setMargin(headerLevel * headerLineHeight_.toPixels(), Side::Top);

  /* Resize handle (or border-right 1 stub) */

  int rightBorderLevel = headerLevel;
  if (model_) {
    int rightColumn = modelColumnIndex(visibleColumnIndex(column) + 1);
    if (rightColumn != -1) {
      WFlags<HeaderFlag> flagsLeft = model_->headerFlags(column);
      WFlags<HeaderFlag> flagsRight = model_->headerFlags(rightColumn);
      
      int rightHeaderLevel = this->headerLevel(rightColumn);

      if (flagsLeft.test(HeaderFlag::ColumnIsExpandedRight))
	rightBorderLevel = headerLevel + 1;
      else if (flagsRight.test(HeaderFlag::ColumnIsExpandedLeft))
	rightBorderLevel = rightHeaderLevel + 1;
      else
	rightBorderLevel = std::min(headerLevel, rightHeaderLevel);
    }
  }

  bool activeRH = columnResize_;

  std::unique_ptr<WContainerWidget> resizeHandle(new WContainerWidget());
  resizeHandle->setStyleClass(std::string("Wt-tv-rh")
			      + (activeRH ? "" : " Wt-tv-no-rh" )
			      + " Wt-tv-br headerrh");

  if (activeRH)
    resizeHandle->mouseWentDown().connect(resizeHandleMDownJS_);

  resizeHandle->setMargin(rightBorderLevel * headerLineHeight_.toPixels(),
			  Side::Top);

  /*
   * Extra widget
   */

  if (!columnInfo(column).extraHeaderWidget)
    columnInfo(column).extraHeaderWidget
      = createExtraHeaderWidget(column).release();

  std::unique_ptr<WWidget> extraW(columnInfo(column).extraHeaderWidget.get());

  /*
   * Assemble into result
   */
  std::unique_ptr<WContainerWidget> result(new WContainerWidget());
  result->setStyleClass(info.styleClass() + " Wt-tv-c headerrh");
  result->addWidget(std::move(resizeHandle));

  std::unique_ptr<WContainerWidget> main(new WContainerWidget());
  main->setOverflow(Overflow::Hidden);
  main->setContentAlignment(info.headerHAlignment);
  main->addWidget(std::move(contents));

  if (info.headerVAlignment == AlignmentFlag::Middle)
    main->setLineHeight(headerLineHeight_);
  else {
    main->setLineHeight(WLength::Auto);
    if (info.headerWordWrap)
      main->addStyleClass("Wt-wwrap");
  }

  if (extraW)
    main->addWidget(std::move(extraW));

  main->clicked().connect(this, 
	  std::bind(&WAbstractItemView::handleHeaderClicked, this,
		    info.id, std::placeholders::_1));
  main->mouseWentDown().connect(this,
          std::bind(&WAbstractItemView::handleHeaderMouseDown, this,
		    info.id, std::placeholders::_1));
  main->mouseWentUp().connect(this,
          std::bind(&WAbstractItemView::handleHeaderMouseUp, this,
		    info.id, std::placeholders::_1));
  main->doubleClicked().connect(this,
	  std::bind(&WAbstractItemView::handleHeaderDblClicked, this,
		    info.id, std::placeholders::_1));

  result->addWidget(std::move(main));

  WT_USTRING sc = asString(index.data(ItemDataRole::StyleClass));
  if (!sc.empty())
    result->addStyleClass(sc);

  return std::move(result);
}

void WAbstractItemView::enableAjax()
{
  WCompositeWidget::enableAjax();
  if (uDragWidget_) {
    headerContainer()->addWidget(std::move(uDragWidget_));
    configureModelDragDrop();
  }
}

void WAbstractItemView::setDragEnabled(bool enable)
{
  if (dragEnabled_ != enable) {
    dragEnabled_ = enable;

    if (enable) {
      uDragWidget_ = std::unique_ptr<WText>(new WText());
      dragWidget_ = uDragWidget_.get();
      dragWidget_->setId(id() + "dw");
      dragWidget_->setInline(false);
      dragWidget_->hide();
      setAttributeValue("dwid", dragWidget_->id());
      if (headerContainer())
        headerContainer()->addWidget(std::move(uDragWidget_));

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
    for (unsigned int i = 0; i < columns_.size(); ++i)
      if (!columns_[i].hidden)
	result = std::max(result, headerLevel(i));

  return result + 1;
}

void WAbstractItemView::setHeaderHeight(const WLength& height)
{
  headerLineHeight_ = height;  

  int lineCount = headerLevelCount();
  WLength headerHeight = headerLineHeight_ * lineCount;

  if (columns_.size() > 0) {
    WWidget *w = headerWidget(0, false);
    if (w)
      w->scheduleRender(RepaintFlag::SizeAffected); // for layout
  }

  headerHeightRule_->templateWidget()->resize(WLength::Auto, headerHeight);
}

void WAbstractItemView::bindObjJS(JSlot& slot, const std::string& jsMethod)
{
  slot.setJavaScript
    ("function(obj, event) {"
     """" + jsRef() + ".wtObj." + jsMethod + "(obj, event);"
     "}");
}

void WAbstractItemView::connectObjJS(EventSignalBase& s,
				     const std::string& jsMethod)
{
  s.connect
    ("function(obj, event) {"
     """" + jsRef() + ".wtObj." + jsMethod + "(obj, event);"
     "}");
}

void WAbstractItemView::modelLayoutAboutToBeChanged()
{
  if (rootIndex_.isValid())
    rootIndex_.encodeAsRawIndex();

  for (EditorMap::iterator i = editedItems_.begin(); i != editedItems_.end();
       ++i) {
    persistEditor(i->first, i->second);
    (const_cast<WModelIndex &>(i->first)).encodeAsRawIndex();
  }

  selectionModel_->modelLayoutAboutToBeChanged();
}

void WAbstractItemView::modelLayoutChanged()
{
  if (rootIndex_.isValid())
    rootIndex_ = rootIndex_.decodeFromRawIndex();

  EditorMap newEditorMap;
  for (EditorMap::iterator i = editedItems_.begin(); i != editedItems_.end();
       ++i) {
    WModelIndex m = i->first.decodeFromRawIndex();
    if (m.isValid())
      newEditorMap[m] = i->second;
  }
  editedItems_.swap(newEditorMap);

  selectionModel_->modelLayoutChanged();

  scheduleRerender(RenderState::NeedRerenderData);

  selectionChanged().emit();
}

std::unique_ptr<WWidget> WAbstractItemView::createPageNavigationBar()
{
  return std::unique_ptr<WWidget>(new DefaultPagingBar(this));
}

void WAbstractItemView::collapseColumn(int columnid)
{
  model_->collapseColumn(columnById(columnid));
  scheduleRerender(RenderState::NeedRerenderHeader);
  setHeaderHeight(headerLineHeight_);
}

void WAbstractItemView::expandColumn(int columnid)
{
  model_->expandColumn(columnById(columnid));
  scheduleRerender(RenderState::NeedRerenderHeader);
  setHeaderHeight(headerLineHeight_);
}

void WAbstractItemView::handleClick(const WModelIndex& index,
				    const WMouseEvent& event)
{
  if (dragEnabled_ && delayedClearAndSelectIndex_.isValid()) {
    Coordinates delta = event.dragDelta();
    if ((delta.x < 0 ? -delta.x : delta.x) < 4 && (delta.y < 0 ? -delta.y : delta.y) < 4)
      select(delayedClearAndSelectIndex_, SelectionFlag::ClearAndSelect);
  }

  bool doEdit = index.isValid() && editTriggers().test(EditTrigger::SingleClicked);

  if (doEdit)
    edit(index);

  clicked_.emit(index, event);
}

void WAbstractItemView::handleDoubleClick(const WModelIndex& index,
					  const WMouseEvent& event)
{
  bool doEdit = index.isValid() && editTriggers().test(EditTrigger::DoubleClicked);
  if (doEdit)
    edit(index);

  doubleClicked_.emit(index, event);
}

void WAbstractItemView::handleMouseDown(const WModelIndex& index,
					const WMouseEvent& event)
{
  // Needed because mousedown signal is emitted after a touchstart signal
  if (touchRegistered_)
    return;

  bool doEdit = index.isValid() &&
    editTriggers().test(EditTrigger::SelectedClicked) && isSelected(index);

  delayedClearAndSelectIndex_ = WModelIndex();

  if (index.isValid() && event.button() == MouseButton::Left)
    selectionHandleClick(index, event.modifiers());

  if (doEdit)
    edit(index);

  mouseWentDown_.emit(index, event);
  touchRegistered_ = false;
}

void WAbstractItemView::handleMouseUp(const WModelIndex& index,
				      const WMouseEvent& event)
{
  mouseWentUp_.emit(index, event);
}

void WAbstractItemView::handleTouchSelect(const std::vector<WModelIndex>& indices,
                                          const WTouchEvent& event)
{
  if (indices.empty())
    return; // no indices, likely due to faulty input

  const WModelIndex& index = indices[0];
  touchRegistered_ = true;
  delayedClearAndSelectIndex_ = WModelIndex();
  if (indices.size() == 1) {

    bool doEdit = index.isValid() &&
        (editTriggers().test(EditTrigger::SelectedClicked)) && isSelected(index);

    if (doEdit)
      edit(index);
  }
  if (indices[0].isValid() && indices[indices.size()-1].isValid()){
    selectionHandleTouch(indices, event);
  }

  touchStart_.emit(index, event);
}

void WAbstractItemView::handleTouchStart(const std::vector<WModelIndex>& indices,
					   const WTouchEvent& event)
{
  touchStarted_.emit(indices, event);
}

void WAbstractItemView::handleTouchMove(const std::vector<WModelIndex>& indices,
                                        const WTouchEvent& event)
{
  touchMoved_.emit(indices, event);
}

void WAbstractItemView::handleTouchEnd(const std::vector<WModelIndex>& indices,
				       const WTouchEvent& event)
{
  touchEnded_.emit(indices, event);
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
  if (index.flags().test(ItemFlag::Editable) && !isEditing(index)) {
    if (editOptions_.test(EditOption::SingleEditor)) {
      while (!editedItems_.empty())
	closeEditor(editedItems_.begin()->first, false);
    }

    Utils::insert(editedItems_, index, Editor());
    editedItems_[index].widget = nullptr;
    editedItems_[index].stateSaved = false;

    modelDataChanged(index, index);
  }
}

void WAbstractItemView::closeEditorWidget(WWidget *editor, bool saveData)
{
  for (EditorMap::iterator i = editedItems_.begin();
       i != editedItems_.end(); ++i)
    if (i->second.widget.get() == editor) {
      if (editOptions_.test(EditOption::LeaveEditorsOpen)) {
	// Save data, but keep editor open
	if (saveData)
	  saveEditedValue(i->first, i->second);
      } else
	closeEditor(i->first, saveData);

      return;
    }
}

void WAbstractItemView::closeEditor(const WModelIndex& index, bool saveData)
{
  EditorMap::iterator i = editedItems_.find(index);

  if (i != editedItems_.end()) {
    Editor editor = i->second;

    WModelIndex closed = index;
#ifndef WT_TARGET_JAVA
    editedItems_.erase(i);
#else
    editedItems_.erase(index);
#endif

    if (saveData || editOptions_.test(EditOption::SaveWhenClosed))
      saveEditedValue(closed, editor);

    modelDataChanged(closed, closed);
  }
}

void WAbstractItemView::closeEditors(bool saveData) 
{
  while (!editedItems_.empty()) {
    closeEditor(editedItems_.begin()->first, saveData);
  }
}

ValidationState WAbstractItemView::validateEditor(const WModelIndex& index)
{
  EditorMap::iterator i = editedItems_.find(index);

  if (i != editedItems_.end()) {
    auto delegate = itemDelegate(index);

    cpp17::any editState;

    Editor& editor = i->second;

    if (editor.widget)
      editState = delegate->editState(editor.widget.get(), index);
    else
      editState = editor.editState;

    ValidationState state = delegate->validate(index, editState);
    editor.valid = (state == ValidationState::Valid);
    
    return state;
  }
  
  return ValidationState::Invalid;
}

ValidationState WAbstractItemView::validateEditors() 
{
  ValidationState state = ValidationState::Valid;

  for (EditorMap::const_iterator i = editedItems_.begin();
       i != editedItems_.end(); ++i) {
    ValidationState s = validateEditor(i->first);
    if (s < state)
      state = s;
  }

  return state;
}

bool WAbstractItemView::isEditing(const WModelIndex& index) const
{
  return editedItems_.find(index) != editedItems_.end();
}

bool WAbstractItemView::shiftEditorRows(const WModelIndex& parent,
					int start, int count,
					bool persistWhenShifted)
{
  /* Returns whether an editor with a widget shifted */
  bool result = false;

  if (!editedItems_.empty()) {
    std::vector<WModelIndex> toClose;

    EditorMap newMap;

    for (EditorMap::iterator i = editedItems_.begin(); i != editedItems_.end();
	 ++i) {
      WModelIndex c = i->first;

      WModelIndex p = c.parent();

      if (p != parent && !WModelIndex::isAncestor(p, parent))
	newMap[c] = i->second;
      else {
	if (p == parent) {
	  if (c.row() >= start) {
	    if (c.row() < start - count) {
	      toClose.push_back(c);
	    } else {
	      WModelIndex shifted
		= model_->index(c.row() + count, c.column(), p);
	      newMap[shifted] = i->second;
	      if (i->second.widget) {
		if (persistWhenShifted)
	 	  persistEditor(shifted, i->second);
		result = true;
              }
	    }
	  } else
	    newMap[c] = i->second;
	} else if (count < 0) {
	  do {
	    if (p.parent() == parent
		&& p.row() >= start
		&& p.row() < start - count) {
	      toClose.push_back(c);
	      break;
	    } else
	      p = p.parent();
	  } while (p != parent);
	}
      }
    }

    for (unsigned i = 0; i < toClose.size(); ++i)
      closeEditor(toClose[i]);

    editedItems_ = newMap;
  }

  return result;
}

bool WAbstractItemView::shiftEditorColumns(const WModelIndex& parent,
					   int start, int count,
					   bool persistWhenShifted)
{
  /* Returns whether an editor with a widget shifted */
  bool result = false;

  if (!editedItems_.empty()) {
    std::vector<WModelIndex> toClose;

    EditorMap newMap;

    for (EditorMap::iterator i = editedItems_.begin(); i != editedItems_.end();
	 ++i) {
      WModelIndex c = i->first;

      WModelIndex p = c.parent();

      if (p != parent && !WModelIndex::isAncestor(p, parent))
	newMap[c] = i->second;
      else {
	if (p == parent) {
	  if (c.column() >= start) {
	    if (c.column() < start - count) {
	      toClose.push_back(c);
	    } else {
	      WModelIndex shifted
		= model_->index(c.row(), c.column() + count, p);
	      newMap[shifted] = i->second;
	      if (i->second.widget) {
		if (persistWhenShifted)
	 	  persistEditor(shifted, i->second);
		result = true;
              }
	    }
	  } else
	    newMap[c] = i->second;
	} else if (count < 0) {
	  do {
	    if (p.parent() == parent
		&& p.column() >= start
		&& p.column() < start - count) {
	      toClose.push_back(c);
	      break;
	    } else
	      p = p.parent();
	  } while (p != parent);
	}
      }
    }

    for (unsigned i = 0; i < toClose.size(); ++i)
      closeEditor(toClose[i]);

    editedItems_ = newMap;
  }

  return result;
}

bool WAbstractItemView::isValid(const WModelIndex& index) const
{  
  EditorMap::const_iterator i = editedItems_.find(index);

  if (i != editedItems_.end()) {
    const Editor& editor = i->second;
    return editor.valid;
  } else
    return false;
}

bool WAbstractItemView::hasEditFocus(const WModelIndex& index) const
{
  /*
   * Later, we may want to only return true for the 'smallest' index
   * satisfying all these conditions
   */
  EditorMap::const_iterator i = editedItems_.find(index);

  if (i != editedItems_.end()) {
    const Editor& editor = i->second;
    return !editor.widget && !editor.stateSaved;
  } else
    return false;
}

bool WAbstractItemView::isEditing() const
{
  return !editedItems_.empty();
}

void WAbstractItemView::saveEditedValue(const WModelIndex& index,
					Editor& editor)
{
  cpp17::any editState;
  auto delegate = itemDelegate(index);
    
  if (editor.widget)
    editState = delegate->editState(editor.widget.get(), index);
  else
    editState = editor.editState;

  delegate->setModelData(editState, model_.get(), index);
}

void WAbstractItemView::persistEditor(const WModelIndex& index)
{
  EditorMap::iterator i = editedItems_.find(index);

  if (i != editedItems_.end())
    persistEditor(index, i->second);
}

void WAbstractItemView::persistEditor(const WModelIndex& index, Editor& editor)
{
  if (editor.widget) {
    editor.editState 
      = itemDelegate(index)->editState(editor.widget.get(), index);
    editor.stateSaved = true;
    editor.widget = nullptr;
  }
}

void WAbstractItemView::setEditState(const WModelIndex& index,
				     const cpp17::any& editState)
{
  editedItems_[index].editState = editState;
}

void WAbstractItemView::setEditorWidget(const WModelIndex& index,
					WWidget *editor)
{
  editedItems_[index].widget = editor;
  editedItems_[index].stateSaved = !editor;
}

cpp17::any WAbstractItemView::editState(const WModelIndex& index) const
{
  EditorMap::const_iterator i = editedItems_.find(index);

  if (i != editedItems_.end())
    return i->second.editState;
  else
    return cpp17::any();
}

EventSignal<WKeyEvent>& WAbstractItemView::keyWentDown()
{
  impl_->setCanReceiveFocus(true);
  return impl_->keyWentDown();
}

EventSignal<WKeyEvent>& WAbstractItemView::keyPressed()
{
  impl_->setCanReceiveFocus(true);
  return impl_->keyPressed();
}

EventSignal<WKeyEvent>& WAbstractItemView::keyWentUp()
{
  impl_->setCanReceiveFocus(true);
  return impl_->keyWentUp();
}

}

