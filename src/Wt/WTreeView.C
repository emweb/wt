/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cmath>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include "Wt/WAbstractItemModel"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WIconPair"
#include "Wt/WItemDelegate"
#include "Wt/WItemSelectionModel"
#include "Wt/WImage"
#include "Wt/WTable"
#include "Wt/WTableCell"
#include "Wt/WText"
#include "Wt/WTreeView"
#include "Wt/WVBoxLayout"
#include "Wt/WWebWidget"

#include "EscapeOStream.h"
#include "Utils.h"

/*
  TODO:

  nice to have:
   - stateless slot implementations
   - keyboard navigation ?

  editing triggers
*/

#ifndef DOXYGEN_ONLY

// Widest scrollbar found ? My Gnome Firefox has this
#define SCROLLBAR_WIDTH_TEXT "19"
#define SCROLLBAR_WIDTH      19

namespace {
// returns true if i2 is an ancestor of i1
  bool isAncestor(const Wt::WModelIndex& i1, const Wt::WModelIndex& i2) {
    if (!i1.isValid())
      return false;

    for (Wt::WModelIndex p = i1.parent(); p.isValid(); p = p.parent()) {
      if (p == i2)
	return true;
    }

    return !i2.isValid();
  }
}

namespace Wt {

class ToggleButtonConfig
{
public:
  ToggleButtonConfig(WWidget *parent) : toggleJS_(0)
  {
    toggleJS_ = new JSlot(parent);
  }

  ~ToggleButtonConfig() {
    delete toggleJS_;
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
      ""  "if (s.className == states[i]) {"
      "" << app->javaScriptClass() << ".emit(s, 't-'+s.className);"
      ""    "s.className = states[(i+1) % " << states_.size() << "];"
      ""    "break;"
      ""  "}"
      """}"
      "}";
    
    toggleJS_->setJavaScript(js.str());
  }

  const std::vector<std::string>& states() const { return states_; }

private:
  std::vector<std::string> states_;
  JSlot *toggleJS_;

  friend class ToggleButton;
};

class ToggleButton : public Wt::WText
{
public:
  ToggleButton(ToggleButtonConfig *config, WContainerWidget *parent = 0) 
    : WText(parent),
      config_(config)
  {
    setInline(false);
    clicked().connect(*config_->toggleJS_);

    for (unsigned i = 0; i < config_->states().size(); ++i)
      signals_.push_back(new JSignal<>(this, "t-" + config_->states()[i]));
  }

  virtual ~ToggleButton() {
    for (unsigned i = 0; i < signals_.size(); ++i)
      delete signals_[i];
  }

  JSignal<>& signal(int i) { return *signals_[i]; }

  void setState(int i)
  {
    setStyleClass(config_->states()[i]);
  }

private:
  std::vector<JSignal<> *> signals_;
  ToggleButtonConfig      *config_;
};

class RowSpacer : public Wt::WWebWidget
{
public:
  RowSpacer(Wt::WTreeViewNode *node, int height)
    : node_(node),
      height_(0)
  {
    setRows(height);
    setInline(false);
    setStyleClass("Wt-spacer");
  }

  void setRows(int height, bool force = false);

  int rows() const { return height_; }
  Wt::WTreeViewNode *node() const { return node_; }

  int renderedRow(int lowerBound = 0,
		  int upperBound = std::numeric_limits<int>::max());

protected:
  virtual Wt::DomElementType domElementType() const {
    return Wt::DomElement_DIV;
  }

private:
  Wt::WTreeViewNode *node_;
  int height_;
};

class WTreeViewNode : public WTable
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

private:
  WTreeView     *view_;
  WModelIndex    index_;
  int            childrenHeight_;
  WTreeViewNode *parentNode_;

  bool           childrenLoaded_;
  ToggleButton  *expandButton_;
  WText         *noExpandIcon_;

  void loadChildren();

  WModelIndex childIndex(int column);

  WWidget   *widget(int column);
  void       setWidget(int column, WWidget *w, bool replace);
};

void RowSpacer::setRows(int height, bool force)
{
  if (force || height != height_) {
    height_ = height;
    resize(Wt::WLength::Auto, node_->view()->rowHeight() * height);
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
    index_(index),
    childrenHeight_(childrenHeight),
    parentNode_(parent),
    childrenLoaded_(false),
    expandButton_(0),
    noExpandIcon_(0)
{
  setStyleClass("Wt-tv-node");

  int selfHeight = 0;

  if (!view->isExpanded(index_))
    rowAt(1)->hide();

  bool needLoad = view_->isExpanded(index_);

  if (needLoad) {
    childrenLoaded_ = true;
    if (childrenHeight_ == -1)
      childrenHeight_ = view_->subTreeHeight(index_) - selfHeight;

    if (childrenHeight_ > 0)
      setTopSpacerHeight(childrenHeight_);
  } else
    childrenHeight_ = 0;

  if (index_ != view_->rootIndex()) {
    elementAt(0, 1)->setStyleClass("c1");
    WContainerWidget *w = new WContainerWidget(elementAt(0, 1));
    w->setStyleClass("rh c1div");

    updateGraphics(isLast, view_->model()->rowCount(index_) == 0);
    insertColumns(0, view_->columnCount());

    selfHeight = 1;

    if (view_->selectionBehavior() == SelectRows && view_->isSelected(index_))
      renderSelected(true, 0);

  } else
    if (WApplication::instance()->environment().agentIsIE())
      elementAt(0, 0)->resize(1, WLength::Auto); 

  view_->addRenderedNode(this);
}

WTreeViewNode::~WTreeViewNode()
{
  view_->removeRenderedNode(this);
}

void WTreeViewNode::update(int firstColumn, int lastColumn)
{
  WModelIndex parent = index_.parent();
  lastColumn = std::min(lastColumn, view_->model()->columnCount(parent) - 1);

  for (int i = firstColumn; i <= lastColumn; ++i) {
    WModelIndex child = childIndex(i);

    WWidget *currentW = widget(i);

    WFlags<ViewItemRenderFlag> renderFlags = 0;
    if (view_->selectionBehavior() == SelectItems && view_->isSelected(child))
      renderFlags |= RenderSelected;

    WWidget *newW = view_->itemDelegate(child)->update(currentW, child,
						       renderFlags);

    if (newW != currentW)
      setWidget(i, newW, currentW != 0);
  }
}

void WTreeViewNode::updateGraphics(bool isLast, bool isEmpty)
{
  if (index_ == view_->rootIndex())
    return;

  if (index_.parent() == view_->rootIndex() && !view_->rootIsDecorated()) {
    delete expandButton_;
    expandButton_ = 0;
    delete noExpandIcon_;
    noExpandIcon_ = 0;
    elementAt(0, 0)->setStyleClass("c0");
    elementAt(1, 0)->setStyleClass("c0");

    return;
  }

  if (!isEmpty) {
    if (!expandButton_) {
      delete noExpandIcon_;
      noExpandIcon_ = 0;
      expandButton_ = new ToggleButton(view_->expandConfig_);
      if (WApplication::instance()->environment().agentIsIE())
	expandButton_->resize(19, WLength::Auto);
      elementAt(0, 0)->addWidget(expandButton_);

      expandButton_->signal(0).connect(SLOT(this, WTreeViewNode::doExpand));
      expandButton_->signal(1).connect(SLOT(this, WTreeViewNode::doCollapse));

      expandButton_->setState(isExpanded() ? 1 : 0);
    }
  } else {
    if (!noExpandIcon_) {
      delete expandButton_;
      expandButton_ = 0;
      noExpandIcon_ = new WText();
      noExpandIcon_->setInline(false);
      noExpandIcon_->setStyleClass("Wt-noexpand");
      if (WApplication::instance()->environment().agentIsIE())
	noExpandIcon_->resize(19, WLength::Auto);
      elementAt(0, 0)->addWidget(noExpandIcon_);
    }
  }

  if (!isLast) {
    elementAt(0, 0)->setStyleClass("Wt-trunk c0");
    elementAt(1, 0)->setStyleClass("Wt-trunk c0");
  } else {
    elementAt(0, 0)->setStyleClass("Wt-end c0");
    elementAt(1, 0)->setStyleClass("c0");
  }
}

void WTreeViewNode::insertColumns(int column, int count)
{
  WTableCell *tc = elementAt(0, 1);
  WContainerWidget *w = dynamic_cast<WContainerWidget *>(tc->widget(0));
  w->clear();

  if (view_->columnCount() > 1) {
    WContainerWidget *row = new WContainerWidget();

    if (view_->column1Fixed_) {
      row->setStyleClass("Wt-tv-rowc rh");
      WContainerWidget *rowWrap = new WContainerWidget();
      rowWrap->addWidget(row);
      row = rowWrap;
    }

    row->setStyleClass("Wt-tv-row rh");
    w->insertWidget(0, row);
  }

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

void WTreeViewNode::setWidget(int column, WWidget *newW, bool replace)
{
  WTableCell *tc = elementAt(0, 1);
  WContainerWidget *w = dynamic_cast<WContainerWidget *>(tc->widget(0));

  WWidget *current = replace ? widget(column) : 0;

  if (current) {
    newW->setStyleClass(current->styleClass());
    current->setStyleClass(WString());
  } else {
    EscapeOStream s;
    s << "Wt-tv-c rh " << view_->columnStyleClass(column) << ' '
      << newW->styleClass().toUTF8();

    newW->setStyleClass(WString::fromUTF8(s.c_str()));
  }

  if (column == 0) {
    if (current)
      w->removeWidget(current);

    w->addWidget(newW);
  } else {
    WContainerWidget *row = dynamic_cast<WContainerWidget *>(w->widget(0));
    if (view_->column1Fixed_)
      row = dynamic_cast<WContainerWidget *>(row->widget(0));

    if (current)
      row->removeWidget(current);

    row->insertWidget(column - 1, newW);
  }

  if (!WApplication::instance()->environment().ajax()) {
    WInteractWidget *wi = dynamic_cast<WInteractWidget *>(newW);
    if (wi)
      view_->clickedMapper_->mapConnect(wi->clicked(), childIndex(column));
  }
}

WWidget *WTreeViewNode::widget(int column)
{
  WTableCell *tc = elementAt(0, 1);
  WContainerWidget *w = dynamic_cast<WContainerWidget *>(tc->widget(0));

  if (column == 0)
    return w->count() > 1 ? w->widget(w->count() - 1) : 0;
  else {
    WContainerWidget *row = dynamic_cast<WContainerWidget *>(w->widget(0));
    if (view_->column1Fixed_)
      row = dynamic_cast<WContainerWidget *>(row->widget(0));

    return row->count() >= column ? row->widget(column - 1) : 0;
  }
}

void WTreeViewNode::doExpand()
{
  if (isExpanded())
    return;

  loadChildren();

  if (expandButton_)
    expandButton_->setState(1);

  view_->expandedSet_.insert(index_);

  rowAt(1)->show();
  if (parentNode())
    parentNode()->adjustChildrenHeight(childrenHeight_);

  view_->adjustRenderedNode(this, renderedRow());
  view_->scheduleRerender(WTreeView::NeedAdjustViewPort);

  view_->expanded_.emit(index_);
}

void WTreeViewNode::doCollapse()
{
  if (!isExpanded())
    return;

  if (expandButton_)
    expandButton_->setState(0);

  view_->setCollapsed(index_);

  rowAt(1)->hide();
  if (parentNode())
    parentNode()->adjustChildrenHeight(-childrenHeight_);

  view_->renderedRowsChanged(renderedRow(), -childrenHeight_);

  view_->collapsed_.emit(index_);
}

bool WTreeViewNode::isExpanded()
{
  return !rowAt(1)->isHidden();
}

void WTreeViewNode::normalizeSpacers()
{
  if (childrenLoaded_
      && childContainer()->count() == 2
      && topSpacer() && bottomSpacer()) {
    addTopSpacerHeight(bottomSpacerHeight());
    delete bottomSpacer();
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
  }
}

WContainerWidget *WTreeViewNode::childContainer()
{
  return elementAt(index_ == view_->rootIndex() ? 0 : 1, 1);
}

WWidget *WTreeViewNode::widgetForModelRow(int modelRow)
{
  if (!childrenLoaded_)
    return 0;

  WContainerWidget *c = childContainer();

  int first = topSpacer() ? 1 : 0;

  if (first < c->count()) {
    WTreeViewNode *n = dynamic_cast<WTreeViewNode *>(c->widget(first));
    if (n) {
      int row = n->index_.row();
      int index = first + (modelRow - row);

      if (index < first)
	return topSpacer();
      else if (index < c->count())
	return c->widget(index);
      else
	return bottomSpacer();
    } else
      return bottomSpacer();
  } else
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
      int lastColumn = std::min(view_->columnCount() - 1,
				view_->model()->columnCount(index_) - 1);

      for (int j = 0; j <= lastColumn; ++j) {
	WModelIndex child = n->childIndex(j);
	view_->itemDelegate(child)->updateModelIndex(n->widget(j), child);
      }

      view_->addRenderedNode(n);
    }
  }
}

int WTreeViewNode::renderedHeight()
{
  return (index_ != view_->rootIndex() ? 1 : 0)
    + (isExpanded() ? childrenHeight_ : 0);
}

int WTreeViewNode::topSpacerHeight()
{
  RowSpacer *s = topSpacer();

  return s ? s->rows() : 0;
}

void WTreeViewNode::setTopSpacerHeight(int rows)
{
  if (rows == 0)
    delete topSpacer();
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

  RowSpacer *result = 0;
  if (c->count() == 0 || !(result = dynamic_cast<RowSpacer *>(c->widget(0))))
    if (!create)
      return 0;
    else {
      result = new RowSpacer(this, 0);
      c->insertWidget(0, result);
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
  if (!rows)
    delete bottomSpacer();
  else
    bottomSpacer(true)->setRows(rows);
}

void WTreeViewNode::addBottomSpacerHeight(int rows)
{
  setBottomSpacerHeight(bottomSpacerHeight() + rows);
}

RowSpacer *WTreeViewNode::bottomSpacer(bool create)
{
  WContainerWidget *c = childContainer();

  RowSpacer *result = 0;
  if (c->count() == 0
      || !(result = dynamic_cast<RowSpacer *>(c->widget(c->count() - 1))))
    if (!create)
      return 0;
    else {
      result = new RowSpacer(this, 0);
      c->addWidget(result);
    }

  return result;
}

WTreeViewNode *WTreeViewNode::nextChildNode(WTreeViewNode *prev)
{
  if (!childrenLoaded_)
    return 0;

  WContainerWidget *c = childContainer();

  int nextI = prev ? c->indexOf(prev) + 1 : topSpacer() ? 1 : 0;

  if (nextI < c->count())
    return dynamic_cast<WTreeViewNode *>(c->widget(nextI));
  else
    return 0;
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

  for (WTreeViewNode *c = nextChildNode(0); c; c = nextChildNode(c)) {
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
  if (view_->selectionBehavior() == SelectRows)
    rowAt(0)->setStyleClass(selected ? "Wt-selected" : "");
  else {
    WWidget *w = widget(column);
    if (selected)
      w->setStyleClass(Utils::addWord(w->styleClass().toUTF8(), "Wt-selected"));
    else
      w->setStyleClass(Utils::eraseWord(w->styleClass().toUTF8(), "Wt-selected"));
  }
}

void WTreeViewNode::selfCheck()
{
  assert(renderedHeight() == view_->subTreeHeight(index_));

  int childNodesHeight = 0;
  for (WTreeViewNode *c = nextChildNode(0); c; c = nextChildNode(c)) {
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

WTreeView::ColumnInfo::ColumnInfo(const WTreeView *view, WApplication *app,
				  int anId, int column)
  : id(anId),
    sortOrder(AscendingOrder),
    alignment(AlignLeft),
    headerAlignment(AlignLeft),
    extraHeaderWidget(0),
    sorting(view->sorting_),
    itemDelegate_(0)
{
  styleRule = new WCssTemplateRule("#" + view->id()
				   + " ." + this->styleClass());
  if (column != 0) {
    width = WLength(150);
    styleRule->templateWidget()->resize(width, WLength::Auto);
  }

  app->styleSheet().addRule(styleRule);
}

std::string WTreeView::ColumnInfo::styleClass() const
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

WTreeView::WTreeView(WContainerWidget *parent)
  : WCompositeWidget(0),
    model_(0),
    itemDelegate_(new WItemDelegate(this)),
    selectionModel_(new WItemSelectionModel(0, this)),
    rowHeight_(20),
    headerLineHeight_(20),
    rootNode_(0),
    borderColorRule_(0),
    alternatingRowColors_(false),
    rootIsDecorated_(true),
    selectionMode_(NoSelection),
    sorting_(true),
    columnResize_(true),
    multiLineHeader_(false),
    column1Fixed_(false),
    nextColumnId_(1),
    collapsed_(this),
    expanded_(this),
    clicked_(this),
    doubleClicked_(this),
    mouseWentDown_(this),
    selectionChanged_(this),
    viewportTop_(0),
    viewportHeight_(30),
    nodeLoad_(0),
    currentSortColumn_(-1),
    contentsContainer_(0),
    scrollBarC_(0),
    resizeHandleMDownJS_(this),
    resizeHandleMMovedJS_(this),
    resizeHandleMUpJS_(this),
    tieContentsHeaderScrollJS_(this),
    tieRowsScrollJS_(this),
    itemClickedJS_(this),
    itemDoubleClickedJS_(this),
    itemMouseDownJS_(this),
    itemEvent_(this, "itemEvent"),
    dragEnabled_(false),
    dropsEnabled_(false)
{
  setImplementation(impl_ = new WContainerWidget());

  renderState_ = NeedRerender;

  WApplication *app = WApplication::instance();

  clickedForSortMapper_ = new WSignalMapper<int>(this);
  clickedForSortMapper_->mapped().connect(SLOT(this,
					     WTreeView::toggleSortColumn));

  clickedForCollapseMapper_ = new WSignalMapper<int>(this);
  clickedForCollapseMapper_->mapped().connect(SLOT(this,
						   WTreeView::collapseColumn));

  clickedForExpandMapper_ = new WSignalMapper<int>(this);
  clickedForExpandMapper_->mapped().connect(SLOT(this,
						 WTreeView::expandColumn));

  if (!app->environment().ajax()) {
    clickedMapper_ = new WSignalMapper<WModelIndex>(this);
    clickedMapper_->mapped().connect(SLOT(this, WTreeView::handleClick));
  }

  itemEvent_.connect(SLOT(this, WTreeView::onItemEvent));

  setStyleClass("Wt-treeview");

  const char *CSS_RULES_NAME = "Wt::WTreeView";

  expandConfig_ = new ToggleButtonConfig(this);
  expandConfig_->addState("Wt-expand");
  expandConfig_->addState("Wt-collapse");
  expandConfig_->generate();

  if (!app->styleSheet().isDefined(CSS_RULES_NAME)) {
    /* header */
    app->styleSheet().addRule
      (".Wt-treeview .Wt-headerdiv",
       "-moz-user-select: none;"
       "-khtml-user-select: none;"
       "user-select: none;"
       "background-color: #EEEEEE;"
       "overflow: hidden;"
       "width: 100%;", CSS_RULES_NAME);

    if (app->environment().agentIsIE())
      app->styleSheet().addRule
	(".Wt-treeview .Wt-header .Wt-label",
	 "zoom: 1;");

    app->styleSheet().addRule
      (".Wt-treeview table", "width: 100%");

    app->styleSheet().addRule
      (".Wt-treeview .c1", "width: 100%");

    app->styleSheet().addRule
      (".Wt-treeview .c1div", "overflow: hidden; width: 100%");

    app->styleSheet().addRule
      (".Wt-treeview .c0",
       "width: 19px; vertical-align: middle");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-row", "float: right; overflow: hidden;");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-row .Wt-tv-c",
       "display: block; float: left;"
       "padding: 0px 3px;"
       "text-overflow: ellipsis;"
       "overflow: hidden;"
       "white-space: nowrap;");

    app->styleSheet().addRule
      (".Wt-treeview img.icon, .Wt-treeview input.icon",
       "margin: 0px 3px 2px 0px; vertical-align: middle");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-node img.w0",
       "width: 0px");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-node .c0 img, .Wt-treeview .Wt-tv-node .c0 input",
       "margin-right: 0px; margin: -4px 0px;");

    /* resize handles */
    app->styleSheet().addRule
      (".Wt-treeview div.Wt-tv-rh",
       "float: right; width: 4px; cursor: col-resize;"
       "padding-left: 0px;");

    if (app->environment().agentIsIE()) {
      app->styleSheet().addRule
	(".Wt-treeview .Wt-header .Wt-tv-c",
	 "padding: 0px;"
	 "padding-left: 7px;");
    } else
      app->styleSheet().addRule
	(".Wt-treeview .Wt-header .Wt-tv-c",
	 "padding: 0px;"
	 "margin-left: 7px;");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-rh:hover",
       "background-color: #DDDDDD;");

    app->styleSheet().addRule
      (".Wt-treeview div.Wt-tv-rhc0",
       "float: left; width: 4px;");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-sh",
       "float: right; width: 16px; "
       "margin-top: 6px; cursor: pointer; cursor:hand;");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-sh-nrh",
       "float: right; width: 16px; "
       "margin-top: 6px; margin-right: 4px; cursor: pointer; cursor:hand;");

    /* borders: needed here for IE */
    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-br, "                     // header columns 1-n
       ".Wt-treeview .Wt-tv-node .Wt-tv-row .Wt-tv-c", // data columns 1-n
       "border-right: 1px solid #FFFFFF;");
    app->styleSheet().addRule
      (".Wt-treeview .header .Wt-tv-row, "    // header column 0
       ".Wt-treeview .Wt-tv-node .Wt-tv-row", // data column 0
       "border-left: 1px solid #FFFFFF;");

    /* sort handles */
    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-sh", std::string() +
       "float: right; width: 16px; height: 10px;"
       + (app->environment().agent() != WEnvironment::IE6 ?
	  "margin-top: 6px;" : "") +
       "cursor: pointer; cursor:hand;");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-sh-nrh", std::string() + 
       "float: right; width: 16px; height: 10px;"
       + (app->environment().agent() != WEnvironment::IE6 ?
	  "margin-top: 6px;" : "") +
       "margin-right: 4px;"
       "cursor: pointer; cursor:hand;");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-shc0", "float: left;");

    /* bottom scrollbar */
    if (app->environment().agentIsWebKit() || app->environment().agentIsOpera())
      app->styleSheet().addRule
	(".Wt-treeview .Wt-tv-rowc", "position: relative;");

    if (app->environment().agentIsIE())
      app->styleSheet().addRule
	(".Wt-treeview .Wt-scroll",
	 "position: absolute; overflow-x: scroll;"
	 "height: " SCROLLBAR_WIDTH_TEXT "px;");
    else
      app->styleSheet().addRule
	(".Wt-treeview .Wt-scroll", "overflow: scroll;"
	 "height: " SCROLLBAR_WIDTH_TEXT "px;");
    app->styleSheet().addRule
      (".Wt-treeview .Wt-scroll div", "height: 1px;");
  }

  setColumnBorder(white);

  app->styleSheet().addRule("#" + id() + " .cwidth", "height: 1px;");

  /* item drag & drop */
  app->styleSheet().addRule
    ("#" + id() + "dw",
     "width: 32px; height: 32px;"
     "background: url(" + WApplication::resourcesUrl() + "items-not-ok.gif);");

  app->styleSheet().addRule
    ("#" + id() + "dw.Wt-valid-drop",
     "width: 32px; height: 32px;"
     "background: url(" + WApplication::resourcesUrl() + "items-ok.gif);");

  rowHeightRule_ = new WCssTemplateRule("#" + id() + " .rh");
  app->styleSheet().addRule(rowHeightRule_);

  rowWidthRule_ = new WCssTemplateRule("#" + id() + " .Wt-tv-row");
  app->styleSheet().addRule(rowWidthRule_);

  rowContentsWidthRule_ = new WCssTemplateRule("#" + id() +" .Wt-tv-rowc");
  app->styleSheet().addRule(rowContentsWidthRule_);

  c0WidthRule_ = new WCssTemplateRule("#" + id() + " .c0w");
  c0WidthRule_->templateWidget()->resize(150, WLength::Auto);
  app->styleSheet().addRule(c0WidthRule_);

  setRowHeight(rowHeight_);

  /*
   * Setup main layout
   */

  if (app->environment().javaScript())
    impl_->setPositionScheme(Relative);

  WVBoxLayout *layout = new WVBoxLayout();
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);

  layout->addWidget(headerContainer_ = new WContainerWidget());
  headerContainer_->setOverflow(WContainerWidget::OverflowHidden);
  headerContainer_->setStyleClass("Wt-header headerrh cwidth");
  headers_ = new WContainerWidget(headerContainer_);
  headers_->setStyleClass("Wt-headerdiv headerrh");
  headers_->setSelectable(false);

  headerHeightRule_ = new WCssTemplateRule("#" + id() + " .headerrh");
  app->styleSheet().addRule(headerHeightRule_);
  setHeaderHeight(headerLineHeight_);

  contentsContainer_ = new WContainerWidget();
  contentsContainer_->setStyleClass("cwidth");
  contentsContainer_->setOverflow(WContainerWidget::OverflowAuto);
  contentsContainer_->addWidget(contents_ = new WContainerWidget());
  contentsContainer_->scrolled().connect(SLOT(this, WTreeView::onViewportChange));
  contentsContainer_->scrolled().connect(tieContentsHeaderScrollJS_);
  contents_->addWidget(new WContainerWidget()); // wrapRoot

  if (app->environment().agentIsIE())
    contents_->setAttributeValue("style", "zoom: 1"); // trigger hasLayout

  layout->addWidget(contentsContainer_, 1);

  impl_->setLayout(layout);

  selectionChanged().connect(SLOT(this, WTreeView::checkDragSelection));

  app->declareJavaScriptFunction
    ("getItem",
     "function(event) {"
     """var columnId = -1, nodeId = null, selected = false, "
     ""   "drop = false, el = null;"
     """var t = event.target || event.srcElement;"
     """while (t) {"
     ""  "if (t.className.indexOf('c1 rh') == 0) {"
     ""    "if (columnId == -1)"
     ""      "columnId = 0;"
     ""  "} else if (t.className.indexOf('Wt-tv-c') == 0) {"
     ""    "if (t.className.indexOf('Wt-tv-c rh Wt-tv-c') == 0)"
     ""      "columnId = t.className.split(' ')[2].substring(7) * 1;"
     ""    "else if (columnId == -1)"
     ""      "columnId = 0;"
     ""    "if (t.getAttribute('drop') === 'true')"
     ""      "drop = true;"
     ""      "el = t;"
     ""  "} else if (t.className == 'Wt-tv-node') {"
     ""    "nodeId = t.id;"
     ""    "break;"
     ""  "}"
     ""  "if (t.className === 'Wt-selected')"
     ""    "selected = true;"
     ""  "t = t.parentNode;"
     ""  "if (" WT_CLASS ".hasTag(t, 'BODY'))"
     ""    "break;"
     """}"
     """return { columnId: columnId, nodeId: nodeId, selected: selected, "
     ""         "drop: drop, el: el };"
     "}");

  itemClickedJS_.setJavaScript
    ("function(obj, event) {"
     """var item=" + app->javaScriptClass() + ".getItem(event);"
     """if (item.columnId != -1) {"
     "" + itemEvent_.createEventCall("item.el", "event", "item.nodeId",
				     "item.columnId", "'clicked'",
				     "''", "''") + ";"
     """}"
     "}");

  itemDoubleClickedJS_.setJavaScript
    ("function(obj, event) {"
     """var item=" + app->javaScriptClass() + ".getItem(event);"
     """if (item.columnId != -1)"
     "" + itemEvent_.createEventCall("item.el", "event", "item.nodeId",
				     "item.columnId", "'dblclicked'",
				     "''", "''") + ";"
     "}");

  itemMouseDownJS_.setJavaScript
    ("function(obj, event) {"
     """var APP=" + app->javaScriptClass() +", tv=" + jsRef() + ";"
     """APP._p_.capture(null);"
     """var item=APP.getItem(event);"
     """if (item.columnId != -1) {"
     "" + itemEvent_.createEventCall("item.el", "event", "item.nodeId",
				     "item.columnId", "'mousedown'",
				     "''", "''") + ";"
     ""  "if (tv.getAttribute('drag') === 'true' && item.selected)"
     ""    "APP._p_.dragStart(tv, event);"
     """}"
     "}");

  resizeHandleMDownJS_.setJavaScript
    ("function(obj, event) {"
     """var pc = " WT_CLASS ".pageCoordinates(event);"
     """obj.setAttribute('dsx', pc.x);"
     "}");

  resizeHandleMMovedJS_.setJavaScript
    ("function(obj, event) {"
     """var WT = " WT_CLASS ","
     ""    "lastx = obj.getAttribute('dsx'),"
     ""    "t = " + contents_->jsRef() + ".firstChild,"
     ""    "h=" + headers_->jsRef() + ","
     ""    "hh=h.firstChild;"
     """if (lastx != null && lastx != '') {"
     ""  "nowxy = WT.pageCoordinates(event);"
     ""  "var parent = obj.parentNode.parentNode,"
     ""      "diffx = Math.max(nowxy.x - lastx, -parent.offsetWidth),"
     ""      "c = parent.className.split(' ')[2];"
     ""  "if (c == 'unselectable')"
     ""    "c = null;"
     ""  "if (c) {"
     ""    "var r = WT.getCssRule('#" + id() + " .' + c),"
     ""        "tw = WT.pxself(r, 'width');"
     ""    "if (tw == 0) tw = parent.offsetWidth;" 
     ""    "r.style.width = Math.max(0, tw + diffx) + 'px';"
     ""  "}"
     + jsRef() + ".adjustHeaderWidth(c, diffx);"
     ""  "obj.setAttribute('dsx', nowxy.x);"
     ""  "WT.cancelEvent(event);"
     "  }"
     "}");

  resizeHandleMUpJS_.setJavaScript
    ("function(obj, event) {"
     """obj.removeAttribute('dsx');"
     "" WT_CLASS ".cancelEvent(event);"
     "}");

  tieContentsHeaderScrollJS_.setJavaScript
    ("function(obj, event) {"
     "" + headerContainer_->jsRef() + ".scrollLeft=obj.scrollLeft;"
     /* the following is a workaround for IE7 */
     """var t = " + contents_->jsRef() + ".firstChild;"
     """var h = " + headers_->jsRef() + ";"
     """h.style.width = (t.offsetWidth - 1) + 'px';"
     """h.style.width = t.offsetWidth + 'px';"
     "}");

  if (app->environment().agentIsWebKit() || app->environment().agentIsOpera())
    tieRowsScrollJS_.setJavaScript
      ("function(obj, event) {"
       "" WT_CLASS ".getCssRule('#" + id() + " .Wt-tv-rowc').style.left"
       ""  "= -obj.scrollLeft + 'px';"
       "}");
  else {
    /* this is for some reason very very slow in webkit: */
    tieRowsScrollJS_.setJavaScript
      ("function(obj, event) {"
       "var c =" WT_CLASS ".getElementsByClassName('Wt-tv-rowc', "
       + jsRef() + ");"
       "for (var i = 0, length = c.length; i < length; ++i) {"
       """var cc=c[i];"
       """if (cc.parentNode.scrollLeft != obj.scrollLeft)"
       ""  "cc.parentNode.scrollLeft=obj.scrollLeft;"
       "}"
       "}");
  }

  /*
   * This continuously adjusts:
   *  - changes to the total width (tw)
   *  - whether scrollbars are needed (vscroll), and thus the actual
   *    contents width
   *  - when column1 is fixed: the width of the other columns
   */
  app->addAutoJavaScript
    ("{var e=" + contentsContainer_->jsRef() + ";"
     "var s=" + jsRef() + ";"
     "var WT=" WT_CLASS ";"
     "if (e) {"
     """var tw=s.offsetWidth-WT.px(s, 'borderLeftWidth')"
     ""       "-WT.px(s, 'borderRightWidth'),"
     ""    "vscroll=e.scrollHeight > e.offsetHeight;"
     ""    "c0w = null;"
     ""
     """if (s.className.indexOf('column1') != -1)"
     """  c0w = WT.pxself(WT.getCssRule('#" + id() + " .c0w'), 'width');"
     ""
     """if (tw > 200 " // XXX: IE's incremental rendering foobars completely
     ""    "&& (tw != e.tw || vscroll != e.vscroll || c0w != e.c0w)) {"
     ""  "e.vscroll = vscroll;"
     ""  "e.tw = tw;"
     ""  "e.c0w = c0w;"
     ""  "var h= " + headers_->jsRef() + ","
     ""      "hh=h.firstChild,"
     ""      "t=" + contents_->jsRef() + ".firstChild,"
     ""      "r= WT.getCssRule('#" + id() + " .cwidth'),"
     ""      "contentstoo=(r.style.width == h.style.width);"
     ""  "r.style.width=(tw - (vscroll ? " SCROLLBAR_WIDTH_TEXT " : 0)) + 'px';"
     ""  "e.style.width=tw + 'px';"
     ""  "h.style.width=t.offsetWidth + 'px';"
     ""  "if (c0w != null) {"
     ""    "var hh=h.firstChild,"
     ""        "w=tw - c0w - (vscroll ? " SCROLLBAR_WIDTH_TEXT " : 0);"
     ""    "if (w > 0) {"
     ""      "WT.getCssRule('#" + id() + " .Wt-tv-row').style.width = w + 'px';"
     ""      "var extra = "
     ""      "hh.childNodes.length > 1"
     // all browsers except for IE6 would do with 21 : 6
     ""        "? (hh.childNodes[1].className.indexOf('Wt-tv-sh') != -1 ? 22 : 7) : 0;"
     ""      "hh.style.width= (w + extra) + 'px';"
     ""    "}"
     ""  "} else if (contentstoo) {"
     ""    "h.style.width=r.style.width;"
     ""    "t.style.width=r.style.width;"
     ""  "}"
     ""  "if (s.adjustHeaderWidth)"
     ""    "s.adjustHeaderWidth(1, 0);"
     """}"
     "}"
     "}"
     );

  if (parent)
    parent->addWidget(this);
}

void WTreeView::refresh()
{
  needDefineJS_ = false;

  WApplication *app = WApplication::instance();

  std::string columnsWidth = std::string() +
    "var WT=" WT_CLASS ","
    ""  "t=" + contents_->jsRef() + ".firstChild,"
    ""  "h=" + headers_->jsRef() + ","
    ""  "hh=h.firstChild,"
    ""  "hc=hh.firstChild" + (column1Fixed_ ? ".firstChild" : "") + ","
    ""  "totalw=0,"
    ""  "extra=" + (column1Fixed_ ? "1" : "4") + ""
    ""     "+ (hh.childNodes.length > 1"
    // all browsers except for IE6 would do with 17 : 6
    ""       "? (hh.childNodes[1].className.indexOf('Wt-tv-sh') != -1 ? 18 : 7)"
    ""       ": 0);"

    "if(" + jsRef() + ".offsetWidth == 0) return;"

    "for (var i=0, length=hc.childNodes.length; i < length; ++i) {"
    """if (hc.childNodes[i].className) {" // IE may have only a text node
    ""  "var cl = hc.childNodes[i].className.split(' ')[2],"
    ""      "r = WT.getCssRule('#" + id() + " .' + cl);"
         // 7 = 2 * 3px (padding) + 1px border
    ""  "totalw += WT.pxself(r, 'width') + 7;"
    """}"
    "}"

    "var cw = WT.pxself(hh, 'width'),"
    ""  "hdiff = c ? (cw == 0 ? 0 : (totalw - (cw - extra))) : diffx;\n";
    //"alert(hh + ' ' + cw + ' ' + totalw + ' -> ' + hdiff);";
  if (!column1Fixed_)
    columnsWidth +=
      "t.style.width = (t.offsetWidth + hdiff) + 'px';"
      "h.style.width = t.offsetWidth + 'px';"
      "hh.style.width = (totalw + extra) + 'px';";
  else
    columnsWidth +=
      "var r = WT.getCssRule('#" + id() + " '"
      ""                    "+ (c ? '.Wt-tv-rowc' : '.c0w'));"
      "totalw += 'px';"
      "if (c) {"
      """r.style.width = totalw;"
      + (app->environment().agentIsIE() ? 
	 "var c =" WT_CLASS ".getElementsByClassName('Wt-tv-rowc', "
	 + jsRef() + ");"
	 "for (var i = 0, length = c.length; i < length; ++i) {"
	 """var cc=c[i];"
	 """cc.style.width = totalw;"
	 "}"
	 : "") +
      "} else {"
      """r.style.width = (WT.pxself(r, 'width') + diffx) + 'px';"
      +  app->javaScriptClass() + "._p_.autoJavaScript();"
      "}";

  app->doJavaScript
    (jsRef() + ".adjustHeaderWidth=function(c, diffx) {"
     "if (" + contentsContainer_->jsRef() + ") {"
     + columnsWidth + "}};");

  app->doJavaScript
    (jsRef() + ".handleDragDrop=function(action, object, event, "
     ""                                 "sourceId, mimeType) {"
     """var self=" + jsRef() + ";"
     """if (self.dropEl) {"
     ""  "self.dropEl.className = self.dropEl.classNameOrig;"
     ""  "self.dropEl = null;"
     """}"
     """if (action=='end')"
     ""  "return;"
     """var item=" + app->javaScriptClass() + ".getItem(event);"
     """if (!item.selected && item.drop && item.columnId != -1) {"
     ""  "if (action=='drop') {"
     ""    + itemEvent_.createCall("item.nodeId", "item.columnId", "'drop'",
				   "sourceId", "mimeType") + ";"
     ""  "} else {"
     ""    "object.className = 'Wt-valid-drop';"
     ""    "self.dropEl = item.el;"
     ""    "self.dropEl.classNameOrig = self.dropEl.className;"
     ""    "self.dropEl.className = self.dropEl.className + ' Wt-drop-site';"
     ""  "}"
     """} else {"
     ""  "object.className = '';"
     """}"
     "};");
}

void WTreeView::initLayoutJavaScript()
{
  refresh();
}

void WTreeView::setColumn1Fixed(bool fixed)
{
  if (fixed && !column1Fixed_) {
    column1Fixed_ = fixed;

    setStyleClass("Wt-treeview column1");
    WContainerWidget *rootWrap
      = dynamic_cast<WContainerWidget *>(contents_->widget(0));
    rootWrap->resize(WLength(100, WLength::Percentage), WLength::Auto);
    rootWrap->setOverflow(WContainerWidget::OverflowHidden);

    // needed for IE, otherwise row expands automatically to content width
    rowWidthRule_->templateWidget()->resize(0, WLength::Auto);

    WContainerWidget *scrollBarContainer = new WContainerWidget();
    scrollBarContainer->setStyleClass("cwidth");
    scrollBarContainer->resize(WLength::Auto, SCROLLBAR_WIDTH);
    scrollBarC_ = new WContainerWidget(scrollBarContainer);
    scrollBarC_->setStyleClass("Wt-tv-row Wt-scroll");
    scrollBarC_->scrolled().connect(tieRowsScrollJS_);

    WApplication *app = WApplication::instance();

    if (app->environment().agentIsIE()) {
      scrollBarContainer->setPositionScheme(Relative);
      scrollBarC_->setAttributeValue("style", "right: 0px");
      // and still it doesn't work properly...
    }

    WContainerWidget *scrollBar = new WContainerWidget(scrollBarC_);
    scrollBar->setStyleClass("Wt-tv-rowc");
    if (app->environment().agentIsWebKit() || app->environment().agentIsOpera())
      scrollBar->setAttributeValue("style", "left: 0px;");
    impl_->layout()->addWidget(scrollBarContainer);
  }
}

void WTreeView::load()
{
  needDefineJS_ = true;

  WCompositeWidget::load();
}

WTreeView::~WTreeView()
{ 
  delete expandConfig_;

  impl_->clear();
  delete rowHeightRule_;

  for (unsigned i = 0; i < columns_.size(); ++i)
    delete columns_[i].styleRule;
}

int WTreeView::columnCount() const
{
  return columns_.size();
}

std::string WTreeView::columnStyleClass(int column) const
{
  return columnInfo(column).styleClass();
}

#ifndef WT_DEPRECATED_3_0_0
void WTreeView::setColumnFormat(int column, const WT_USTRING& format)
{
  ColumnInfo& info = columnInfo(column);

  WItemDelegate *id;
  if (info.itemDelegate_)
    id = dynamic_cast<WItemDelegate *>(info.itemDelegate_);
  else
    info.itemDelegate_ = id = new WItemDelegate(this);

  if (id)
    id->setTextFormat(format);
}

WT_USTRING WTreeView::columnFormat(int column) const
{
  ColumnInfo& info = columnInfo(column);
  if (info.itemDelegate_) {
    WItemDelegate *id = dynamic_cast<WItemDelegate *>(info.itemDelegate_);
    return id ? id->textFormat() : WT_USTRING();
  } else
    return WT_USTRING();
}
#endif // WT_DEPRECATED_3_0_0

void WTreeView::setColumnWidth(int column, const WLength& width)
{
  columnInfo(column).width = width;

  if (column != 0)
    columnInfo(column).styleRule->templateWidget()
      ->resize(width, WLength::Auto);
  else
    if (column1Fixed_)
      c0WidthRule_->templateWidget()
	->resize(width.toPixels(), WLength::Auto);

  if (!column1Fixed_ && !columnInfo(0).width.isAuto()) {
    // column 0 is sized implicitly by sizing the total table width
    double total = 0;
    for (int i = 0; i < columnCount(); ++i)
      total += columnInfo(i).width.toPixels() + 7;

    headers_->resize(total, headers_->height());

    WContainerWidget *wrapRoot
      = dynamic_cast<WContainerWidget *>(contents_->widget(0));

    wrapRoot->resize(total, wrapRoot->height());
  }
}

WLength WTreeView::columnWidth(int column) const
{
  return columnInfo(column).width;
}

void WTreeView::setColumnAlignment(int column, AlignmentFlag alignment)
{
  WWidget *w = columnInfo(column).styleRule->templateWidget();

  if (column != 0) {
    const char *align = 0;
    switch (alignment) {
    case AlignLeft: align = "left"; break;
    case AlignCenter: align = "center"; break;
    case AlignRight: align = "right"; break;
    case AlignJustify: align = "justify"; break;
    default:
      break;
    }
    if (align)
      w->setAttributeValue("style", std::string("text-align: ") + align);
  } else
    if (alignment == AlignRight)
      w->setFloatSide(Right);
}

AlignmentFlag WTreeView::columnAlignment(int column) const
{
  return columnInfo(column).alignment;
}

void WTreeView::setHeaderAlignment(int column, AlignmentFlag alignment)
{
  columnInfo(column).headerAlignment = alignment;

  if (renderState_ >= NeedRerenderHeader)
    return;

  WContainerWidget *wc;
  if (column != 0)
    wc = dynamic_cast<WContainerWidget *>(headerWidget(column));
  else
    wc = headers_;

  wc->setContentAlignment(alignment);
}

AlignmentFlag WTreeView::headerAlignment(int column) const
{
  return columnInfo(column).headerAlignment;
}

void WTreeView::setColumnBorder(const WColor& color)
{
  delete borderColorRule_;
  borderColorRule_
    = new WCssTextRule
    (".Wt-treeview .Wt-tv-br, "             // header columns 1-n
     ".Wt-treeview .header .Wt-tv-row, "    // header column 0
     ".Wt-treeview .Wt-tv-node .Wt-tv-row .Wt-tv-c, "   // data columns 1-n
     ".Wt-treeview .Wt-tv-node .Wt-tv-row", // data column 0
     "border-color: " + color.cssText());
  WApplication::instance()->styleSheet().addRule(borderColorRule_);
}

void WTreeView::setColumnResizeEnabled(bool enabled)
{
  if (enabled != columnResize_) {
    columnResize_ = enabled;
    scheduleRerender(NeedRerenderHeader);
  }
}

void WTreeView::setAlternatingRowColors(bool enable)
{
  if (alternatingRowColors_ != enable) {
    alternatingRowColors_ = enable;
    setRootNodeStyle();
  }
}

void WTreeView::setRootIsDecorated(bool show)
{
  rootIsDecorated_ = show;
}

void WTreeView::setRootNodeStyle()
{
  if (!rootNode_)
    return;

  if (alternatingRowColors_)
    rootNode_->decorationStyle().setBackgroundImage
      (WApplication::resourcesUrl() + "stripes/stripe-"
       + boost::lexical_cast<std::string>
         (static_cast<int>(rowHeight_.toPixels()))
       + "px.gif");
  else
    rootNode_->decorationStyle().setBackgroundImage("");
}

void WTreeView::setRowHeight(const WLength& rowHeight)
{
  rowHeight_ = rowHeight;

  rowHeightRule_->templateWidget()->resize(WLength::Auto, rowHeight_);
  rowHeightRule_->templateWidget()->setLineHeight(rowHeight_);

  setRootNodeStyle();

  for (NodeMap::const_iterator i = renderedNodes_.begin();
       i != renderedNodes_.end(); ++i)
    i->second->rerenderSpacers();

  if (rootNode_)
    scheduleRerender(NeedAdjustViewPort);
}

void WTreeView::setHeaderHeight(const WLength& height, bool multiLine)
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

  headers_->resize(headers_->width(), headerHeight);
  headerContainer_->resize(headerContainer_->width(), headerHeight);

  if (renderState_ >= NeedRerenderHeader)
    return;

  // XX: cannot do it for column 0!
  if (!WApplication::instance()->environment().agentIsIE())
    for (int i = 1; i < columnCount(); ++i)
      headerTextWidget(i)->setWordWrap(multiLine);
}

int WTreeView::headerLevelCount() const
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

void WTreeView::resize(const WLength& width, const WLength& height)
{
  if (!height.isAuto()) {
    viewportHeight_ = static_cast<int>
      (std::ceil(height.toPixels() / rowHeight_.toPixels()));

    scheduleRerender(NeedAdjustViewPort);
  }

  WLength w
    = WApplication::instance()->environment().ajax() ? WLength::Auto : width;

  contentsContainer_->resize(w, WLength::Auto);
  headerContainer_->resize(w, WLength::Auto);

  WCompositeWidget::resize(width, height);
}

int WTreeView::calcOptimalFirstRenderedRow() const
{
  if (WApplication::instance()->environment().ajax())
    return std::max(0, viewportTop_ - viewportHeight_ - viewportHeight_ / 2);
  else
    return 0;
}

int WTreeView::calcOptimalRenderedRowCount() const
{
  if (WApplication::instance()->environment().ajax())
    return 4 * viewportHeight_;
  else
    return rootNode_->renderedHeight();
}

void WTreeView::setModel(WAbstractItemModel *model)
{
  if (model_) {
    /* disconnect slots from previous model */
    for (unsigned i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();
    modelConnections_.clear();
  }

  model_ = model;

  rootIndex_ = WModelIndex();

  /* connect slots to new model */
  modelConnections_.push_back(model_->columnsInserted().connect
     (SLOT(this, WTreeView::modelColumnsInserted)));
  modelConnections_.push_back(model_->columnsAboutToBeRemoved().connect
     (SLOT(this, WTreeView::modelColumnsAboutToBeRemoved)));
  modelConnections_.push_back(model_->columnsRemoved().connect
     (SLOT(this, WTreeView::modelColumnsRemoved)));
  modelConnections_.push_back(model_->rowsInserted().connect
     (SLOT(this, WTreeView::modelRowsInserted)));
  modelConnections_.push_back(model_->rowsAboutToBeRemoved().connect
     (SLOT(this, WTreeView::modelRowsAboutToBeRemoved)));
  modelConnections_.push_back(model_->rowsRemoved().connect
     (SLOT(this, WTreeView::modelRowsRemoved)));
  modelConnections_.push_back(model_->dataChanged().connect
     (SLOT(this, WTreeView::modelDataChanged)));
  modelConnections_.push_back(model_->headerDataChanged().connect
     (SLOT(this, WTreeView::modelHeaderDataChanged)));
  modelConnections_.push_back(model_->layoutAboutToBeChanged().connect
     (SLOT(this, WTreeView::modelLayoutAboutToBeChanged)));
  modelConnections_.push_back(model_->layoutChanged().connect
     (SLOT(this, WTreeView::modelLayoutChanged)));
  modelConnections_.push_back(model_->modelReset().connect
     (SLOT(this, WTreeView::modelReset)));

  WItemSelectionModel *oldSelectionModel = selectionModel_;
  selectionModel_ = new WItemSelectionModel(model, this);
  selectionModel_->setSelectionBehavior(oldSelectionModel->selectionBehavior());

  expandedSet_.clear();

  for (int i = columns_.size(); i < model_->columnCount(); ++i)
    columnInfo(i);

  while (static_cast<int>(columns_.size()) > model->columnCount()) {
    delete columns_.back().styleRule;
    columns_.erase(columns_.begin() + columns_.size() - 1);
  }

  configureModelDragDrop();

  scheduleRerender(NeedRerender);
  setHeaderHeight(headerLineHeight_, multiLineHeader_);
}

void WTreeView::setRootIndex(const WModelIndex& rootIndex)
{
  if (rootIndex != rootIndex_) {
    rootIndex_ = rootIndex;

    if (model_)
      scheduleRerender(NeedRerenderTree);
  }
}

void WTreeView::setSelectionMode(SelectionMode mode)
{
  if (mode != selectionMode_) {
    clearSelection();
    selectionMode_ = mode;
  }
}

void WTreeView::setSelectionBehavior(SelectionBehavior behavior)
{
  if (behavior != selectionBehavior()) {
    clearSelection();
    selectionModel_->setSelectionBehavior(behavior);
  }
}

SelectionBehavior WTreeView::selectionBehavior() const
{
  return selectionModel_->selectionBehavior();
}

void WTreeView::setItemDelegate(WAbstractItemDelegate *delegate)
{
  itemDelegate_ = delegate;
}

void WTreeView::setItemDelegateForColumn(int column,
					 WAbstractItemDelegate *delegate)
{
  columnInfo(column).itemDelegate_ = delegate;
}

WAbstractItemDelegate *WTreeView::itemDelegateForColumn(int column) const
{
  return columnInfo(column).itemDelegate_;
}

WAbstractItemDelegate *WTreeView::itemDelegate(const WModelIndex& index) const
{
  WAbstractItemDelegate *result = itemDelegateForColumn(index.column());

  return result ? result : itemDelegate_;
}

void WTreeView::render()
{
  while (renderState_ != RenderOk) {
    RenderState s = renderState_;
    renderState_ = RenderOk;

    switch (s) {
    case NeedRerender:
      initLayoutJavaScript();
      rerenderHeader();
      rerenderTree();
      break;
    case NeedRerenderHeader:
      rerenderHeader();
      break;
    case NeedRerenderTree:
      rerenderTree();
      break;
    case NeedAdjustViewPort:
      adjustToViewport();
      break;
    default:
      break;
    }
  }

  if (needDefineJS_)
    initLayoutJavaScript();

  WCompositeWidget::render();
}

void WTreeView::scheduleRerender(RenderState what)
{
  if (!isRendered())
    return;

  if ((what == NeedRerenderHeader && renderState_ == NeedRerenderTree)
      || (what == NeedRerenderTree && renderState_ == NeedRerenderHeader))
    renderState_ = NeedRerender;
  else
    renderState_ = std::max(what, renderState_);

  askRerender();
}

void WTreeView::rerenderHeader()
{
  WApplication *app = WApplication::instance();

  for (int i = 0; i < columnCount(); ++i) {
    WWidget *w = columnInfo(i).extraHeaderWidget;
    if (!w)
      columnInfo(i).extraHeaderWidget = createExtraHeaderWidget(i);
    else
      dynamic_cast<WContainerWidget *>(w->parent())->removeWidget(w);
  }

  headers_->clear();

  WContainerWidget *rowc = new WContainerWidget(headers_);
  rowc->setFloatSide(Right);
  WContainerWidget *row = new WContainerWidget(rowc);
  row->setStyleClass("Wt-tv-row headerrh");

  if (column1Fixed_) {
    row = new WContainerWidget(row);
    row->setStyleClass("Wt-tv-rowc");
  }

  /* sort and resize handles for col 0 */
  if (columnInfo(0).sorting) {
    WText *sortIcon = new WText(rowc);
    sortIcon->setObjectName("sort");
    sortIcon->setInline(false);
    if (!columnResize_)
      sortIcon->setMargin(4, Right);
    if (!app->environment().agentIsIE())
      sortIcon->setMargin(6, Top);
    sortIcon->setFloatSide(Left);
    sortIcon->setStyleClass("Wt-tv-sh Wt-tv-sh-none");
    clickedForSortMapper_->mapConnect(sortIcon->clicked(), 0);
  }

  if (columnResize_) {
    WContainerWidget *resizeHandle = new WContainerWidget(rowc);
    resizeHandle->setStyleClass("Wt-tv-rh headerrh Wt-tv-rhc0");
    resizeHandle->mouseWentDown().connect(resizeHandleMDownJS_);
    resizeHandle->mouseWentUp().connect(resizeHandleMUpJS_);
    resizeHandle->mouseMoved().connect(resizeHandleMMovedJS_);
  }

  /* cols 1.. */
  for (int i = 1; i < columnCount(); ++i) {
    WWidget *w = createHeaderWidget(app, i);
    row->addWidget(w);
  }

  /* col 0 */
  WText *t = new WText("&nbsp;");
  t->setObjectName("text");
  if (columnCount() > 0)
    if (!multiLineHeader_)
      t->setStyleClass(columnStyleClass(0) + " headerrh Wt-label");
    else
      t->setStyleClass(columnStyleClass(0) + " Wt-label");
  t->setInline(false);
  t->setAttributeValue("style", "float: none; margin: 0px auto;"
		       "padding-left: 6px;");

  if (columnInfo(0).extraHeaderWidget) {
    WContainerWidget *c = new WContainerWidget(headers_);
    c->setInline(true); // For IE7
    c->addWidget(t);
    c->addWidget(columnInfo(0).extraHeaderWidget);
  } else
    headers_->addWidget(t);

  if (currentSortColumn_ != -1) {
    SortOrder order = columnInfo(currentSortColumn_).sortOrder;
    headerSortIconWidget(currentSortColumn_)
      ->setStyleClass(order == AscendingOrder
		      ? "Wt-tv-sh Wt-tv-sh-up" : "Wt-tv-sh Wt-tv-sh-down");
  }

  if (model_)
    modelHeaderDataChanged(Horizontal, 0, columnCount() - 1);
}

std::string repeat(const std::string& s, int times)
{
  std::string result;
  for (int i = 0; i < times; ++i) {
    result += s;
  }
  return result;
}

WWidget *WTreeView::createHeaderWidget(WApplication *app, int column)
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
  result->setFloatSide(Left);

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
  result->setStyleClass("Wt-tv-c headerrh " + info.styleClass());
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

void WTreeView::enableAjax()
{
  rootNode_->clicked().connect(itemClickedJS_);
  rootNode_->doubleClicked().connect(itemDoubleClickedJS_);
  if (mouseWentDown_.isConnected() || dragEnabled_)
    rootNode_->mouseWentDown().connect(itemMouseDownJS_);

  WCompositeWidget::enableAjax();
}

void WTreeView::rerenderTree()
{
  WContainerWidget *wrapRoot
    = dynamic_cast<WContainerWidget *>(contents_->widget(0));

  wrapRoot->clear();

  firstRenderedRow_ = calcOptimalFirstRenderedRow();
  validRowCount_ = 0;

  rootNode_ = new WTreeViewNode(this, rootIndex_, -1, true, 0);
  rootNode_->resize(WLength(100, WLength::Percentage), 1);
  rootNode_->setSelectable(false);

  if (WApplication::instance()->environment().ajax()) {
    rootNode_->clicked().connect(itemClickedJS_);
    rootNode_->doubleClicked().connect(itemDoubleClickedJS_);
    if (mouseWentDown_.isConnected() || dragEnabled_)
      rootNode_->mouseWentDown().connect(itemMouseDownJS_);
  }

  setRootNodeStyle();

  wrapRoot->addWidget(rootNode_);

  adjustToViewport();
}

void WTreeView::onViewportChange(WScrollEvent e)
{
  viewportTop_ = static_cast<int>
    (std::floor(e.scrollY() / rowHeight_.toPixels()));

  viewportHeight_ = static_cast<int>
    (std::ceil(e.viewportHeight() / rowHeight_.toPixels()));

  scheduleRerender(NeedAdjustViewPort);
}

void WTreeView::onItemEvent(std::string nodeId, int columnId, std::string type,
			    std::string extra1, std::string extra2,
			    WMouseEvent event)
{
  int column = (columnId == 0 ? 0 : -1);
  for (unsigned i = 0; i < columns_.size(); ++i)
    if (columns_[i].id == columnId) {
      column = i;
      break;
    }

  if (column == -1)
    return; // illegal column Id

  WModelIndex c0index;
  for (NodeMap::const_iterator i = renderedNodes_.begin();
       i != renderedNodes_.end(); ++i) {
    if (i->second->id() == nodeId) {
      c0index = i->second->modelIndex();
      break;
    }
  }

  if (!c0index.isValid()) {
    std::cerr << "Warning (error?): illegal id in WTreeView::onItemEvent()"
	      << std::endl;
    return; // illegal node Id
  }

  WModelIndex index = model_->index(c0index.row(), column, c0index.parent());

  if (type == "clicked") {
    selectionHandleClick(index, event.modifiers());
    clicked_.emit(index, event);
  } else if (type == "dblclicked") {
    doubleClicked_.emit(index, event);
  } else if (type == "mousedown") {
    mouseWentDown_.emit(index, event);
  } else if (type == "drop") {
    WDropEvent e(WApplication::instance()->decodeObject(extra1), extra2, event);
    dropEvent(e, index);
  }
}

void WTreeView::handleClick(const WModelIndex& index)
{
  selectionHandleClick(index, 0);
  clicked_.emit(index, WMouseEvent());
}

void WTreeView::dropEvent(const WDropEvent& e, const WModelIndex& index)
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

void WTreeView::configureModelDragDrop()
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

void WTreeView::setDragEnabled(bool enable)
{
  if (dragEnabled_ != enable) {
    dragEnabled_ = enable;

    if (enable) {
      dragWidget_ = new WText(headerContainer_);
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

void WTreeView::setDropsEnabled(bool enable)
{
  if (dropsEnabled_ != enable) {
    dropsEnabled_ = enable;

    configureModelDragDrop();
  }
}

void WTreeView::checkDragSelection()
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

int WTreeView::subTreeHeight(const WModelIndex& index,
			     int lowerBound, int upperBound)
{
  int result = 0;

  if (index != rootIndex_)
    ++result;

  if (result >= upperBound)
    return result;

  if (isExpanded(index)) {
    int childCount = model_->rowCount(index);

    for (int i = 0; i < childCount; ++i) {
      WModelIndex childIndex = model_->index(i, 0, index);

      result += subTreeHeight(childIndex, upperBound - result);

      if (result >= upperBound)
	return result;
    }
  }

  return result;
}

bool WTreeView::isExpanded(const WModelIndex& index) const
{
  return index == rootIndex_
    || expandedSet_.find(index) != expandedSet_.end();
}

void WTreeView::setCollapsed(const WModelIndex& index)
{
  expandedSet_.erase(index);

  bool selectionHasChanged = false;
  WModelIndexSet& selection = selectionModel_->selection_;

  for (WModelIndexSet::iterator it = selection.lower_bound(index);
       it != selection.end();) {
#ifndef WT_TARGET_JAVA
    WModelIndexSet::iterator n = it;
    ++n;
#endif

    WModelIndex i = *it;
    if (i == index) {
    } else if (isAncestor(i, index)) {
      if (internalSelect(i, Deselect))
	selectionHasChanged = true;
    } else
      break;

#ifndef WT_TARGET_JAVA
    it = n;
#endif
  }

  if (selectionHasChanged)
    selectionChanged_.emit();
}

void WTreeView::setExpanded(const WModelIndex& index, bool expanded)
{
  if (isExpanded(index) != expanded) {
    WWidget *w = widgetForIndex(index);

    WTreeViewNode *node = w ? dynamic_cast<WTreeViewNode *>(w) : 0;
    if (node) {
      if (expanded)
	node->doExpand();
      else
	node->doCollapse();
    } else {
      if (expanded)
	expandedSet_.insert(index);
      else
	setCollapsed(index);

      if (w) {
	RowSpacer *spacer = dynamic_cast<RowSpacer *>(w);

	int height = subTreeHeight(index);
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

void WTreeView::expandToDepth(int depth)
{
  if (depth > 0)
    expandChildrenToDepth(rootIndex_, depth);
}

void WTreeView::expandChildrenToDepth(const WModelIndex& index, int depth)
{
  for (int i = 0; i < model_->rowCount(index); ++i) {
    WModelIndex c = model_->index(i, 0, index);

    expand(c);

    if (depth > 1)
      expandChildrenToDepth(c, depth - 1);
  }
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
    return 0;

  NodeMap::const_iterator i = renderedNodes_.find(index);

  if (i != renderedNodes_.end())
    return i->second;
  else {
    if (!isExpanded(index.parent()))
      return 0;

    WWidget *parent = widgetForIndex(index.parent());
    WTreeViewNode *parentNode = dynamic_cast<WTreeViewNode *>(parent);

    if (parentNode)
      return parentNode->widgetForModelRow(index.row());
    else
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
      columns_.insert(columns_.begin() + i,
		      ColumnInfo(this, app, ++nextColumnId_, i));

    if (renderState_ < NeedRerenderHeader) {
      if (start == 0)
	scheduleRerender(NeedRerenderHeader);
      else {
	double newWidth = 0;
	for (int i = start; i < start + count; ++i)
	  newWidth += columns_[i].width.toPixels() + 7;

	app->doJavaScript(jsRef() + ".adjustHeaderWidth(null, " +
			  (column1Fixed_
			   ? "1"
			   : boost::lexical_cast<std::string>(newWidth))
			  + ");");

	WContainerWidget *row = headerRow();

	for (int i = start; i < start + count; ++i)
	  row->insertWidget(i - 1, createHeaderWidget(app, i));
      }
    }
  }

  if (renderState_ == NeedRerender || renderState_ == NeedRerenderTree)
    return;

  if (start == 0)
    scheduleRerender(NeedRerenderTree);
  else {
    WWidget *parentWidget = widgetForIndex(parent);
    if (parentWidget) {
      WTreeViewNode *node = dynamic_cast<WTreeViewNode *>(parentWidget);
      if (node) {
	for (WTreeViewNode *c = node->nextChildNode(0); c;
	     c = node->nextChildNode(c))
	  c->insertColumns(start, count);
      }
    }
  }
}

void WTreeView::modelColumnsAboutToBeRemoved(const WModelIndex& parent,
					     int start, int end)
{
  int count = end - start + 1;
  if (!parent.isValid()) {
    if (renderState_ < NeedRerenderHeader) {
      WApplication *app = wApp;
      for (int i = start; i < start + count; ++i) {
	std::string c = columns_[i].styleClass();
	if (!column1Fixed_)
	  app->doJavaScript(jsRef() + ".adjustHeaderWidth(null ,"
			    "-WT.pxself(WT.getCssRule('#"
			    + id() + " ." + c + "'), 'width') - 7);");
      }

      if (column1Fixed_)
	app->doJavaScript(jsRef() + ".adjustHeaderWidth(1, 0);");
    }

    columns_.erase(columns_.begin() + start, columns_.begin() + start + count);

    if (renderState_ < NeedRerenderHeader) {
      if (start == 0)
	scheduleRerender(NeedRerenderHeader);
      else {
	for (int i = start; i < start + count; ++i)
	  delete headerWidget(start, false);
      }
    }
  }

  if (start == 0)
    scheduleRerender(NeedRerenderTree);
}

void WTreeView::modelColumnsRemoved(const WModelIndex& parent,
				    int start, int end)
{
  if (renderState_ == NeedRerender || renderState_ == NeedRerenderTree)
    return;

  int count = end - start + 1;

  if (start != 0) {
    WWidget *parentWidget = widgetForIndex(parent);
    if (parentWidget) {
      WTreeViewNode *node = dynamic_cast<WTreeViewNode *>(parentWidget);
      if (node) {
	for (WTreeViewNode *c = node->nextChildNode(0); c;
	     c = node->nextChildNode(c))
	  c->removeColumns(start, count);
      }
    }
  }
}

void WTreeView::modelRowsInserted(const WModelIndex& parent,
				  int start, int end)
{
  int count = end - start + 1;
  shiftModelIndexes(parent, start, count);

  if (renderState_ == NeedRerender || renderState_ == NeedRerenderTree)
    return;

  WWidget *parentWidget = widgetForIndex(parent);

  if (parentWidget) {
    WTreeViewNode *parentNode = dynamic_cast<WTreeViewNode *>(parentWidget);

    if (parentNode) {
      if (parentNode->childrenLoaded()) {
	WWidget *startWidget = 0;

	if (end < model_->rowCount(parent) - 1)
	  startWidget = parentNode->widgetForModelRow(start);
	else if (parentNode->bottomSpacerHeight() != 0)
	  startWidget = parentNode->bottomSpacer();

	parentNode->adjustChildrenHeight(count);
	parentNode->shiftModelIndexes(start, count);

	if (startWidget && startWidget == parentNode->topSpacer()) {
	  parentNode->addTopSpacerHeight(count);
	  renderedRowsChanged(renderedRow(model_->index(start, 0, parent),
					  parentNode->topSpacer(),
					  renderLowerBound(),
					  renderUpperBound()),
			      count);

	} else if (startWidget && startWidget == parentNode->bottomSpacer()) {
	  parentNode->addBottomSpacerHeight(count);
	  renderedRowsChanged(renderedRow(model_->index(start, 0, parent),
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

	  int parentRowCount = model_->rowCount(parent);

	  int nodesToAdd = std::min(count, maxRenderHeight);

	  WTreeViewNode *first = 0;
	  for (int i = 0; i < nodesToAdd; ++i) {
	    WTreeViewNode *n
	      = new WTreeViewNode(this, model_->index(start + i, 0, parent), -1,
				  start + i == parentRowCount - 1,
				  parentNode);
	    parentNode->childContainer()->insertWidget(containerIndex + i, n);

	    ++validRowCount_;

	    if (!first)
	      first = n;
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
	      validRowCount_ -= n->renderedHeight();

	      delete n;
	    }

	    if (extraBottomSpacer)
	      parentNode->addBottomSpacerHeight(extraBottomSpacer);

	    parentNode->normalizeSpacers();
	  }

	  if (first)
	    renderedRowsChanged(first->renderedRow(renderLowerBound(),
						   renderUpperBound()),
				nodesToAdd);

	  // Update graphics if the last node has changed, i.e. if we are
	  // adding rows at the back
	  if (end == model_->rowCount(parent) - 1 && start >= 1) {
	    WTreeViewNode *n = dynamic_cast<WTreeViewNode *>
	      (parentNode->widgetForModelRow(start - 1));

	    if (n)
	      n->updateGraphics(false,
				model_->rowCount(n->modelIndex()) == 0);
	  }
	}
      } /* else:
	   children not loaded -- so we do not need to bother
	 */

      // Update graphics for parent when first children have een added
      if (model_->rowCount(parent) == count)
	parentNode->updateGraphics(parentNode->isLast(), false);
    } else {
      RowSpacer *s = dynamic_cast<RowSpacer *>(parentWidget);

      s->setRows(s->rows() + count);
      s->node()->adjustChildrenHeight(count);

      renderedRowsChanged(renderedRow(model_->index(start, 0, parent), s,
				      renderLowerBound(), renderUpperBound()),
			  count);
    }
  } /* else:
       parentWidget is 0: it means it is not even part of any spacer.
     */
}

void WTreeView::modelRowsAboutToBeRemoved(const WModelIndex& parent,
					  int start, int end)
{
  int count = end - start + 1;

  if (renderState_ != NeedRerender || renderState_ != NeedRerenderTree) {
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
	      WModelIndex childIndex = model_->index(i, 0, parent);

	      if (i == start)
		firstRemovedRow_ = renderedRow(childIndex, w);

	      int childHeight = subTreeHeight(childIndex);
	      removedHeight_ += childHeight;

	      s->setRows(s->rows() - childHeight);
	    } else {
	      WTreeViewNode *node = dynamic_cast<WTreeViewNode *>(w);

	      if (i == start)
		firstRemovedRow_ = node->renderedRow();

	      removedHeight_ += node->renderedHeight();
	      delete w;
	    }
	  }

	  parentNode->normalizeSpacers();

	  parentNode->adjustChildrenHeight(-removedHeight_);
	  parentNode->shiftModelIndexes(start, -count);

	  // Update graphics for last node in parent, if we are removing rows
	  // at the back
	  if (end == model_->rowCount(parent) - 1 && start >= 1) {
	    WTreeViewNode *n = dynamic_cast<WTreeViewNode *>
	      (parentNode->widgetForModelRow(start - 1));

	    if (n)
	      n->updateGraphics(true, model_->rowCount(n->modelIndex()) == 0);
	  }
	} /* else:
	     children not loaded -- so we do not need to bother
	  */

	// Update graphics for parent when all rows have been removed
	if (model_->rowCount(parent) == count)
	  parentNode->updateGraphics(parentNode->isLast(), true);
      } else {
	RowSpacer *s = dynamic_cast<RowSpacer *>(parentWidget);

	for (int i = start; i <= end; ++i) {
	  WModelIndex childIndex = model_->index(i, 0, parent);
	  int childHeight = subTreeHeight(childIndex);
	  removedHeight_ += childHeight;

	  if (i == start)
	    firstRemovedRow_ = renderedRow(childIndex, s);
	}

	s->setRows(s->rows() - removedHeight_);
	s->node()->adjustChildrenHeight(-removedHeight_);
      }
    } /* else:
       parentWidget is 0: it means it is not even part of any spacer.
     */
  }

  shiftModelIndexes(parent, start, -count);
}

void WTreeView::modelRowsRemoved(const WModelIndex& parent,
				 int start, int end)
{
  renderedRowsChanged(firstRemovedRow_, -removedHeight_);
}

void WTreeView::modelDataChanged(const WModelIndex& topLeft,
				 const WModelIndex& bottomRight)
{
  if (renderState_ == NeedRerender || renderState_ == NeedRerenderTree)
    return;
  
  WModelIndex parent = topLeft.parent();  
  WWidget *parentWidget = widgetForIndex(parent);

  if (parentWidget) {
    WTreeViewNode *parentNode = dynamic_cast<WTreeViewNode *>(parentWidget);

    if (parentNode) {
      if (parentNode->childrenLoaded()) {
	for (int r = topLeft.row(); r <= bottomRight.row(); ++r) {
	  WModelIndex index = model_->index(r, 0, parent);

	  WTreeViewNode *n
	    = dynamic_cast<WTreeViewNode *>(widgetForIndex(index));

	  if (n)
	    n->update(topLeft.column(), bottomRight.column());
	}
      } /* else:
	   children not loaded -- so we do not need to bother
	 */
    } /* else:
	 parentWidget is a spacer -- we do not need to bother
       */
  } /* else:
       parent is not displayed
    */
}

void WTreeView::modelHeaderDataChanged(Orientation orientation,
				       int start, int end)
{
  if (renderState_ < NeedRerenderHeader) {
    if (orientation == Horizontal) {
      for (int i = start; i <= end; ++i) {
	WString label = asString(model_->headerData(i));
	headerTextWidget(i)->setText(label);
      }
    }
  }
}

WText *WTreeView::headerSortIconWidget(int column)
{
  if (!columnInfo(column).sorting)
    return 0;

  if (column == 0) {
    WContainerWidget *row
      = dynamic_cast<WContainerWidget *>(headers_->widget(0));

    return dynamic_cast<WText *>(row->widget(1));
  } else
    return dynamic_cast<WText *>(headerWidget(column)->find("sort"));
}

WText *WTreeView::headerTextWidget(int column)
{
  return dynamic_cast<WText *>(headerWidget(column)->find("text"));
}

WWidget *WTreeView::headerWidget(int column, bool contentsOnly)
{
  WWidget *result = 0;

  if (column == 0)
    result = headers_->widget(headers_->count() - 1);
  else
    result = headerRow()->widget(column - 1);

  if (contentsOnly && column != 0)
    return result->find("contents");
  else
    return result;
}

WWidget *WTreeView::createExtraHeaderWidget(int column)
{
  return 0;
}

WWidget *WTreeView::extraHeaderWidget(int column)
{
  return columnInfo(column).extraHeaderWidget;
}

WContainerWidget *WTreeView::headerRow()
{
  WContainerWidget *row
    = dynamic_cast<WContainerWidget *>(headers_->widget(0));
  row = dynamic_cast<WContainerWidget *>(row->widget(0));
  if (column1Fixed_)
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
			   int lowerBound, int upperBound)
{
  if (child == ancestor)
    return 0;
  else {
    WModelIndex parent = child.parent();

    int result = 0;
    for (int r = 0; r < child.row(); ++r) {
      result += subTreeHeight(model_->index(r, 0, parent), 0,
			      upperBound - result);
      if (result >= upperBound)
	return result;
    }

    return result + getIndexRow(parent, ancestor,
				lowerBound - result, upperBound - result);
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

  scheduleRerender(NeedAdjustViewPort);
}

void WTreeView::adjustToViewport(WTreeViewNode *changed)
{
  firstRenderedRow_ = std::max(0, firstRenderedRow_);
  validRowCount_
    = std::max(0, std::min(validRowCount_,
			   rootNode_->renderedHeight() - firstRenderedRow_));

  int viewportBottom = viewportTop_ + viewportHeight_;
  int lastValidRow = firstRenderedRow_ + validRowCount_;

  bool renderMore =
       (viewportTop_ > firstRenderedRow_ - viewportHeight_)
    || (viewportBottom < lastValidRow + viewportHeight_);

  bool pruneFirst = false;

  if (renderMore) {
    int newFirstRenderedRow = std::min(firstRenderedRow_,
				       calcOptimalFirstRenderedRow());
    int newLastValidRow = std::max(lastValidRow,
				   std::min(rootNode_->renderedHeight(),
					    calcOptimalFirstRenderedRow()
					    + calcOptimalRenderedRowCount()));

    int newValidRowCount = newLastValidRow - newFirstRenderedRow;

    int newRows = std::max(0, firstRenderedRow_ - newFirstRenderedRow)
      + std::max(0, newLastValidRow - lastValidRow);

    if (nodeLoad_ + newRows > 9 * viewportHeight_) {
      pruneFirst = true;
    } else
      if (newFirstRenderedRow < firstRenderedRow_
	  || newLastValidRow > lastValidRow) {
	firstRenderedRow_ = newFirstRenderedRow;
	validRowCount_ = newValidRowCount;
	adjustRenderedNode(rootNode_, 0);
      }
  }

  if (pruneFirst || nodeLoad_ > 5 * viewportHeight_) {
    firstRenderedRow_ = calcOptimalFirstRenderedRow();
    validRowCount_ = calcOptimalRenderedRowCount();

    if (WApplication::instance()->environment().ajax())
      pruneNodes(rootNode_, 0);

    if (pruneFirst && nodeLoad_ < calcOptimalRenderedRowCount()) {
      adjustRenderedNode(rootNode_, 0);
    } 
  }

  if (column1Fixed_)
    tieRowsScrollJS_.exec(scrollBarC_->jsRef());
}

int WTreeView::adjustRenderedNode(WTreeViewNode *node, int theNodeRow)
{
  WModelIndex index = node->modelIndex();

  if (index != rootIndex_)
    ++theNodeRow;

  if (!isExpanded(index) && !node->childrenLoaded())
    return theNodeRow;

  int nodeRow = theNodeRow;

  if (node->isAllSpacer()) {
    if (nodeRow + node->childrenHeight() > firstRenderedRow_
	&& nodeRow < firstRenderedRow_ + validRowCount_) {
      // replace spacer by some nodes
      int childCount = model_->rowCount(index);

      bool firstNode = true;
      int rowStubs = 0;

      for (int i = 0; i < childCount; ++i) {
	WModelIndex childIndex = model_->index(i, 0, index);

	int childHeight = subTreeHeight(childIndex);

	if (nodeRow <= firstRenderedRow_ + validRowCount_
	    && nodeRow + childHeight > firstRenderedRow_) {
	  if (firstNode) {
	    firstNode = false;
	    node->setTopSpacerHeight(rowStubs);
	    rowStubs = 0;
	  }

	  WTreeViewNode *n = new WTreeViewNode(this, childIndex,
					       childHeight - 1,
					       i == childCount - 1, node);
	  node->childContainer()->addWidget(n);

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
    WTreeViewNode *child = node->nextChildNode(0);

    int childCount = model_->rowCount(index);
    while (topSpacerHeight != 0
	   && nodeRow + topSpacerHeight > firstRenderedRow_) {
      // eat from top spacer and replace with actual nodes
      WTreeViewNode *n
	= dynamic_cast<WTreeViewNode *>(node->childContainer()->widget(1));

      assert(n);

      WModelIndex childIndex
	= model_->index(n->modelIndex().row() - 1, 0, index);

      int childHeight = subTreeHeight(childIndex);

      n = new WTreeViewNode(this, childIndex, childHeight - 1,
			    childIndex.row() == childCount - 1, node);
      node->childContainer()->insertWidget(1, n);

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
	= model_->index(n->modelIndex().row() + 1, 0, index);

      int childHeight = subTreeHeight(childIndex);

      n = new WTreeViewNode(this, childIndex, childHeight - 1,
			    childIndex.row() == childCount - 1, node);
      node->childContainer()->insertWidget(lastNodeIndex + 1, n);

      nestedNodeRow = nodeRow + bottomSpacerStart;
      nestedNodeRow = adjustRenderedNode(n, nestedNodeRow);
      assert(nestedNodeRow == nodeRow + bottomSpacerStart + childHeight);

      node->addBottomSpacerHeight(-childHeight);
      bottomSpacerStart += childHeight;
    }

    nodeRow += nch;
  }

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
    WTreeViewNode *c = 0;

    for (; nodeRow < firstRenderedRow_; ) {
      c = node->nextChildNode(0);
      if (!c) {
	done = true;
	break;
      }

      if (nodeRow + c->renderedHeight() < firstRenderedRow_) {
	node->addTopSpacerHeight(c->renderedHeight());
	nodeRow += c->renderedHeight();
	delete c;
	c = 0;
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

      if (c != 0) {
	int i = node->childContainer()->indexOf(c);
	int prunedHeight = 0;

	while (c && i < node->childContainer()->count()) {
	  c = dynamic_cast<WTreeViewNode *> (node->childContainer()->widget(i));
	  if (c) {
	    prunedHeight += c->renderedHeight();
	    delete c;
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
	WTreeViewNode *c = node->nextChildNode(0);
	if (!c)
	  break;

	prunedHeight += c->renderedHeight();
	delete c;
      }

      node->addBottomSpacerHeight(prunedHeight);
      node->normalizeSpacers();
    }

  return nodeRow;
}

int WTreeView::shiftModelIndexes(const WModelIndex& parent,
				 int start, int count,
				 WAbstractItemModel *model,
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
    if (p != parent && !isAncestor(p, parent))
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

void WTreeView::shiftModelIndexes(const WModelIndex& parent,
				  int start, int count)
{
  shiftModelIndexes(parent, start, count, model_, expandedSet_);

  int removed = shiftModelIndexes(parent, start, count, model_,
				  selectionModel_->selection_);

  if (removed)
    selectionChanged_.emit();
}

WTreeView::ColumnInfo& WTreeView::columnInfo(int column) const
{
  while (column >= (int)columns_.size())
    columns_.push_back(ColumnInfo(this, WApplication::instance(),
				  ++nextColumnId_,
				  column));

  return columns_[column];
}

void WTreeView::collapseColumn(int columnid)
{
  model_->collapseColumn(columnById(columnid));
  scheduleRerender(NeedRerenderHeader);
  setHeaderHeight(headerLineHeight_, multiLineHeader_);
}

void WTreeView::expandColumn(int columnid)
{
  model_->expandColumn(columnById(columnid));
  scheduleRerender(NeedRerenderHeader);
  setHeaderHeight(headerLineHeight_, multiLineHeader_);
}

void WTreeView::toggleSortColumn(int columnid)
{
  int column = columnById(columnid);

  if (column != currentSortColumn_)
    sortByColumn(column, columnInfo(column).sortOrder);
  else
    sortByColumn(column, columnInfo(column).sortOrder == AscendingOrder
		 ? DescendingOrder : AscendingOrder);
}

int WTreeView::columnById(int columnid) const
{
  for (int i = 0; i < columnCount(); ++i)
    if (columnInfo(i).id == columnid)
      return i;

  return 0;
}

void WTreeView::sortByColumn(int column, SortOrder order)
{
  if (currentSortColumn_ != -1)
    headerSortIconWidget(currentSortColumn_)
      ->setStyleClass("Wt-tv-sh Wt-tv-sh-none");

  currentSortColumn_ = column;
  columnInfo(column).sortOrder = order;

  if (renderState_ != NeedRerender)
    headerSortIconWidget(currentSortColumn_)
      ->setStyleClass(order == AscendingOrder
		      ? "Wt-tv-sh Wt-tv-sh-up" : "Wt-tv-sh Wt-tv-sh-down");

  model_->sort(column, order);
}

void WTreeView::setSortingEnabled(bool enabled)
{
  sorting_ = enabled;
  for (int i = 0; i < columnCount(); ++i)
    columnInfo(i).sorting = enabled;

  scheduleRerender(NeedRerenderHeader);
}

void WTreeView::setSortingEnabled(int column, bool enabled)
{
  columnInfo(column).sorting = enabled;

  scheduleRerender(NeedRerenderHeader);
}

void WTreeView::convertToRaw(WModelIndexSet& set, std::vector<void *>& result)
{
  for (WModelIndexSet::const_iterator i = set.begin(); i != set.end(); ++i) {
    void *rawIndex = model_->toRawIndex(*i);
    if (rawIndex)
      result.push_back(rawIndex);
  }

  set.clear();
}

void WTreeView::modelLayoutAboutToBeChanged()
{
  convertToRaw(expandedSet_, expandedRaw_);
  convertToRaw(selectionModel_->selection_, selectionRaw_);
  rawRootIndex_ = model_->toRawIndex(rootIndex_);
}

void WTreeView::modelLayoutChanged()
{
  if (rawRootIndex_)
    rootIndex_ = model_->fromRawIndex(rawRootIndex_);
  else
    rootIndex_ = WModelIndex();

  for (unsigned i = 0; i < expandedRaw_.size(); ++i)
    expandedSet_.insert(model_->fromRawIndex(expandedRaw_[i]));
  expandedRaw_.clear();

  for (unsigned i = 0; i < selectionRaw_.size(); ++i) {
    WModelIndex index = model_->fromRawIndex(selectionRaw_[i]);
    if (index.isValid())
      selectionModel_->selection_.insert(index);
  }
  selectionRaw_.clear();

  renderedNodes_.clear();

  scheduleRerender(NeedRerenderTree);
}

void WTreeView::modelReset()
{
  setModel(model_);
}

void WTreeView::addRenderedNode(WTreeViewNode *node)
{
  renderedNodes_[node->modelIndex()] = node;
  ++nodeLoad_;
}

void WTreeView::removeRenderedNode(WTreeViewNode *node)
{
  renderedNodes_.erase(node->modelIndex());
  --nodeLoad_;
}

void WTreeView::clearSelection()
{
  WModelIndexSet& nodes = selectionModel_->selection_;

  while (!nodes.empty()) {
    WModelIndex i = *nodes.begin();
    internalSelect(i, Deselect);
  }
}

void WTreeView::setSelectedIndexes(const WModelIndexSet& indexes)
{
  if (indexes.empty() && selectionModel_->selection_.empty())
    return;

  clearSelection();

  for (WModelIndexSet::const_iterator i = indexes.begin();
       i != indexes.end(); ++i)
    internalSelect(*i, Select);

  selectionChanged_.emit();
}

WModelIndexSet WTreeView::selectedIndexes() const
{
  return selectionModel_->selection_;
}

bool WTreeView::isSelected(const WModelIndex& index) const
{
  return selectionModel_->isSelected(index);
}

void WTreeView::select(const WModelIndex& index, SelectionFlag option)
{
  if (internalSelect(index, option))
    selectionChanged_.emit();
}

bool WTreeView::internalSelect(const WModelIndex& index, SelectionFlag option)
{
  if (selectionBehavior() == SelectRows && index.column() != 0)
    return internalSelect(model_->index(index.row(), 0, index.parent()),
			  option);

  if (!(index.flags() & ItemIsSelectable)
      || selectionMode_ == NoSelection)
    return false;

  if (option == ToggleSelect)
    option = isSelected(index) ? Deselect : Select;
  else if (option == ClearAndSelect) {
    clearSelection();
    option = Select;
  } else if (selectionMode_ == SingleSelection && option == Select)
    clearSelection();

  /*
   * now option is either Select or Deselect and we only need to do
   * exactly that one thing
   */
  if (option == Select)
    selectionModel_->selection_.insert(index);
  else
    if (!selectionModel_->selection_.erase(index))
      return false;

  WModelIndex column0Index = model_->index(index.row(), 0, index.parent());

  WWidget *w = widgetForIndex(column0Index);
  WTreeViewNode *node = dynamic_cast<WTreeViewNode *>(w);

  if (node)
    node->renderSelected(option == Select, index.column());

  return true;
}

void WTreeView::selectRange(const WModelIndex& first, const WModelIndex& last)
{
  clearSelection();

  WModelIndex index = first;
  for (;;) {
    for (int c = first.column(); c <= last.column(); ++c) {
      WModelIndex ic = model_->index(index.row(), c, index.parent());
      internalSelect(ic, Select);

      if (ic == last)
	return;
    }

    WModelIndex indexc0
      = index.column() == 0 ? index
      : model_->index(index.row(), 0, index.parent());

    if (isExpanded(indexc0) && model_->rowCount(indexc0) > 0)
      index = model_->index(0, first.column(), indexc0);
    else {
      for (;;) {
	// next row in parent, if one is available
	WModelIndex parent = index.parent();
	if (index.row() + 1 < model_->rowCount(parent)) {
	  index = model_->index(index.row() + 1, first.column(), parent);
	  break;
	} else
	  // otherwise go up one level
	  index = index.parent();
      }
    }
  }
}

void WTreeView::extendSelection(const WModelIndex& index)
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
     * Only indexs with expanded ancestors can be
     * part of the selection: this is asserted when collapsing a index.
     */
    WModelIndex top = Utils::first(selectionModel_->selection_);
    if (top < index)
      selectRange(top, index);
    else {
      WModelIndex bottom = Utils::last(selectionModel_->selection_);
      selectRange(index, bottom);
    }
  }

  selectionChanged_.emit();
}

void WTreeView::selectionHandleClick(const WModelIndex& index,
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

}

#endif // DOXYGEN_ONLY
