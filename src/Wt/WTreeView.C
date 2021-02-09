/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <math.h>
#include <cmath>
#include <iostream>
#include <boost/algorithm/string.hpp>

#include "Wt/WAbstractItemModel.h"
#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WItemDelegate.h"
#include "Wt/WItemSelectionModel.h"
#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"
#include "Wt/WTemplate.h"
#include "Wt/WText.h"
#include "Wt/WTheme.h"
#include "Wt/WTreeView.h"
#include "Wt/WVBoxLayout.h"
#include "Wt/WWebWidget.h"

#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WTreeView.min.js"
#endif

#include <limits>

#if defined(_MSC_VER) && (_MSC_VER < 1800)
namespace {
  double round(double x)
  {
    return floor(x + 0.5);
  }
}
#endif

/*
  TODO:

  nice to have:
   - stateless slot implementations
   - keyboard navigation ?
*/

#ifndef DOXYGEN_ONLY

// Widest scrollbar found ? My Gnome Firefox has this
#define SCROLLBAR_WIDTH         22
#define UNKNOWN_VIEWPORT_HEIGHT 30

namespace Wt {

LOGGER("WTreeView");

class ContentsContainer final : public WContainerWidget
{
public:
  ContentsContainer(WTreeView *treeView)
    : treeView_(treeView)
  { 
    setLayoutSizeAware(true);
  }

protected:
  virtual void layoutSizeChanged(int width, int height) override
  {
    treeView_->contentsSizeChanged(width, height);
  }

private:
  WTreeView *treeView_;
};

class ToggleButtonConfig
{
public:
  ToggleButtonConfig(WWidget *parent, const std::string& styleClass)
    : styleClass_(styleClass)
  {
    toggleJS_.reset(new JSlot(parent));
  }

  void addState(const std::string& className) {
    states_.push_back(className);
  }

  void generate() {
    WApplication *app = WApplication::instance();

    std::stringstream js;
    js << 
      "function(s, e) {"
      """var states = new Array(";

    for (unsigned i = 0; i < states_.size(); ++i) {
      if (i != 0)
	js << ',';
      js << '\'' << states_[i] << '\'';
    }

    js <<
      """), i, il;"
      """for (i=0; i<" << states_.size() << "; ++i) {"
      ""  "if ($(s).hasClass(states[i])) {"
      "" << app->javaScriptClass() << ".emit(s, 't-'+states[i]);"
      ""    "$(s).removeClass(states[i])"
      ""        ".addClass(states[(i+1) % " << states_.size() << "]);"
      ""    "break;"
      ""  "}"
      """}"
      "}";
    
    toggleJS_->setJavaScript(js.str());
  }

  const std::vector<std::string>& states() const { return states_; }
  const std::string& styleClass() const { return styleClass_; }

private:
  std::vector<std::string> states_;
  std::unique_ptr<JSlot> toggleJS_;
  std::string styleClass_;

  friend class ToggleButton;
};

class ToggleButton : public Wt::WText
{
public:
  ToggleButton(ToggleButtonConfig *config) 
    : config_(config)
  {
    setStyleClass(config_->styleClass());

    setInline(false);

    if (WApplication::instance()->environment().ajax()) {
      clicked().connect(*config_->toggleJS_);
      clicked().preventPropagation();

      for (unsigned i = 0; i < config_->states().size(); ++i)
	signals_.push_back(std::unique_ptr<JSignal<> >(new JSignal<>(this, "t-" +  config_->states()[i])));
    } else {
      clicked().connect(this, &ToggleButton::handleClick);
      for (unsigned i = 0; i < config_->states().size(); ++i)
	signals_.push_back(std::unique_ptr<Signal<> >(new Signal<>()));
    }
  }

  SignalBase& signal(int i) { return *signals_[i]; }

  void setState(int i)
  {
    setStyleClass(config_->styleClass() + config_->states()[i]);
  }

private:
  std::vector<std::unique_ptr<SignalBase> > signals_;
  ToggleButtonConfig       *config_;

  void handleClick() {
    for (unsigned i = 0; i < config_->states().size(); ++i)
      if (boost::ends_with(styleClass().toUTF8(), config_->states()[i])) {
	(dynamic_cast<Signal<> *>(signals_[i].get()))->emit();
	break;
      }
  }
};

class RowSpacer final : public Wt::WWebWidget
{
public:
  RowSpacer(Wt::WTreeViewNode *node, int height)
    : node_(node),
      height_(0)
  {
    setHeight(0);
    setInline(false);
    setStyleClass("Wt-spacer");
  }

  void setRows(int height, bool force = false);

  int rows() const { return height_; }
  Wt::WTreeViewNode *node() const { return node_; }

  int renderedRow(int lowerBound = 0,
		  int upperBound = std::numeric_limits<int>::max());

protected:
  virtual Wt::DomElementType domElementType() const override
  {
    return Wt::DomElementType::DIV;
  }

private:
  Wt::WTreeViewNode *node_;
  int height_;
};

class WTreeViewNode : public WContainerWidget
{
public:
  WTreeViewNode(WTreeView *view, const WModelIndex& index,
		int childrenHeight, bool isLast, WTreeViewNode *parent);
  ~WTreeViewNode();

  void update(int firstColumn, int lastColumn);
  void updateGraphics(bool isLast, bool isEmpty);
  void insertColumns(int column, int count);
  void removeColumns(int column, int count);
  bool isLast() const;

  void rerenderSpacers();

  const WModelIndex& modelIndex() const { return index_; }
  int childrenHeight() const { return childrenHeight_; }
  int renderedHeight();
  bool childrenLoaded() const { return childrenLoaded_; }

  WWidget *widgetForModelRow(int row);
  WTreeViewNode *nextChildNode(WTreeViewNode *n);

  bool isAllSpacer();

  void setTopSpacerHeight(int rows);
  void addTopSpacerHeight(int rows);
  int topSpacerHeight();

  void setBottomSpacerHeight(int rows);
  void addBottomSpacerHeight(int rows);
  int bottomSpacerHeight();

  RowSpacer *topSpacer(bool create = false);
  RowSpacer *bottomSpacer(bool create = false);

  WContainerWidget *childContainer();

  void shiftModelIndexes(int start, int count);

  WTreeViewNode *parentNode() const { return parentNode_; }

  bool isExpanded();

  void adjustChildrenHeight(int diff);
  void normalizeSpacers();

  void selfCheck();

  WTreeView *view() const { return view_; }

  int renderedRow(int lowerBound = 0,
		  int upperBound = std::numeric_limits<int>::max());
  int renderedRow(WTreeViewNode *node,
		  int lowerBound = 0,
		  int upperBound = std::numeric_limits<int>::max());

  void renderSelected(bool selected, int column);

  void doExpand();
  void doCollapse();

  WWidget *cellWidget(int column);

private:
  WTreeView *view_;
  WTemplate *nodeWidget_;
  WContainerWidget *childContainer_;
  WModelIndex index_;
  int childrenHeight_;
  WTreeViewNode *parentNode_;
  bool childrenLoaded_;
  void loadChildren();

  WModelIndex childIndex(int column);

  void setCellWidget(int column, std::unique_ptr<WWidget> w);
  void addColumnStyleClass(int column, WWidget *w);
};

void RowSpacer::setRows(int height, bool force)
{
  if (height < 0) {
    LOG_ERROR("RowSpacer::setRows() with heigth " << height);
    height = 0;
  }

  if (height == 0)
    removeFromParent();
  else
    if (force || height != height_) {
      height_ = height;
      setHeight(node_->view()->rowHeight() * height);
    }
}

int RowSpacer::renderedRow(int lowerBound, int upperBound)
{
  Wt::WTreeViewNode *n = node();

  int result = 0;
  if (this == n->bottomSpacer())
    result = n->childrenHeight() - n->bottomSpacerHeight();

  if (result > upperBound)
    return result;
  else
    return result
      + n->renderedRow(lowerBound - result, upperBound - result);
}

WTreeViewNode::WTreeViewNode(WTreeView *view, const WModelIndex& index,
			     int childrenHeight, bool isLast,
			     WTreeViewNode *parent)
  : view_(view),
    nodeWidget_(nullptr),
    childContainer_(nullptr),
    index_(index),
    childrenHeight_(childrenHeight),
    parentNode_(parent),
    childrenLoaded_(false)
{
  nodeWidget_ = addWidget(std::make_unique<WTemplate>(tr("Wt.WTreeViewNode.template")));
  nodeWidget_->setStyleClass("Wt-item");
  nodeWidget_->bindEmpty("cols-row");
  nodeWidget_->bindEmpty("expand");
  nodeWidget_->bindEmpty("no-expand");
  nodeWidget_->bindEmpty("col0");

  const int selfHeight = index_ == view_->rootIndex() ? 0 : 1;
  bool needLoad = view_->isExpanded(index_);

  if (index_ != view_->rootIndex() && !needLoad)
    childContainer()->hide();

  if (needLoad) {
    childrenLoaded_ = true;
    if (childrenHeight_ == -1)
      childrenHeight_ = view_->subTreeHeight(index_) - selfHeight;

    if (childrenHeight_ > 0)
      setTopSpacerHeight(childrenHeight_);
  } else
    childrenHeight_ = 0;

  if (index_ != view_->rootIndex()) {
    updateGraphics(isLast, !view_->model()->hasChildren(index_));
    insertColumns(0, view_->columnCount());

    if (view_->selectionBehavior() == SelectionBehavior::Rows &&
	view_->isSelected(index_))
      renderSelected(true, 0);
  }

  view_->addRenderedNode(this);
}

WTreeViewNode::~WTreeViewNode()
{
  view_->removeRenderedNode(this);

  if (view_->isEditing()) {
    WModelIndex parent = index_.parent();

    int thisNodeCount = view_->model()->columnCount(parent);

    for (int i = 0; i < thisNodeCount; ++i) {
      WModelIndex child = childIndex(i);
      view_->persistEditor(child);
    }
  }
}

void WTreeViewNode::update(int firstColumn, int lastColumn)
{
  WModelIndex parent = index_.parent();

  int thisNodeCount = view_->model()->columnCount(parent);

  for (int i = firstColumn; i <= lastColumn; ++i) {
    WModelIndex child = i < thisNodeCount ? childIndex(i) : WModelIndex();

    WWidget *w = cellWidget(i);

    WFlags<ViewItemRenderFlag> renderFlags = None;
    if (view_->selectionBehavior() == SelectionBehavior::Items &&
	view_->isSelected(child))
      renderFlags |= ViewItemRenderFlag::Selected;

    if (view_->isEditing(child)) {
      renderFlags |= ViewItemRenderFlag::Editing;
      if (view_->hasEditFocus(child))
	renderFlags |= ViewItemRenderFlag::Focused;
    }

    if (!view_->isValid(child)) {
      renderFlags |= ViewItemRenderFlag::Invalid;
    }
    
    std::unique_ptr<WWidget> wAfter = view_->itemDelegate(i)->update(w, child, renderFlags);
    if (wAfter)
      w = wAfter.get();

    if (renderFlags.test(ViewItemRenderFlag::Editing))
      view_->setEditorWidget(child, w);

    if (wAfter) {
      setCellWidget(i, std::move(wAfter));

      /*
       * If we are creating a new editor, then reset its current edit
       * state.
       */
      if (renderFlags.test(ViewItemRenderFlag::Editing)) {
	cpp17::any state = view_->editState(child);
	if (cpp17::any_has_value(state))
	  view_->itemDelegate(i)->setEditState(w, child, state);
      }
    } else
      addColumnStyleClass(i, w);
  }
}

void WTreeViewNode::updateGraphics(bool isLast, bool isEmpty)
{
  if (index_ == view_->rootIndex())
    return;

  if (index_.parent() == view_->rootIndex() && !view_->rootIsDecorated()) {
    nodeWidget_->bindEmpty("expand");
    nodeWidget_->bindEmpty("no-expand");
    return;
  }

  if (!isEmpty) {
    ToggleButton *expandButton = nodeWidget_->resolve<ToggleButton *>("expand");
    if (!expandButton) {
      nodeWidget_->bindEmpty("no-expand");
      expandButton = 
	nodeWidget_->bindWidget
	("expand", std::make_unique<ToggleButton>(view_->expandConfig_.get()));

      if (WApplication::instance()->environment().agentIsIE())
	expandButton->setWidth(19);

      expandButton->signal(0).connect(this, &WTreeViewNode::doExpand);
      expandButton->signal(1).connect(this, &WTreeViewNode::doCollapse);

      expandButton->setState(isExpanded() ? 1 : 0);
    }
  } else {
    WText *noExpandIcon = nodeWidget_->resolve<WText *>("no-expand");
    if (!noExpandIcon) {
      noExpandIcon
	= nodeWidget_->bindWidget("no-expand", std::make_unique<WText>());
      noExpandIcon->setInline(false);
      noExpandIcon->setStyleClass("Wt-ctrl rh noexpand");
      if (WApplication::instance()->environment().agentIsIE())
	noExpandIcon->setWidth(19);
    }
  }

  toggleStyleClass("Wt-trunk", !isLast);
  nodeWidget_->toggleStyleClass("Wt-end", isLast);
  nodeWidget_->toggleStyleClass("Wt-trunk", !isLast);
}

void WTreeViewNode::insertColumns(int column, int count)
{
  WContainerWidget *row = nodeWidget_->resolve<WContainerWidget *>("cols-row");

  if (view_->columnCount() > 1) {
    if (!row) {
      std::unique_ptr<WContainerWidget> newRow(new WContainerWidget());

      if (view_->rowHeaderCount()) {
	newRow->setStyleClass("Wt-tv-rowc rh");
	std::unique_ptr<WContainerWidget> rowWrap(new WContainerWidget());
	rowWrap->addWidget(std::move(newRow));
	newRow = std::move(rowWrap);
      }

      newRow->setStyleClass("Wt-tv-row rh");

      nodeWidget_->bindWidget("cols-row", std::move(newRow));
    }
  } else
    if (row)
      row->removeFromParent();

  update(0, view_->columnCount() - 1);
}

void WTreeViewNode::removeColumns(int column, int count)
{
  insertColumns(0, 0);
}

bool WTreeViewNode::isLast() const
{
  return index_ == view_->rootIndex()
    || (index_.row() == view_->model()->rowCount(index_.parent()) - 1);
}

WModelIndex WTreeViewNode::childIndex(int column)
{
  return view_->model()->index(index_.row(), column, index_.parent());
}

void WTreeViewNode::addColumnStyleClass(int column, WWidget *w)
{
  WStringStream s;

  s << view_->columnStyleClass(column) << " Wt-tv-c rh "
    << w->styleClass().toUTF8();

  w->setStyleClass(WString::fromUTF8(s.c_str()));
}

void WTreeViewNode::setCellWidget(int column, std::unique_ptr<WWidget> newW)
{
  WWidget *current = cellWidget(column);

  addColumnStyleClass(column, newW.get());

  if (current)
    current->setStyleClass(WString::Empty);

  if (!WApplication::instance()->environment().ajax()) {
    WInteractWidget *wi = dynamic_cast<WInteractWidget *>(newW.get());
    const WModelIndex ci = childIndex(column);
    if (wi) {
      wi->clicked().connect
	(view_, std::bind(&WTreeView::handleClick, view_, ci,
			  std::placeholders::_1));
    }
  }

  if (column == 0) {
    newW->setInline(false);
    nodeWidget_->bindWidget("col0", std::move(newW));
  } else {
    WContainerWidget *row 
      = nodeWidget_->resolve<WContainerWidget *>("cols-row");
    if (view_->rowHeaderCount())
      row = dynamic_cast<WContainerWidget *>(row->widget(0));

    if (current)
      current->removeFromParent();

    row->insertWidget(column - 1, std::move(newW));
  }
}

WWidget *WTreeViewNode::cellWidget(int column)
{
  if (column == 0)
    return nodeWidget_->resolveWidget("col0");
  else {
    WContainerWidget *row 
      = nodeWidget_->resolve<WContainerWidget *>("cols-row");

    if (view_->rowHeaderCount())
      row = dynamic_cast<WContainerWidget *>(row->widget(0));

    return row->count() >= column ? row->widget(column - 1) : nullptr;
  }
}

void WTreeViewNode::doExpand()
{
  if (isExpanded())
    return;

  loadChildren();

  ToggleButton *expandButton = nodeWidget_->resolve<ToggleButton *>("expand");
  if (expandButton)
    expandButton->setState(1);

  view_->expandedSet_.insert(index_);

  childContainer()->show();

  if (parentNode())
    parentNode()->adjustChildrenHeight(childrenHeight_);

  view_->adjustRenderedNode(this, renderedRow());
  view_->scheduleRerender(WTreeView::RenderState::NeedAdjustViewPort);

  view_->expanded_.emit(index_);
}

void WTreeViewNode::doCollapse()
{
  if (!isExpanded())
    return;

  ToggleButton *expandButton = nodeWidget_->resolve<ToggleButton *>("expand");
  if (expandButton)
    expandButton->setState(0);

  view_->setCollapsed(index_);

  childContainer()->hide();

  if (parentNode())
    parentNode()->adjustChildrenHeight(-childrenHeight_);

  view_->renderedRowsChanged(renderedRow(), -childrenHeight_);

  view_->collapsed_.emit(index_);
}

bool WTreeViewNode::isExpanded()
{
  return index_ == view_->rootIndex() || !childContainer()->isHidden();
}

void WTreeViewNode::normalizeSpacers()
{
  if (childrenLoaded_ && childContainer()->count() == 2) {
    RowSpacer *top = topSpacer();
    RowSpacer *bottom = bottomSpacer();

    if (top && bottom && top != bottom) {
      top->setRows(top->rows() + bottom->rows());
      if (bottom)
	bottom->removeFromParent();
    }
  }
}

void WTreeViewNode::rerenderSpacers()
{
  RowSpacer *s = topSpacer();
  if (s)
    s->setRows(topSpacerHeight(), true);

  s = bottomSpacer();
  if (s)
    s->setRows(bottomSpacerHeight(), true);
}

bool WTreeViewNode::isAllSpacer()
{
  return childrenLoaded_ && topSpacer() && (topSpacer() == bottomSpacer());
}

void WTreeViewNode::loadChildren()
{
  if (!childrenLoaded_) {
    childrenLoaded_ = true;

    view_->expandedSet_.insert(index_);
    childrenHeight_ = view_->subTreeHeight(index_) - 1;
    view_->expandedSet_.erase(index_);

    if (childrenHeight_ > 0)
      setTopSpacerHeight(childrenHeight_);
  }
}

void WTreeViewNode::adjustChildrenHeight(int diff)
{
  childrenHeight_ += diff;

  if (isExpanded()) {
    WTreeViewNode *parent = parentNode();

    if (parent)
      parent->adjustChildrenHeight(diff);
    else
      view_->pageChanged().emit();
  }
}

WContainerWidget *WTreeViewNode::childContainer()
{
  if (!childContainer_) {
    childContainer_ = addWidget(std::make_unique<WContainerWidget>());
    childContainer_->setList(true);

    if (index_ == view_->rootIndex())
      childContainer_->addStyleClass("Wt-tv-root");
  }

  return childContainer_;
}

WWidget *WTreeViewNode::widgetForModelRow(int modelRow)
{
  if (!childrenLoaded_)
    return nullptr;

  WContainerWidget *c = childContainer();

  int first = topSpacer() ? 1 : 0;

  if (first < c->count()) {
    WTreeViewNode *n = dynamic_cast<WTreeViewNode *>(c->widget(first));
    if (n) {
      int row = topSpacerHeight();
      int index = first + (modelRow - row);

      if (index < first)
	return topSpacer();
      else if (index < c->count())
	return c->widget(index);
      else
	return bottomSpacer();
    } else
      return bottomSpacer();
  } else // isAllSpacer()
    return topSpacer();
}

void WTreeViewNode::shiftModelIndexes(int start, int offset)
{
  if (!childrenLoaded_)
    return;

  WContainerWidget *c = childContainer();

  int first, end, inc;

  if (offset > 0) {
    first = c->count() - 1;
    end = -1;
    inc = -1;
  } else {
    first = 0;
    end = c->count();
    inc = 1;
  }

  for (int i = first; i != end; i += inc) {
    WTreeViewNode *n = dynamic_cast<WTreeViewNode *>(c->widget(i));

    if (n && n->modelIndex().row() >= start) {
      view_->removeRenderedNode(n);

      n->index_ = view_->model()->index(n->modelIndex().row() + offset,
					n->modelIndex().column(), index_);

      // update items through delegate
      int lastColumn = view_->columnCount() - 1;
      int thisNodeCount = view_->model()->columnCount(index_);

      for (int j = 0; j <= lastColumn; ++j) {
	WModelIndex child = j < thisNodeCount
	  ? n->childIndex(j) : WModelIndex();
	view_->itemDelegate(j)->updateModelIndex(n->cellWidget(j), child);
      }

      view_->addRenderedNode(n);
    }
  }
}

int WTreeViewNode::renderedHeight()
{
  return index_ == view_->rootIndex() ? childrenHeight_ :
    (1 + (isExpanded() ? childrenHeight_ : 0));
}

int WTreeViewNode::topSpacerHeight()
{
  RowSpacer *s = topSpacer();

  return s ? s->rows() : 0;
}

void WTreeViewNode::setTopSpacerHeight(int rows)
{
  if (rows == 0){
    childContainer()->removeWidget(topSpacer());
  }
  else
    topSpacer(true)->setRows(rows);
}

void WTreeViewNode::addTopSpacerHeight(int rows)
{
  setTopSpacerHeight(topSpacerHeight() + rows);
}

RowSpacer *WTreeViewNode::topSpacer(bool create)
{
  WContainerWidget *c = childContainer();

  RowSpacer *result = nullptr;
  if (c->count() == 0 || !(result = dynamic_cast<RowSpacer *>(c->widget(0)))) {
    if (!create)
      return nullptr;
    else {
      result = new RowSpacer(this, 0);
      c->insertWidget(0, std::unique_ptr<WWidget>(result));
    }
  }

  return result;
}

int WTreeViewNode::bottomSpacerHeight()
{
  RowSpacer *s = bottomSpacer();

  return s ? s->rows() : 0;
}

void WTreeViewNode::setBottomSpacerHeight(int rows)
{
  if (!rows) {
    auto bottom = bottomSpacer();
    if (bottom)
      bottom->removeFromParent();
  } else
    bottomSpacer(true)->setRows(rows);
}

void WTreeViewNode::addBottomSpacerHeight(int rows)
{
  setBottomSpacerHeight(bottomSpacerHeight() + rows);
}

RowSpacer *WTreeViewNode::bottomSpacer(bool create)
{
  WContainerWidget *c = childContainer();

  RowSpacer *result = nullptr;
  if (c->count() == 0
      || !(result = dynamic_cast<RowSpacer *>(c->widget(c->count() - 1)))) {
    if (!create)
      return nullptr;
    else {
      result = c->addWidget(std::make_unique<RowSpacer>(this, 0));
    }
  }

  return result;
}

WTreeViewNode *WTreeViewNode::nextChildNode(WTreeViewNode *prev)
{
  if (!childrenLoaded_)
    return nullptr;

  WContainerWidget *c = childContainer();

  int nextI = prev ? c->indexOf(prev) + 1 : topSpacer() ? 1 : 0;

  if (nextI < c->count())
    return dynamic_cast<WTreeViewNode *>(c->widget(nextI));
  else
    return nullptr;
}

int WTreeViewNode::renderedRow(int lowerBound, int upperBound)
{
  if (!parentNode_)
    return 0;
  else {
    int result = parentNode_->renderedRow(0, upperBound);

    if (result > upperBound)
      return result;

    return result
      + parentNode_->renderedRow(this, lowerBound - result,
				 upperBound - result);
  }
}

int WTreeViewNode::renderedRow(WTreeViewNode *node,
			       int lowerBound, int upperBound)
{
  if (renderedHeight() < lowerBound)
    return renderedHeight();

  int result = topSpacerHeight();

  if (result > upperBound)
    return result;

  for (WTreeViewNode *c = nextChildNode(nullptr); c; c = nextChildNode(c)) {
    if (c == node)
      return result;
    else {
      result += c->renderedHeight();
      if (result > upperBound)
	return result;
    }
  }

  assert(false);
  return 0;
}

void WTreeViewNode::renderSelected(bool selected, int column)
{
  std::string cl = WApplication::instance()->theme()->activeClass();

  if (view_->selectionBehavior() == SelectionBehavior::Rows) {
    nodeWidget_->toggleStyleClass(cl, selected);
  } else {
    WWidget *w = cellWidget(column);
    w->toggleStyleClass(cl, selected);
  }
}

void WTreeViewNode::selfCheck()
{
  assert(renderedHeight() == view_->subTreeHeight(index_));

  int childNodesHeight = 0;
  for (WTreeViewNode *c = nextChildNode(nullptr); c; c = nextChildNode(c)) {
    c->selfCheck();
    childNodesHeight += c->renderedHeight();
  }

  if (childNodesHeight == 0) {
    assert(topSpacer() == bottomSpacer());
    assert(topSpacerHeight() == childrenHeight());
  } else {
    assert(topSpacerHeight() + childNodesHeight + bottomSpacerHeight()
	   == childrenHeight());
  }
}

WTreeView::WTreeView()
  : skipNextMouseEvent_(false),
    renderedNodesAdded_(false),
    rootNode_(nullptr),
    rowHeightRule_(nullptr),
    rowWidthRule_(nullptr),
    rowContentsWidthRule_(nullptr),
    c0StyleRule_(nullptr),
    rootIsDecorated_(true),
    viewportTop_(0),
    viewportHeight_(UNKNOWN_VIEWPORT_HEIGHT),
    firstRenderedRow_(0),
    validRowCount_(0),
    nodeLoad_(0),
    headerContainer_(nullptr),
    contentsContainer_(nullptr),
    scrollBarC_(nullptr),
    firstRemovedRow_(0),
    removedHeight_(0),
    itemEvent_(impl_, "itemEvent"),
    itemTouchEvent_(impl_, "itemTouchEvent")
{
  setSelectable(false);

  expandConfig_.reset(new ToggleButtonConfig(this, "Wt-ctrl rh "));
  expandConfig_->addState("expand");
  expandConfig_->addState("collapse");
  expandConfig_->generate();

  setStyleClass("Wt-itemview Wt-treeview");

  const char *CSS_RULES_NAME = "Wt::WTreeView";

  WApplication *app = WApplication::instance();

  if (app->environment().agentIsWebKit() || app->environment().agentIsOpera())
    if (!app->styleSheet().isDefined(CSS_RULES_NAME))
      /* bottom scrollbar */
      addCssRule
	(".Wt-treeview .Wt-tv-rowc", "position: relative;", CSS_RULES_NAME);

  setup();
}

void WTreeView::setup()
{
  WApplication *app = WApplication::instance();

  impl_->clear();

  rootNode_ = nullptr;

  /*
   * Setup main layout
   */
  headers_ = new WContainerWidget();
  headers_->setStyleClass("Wt-headerdiv headerrh");

  contents_ = new WContainerWidget();
  WContainerWidget *wrapRoot 
    = contents_->addWidget(std::make_unique<WContainerWidget>());

  if (app->environment().agentIsIE()) {
    wrapRoot->setAttributeValue("style", "zoom: 1");
    contents_->setAttributeValue("style", "zoom: 1");
  }

  if (app->environment().ajax()) {
    impl_->setPositionScheme(PositionScheme::Relative);

    std::unique_ptr<WVBoxLayout> layout(new WVBoxLayout());
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    headerContainer_ = new WContainerWidget();
    headerContainer_->setOverflow(Overflow::Hidden);
    headerContainer_->setStyleClass("Wt-header headerrh cwidth");
    headerContainer_->addWidget(std::unique_ptr<WWidget>(headers_));

    contentsContainer_ = new ContentsContainer(this);
    contentsContainer_->setStyleClass("cwidth");
    contentsContainer_->setOverflow(Overflow::Auto);
    contentsContainer_->scrolled().connect(this, &WTreeView::onViewportChange);
    contentsContainer_->addWidget(std::unique_ptr<WWidget>(contents_));

    layout->addWidget(std::unique_ptr<WWidget>(headerContainer_));
    layout->addWidget(std::unique_ptr<WWidget>(contentsContainer_), 1);

    impl_->setLayout(std::move(layout));
  } else {
    contentsContainer_ = new WContainerWidget();
    contentsContainer_->addWidget(std::unique_ptr<WWidget>(contents_));
    contentsContainer_->setOverflow(Overflow::Hidden);

    impl_->setPositionScheme(PositionScheme::Relative);
    contentsContainer_->setPositionScheme(PositionScheme::Relative);
    contents_->setPositionScheme(PositionScheme::Relative);

    impl_->addWidget(std::unique_ptr<WWidget>(headers_));
    impl_->addWidget(std::unique_ptr<WWidget>(contentsContainer_));

    viewportHeight_ = 1000;

    resize(width(), height());
  }

  setRowHeight(rowHeight());

  bindObjJS(itemClickedJS_, "click");
  bindObjJS(rootClickedJS_, "rootClick");
  bindObjJS(itemDoubleClickedJS_, "dblClick");
  bindObjJS(rootDoubleClickedJS_, "rootDblClick");
  bindObjJS(itemMouseDownJS_, "mouseDown");
  bindObjJS(rootMouseDownJS_, "rootMouseDown");
  bindObjJS(itemMouseUpJS_, "mouseUp");
  bindObjJS(rootMouseUpJS_, "rootMouseUp");
  bindObjJS(touchStartedJS_, "touchStart");
  bindObjJS(touchMovedJS_, "touchMove");
  bindObjJS(touchEndedJS_, "touchEnd");
}

void WTreeView::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  if (!app->environment().ajax())
    return;

  LOAD_JAVASCRIPT(app, "js/WTreeView.js", "WTreeView", wtjs1);

  setJavaScriptMember(" WTreeView", "new " WT_CLASS ".WTreeView("
		      + app->javaScriptClass() + "," + jsRef() + ","
		      + contentsContainer_->jsRef() + ","
		      + headerContainer_->jsRef() + ","
		      + std::to_string(rowHeaderCount())+ ",'"
		      + WApplication::instance()->theme()->activeClass()
		      + "');");

  setJavaScriptMember(WT_RESIZE_JS,
                      "function(self,w,h,s) {"
                      """self.wtObj.wtResize();"
                      "}");
}

void WTreeView::setRowHeaderCount(int count)
{
  WApplication *app = WApplication::instance();

  // This kills progressive enhancement too
  if (!app->environment().ajax())
    return;

  int oldCount = rowHeaderCount();

  if (count != 0 && count != 1)
    throw WException("WTreeView::setRowHeaderCount: count must be 0 or 1");

  WAbstractItemView::setRowHeaderCount(count);

  if (count && !oldCount) {
    addStyleClass("column1");
    WContainerWidget *rootWrap
      = dynamic_cast<WContainerWidget *>(contents_->widget(0));
    rootWrap->setWidth(WLength(100, LengthUnit::Percentage));
    rootWrap->setOverflow(Overflow::Hidden);
    contents_->setPositionScheme(PositionScheme::Relative);
    rootWrap->setPositionScheme(PositionScheme::Absolute);

    bool useStyleLeft
      = app->environment().agentIsWebKit()
      || app->environment().agentIsOpera();

    if (useStyleLeft) {
      bool rtl = app->layoutDirection() == LayoutDirection::RightToLeft;

      tieRowsScrollJS_.setJavaScript
	("function(obj, event) {"
	 "" WT_CLASS ".getCssRule('#" + id() + " .Wt-tv-rowc').style.left"
	 ""  "= -obj.scrollLeft "
	 + (rtl ? "+ (obj.firstChild.offsetWidth - obj.offsetWidth)" : "")
	 + "+ 'px';"
	 "}");
    } else {
      /*
       * this is for some reason very very slow in webkit, and with
       * application/xml on Firefox (jQuery suffers)
       */
      tieRowsScrollJS_.setJavaScript
	("function(obj, event) {"
	 "$('#" + id() + " .Wt-tv-rowc').parent().scrollLeft(obj.scrollLeft);"
	 "}");
    }

    WContainerWidget *scrollBarContainer = new WContainerWidget();
    scrollBarContainer->setStyleClass("cwidth");
    scrollBarContainer->setHeight(SCROLLBAR_WIDTH);
    scrollBarC_ 
      = scrollBarContainer->addWidget(std::make_unique<WContainerWidget>());
    scrollBarC_->setStyleClass("Wt-tv-row Wt-scroll");
    scrollBarC_->scrolled().connect(tieRowsScrollJS_);

    if (app->environment().agentIsIE()) {
      scrollBarContainer->setPositionScheme(PositionScheme::Relative);
      bool rtl = app->layoutDirection() == LayoutDirection::RightToLeft;
      scrollBarC_->setAttributeValue("style",
				     std::string(rtl ? "left:" : "right:")
				     + "0px");
      // and still it doesn't work properly...
    }

    WContainerWidget *scrollBar 
      = scrollBarC_->addWidget(std::make_unique<WContainerWidget>());
    scrollBar->setStyleClass("Wt-tv-rowc");
    if (useStyleLeft)
      scrollBar->setAttributeValue("style", "left: 0px;");
    impl_->layout()->addWidget(std::unique_ptr<WWidget>(scrollBarContainer));
  }
}

WTreeView::~WTreeView()
{
  wApp->styleSheet().removeRule(rowHeightRule_);
  wApp->styleSheet().removeRule(rowWidthRule_);
  wApp->styleSheet().removeRule(rowContentsWidthRule_);
  wApp->styleSheet().removeRule(c0StyleRule_);

  impl_->clear();
}

std::string WTreeView::columnStyleClass(int column) const
{
  return columnInfo(column).styleClass();
}

void WTreeView::setColumnWidth(int column, const WLength& width)
{
  if (!width.isAuto())
    columnInfo(column).width = WLength(round(width.value()), width.unit());
  else
    columnInfo(column).width = WLength::Auto;

  WWidget *toResize = columnInfo(column).styleRule->templateWidget();
  toResize->setWidth(0);
  toResize->setWidth(columnInfo(column).width.toPixels());

  WApplication *app = WApplication::instance();

  if (app->environment().ajax() && 
      static_cast<unsigned int>(renderState_) < 
      static_cast<unsigned int>(RenderState::NeedRerenderHeader))
    doJavaScript(jsRef() + ".wtObj.adjustColumns();");

  if (!app->environment().ajax() && column == 0 && !width.isAuto()) {
    double total = 0;
    for (int i = 0; i < columnCount(); ++i)
      if (!columnInfo(i).hidden)
	total += columnWidth(i).toPixels();

    resize(total, height());
  }
}

void WTreeView::setColumnHidden(int column, bool hidden)
{
  if (columnInfo(column).hidden != hidden) {
    WAbstractItemView::setColumnHidden(column, hidden);

    WWidget *toHide = columnInfo(column).styleRule->templateWidget();
    toHide->setHidden(hidden);

    setColumnWidth(column, columnWidth(column));
  }
}

void WTreeView::setHeaderHeight(const WLength& height)
{
  WAbstractItemView::setHeaderHeight(height);
}

void WTreeView::setRootIsDecorated(bool show)
{
  rootIsDecorated_ = show;
}

void WTreeView::setAlternatingRowColors(bool enable)
{
  WAbstractItemView::setAlternatingRowColors(enable);
  setRootNodeStyle();
}

void WTreeView::setRootNodeStyle()
{
  if (!rootNode_)
    return;

  if (alternatingRowColors())
    rootNode_->decorationStyle().setBackgroundImage
      (WLink(WApplication::instance()->theme()->resourcesUrl()
	     + "stripes/stripe-" + std::to_string
	     (static_cast<int>(rowHeight().toPixels())) + "px.gif"));
   else
     rootNode_->decorationStyle().setBackgroundImage(WLink(""));
 }

void WTreeView::setRowHeight(const WLength& rowHeight)
{
  WAbstractItemView::setRowHeight(rowHeight);

  if (rowHeightRule_) {
    rowHeightRule_->templateWidget()->setHeight(rowHeight);
    rowHeightRule_->templateWidget()->setLineHeight(rowHeight);
  }

  if (!WApplication::instance()->environment().ajax() && !height().isAuto())
    viewportHeight_ = static_cast<int>(contentsContainer_->height().toPixels()
				       / rowHeight.toPixels());

  setRootNodeStyle();

  for (NodeMap::const_iterator i = renderedNodes_.begin();
       i != renderedNodes_.end(); ++i)
    i->second->rerenderSpacers();

  if (rootNode_)
    scheduleRerender(RenderState::NeedAdjustViewPort);
}

void WTreeView::resize(const WLength& width, const WLength& height)
{
  WApplication *app = WApplication::instance();
  WLength w = app->environment().ajax() ? WLength::Auto : width;

  if (app->environment().ajax())
    contentsContainer_->setWidth(w);
  
  if (headerContainer_)
    headerContainer_->setWidth(w);

  if (!height.isAuto()) {
    if (!app->environment().ajax()) {
      if (impl_->count() < 3)
	impl_->addWidget(createPageNavigationBar());

      double navigationBarHeight = 35;
      double headerHeight = this->headerHeight().toPixels();

      int h = (int)(height.toPixels() - navigationBarHeight - headerHeight);
      contentsContainer_->setHeight(std::max(h, (int)rowHeight().value()));

      viewportHeight_
	= static_cast<int>(contentsContainer_->height().toPixels()
			   / rowHeight().toPixels());
    } else
      viewportHeight_ = static_cast<int>
	(std::ceil(height.toPixels() / rowHeight().toPixels()));
  } else {
    if (app->environment().ajax())
      viewportHeight_ = UNKNOWN_VIEWPORT_HEIGHT;

    scheduleRerender(RenderState::NeedAdjustViewPort);
  }

  WCompositeWidget::resize(width, height);
}

int WTreeView::calcOptimalFirstRenderedRow() const
{
  if (WApplication::instance()->environment().ajax())
    return std::max(0, viewportTop_ - viewportHeight_ - viewportHeight_ / 2);
  else
    return viewportTop_;
}

int WTreeView::calcOptimalRenderedRowCount() const
{
  if (WApplication::instance()->environment().ajax())
    return 4 * viewportHeight_;
  else
    return viewportHeight_ + 5; // some margin... something inaccurate going on?
}

void WTreeView::setModel(const std::shared_ptr<WAbstractItemModel>& model)
{
  WAbstractItemView::setModel(model);

  typedef WTreeView Self;

  /* connect slots to new model */
  modelConnections_.push_back(model->columnsInserted().connect
			      (this, &Self::modelColumnsInserted));
  modelConnections_.push_back(model->columnsAboutToBeRemoved().connect
			      (this, &Self::modelColumnsAboutToBeRemoved));
  modelConnections_.push_back(model->columnsRemoved().connect
			      (this, &Self::modelColumnsRemoved));
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

  expandedSet_.clear();

  WApplication *app = WApplication::instance();
  while (static_cast<int>(columns_.size()) > model->columnCount()) {
    app->styleSheet().removeRule(columns_.back().styleRule.get());
    columns_.erase(columns_.begin() + columns_.size() - 1);
  }

  pageChanged().emit();
}

void WTreeView::scheduleRerender(RenderState what)
{
  if (what == RenderState::NeedRerender || what == RenderState::NeedRerenderData) {
    if(rootNode_){
      rootNode_->removeFromParent();
      rootNode_ = nullptr;
    }
  }

  WAbstractItemView::scheduleRerender(what);
}

void WTreeView::render(WFlags<RenderFlag> flags)
{
  WApplication *app = WApplication::instance();

  if (flags.test(RenderFlag::Full)) {
    defineJavaScript();

    if (!itemTouchEvent_.isConnected())
      itemTouchEvent_.connect(this, &WTreeView::onItemTouchEvent);

    if (!itemEvent_.isConnected()) {
      itemEvent_.connect(this, &WTreeView::onItemEvent);

      addCssRule("#" + id() + " .cwidth", "");

      rowHeightRule_ = app->styleSheet().addRule
	(std::unique_ptr<WCssTemplateRule>(new WCssTemplateRule("#" + id() + " .rh")));
      rowHeightRule_->templateWidget()->setHeight(rowHeight());
      rowHeightRule_->templateWidget()->setLineHeight(rowHeight());

      rowWidthRule_ = app->styleSheet().addRule
	(std::unique_ptr<WCssTemplateRule>(new WCssTemplateRule("#" + id() + " .Wt-tv-row")));
      rowContentsWidthRule_ = app->styleSheet().addRule
	(std::unique_ptr<WCssTemplateRule>(new WCssTemplateRule("#" + id() + " .Wt-tv-rowc")));

      if (app->environment().ajax()) {
	contentsContainer_->scrolled().connect
	  ("function(obj, event) {"
	   /*
	    * obj.sb: workaround for Konqueror to prevent recursive
	    * invocation because reading scrollLeft triggers onscroll()
	    */
	   """if (obj.sb) return;"
	   """obj.sb = true;"
	   "" + headerContainer_->jsRef() + ".scrollLeft=obj.scrollLeft;"
	   /* the following is a workaround for IE7 */
	   """var t = " + contents_->jsRef() + ".firstChild;"
	   """var h = " + headers_->jsRef() + ";"
	   """h.style.width = (t.offsetWidth - 1) + 'px';"
	   """h.style.width = t.offsetWidth + 'px';"
	   """obj.sb = false;"
	   "}");	
      }

      c0StyleRule_ = addCssRule("#" + id() + " li .none",
				"width: auto;"
				"text-overflow: ellipsis;"
				"overflow: hidden");
   
      if (columns_.size() > 0) {
	ColumnInfo& ci = columnInfo(0);
	c0StyleRule_->setSelector("#" + id() + " li ." + ci.styleClass());
      }
    }
  }

  while (renderState_ != RenderState::RenderOk) {
    RenderState s = renderState_;
    renderState_ = RenderState::RenderOk;

    switch (s) {
    case RenderState::NeedRerender:
      rerenderHeader();
      rerenderTree();
      break;
    case RenderState::NeedRerenderHeader:
      rerenderHeader();
      break;
    case RenderState::NeedRerenderData:
      rerenderTree();
      break;
    case RenderState::NeedAdjustViewPort:
      adjustToViewport();
      break;
    default:
      break;
    }
  }

  // set contents height to retain scroll-position (issue #7998)
  contents_->setHeight(subTreeHeight(rootIndex()) * rowHeight().toPixels());

  if (app->environment().ajax() && rowHeaderCount() && renderedNodesAdded_) {
    doJavaScript("{var s=" + scrollBarC_->jsRef() + ";"
		 """if (s) {" + tieRowsScrollJS_.execJs("s") + "}"
		 "}");
    renderedNodesAdded_ = false;
  }

  // update the rowHeight (needed for scrolling fix)
  WStringStream s;
  s << jsRef() << ".wtObj.setRowHeight("
    <<  static_cast<int>(this->rowHeight().toPixels())
    << ");";

  if (app->environment().ajax()) 
    doJavaScript(s.str());

  WAbstractItemView::render(flags);
}

void WTreeView::rerenderHeader()
{
  WApplication *app = WApplication::instance();

  saveExtraHeaderWidgets();
  headers_->clear();

  WContainerWidget *row
    = headers_->addWidget(std::make_unique<WContainerWidget>());
  row->setFloatSide(Side::Right);

  if (rowHeaderCount()) {
    row->setStyleClass("Wt-tv-row headerrh background");
    auto r = row->addWidget(std::make_unique<WContainerWidget>());
    row = r;
    row->setStyleClass("Wt-tv-rowc headerrh");
  } else
    row->setStyleClass("Wt-tv-row");

  for (int i = 0; i < columnCount(); ++i) {
    std::unique_ptr<WWidget> w = createHeaderWidget(i);

    if (i != 0) {
      w->setFloatSide(Side::Left);
      row->addWidget(std::move(w));
    } else
      headers_->addWidget(std::move(w));
  }

  if (app->environment().ajax())
    doJavaScript(jsRef() + ".wtObj.adjustColumns();");
}

void WTreeView::enableAjax()
{
  saveExtraHeaderWidgets();

  setup();
  defineJavaScript();

  scheduleRerender(RenderState::NeedRerender);

  WAbstractItemView::enableAjax();
}

void WTreeView::rerenderTree()
{
  WContainerWidget *wrapRoot
    = dynamic_cast<WContainerWidget *>(contents_->widget(0));

  wrapRoot->clear();

  firstRenderedRow_ = calcOptimalFirstRenderedRow();
  validRowCount_ = 0;

  rootNode_ = wrapRoot->addWidget
    (std::make_unique<WTreeViewNode>(this, rootIndex(), -1, true, nullptr));

  if (WApplication::instance()->environment().ajax()) {

    if (editTriggers().test(EditTrigger::SingleClicked) || 
	clicked().isConnected()) {
      rootNode_->clicked().connect(itemClickedJS_);
      contentsContainer_->clicked().connect(rootClickedJS_);
    }

    if (editTriggers().test(EditTrigger::DoubleClicked) || 
	doubleClicked().isConnected()) {
      rootNode_->doubleClicked().connect(itemDoubleClickedJS_);
      contentsContainer_->doubleClicked().connect(rootDoubleClickedJS_);
    }

    rootNode_->mouseWentDown().connect(itemMouseDownJS_);
    contentsContainer_->mouseWentDown().connect(rootMouseDownJS_);

    if (mouseWentUp().isConnected()) { 
	  // Do not stop propagation to avoid mouseDrag event being emitted 
      rootNode_->mouseWentUp().connect(itemMouseUpJS_);
      contentsContainer_->mouseWentUp().connect(rootMouseUpJS_);
    }

#ifdef WT_CNOR
    // workaround because cnor is a bit dumb and does not understand that it
    // can convert EventSignal<TouchEvent>& to EventSignalBase&
    EventSignalBase& a = rootNode_->touchStarted();
#endif
   
    rootNode_->touchStarted().connect(touchStartedJS_);
    rootNode_->touchMoved().connect(touchMovedJS_);
    rootNode_->touchEnded().connect(touchEndedJS_);
  }

  setRootNodeStyle();

  pageChanged().emit();

  adjustToViewport();
}

void WTreeView::onViewportChange(WScrollEvent e)
{
  viewportTop_ = static_cast<int>
    (std::floor(e.scrollY() / rowHeight().toPixels()));

  contentsSizeChanged(0, e.viewportHeight());
}

void WTreeView::contentsSizeChanged(int width, int height)
{
  viewportHeight_
    = static_cast<int>(std::ceil(height / rowHeight().toPixels()));

  scheduleRerender(RenderState::NeedAdjustViewPort);
}

void WTreeView::onItemEvent(std::string nodeAndColumnId, std::string type,
			    std::string extra1, std::string extra2,
			    WMouseEvent event)
{
  // nodeId and columnId are combined because WSignal needed to be changed
  // since MSVS 2012 does not support >5 arguments in std::bind()
  WModelIndex index = calculateModelIndex(nodeAndColumnId);

  /*
   * Every mouse event is emitted twice (because we don't prevent
   * the propagation because it will block the mouseWentUp event
   * and therefore result in mouseDragged being emitted (See #3879)
   */
  if (nodeAndColumnId.empty() && skipNextMouseEvent_) {
    skipNextMouseEvent_ = false; 
    return;
  } else if (!nodeAndColumnId.empty()) {
    skipNextMouseEvent_ = true;
  }

  if (type == "clicked") {
    handleClick(index, event);
  } else if (type == "dblclicked") {
    handleDoubleClick(index, event);
  } else if (type == "mousedown") {
    handleMouseDown(index, event);
  } else if (type == "mouseup") {
    handleMouseUp(index, event);
  } else if (type == "drop") {
    WDropEvent e(WApplication::instance()->decodeObject(extra1), extra2, event);
    dropEvent(e, index);
  }
}

void WTreeView::onItemTouchEvent(std::string nodeAndColumnId, std::string type, WTouchEvent event)
{
  // nodeId and columnId are combined because WSignal needed to be changed
  // since MSVS 2012 does not support >5 arguments in std::bind()
  std::vector<WModelIndex> index;
  index.push_back(calculateModelIndex(nodeAndColumnId));

  if (type == "touchselect")
    handleTouchSelect(index, event);
  else if (type == "touchstart")
    handleTouchStart(index, event);
  else if (type == "touchend")
    handleTouchEnd(index, event);
}

WModelIndex WTreeView::calculateModelIndex(std::string nodeAndColumnId)
{
  std::vector<std::string> nodeAndColumnSplit;
  boost::split(nodeAndColumnSplit, nodeAndColumnId, boost::is_any_of(":"));

  WModelIndex index;

  if (nodeAndColumnSplit.size() == 2) {
    std::string nodeId = nodeAndColumnSplit[0];
    int columnId = -1;
    try {
      columnId = Utils::stoi(std::string(nodeAndColumnSplit[1]));
    } catch (std::exception& e) {
      LOG_ERROR("WTreeview::calculateModelIndex: bad value for format 1: "
		<< nodeAndColumnSplit[1]);
    }

    int column = (columnId == 0 ? 0 : -1);
    for (unsigned i = 0; i < columns_.size(); ++i)
      if (columns_[i].id == columnId) {
	column = i;
	break;
      }

    if (column != -1) {
      WModelIndex c0index;
      for (NodeMap::const_iterator i = renderedNodes_.begin();
	    i != renderedNodes_.end(); ++i) {
	if (i->second->id() == nodeId) {
          c0index = i->second->modelIndex();
 	  break;
	}
      }

      if (c0index.isValid())
	index = model()->index(c0index.row(), column, c0index.parent());
      else
        LOG_ERROR("WTreeView::calculateModelIndex: illegal node id: " << nodeId);
    }
  }
  return index;
}

int WTreeView::subTreeHeight(const WModelIndex& index,
			     int lowerBound, int upperBound) const
{
  int result = 0;

  if (index != rootIndex())
    ++result;

  if (result >= upperBound)
    return result;

  if (model() && isExpanded(index)) {
    int childCount = model()->rowCount(index);

    for (int i = 0; i < childCount; ++i) {
      WModelIndex childIndex = model()->index(i, 0, index);

      result += subTreeHeight(childIndex, upperBound - result);

      if (result >= upperBound)
	return result;
    }
  }

  return result;
}

bool WTreeView::isExpanded(const WModelIndex& index) const
{
  return index == rootIndex()
    || expandedSet_.find(index) != expandedSet_.end();
}

bool WTreeView::isExpandedRecursive(const WModelIndex& index) const
{
  if (isExpanded(index)) {
    if (index != rootIndex())
      return isExpanded(index.parent());
    else
      return false;
  } else
    return false;
}

void WTreeView::setCollapsed(const WModelIndex& index)
{
  expandedSet_.erase(index);

  /*
   * Deselecting everything that is collapsed is not consistent with
   * the allowed initial state. If the user wants this, he can implement
   * this himself.
   */
#if 0
  bool selectionHasChanged = false;
  WModelIndexSet& selection = selectionModel()->selection_;

  WModelIndexSet toDeselect;
  for (WModelIndexSet::iterator it = selection.lower_bound(index);
       it != selection.end(); ++it) {    
    WModelIndex i = *it;
    if (i == index) {
    } else if (WModelIndex::isAncestor(i, index)) {
      toDeselect.insert(i);
    } else
      break;
  }

  for (WModelIndexSet::iterator it = toDeselect.begin();
       it != toDeselect.end(); ++it)
    if (internalSelect(*it, SelectionFlag::Deselect))
      selectionHasChanged = true;

  if (selectionHasChanged)
    selectionChanged().emit();
#endif
}

void WTreeView::setExpanded(const WModelIndex& index, bool expanded)
{
  if (isExpanded(index) != expanded) {
    WWidget *w = widgetForIndex(index);

    WTreeViewNode *node = w ? dynamic_cast<WTreeViewNode *>(w) : nullptr;

    if (node) {
      if (expanded)
	node->doExpand();
      else
	node->doCollapse();
    } else {
      int height = subTreeHeight(index);

      if (expanded)
	expandedSet_.insert(index);
      else
	setCollapsed(index);

      if (w) {
	RowSpacer *spacer = dynamic_cast<RowSpacer *>(w);

	int diff = subTreeHeight(index) - height;

	spacer->setRows(spacer->rows() + diff);
	spacer->node()->adjustChildrenHeight(diff);

	renderedRowsChanged(renderedRow(index, spacer,
					renderLowerBound(), renderUpperBound()),
			    diff);
      }
    }
  }
}

void WTreeView::expand(const WModelIndex& index)
{
  setExpanded(index, true);
}

void WTreeView::collapse(const WModelIndex& index)
{
  setExpanded(index, false);
}

void WTreeView::collapseAll()
{
  while (!expandedSet_.empty())
    collapse(Utils::first(expandedSet_));
}

void WTreeView::expandToDepth(int depth)
{
  if (depth > 0)
    expandChildrenToDepth(rootIndex(), depth);
}

void WTreeView::expandChildrenToDepth(const WModelIndex& index, int depth)
{
  for (int i = 0; i < model()->rowCount(index); ++i) {
    WModelIndex c = model()->index(i, 0, index);

    expand(c);

    if (depth > 1)
      expandChildrenToDepth(c, depth - 1);
  }
}

WWidget *WTreeView::itemWidget(const WModelIndex& index) const
{
  if (!index.isValid())
    return nullptr;

  WTreeViewNode *n = nodeForIndex(index);
  if (n)
    return n->cellWidget(index.column());
  else
    return nullptr;
}

/*
 * Returns the widget that renders the node indicated by index.
 *
 * It may be:
 *  - a tree node (node->modelIndex() == index)
 *  - 0 if index is not somewhere in column 0
 *  - a spacer which includes the 'index', when all intermediate
 *    nodes are expanded, or
 *  - 0 otherwise
 */
WWidget *WTreeView::widgetForIndex(const WModelIndex& index) const
{
  if (!index.isValid())
    return rootNode_;

  if (index.column() != 0)
    return nullptr;

  NodeMap::const_iterator i = renderedNodes_.find(index);

  if (i != renderedNodes_.end())
    return i->second;
  else {
    if (!isExpanded(index.parent()))
      return nullptr;

    WWidget *parent = widgetForIndex(index.parent());
    WTreeViewNode *parentNode = dynamic_cast<WTreeViewNode *>(parent);

    if (parentNode) {
      int row = getIndexRow(index, parentNode->modelIndex(), 0,
			    std::numeric_limits<int>::max());
      return parentNode->widgetForModelRow(row);
    } else
      return parent;
  }
}

void WTreeView::modelColumnsInserted(const WModelIndex& parent,
				     int start, int end)
{
  int count = end - start + 1;
  if (!parent.isValid()) {

    WApplication *app = WApplication::instance();
    for (int i = start; i < start + count; ++i)
      columns_.insert(columns_.begin() + i, createColumnInfo(i));

    if (static_cast<unsigned int>(renderState_) < 
	static_cast<unsigned int>(RenderState::NeedRerenderHeader)) {
      if (start == 0)
	scheduleRerender(RenderState::NeedRerenderHeader);
      else {
	if (app->environment().ajax())
	  doJavaScript(jsRef() + ".wtObj.adjustColumns();");

	WContainerWidget *row = headerRow();

	for (int i = start; i < start + count; ++i) {
	  std::unique_ptr<WWidget> w = createHeaderWidget(i);
	  w->setFloatSide(Side::Left);
	  row->insertWidget(i - 1, std::move(w));
	}
      }
    }
  }

  if (renderState_ == RenderState::NeedRerender ||
      renderState_ == RenderState::NeedRerenderData)
    return;

  if (start == 0)
    scheduleRerender(RenderState::NeedRerenderData);
  else {
    WTreeViewNode *node = nodeForIndex(parent);
    if (node)
      for (WTreeViewNode *c = node->nextChildNode(nullptr); c;
	   c = node->nextChildNode(c))
	c->insertColumns(start, count);
  }
}

void WTreeView::modelColumnsAboutToBeRemoved(const WModelIndex& parent,
					     int start, int end)
{
  int count = end - start + 1;
  if (!parent.isValid()) {
    WApplication *app = wApp;

    if (static_cast<unsigned int>(renderState_) < 
	static_cast<unsigned int>(RenderState::NeedRerenderHeader)) {
      if (app->environment().ajax())
	doJavaScript(jsRef() + ".wtObj.adjustColumns();");
    }

    for (int i=start; i<start+count; i++)
      app->styleSheet().removeRule(columns_[i].styleRule.get());
    columns_.erase(columns_.begin() + start, columns_.begin() + start + count);

    if (static_cast<unsigned int>(renderState_) < 
	static_cast<unsigned int>(RenderState::NeedRerenderHeader)) {
      if (start == 0)
	scheduleRerender(RenderState::NeedRerenderHeader);
      else {
	for (int i = start; i < start + count; ++i) {
	  auto w = headerWidget(start, false);
	  if (w)
	    w->removeFromParent();
	}
      }
    }
  }

  if (start == 0)
    scheduleRerender(RenderState::NeedRerenderData);
}

void WTreeView::modelColumnsRemoved(const WModelIndex& parent,
				    int start, int end)
{
  if (renderState_ == RenderState::NeedRerender ||
      renderState_ == RenderState::NeedRerenderData)
    return;

  int count = end - start + 1;

  if (start != 0) {
    WTreeViewNode *node = nodeForIndex(parent);
    if (node)
      for (WTreeViewNode *c = node->nextChildNode(nullptr); c;
	   c = node->nextChildNode(c))
	c->removeColumns(start, count);
  }

  if (start <= currentSortColumn_ && currentSortColumn_ <= end)
    currentSortColumn_ = -1;
}

void WTreeView::modelRowsInserted(const WModelIndex& parent,
				  int start, int end)
{
  int count = end - start + 1;
  shiftModelIndexes(parent, start, count);

  if (renderState_ == RenderState::NeedRerender || 
      renderState_ == RenderState::NeedRerenderData)
    return;

  WWidget *parentWidget = widgetForIndex(parent);

  bool renderedRowsChange = isExpandedRecursive(parent);

  if (parentWidget) {
    WTreeViewNode *parentNode = dynamic_cast<WTreeViewNode *>(parentWidget);

    if (parentNode) {
      if (parentNode->childrenLoaded()) {
	WWidget *startWidget = nullptr;

	/*
	 * First we decide between inserting in the top spacer, bottom spacer
	 * and in actually rendered nodes.
	 */
	if (end < model()->rowCount(parent) - 1)
	  startWidget = parentNode->widgetForModelRow(start);
	else if (parentNode->bottomSpacerHeight() != 0)
	  startWidget = parentNode->bottomSpacer();

	parentNode->adjustChildrenHeight(count);
	parentNode->shiftModelIndexes(start, count);

	if (startWidget && startWidget == parentNode->topSpacer()) {
	  parentNode->addTopSpacerHeight(count);
	  if (renderedRowsChange)
	    renderedRowsChanged(renderedRow(model()->index(start, 0, parent),
					    parentNode->topSpacer(),
					    renderLowerBound(),
					    renderUpperBound()),
				count);

	} else if (startWidget && startWidget == parentNode->bottomSpacer()) {
	  parentNode->addBottomSpacerHeight(count);
	  if (renderedRowsChange)
	    renderedRowsChanged(renderedRow(model()->index(start, 0, parent),
					    parentNode->bottomSpacer(),
					    renderLowerBound(),
					    renderUpperBound()),
				count);
	} else {
	  int maxRenderHeight
	    = firstRenderedRow_ + std::max(validRowCount_, viewportHeight_)
	    - parentNode->renderedRow() - parentNode->topSpacerHeight();

	  int containerIndex = startWidget ? parentNode->childContainer()
	    ->indexOf(startWidget) : parentNode->childContainer()->count();

	  int parentRowCount = model()->rowCount(parent);

	  int nodesToAdd = std::max(0, std::min(count, maxRenderHeight));

	  WTreeViewNode *first = nullptr;
	  for (int i = 0; i < nodesToAdd; ++i) {
	    std::unique_ptr<WTreeViewNode> n
	      (new WTreeViewNode(this, model()->index(start + i, 0, parent),
				 -1, start + i == parentRowCount - 1,
				 parentNode));
	    if (!first)
	      first = n.get();

	    parentNode->childContainer()->insertWidget(containerIndex + i,
						       std::move(n));

	    if (renderedRowsChange)
	      ++validRowCount_;

	  }

	  if (nodesToAdd < count) {
	    parentNode->addBottomSpacerHeight(count - nodesToAdd);

	    // +1 for bottom spacer
	    int targetSize = containerIndex + nodesToAdd + 1;

	    int extraBottomSpacer = 0;
	    while (parentNode->childContainer()->count() > targetSize) {
	      WTreeViewNode *n
		= dynamic_cast<WTreeViewNode *>(parentNode->childContainer()
						->widget(targetSize - 1));
	      assert(n);
	      extraBottomSpacer += n->renderedHeight();

	      if (renderedRowsChange)
		validRowCount_ -= n->renderedHeight();

	      n->removeFromParent();
	    }

	    if (extraBottomSpacer)
	      parentNode->addBottomSpacerHeight(extraBottomSpacer);

	    parentNode->normalizeSpacers();
	  }

	  if (first && renderedRowsChange)
	    renderedRowsChanged(first->renderedRow(renderLowerBound(),
						   renderUpperBound()),
				nodesToAdd);

	  // Update graphics if the last node has changed, i.e. if we are
	  // adding rows at the back
	  if (end == model()->rowCount(parent) - 1 && start >= 1) {
	    WTreeViewNode *n = dynamic_cast<WTreeViewNode *>
	      (parentNode->widgetForModelRow(start - 1));

	    if (n)
	      n->updateGraphics(false, !model()->hasChildren(n->modelIndex()));
	  }
	}
      } /* else:
	   children not loaded -- so we do not need to bother
	 */

      // Update graphics for parent when first children have een added
      if (model()->rowCount(parent) == count)
	parentNode->updateGraphics(parentNode->isLast(), false);
    } else {
      if (isExpanded(parent)) {
	RowSpacer *s = dynamic_cast<RowSpacer *>(parentWidget);

	s->setRows(s->rows() + count);
	s->node()->adjustChildrenHeight(count);

	if (renderedRowsChange)
	  renderedRowsChanged
	    (renderedRow(model()->index(start, 0, parent), s,
			 renderLowerBound(), renderUpperBound()),
	     count);
      }
    }
  } else {
    /* 
     * parentWidget is 0: it means it is not even part of any spacer.
     */
  }
}

void WTreeView::modelRowsAboutToBeRemoved(const WModelIndex& parent,
					  int start, int end)
{
  int count = end - start + 1;

  bool renderedRowsChange = isExpandedRecursive(parent);

  if (renderState_ != RenderState::NeedRerender && 
      renderState_ != RenderState::NeedRerenderData) {
    firstRemovedRow_ = -1;
    removedHeight_ = 0;

    WWidget *parentWidget = widgetForIndex(parent);

    if (parentWidget) {
      WTreeViewNode *parentNode = dynamic_cast<WTreeViewNode *>(parentWidget);

      if (parentNode) {
	if (parentNode->childrenLoaded()) {
	  for (int i = end; i >= start; --i) {
	    WWidget *w = parentNode->widgetForModelRow(i);
	    assert(w);

	    RowSpacer *s = dynamic_cast<RowSpacer *>(w);
	    if (s) {
	      WModelIndex childIndex = model()->index(i, 0, parent);

	      if (i == start && renderedRowsChange)
		firstRemovedRow_ = renderedRow(childIndex, w);

	      int childHeight = subTreeHeight(childIndex);

	      if (renderedRowsChange)
		removedHeight_ += childHeight;

	      s->setRows(s->rows() - childHeight);
	    } else {
	      WTreeViewNode *node = dynamic_cast<WTreeViewNode *>(w);

	      if (renderedRowsChange) {
		if (i == start)
		  firstRemovedRow_ = node->renderedRow();

		removedHeight_ += node->renderedHeight();
	      }

	      node->removeFromParent();
	    }
	  }
	} /* else:
	     children not loaded -- so we do not need to bother
	  */
      } else {
	/*
	 * Only if the parent is in fact expanded, the spacer reduces
	 * in size
	 */
	if (isExpanded(parent)) {
	  RowSpacer *s = dynamic_cast<RowSpacer *>(parentWidget);

	  for (int i = start; i <= end; ++i) {
	    WModelIndex childIndex = model()->index(i, 0, parent);
	    int childHeight = subTreeHeight(childIndex);

	    if (renderedRowsChange) {
	      removedHeight_ += childHeight;

	      if (i == start)
		firstRemovedRow_ = renderedRow(childIndex, s);
	    }
	  }
        }
      }
    } else {
      /*
	parentWidget is 0: it means it is not even part of any spacer.
      */
    }
  }

  shiftModelIndexes(parent, start, -count);
}

void WTreeView::modelRowsRemoved(const WModelIndex& parent,
				 int start, int end)
{
  int count = end - start + 1;

  if (renderState_ != RenderState::NeedRerender &&
      renderState_ != RenderState::NeedRerenderData) {
    WWidget *parentWidget = widgetForIndex(parent);

    if (parentWidget) {
      WTreeViewNode *parentNode = dynamic_cast<WTreeViewNode *>(parentWidget);

      if (parentNode) {
	if (parentNode->childrenLoaded()) {
	  parentNode->normalizeSpacers();
	  parentNode->adjustChildrenHeight(-removedHeight_);
	  parentNode->shiftModelIndexes(start, -count);

	  // Update graphics for last node in parent, if we are removing rows
	  // at the back. This is not affected by widgetForModelRow() returning
	  // accurate information of rows just deleted and indexes not yet
	  // shifted
	  if (end >= model()->rowCount(parent) && start >= 1) {
	    WTreeViewNode *n = dynamic_cast<WTreeViewNode *>
	      (parentNode->widgetForModelRow(start - 1));

	    if (n)
	      n->updateGraphics(true, !model()->hasChildren(n->modelIndex()));
	  }
	} /* else:
	     children not loaded -- so we do not need to bother
	  */

	// Update graphics for parent when all rows have been removed
        if (model()->rowCount(parent) == 0 && count != 0)
	  parentNode->updateGraphics(parentNode->isLast(), true);
      } else {
	/*
	 * Only if the parent is in fact expanded, the spacer reduces
	 * in size
	 */
	if (isExpanded(parent)) {
	  RowSpacer *s = dynamic_cast<RowSpacer *>(parentWidget);
	  WTreeViewNode *node = s->node();
	  s->setRows(s->rows() - removedHeight_); // could delete s!
	  node->adjustChildrenHeight(-removedHeight_);
        }
      }
    } else {
      /*
	parentWidget is 0: it means it is not even part of any spacer.
      */
    }
  }

  if (renderState_ != RenderState::NeedRerender && 
      renderState_ != RenderState::NeedRerenderData)
    renderedRowsChanged(firstRemovedRow_, -removedHeight_);
}

void WTreeView::modelDataChanged(const WModelIndex& topLeft,
				 const WModelIndex& bottomRight)
{
  if (renderState_ == RenderState::NeedRerender || 
      renderState_ == RenderState::NeedRerenderData)
    return;
  
  WModelIndex parent = topLeft.parent();  
  WTreeViewNode *parentNode = nodeForIndex(parent);

  if (parentNode && parentNode->childrenLoaded()) {
    for (int r = topLeft.row(); r <= bottomRight.row(); ++r) {
      WModelIndex index = model()->index(r, 0, parent);

      WTreeViewNode *n = nodeForIndex(index);

      if (n)
	n->update(topLeft.column(), bottomRight.column());
    }
  }
}

WWidget *WTreeView::headerWidget(int column, bool contentsOnly)
{
  WWidget *result = nullptr;

  if (headers_ && headers_->count() > 0) {
    if (column == 0)
      result = headers_->widget(headers_->count() - 1);
    else
      result = headerRow()->widget(column - 1);
  }

  if (result && contentsOnly)
    return result->find("contents");
  else
    return result;
}

WContainerWidget *WTreeView::headerRow()
{
  WContainerWidget *row
    = dynamic_cast<WContainerWidget *>(headers_->widget(0));
  if (rowHeaderCount())
    row = dynamic_cast<WContainerWidget *>(row->widget(0));
  return row;
}

int WTreeView::renderedRow(const WModelIndex& index, WWidget *w,
			   int lowerBound, int upperBound)
{
  WTreeViewNode *node = dynamic_cast<WTreeViewNode *>(w);

  if (node)
    return node->renderedRow(lowerBound, upperBound);
  else {
    RowSpacer *s = dynamic_cast<RowSpacer *>(w);

    int result = s->renderedRow(0, upperBound);

    if (result > upperBound)
      return result;
    else if (result + s->node()->renderedHeight() < lowerBound)
      return result;
    else
      return result + getIndexRow(index, s->node()->modelIndex(),
				  lowerBound - result, upperBound - result);
  }
}

int WTreeView::getIndexRow(const WModelIndex& child,
			   const WModelIndex& ancestor,
			   int lowerBound, int upperBound) const
{
  if (!child.isValid() || child == ancestor)
    return 0;
  else {
    WModelIndex parent = child.parent();

    int result = 0;
    for (int r = 0; r < child.row(); ++r) {
      result += subTreeHeight(model()->index(r, 0, parent), 0,
			      upperBound - result);
      if (result >= upperBound)
	return result;
    }

    if (parent != ancestor)
      return result + 1 + getIndexRow(parent, ancestor,
                                      lowerBound - result, upperBound - result);
    else
      return result;
  }
}

int WTreeView::renderLowerBound() const
{
  return firstRenderedRow_;
}

int WTreeView::renderUpperBound() const
{
  return firstRenderedRow_ + validRowCount_;
}

void WTreeView::renderedRowsChanged(int row, int count)
{
  if (count < 0
      && row - count >= firstRenderedRow_
      && row < firstRenderedRow_ + validRowCount_)
    validRowCount_ += std::max(firstRenderedRow_ - row + count, count);

  if (row < firstRenderedRow_)
    firstRenderedRow_ += count;

  scheduleRerender(RenderState::NeedAdjustViewPort);
}

void WTreeView::adjustToViewport(WTreeViewNode *changed)
{
  //assert(rootNode_->rowCount() == 1);

  firstRenderedRow_ = std::max(0, firstRenderedRow_);
  validRowCount_
    = std::max(0, std::min(validRowCount_,
			   rootNode_->renderedHeight() - firstRenderedRow_));

  int viewportBottom = std::min(rootNode_->renderedHeight(),
				viewportTop_ + viewportHeight_);
  int lastValidRow = firstRenderedRow_ + validRowCount_;

  bool renderMore =
    (std::max(0,
	      viewportTop_ - viewportHeight_) < firstRenderedRow_)
    || (std::min(rootNode_->renderedHeight(),
		 viewportBottom + viewportHeight_) > lastValidRow);

  bool pruneFirst = false;

  //assert(rootNode_->rowCount() == 1);

  if (renderMore) {
    int newFirstRenderedRow = std::min(firstRenderedRow_,
				       calcOptimalFirstRenderedRow());
    int newLastValidRow = std::max(lastValidRow,
				   std::min(rootNode_->renderedHeight(),
					    calcOptimalFirstRenderedRow()
					    + calcOptimalRenderedRowCount()));
    //assert(rootNode_->rowCount() == 1);

    int newValidRowCount = newLastValidRow - newFirstRenderedRow;

    int newRows = std::max(0, firstRenderedRow_ - newFirstRenderedRow)
      + std::max(0, newLastValidRow - lastValidRow);

    const int pruneFactor
      = WApplication::instance()->environment().ajax() ? 9 : 1;

    if (nodeLoad_ + newRows > pruneFactor * viewportHeight_) {
      pruneFirst = true;
    } else
      if (newFirstRenderedRow < firstRenderedRow_
	  || newLastValidRow > lastValidRow) {
	firstRenderedRow_ = newFirstRenderedRow;
	validRowCount_ = newValidRowCount;
	adjustRenderedNode(rootNode_, 0);
      }
  }

  const int pruneFactor
    = WApplication::instance()->environment().ajax() ? 5 : 1;

  if (pruneFirst || nodeLoad_ > pruneFactor * viewportHeight_) {
    firstRenderedRow_ = calcOptimalFirstRenderedRow();
    validRowCount_ = calcOptimalRenderedRowCount();

    pruneNodes(rootNode_, 0);

    if (pruneFirst && nodeLoad_ < calcOptimalRenderedRowCount()) {
      adjustRenderedNode(rootNode_, 0);
    } 
  }

  //assert(rootNode_->rowCount() == 1);
}

int WTreeView::adjustRenderedNode(WTreeViewNode *node, int theNodeRow)
{
  //assert(rootNode_->rowCount() == 1);

  WModelIndex index = node->modelIndex();

  if (index != rootIndex())
    ++theNodeRow;

  if (!isExpanded(index) && !node->childrenLoaded())
    return theNodeRow;

  int nodeRow = theNodeRow;

  if (node->isAllSpacer()) {
    if (nodeRow + node->childrenHeight() > firstRenderedRow_
	&& nodeRow < firstRenderedRow_ + validRowCount_) {
      // replace spacer by some nodes
      int childCount = model()->rowCount(index);

      bool firstNode = true;
      int rowStubs = 0;

      for (int i = 0; i < childCount; ++i) {
	WModelIndex childIndex = model()->index(i, 0, index);

	int childHeight = subTreeHeight(childIndex);

	if (nodeRow <= firstRenderedRow_ + validRowCount_
	    && nodeRow + childHeight > firstRenderedRow_) {
	  if (firstNode) {
	    firstNode = false;
	    node->setTopSpacerHeight(rowStubs);
	    rowStubs = 0;
	  }

	  // assert(rootNode_->rowCount() == 1);

	  WTreeViewNode *n = node->childContainer()->addWidget
	    (std::make_unique<WTreeViewNode>(this, childIndex, childHeight - 1, i == childCount - 1, node));

	  int nestedNodeRow = nodeRow;
	  nestedNodeRow = adjustRenderedNode(n, nestedNodeRow);
	  assert(nestedNodeRow == nodeRow + childHeight);
	} else {
	  rowStubs += childHeight;
	}
	nodeRow += childHeight;
      }
      node->setBottomSpacerHeight(rowStubs);
    } else
      nodeRow += node->childrenHeight();
  } else {
    // get a reference to the first existing child, which we'll recursively
    // adjust later
    int topSpacerHeight = node->topSpacerHeight();
    int nestedNodeRow = nodeRow + topSpacerHeight;
    WTreeViewNode *child = node->nextChildNode(nullptr);

    int childCount = model()->rowCount(index);
    while (topSpacerHeight != 0
	   && nodeRow + topSpacerHeight > firstRenderedRow_) {
      // eat from top spacer and replace with actual nodes
      WTreeViewNode *n
	= dynamic_cast<WTreeViewNode *>(node->childContainer()->widget(1));

      assert(n);

      WModelIndex childIndex
	= model()->index(n->modelIndex().row() - 1, 0, index);

      assert(childIndex.isValid());

      int childHeight = subTreeHeight(childIndex);

      {
	std::unique_ptr<WTreeViewNode> nn
	  (n = new WTreeViewNode(this, childIndex, childHeight - 1,
			     childIndex.row() == childCount - 1, node));
	node->childContainer()->insertWidget(1, std::move(nn));
      }

      nestedNodeRow = nodeRow + topSpacerHeight - childHeight;
      nestedNodeRow = adjustRenderedNode(n, nestedNodeRow);
      assert(nestedNodeRow == nodeRow + topSpacerHeight);

      topSpacerHeight -= childHeight;
      node->addTopSpacerHeight(-childHeight);
    }

    for (; child; child=node->nextChildNode(child))
      nestedNodeRow = adjustRenderedNode(child, nestedNodeRow);

    int nch = node->childrenHeight();
    int bottomSpacerStart = nch - node->bottomSpacerHeight();

    while (node->bottomSpacerHeight() != 0
	   && nodeRow + bottomSpacerStart
	      <= firstRenderedRow_ + validRowCount_){
      // eat from bottom spacer and replace with actual nodes
      int lastNodeIndex = node->childContainer()->count() - 2;
      WTreeViewNode *n = dynamic_cast<WTreeViewNode *>
	(node->childContainer()->widget(lastNodeIndex));

      assert(n);

      WModelIndex childIndex
	= model()->index(n->modelIndex().row() + 1, 0, index);

      assert (childIndex.isValid());

      int childHeight = subTreeHeight(childIndex);

      {
	std::unique_ptr<WTreeViewNode> nn
	  (n = new WTreeViewNode(this, childIndex, childHeight - 1,
				 childIndex.row() == childCount - 1, node));
	node->childContainer()->insertWidget(lastNodeIndex + 1, std::move(nn));
      }

      nestedNodeRow = nodeRow + bottomSpacerStart;
      nestedNodeRow = adjustRenderedNode(n, nestedNodeRow);
      assert(nestedNodeRow == nodeRow + bottomSpacerStart + childHeight);

      node->addBottomSpacerHeight(-childHeight);
      bottomSpacerStart += childHeight;
    }

    nodeRow += nch;
  }

  // assert(rootNode_->rowCount() == 1);

  // if a node has children loaded but is not currently expanded, then still
  // adjust it, but do not return the calculated nodeRow for it.
  return isExpanded(index) ? nodeRow : theNodeRow;
}

int WTreeView::pruneNodes(WTreeViewNode *node, int nodeRow)
{
  // remove unneeded nodes: nodes within collapsed tree nodes, and nodes
  // beyond the optimal bounds
  WModelIndex index = node->modelIndex();

  ++nodeRow;

  if (isExpanded(index)) {
    // prune away nodes until we are within the rendered region
    nodeRow += node->topSpacerHeight();

    bool done = false;
    WTreeViewNode *c = nullptr;

    for (; nodeRow < firstRenderedRow_; ) {
      c = node->nextChildNode(nullptr);
      if (!c) {
	done = true;
	break;
      }

      if (nodeRow + c->renderedHeight() < firstRenderedRow_) {
	node->addTopSpacerHeight(c->renderedHeight());
	nodeRow += c->renderedHeight();
	c->removeFromParent();
      } else {
	nodeRow = pruneNodes(c, nodeRow);
	break;
      }
    }

    if (!done) {
      for (; nodeRow <= firstRenderedRow_ + validRowCount_; ) {
	c = node->nextChildNode(c);

	if (!c) {
	  done = true;
	  break;
	}

	nodeRow = pruneNodes(c, nodeRow);
      }
    }

    if (!done) {
      c = node->nextChildNode(c);

      if (c != nullptr) {
	int i = node->childContainer()->indexOf(c);
	int prunedHeight = 0;

	while (c && i < node->childContainer()->count()) {
	  c = dynamic_cast<WTreeViewNode *> (node->childContainer()->widget(i));
	  if (c) {
	    prunedHeight += c->renderedHeight();
	    c->removeFromParent();
	  }
	}

	node->addBottomSpacerHeight(prunedHeight);
      }
    }

    nodeRow += node->bottomSpacerHeight();

    node->normalizeSpacers();

  } else
    if (node->childrenLoaded()) {
      int prunedHeight = 0;
      for (;;) {
	WTreeViewNode *c = node->nextChildNode(nullptr);
	if (!c)
	  break;

	prunedHeight += c->renderedHeight();
	c->removeFromParent();
	c = nullptr;
      }

      node->addBottomSpacerHeight(prunedHeight);
      node->normalizeSpacers();
    }

  return nodeRow;
}

int WTreeView::
shiftModelIndexes(const WModelIndex& parent, int start, int count,
		  const std::shared_ptr<WAbstractItemModel>& model,
		  WModelIndexSet& set)
{
  /*
   * handle the set of exanded model indexes:
   *  - collect indexes in the same parent at lower rows that need to
   *    be shifted
   *  - if deleting, delete indexes that are within the range of deleted
   *    rows
   */
  std::vector<WModelIndex> toShift;
  std::vector<WModelIndex> toErase;

  for (WModelIndexSet::iterator it
	 = set.lower_bound(model->index(start, 0, parent)); it != set.end();) {
#ifndef WT_TARGET_JAVA
    WModelIndexSet::iterator n = it;
    ++n;
#endif

    WModelIndex i = *it;

    WModelIndex p = i.parent();
    if (p != parent && !WModelIndex::isAncestor(p, parent))
      break;

    if (p == parent) {
      toShift.push_back(i);
      toErase.push_back(i);
    } else if (count < 0) {
      // delete indexes that are about to be deleted, if they are within
      // the range of deleted indexes
      do {
	if (p.parent() == parent
	    && p.row() >= start
	    && p.row() < start - count) {
	  toErase.push_back(i);
	  break;
	} else
	  p = p.parent();
      } while (p != parent);
    }

#ifndef WT_TARGET_JAVA
    it = n;
#endif
  }

  for (unsigned i = 0; i < toErase.size(); ++i)
    set.erase(toErase[i]);

  int removed = 0;
  for (unsigned i = 0; i < toShift.size(); ++i) {
    // for negative count: only reinsert model indexes that need
    // not be removed (they are currently all removed)
    if (toShift[i].row() + count >= start) {
      WModelIndex newIndex = model->index(toShift[i].row() + count,
					  toShift[i].column(), parent);
      set.insert(newIndex);
    } else
      ++removed;
  }

  return removed;
}

namespace {

// Add all indexes under index i that are in set to toErase
void removalsFromSet(std::vector<WModelIndex> &toErase,
                     std::unordered_set<WModelIndex> &set,
                     const Wt::WModelIndex &i) {
  {
    if (set.find(i) != set.end())
      toErase.push_back(i);
  }

  const int rowCount = i.model()->rowCount(i);
  for (int row = 0; row < rowCount; ++row) {
    WModelIndex c = i.model()->index(row, 0, i);
    removalsFromSet(toErase, set, c);
  }
}

}

int WTreeView::
shiftModelIndexes(const WModelIndex& parent, int start, int count,
                  const std::shared_ptr<WAbstractItemModel>& model,
                  std::unordered_set<WModelIndex>& set)
{
  if (set.empty())
    return 0;

  /*
   * handle the set of exanded model indexes:
   *  - collect indexes in the same parent at lower rows that need to
   *    be shifted
   *  - if deleting, delete indexes that are within the range of deleted
   *    rows
   */
  std::vector<WModelIndex> toShift;
  std::vector<WModelIndex> toErase;

  const int rowCount = model->rowCount(parent);
  for (int row = start; row < rowCount; ++row) {
    WModelIndex i = model->index(row, 0, parent);
    if (row < start - count) {
      removalsFromSet(toErase, set, i);
    } else if (set.find(i) != set.end()) {
      toShift.push_back(i);
      toErase.push_back(i);
    }
  }

  for (unsigned i = 0; i < toErase.size(); ++i)
    set.erase(toErase[i]);

  int removed = 0;
  for (unsigned i = 0; i < toShift.size(); ++i) {
    // for negative count: only reinsert model indexes that need
    // not be removed (they are currently all removed)
    if (toShift[i].row() + count >= start) {
      WModelIndex newIndex = model->index(toShift[i].row() + count,
                                          toShift[i].column(), parent);
      set.insert(newIndex);
    } else
      ++removed;
  }

  return removed;
}

void WTreeView::shiftModelIndexes(const WModelIndex& parent,
				  int start, int count)
{
  shiftModelIndexes(parent, start, count, model(), expandedSet_);

  int removed = shiftModelIndexes(parent, start, count, model(),
				  selectionModel()->selection_);

  shiftEditorRows(parent, start, count, false);

  if (removed)
    selectionChanged().emit();
}

void WTreeView::modelLayoutAboutToBeChanged()
{
  WModelIndex::encodeAsRawIndexes(expandedSet_);

  WAbstractItemView::modelLayoutAboutToBeChanged();
}

void WTreeView::modelLayoutChanged()
{
  WAbstractItemView::modelLayoutChanged();

  expandedSet_ = WModelIndex::decodeFromRawIndexes(expandedSet_);

  renderedNodes_.clear();

  pageChanged().emit();
}

void WTreeView::addRenderedNode(WTreeViewNode *node)
{
  renderedNodes_[node->modelIndex()] = node;
  ++nodeLoad_;
  renderedNodesAdded_ = true;
}

void WTreeView::removeRenderedNode(WTreeViewNode *node)
{
  renderedNodes_.erase(node->modelIndex());
  --nodeLoad_;
}

bool WTreeView::internalSelect(const WModelIndex& index, SelectionFlag option)
{
  if (selectionBehavior() == SelectionBehavior::Rows && 
      index.column() != 0)
    return
      internalSelect(model()->index(index.row(), 0, index.parent()), option);

  if (WAbstractItemView::internalSelect(index, option)) {
    WTreeViewNode *node = nodeForIndex(index);
    if (node)
      node->renderSelected(isSelected(index), index.column());

    return true;
  } else
    return false;
}

WTreeViewNode *WTreeView::nodeForIndex(const WModelIndex& index) const
{
  if (index == rootIndex())
    return rootNode_;
  else {
    WModelIndex column0Index = index.column() == 0 ? index : model()->index(index.row(), 0, index.parent());
    NodeMap::const_iterator i = renderedNodes_.find(column0Index);
    return i != renderedNodes_.end() ? i->second : nullptr;
  }
}

void WTreeView::selectRange(const WModelIndex& first, const WModelIndex& last)
{
  WModelIndex index = first;
  for (;;) {
    for (int c = first.column(); c <= last.column(); ++c) {
      WModelIndex ic = model()->index(index.row(), c, index.parent());
      internalSelect(ic, SelectionFlag::Select);

      if (ic == last)
	return;
    }

    WModelIndex indexc0
      = index.column() == 0 ? index
      : model()->index(index.row(), 0, index.parent());

    if (isExpanded(indexc0) && model()->hasChildren(indexc0))
      index = model()->index(0, first.column(), indexc0);
    else {
      for (;;) {
	// next row in parent, if one is available
	WModelIndex parent = index.parent();
	if (index.row() + 1 < model()->rowCount(parent)) {
	  index = model()->index(index.row() + 1, first.column(), parent);
	  break;
	} else
	  // otherwise go up one level
	  index = index.parent();
      }
    }
  }
}

WAbstractItemView::ColumnInfo WTreeView::createColumnInfo(int column) const
{
  ColumnInfo ci = WAbstractItemView::createColumnInfo(column);

  if (column == 0) {
    // column 0 needs width auto, so we override the ci.styleRule with
    // a more specific rule. We also set the correct overflow attributes

    ci.width = WLength::Auto;
    ci.styleRule->templateWidget()->resize(WLength::Auto, WLength::Auto);

    if (c0StyleRule_) {
      WCssStyleSheet& styleSheet = wApp->styleSheet();
      c0StyleRule_->setSelector("#" + id() + " li ." + ci.styleClass());
      styleSheet.addRule(styleSheet.removeRule(c0StyleRule_)); // needed on rerender
    }
  }

  return ci;
}

void WTreeView::setCurrentPage(int page)
{
  viewportTop_ = page * viewportHeight_;

  contents_->setOffsets(-viewportTop_ * rowHeight().toPixels(), Side::Top);

  pageChanged().emit();

  scheduleRerender(RenderState::NeedAdjustViewPort);
}

int WTreeView::currentPage() const
{
  return viewportTop_ / viewportHeight_;
}

int WTreeView::pageCount() const
{
  if (rootNode_) {
    return (rootNode_->renderedHeight() - 1) / viewportHeight_ + 1;
  } else
    return 1;
}

int WTreeView::pageSize() const
{
  return viewportHeight_;
}

void WTreeView::scrollTo(const WModelIndex& index, ScrollHint hint)
{
  const int row = getIndexRow(index, rootIndex(), 0,
                              std::numeric_limits<int>::max());

  WApplication *app = WApplication::instance();

  if (app->environment().ajax()) {
    if (viewportHeight_ != UNKNOWN_VIEWPORT_HEIGHT) {
      if (hint == ScrollHint::EnsureVisible) {
        if (viewportTop_ + viewportHeight_ <= row)
	  hint = ScrollHint::PositionAtBottom;
	else if (row < viewportTop_)
	  hint = ScrollHint::PositionAtTop;
      }

      switch (hint) {
      case ScrollHint::PositionAtTop:
	viewportTop_ = row; break;
      case ScrollHint::PositionAtBottom:
	viewportTop_ = row - viewportHeight_ + 1; break;
      case ScrollHint::PositionAtCenter:
	viewportTop_ = row - viewportHeight_/2 + 1; break;
      default:
	break;
      }

      if (hint != ScrollHint::EnsureVisible) {
	scheduleRerender(RenderState::NeedAdjustViewPort);
      }
    }

    WStringStream s;

    s << "setTimeout(function() { " << jsRef()
      << ".wtObj.scrollTo(-1, "
      << row << "," << static_cast<int>(rowHeight().toPixels())
      << "," << (int)hint << ");});";

    doJavaScript(s.str());
  } else
    setCurrentPage(row / pageSize());
}

EventSignal<WScrollEvent>& WTreeView::scrolled(){
  if (wApp->environment().ajax() && contentsContainer_ != nullptr)
    return contentsContainer_->scrolled();

  throw WException("Scrolled signal existes only with ajax.");
}

void WTreeView::setId(const std::string &id)
{
  WAbstractItemView::setId(id);
  setup();
}

}

#endif // DOXYGEN_ONLY
