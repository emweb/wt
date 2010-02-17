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

    // FIXME: handle non-JavaScript case using a WSignalMapper ?
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
    resize(Wt::WLength::Auto, 0);
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

  WWidget *widget(int column);
  void setWidget(int column, WWidget *w);
  void addColumnStyleClass(int column, WWidget *w);
};

void RowSpacer::setRows(int height, bool force)
{
  if (height == 0)
    delete this;
  else
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

  if (index_ != view_->rootIndex() && !view->isExpanded(index_))
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
    elementAt(0, 1)->setStyleClass("c1 rh");

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

  int thisNodeCount = view_->model()->columnCount(parent);

  for (int i = firstColumn; i <= lastColumn; ++i) {
 
    WModelIndex child = i < thisNodeCount ? childIndex(i) : WModelIndex();

    WWidget *currentW = widget(i);

    WFlags<ViewItemRenderFlag> renderFlags = 0;
    if (view_->selectionBehavior() == SelectItems && view_->isSelected(child))
      renderFlags |= RenderSelected;

    WWidget *newW = view_->itemDelegate(i)->update(currentW, child,
						   renderFlags);

    if (newW != currentW)
      setWidget(i, newW);
    else
      addColumnStyleClass(i, currentW);
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
  tc->clear();

  if (view_->columnCount() > 1) {
    WContainerWidget *row = new WContainerWidget();

    if (view_->column1Fixed_) {
      row->setStyleClass("Wt-tv-rowc rh");
      WContainerWidget *rowWrap = new WContainerWidget();
      rowWrap->addWidget(row);
      row = rowWrap;
    }

    row->setObjectName("row");
    row->setStyleClass("Wt-tv-row rh");
    tc->insertWidget(0, row);
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

void WTreeViewNode::addColumnStyleClass(int column, WWidget *w)
{
  EscapeOStream s;

  s << view_->columnStyleClass(column) << " Wt-tv-c rh "
    << w->styleClass().toUTF8();

  w->setStyleClass(WString::fromUTF8(s.c_str()));
}

void WTreeViewNode::setWidget(int column, WWidget *newW)
{
  WTableCell *tc = elementAt(0, 1);

  WWidget *current = widget(column);

  addColumnStyleClass(column, newW);

  if (current)
    current->setStyleClass(WString::Empty);

  if (column == 0) {
    if (current)
      tc->removeWidget(current);

    newW->setInline(false);
    tc->addWidget(newW);
  } else {
    WContainerWidget *row = dynamic_cast<WContainerWidget *>(tc->widget(0));
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

  if (column == 0) {
    if (tc->count() > 0) {
      WWidget *result = tc->widget(tc->count() - 1);
      return (tc->count() > 1 || result->objectName() != "row") ? result : 0;
    } else
      return 0;
  } else {
    WContainerWidget *row = dynamic_cast<WContainerWidget *>(tc->widget(0));

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
  return index_ == view_->rootIndex() || !rowAt(1)->isHidden();
}

void WTreeViewNode::normalizeSpacers()
{
  if (childrenLoaded_ && childContainer()->count() == 2) {
    RowSpacer *top = topSpacer();
    RowSpacer *bottom = bottomSpacer();

    if (top && bottom && top != bottom) {
      top->setRows(top->rows() + bottom->rows());
      delete bottom;
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
	view_->itemDelegate(j)->updateModelIndex(n->widget(j), child);
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

WTreeView::WTreeView(WContainerWidget *parent)
  : WAbstractItemView(parent),
    rootNode_(0),
    borderColorRule_(0),
    rootIsDecorated_(true),
    column1Fixed_(false),
    alternatingRowColors_(false),
    collapsed_(this),
    expanded_(this),
    viewportTop_(0),
    viewportHeight_(30),
    nodeLoad_(0),
    contentsContainer_(0),
    scrollBarC_(0),
    itemEvent_(this, "itemEvent")
{
  setImplementation(impl_ = new WContainerWidget());

  renderState_ = NeedRerender;

  WApplication *app = WApplication::instance();

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
  setSelectable(false);

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
       "overflow: hidden;"
       "width: 100%;", CSS_RULES_NAME);

    if (app->environment().agentIsIE())
      app->styleSheet().addRule
	(".Wt-treeview .Wt-header .Wt-label",
	 "zoom: 1;");

    app->styleSheet().addRule
      (".Wt-treeview table", "width: 100%");

    app->styleSheet().addRule
      (".Wt-treeview .c1", "width: 100%; overflow: hidden;");

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
       "width: 0px; margin: 0px;");

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

    /* borders: needed here for IE */
    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-br, "                      // header
       ".Wt-treeview .Wt-tv-node .Wt-tv-row .Wt-tv-c", // data
       "border-right: 1px solid white;");

    /* sort handles */
    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-sh", std::string() +
       "float: right; width: 16px; height: 10px; padding-bottom: 6px;"
       "cursor: pointer; cursor:hand;");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-sh-nrh", std::string() + 
       "float: right; width: 16px; height: 10px;"
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
	 "position: absolute; overflow-x: auto;"
	 "height: " SCROLLBAR_WIDTH_TEXT "px;");
    else
      app->styleSheet().addRule
	(".Wt-treeview .Wt-scroll", "overflow: auto;"
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
     ""    "if (t.className.indexOf('Wt-tv-c') == 0)"
     ""      "columnId = t.className.split(' ')[0].substring(7) * 1;"
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

  itemMouseUpJS_.setJavaScript
    ("function(obj, event) {"
     """var APP=" + app->javaScriptClass() +", tv=" + jsRef() + ";"
     """var item=APP.getItem(event);"
     """if (item.columnId != -1) {"
     "" + itemEvent_.createEventCall("item.el", "event", "item.nodeId",
				     "item.columnId", "'mouseup'",
				     "''", "''") + ";"
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
     ""    "hh=h.firstChild,"
     ""    "h0=h.lastChild,"
     ""    "c0id = h0.className.split(' ')[0],"
     ""    "c0r = WT.getCssRule('#" + id() + " .' + c0id);"
     ""
     """if (lastx != null && lastx != '') {"
     ""  "var nowxy = WT.pageCoordinates(event),"
     ""  "parent = obj.parentNode.parentNode,"
     ""      "diffx = Math.max(nowxy.x - lastx, -parent.offsetWidth),"
     ""      "c = parent.className.split(' ')[0];"
     ""
     ""  "if (c) {"
     ""    "var r = WT.getCssRule('#" + id() + " .' + c),"
     ""        "tw = WT.pxself(r, 'width');"
     ""    "r.style.width = Math.max(0, tw + diffx) + 'px';"
     ""  "}"
     ""  "var s = " + jsRef() + ";"
     ""  "s.adjustColumns();"
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
       "obj.parentNode.style.width = "
       "" WT_CLASS ".getCssRule('#" + id() + " .cwidth').style.width;"
       "$('#" + id() + " .Wt-tv-rowc').parent().scrollLeft(obj.scrollLeft);"
       "}");
  }

  /*
   * This adjusts invariants that depend on the size of the whole
   * treeview:
   * 
   *  - changes to the total width (tw)
   *  - whether scrollbars are needed (vscroll), and thus the actual
   *    contents width
   *  - when column1 is fixed:
   *    * .row width
   *    * table parent width
   */
  std::string extra
    = app->environment().agent() == WEnvironment::IE6 ? "10" : "8";

  app->addAutoJavaScript
    ("{var e=$('#" + contentsContainer_->id() + "').get(0);"
     "var $s=$('#" + id() + "');"
     "var WT=" WT_CLASS ";"
     "if (e) {"
     """var tw=$s.innerWidth(),"
     ""    "vscroll=e.scrollHeight > e.offsetHeight,"
     ""    "c0id, c0w = null;" // for column 1 fixed
     ""
     """if ($s.hasClass('column1')) {"
     ""  "c0id = $('#" + id() + " .Wt-headerdiv').get(0).lastChild"
     ""    ".className.split(' ')[0];"
     ""  "c0w = WT.pxself(WT.getCssRule('#" + id() + " .' + c0id), 'width');"
     """}"
     ""
     """if (tw > 200 " // XXX: IE's incremental rendering foobars completely
     ""    "&& (tw != e.tw || "
     ""        "vscroll != e.vscroll || "
     ""        "c0w != e.c0w || "
     ""        "$s.get(0).changed)) {"
     ""  "e.tw = tw;"
     ""  "e.vscroll = vscroll;"
     ""  "e.c0w = c0w;"
     ""  "var h= " + headers_->jsRef() + ","
     ""      "t=" + contents_->jsRef() + ".firstChild,"
     ""      "r= WT.getCssRule('#" + id() + " .cwidth'),"
     ""      "contentstoo=(r.style.width == h.style.width);"
     ""  "r.style.width=(tw - (vscroll ? " SCROLLBAR_WIDTH_TEXT " : 0)) + 'px';"
     ""  "e.style.width=tw + 'px';"
     ""  "h.style.width=t.offsetWidth + 'px';"
     ""  "if (c0w != null) {"
     ""    "var w=tw - c0w - " + extra + " - (vscroll ? "
     ""                                       SCROLLBAR_WIDTH_TEXT " : 0);"
     ""    "if (w > 0) {"
     ""      "w2 = Math.min(w, "
     ""       "WT.pxself(WT.getCssRule('#" + id() + " .Wt-tv-rowc'), 'width'));"
     ""      "tw -= (w - w2);"
     ""      "WT.getCssRule('#" + id() + " .Wt-tv-row').style.width = w2+'px';"
     ""      "$('#" + id() + " .Wt-tv-row').css('width', w2 + 'px').css('width', '');"
     ""      "tw -= (vscroll ? " SCROLLBAR_WIDTH_TEXT " : 0);"
     ""      "h.style.width=tw + 'px';"
     ""      "t.style.width=tw + 'px';"
     ""    "}"
     ""  "} else if (contentstoo) {"
     ""    "h.style.width=r.style.width;"
     ""    "t.style.width=r.style.width;"
     ""  "}"
     ""  "if (!$s.get(0).changed && $s.get(0).adjustColumns)"
     ""    "$s.get(0).adjustColumns();"
     ""  "$s.get(0).changed = false;"
     """}"
     "}}"
     );

  if (parent)
    parent->addWidget(this);
}

void WTreeView::refresh()
{
  needDefineJS_ = false;

  WApplication *app = WApplication::instance();

  /*
   * this adjusts invariants that take into account column resizes
   *
   * c0w is set as soon as possible.
   *
   *  if (!column1 fixed):
   *    1) width('Wt-headerdiv') = sum(column widths)
   *    2) width('float: right') = sum(column(-1) widths)
   *    3) width(table parent) = width('Wt-headerdiv')
   *  else
   *    4) width('Wt-rowc') = sum(column(-1) widths) 
   */
  std::string extra
    = app->environment().agent() == WEnvironment::IE6 ? "10" : "8";

  std::string columnsWidth =
    "var e=" + jsRef() + ","
    ""  "WT=" WT_CLASS ","
    ""  "t=" + contents_->jsRef() + ".firstChild,"       // table parent
    ""  "h=" + headers_->jsRef() + ","                   // Wt-headerdiv
    ""  "hc=h.firstChild"                                // Wt-tv-row
    ""      + (column1Fixed_ ? ".firstChild" : "") + "," // or Wt-tv-rowc
    ""  "allw_1=0, allw=0,"
    ""  "c0id = h.lastChild.className.split(' ')[0],"
    ""  "c0r = WT.getCssRule('#" + id() + " .' + c0id);"

    "if (WT.isHidden(e) || h.offsetWidth - hc.offsetWidth < 8) return;"

    "for (var i=0, length=hc.childNodes.length; i < length; ++i) {\n"
    """if (hc.childNodes[i].className) {\n" // IE may have only a text node
    ""  "var cl = hc.childNodes[i].className.split(' ')[0],\n"
    ""      "r = WT.getCssRule('#" + id() + " .' + cl);\n"
         // 7 = 2 * 3px (padding) + 1px border
    ""  "allw_1 += WT.pxself(r, 'width') + 7;\n"
    """}"
    "}\n"

    "if (!c0r.style.width) " // first resize and c0 width not set
    """c0r.style.width = (h.offsetWidth - hc.offsetWidth - 8) + 'px';"

    "allw = allw_1 + WT.pxself(c0r, 'width') + " + extra + ";\n";

  if (!column1Fixed_)
    columnsWidth +=
      "h.style.width = t.style.width = allw + 'px';"
      "hc.style.width = allw_1 + 'px';";
  else
    columnsWidth +=
      "var r = WT.getCssRule('#" + id() + " .Wt-tv-rowc');"
      "r.style.width = allw_1 + 'px';"
      "$('#" + id() + " .Wt-tv-rowc')"
      """.css('width', allw_1 + 'px')"
      """.css('width', '');"
      "e.changed=true;"
      + app->javaScriptClass() + "._p_.autoJavaScript();";

  /*
   * Adjust columns: do everything needed when a column is resized,
   * or columns changes somehow.
   */
  app->doJavaScript
    (jsRef() + ".adjustColumns=function() {"
     """if (" + contentsContainer_->jsRef() + ") {" + columnsWidth + "}"
     "};");

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

    app->addAutoJavaScript
      ("{var s=" + scrollBarC_->jsRef() + ";"
       """if (s) {" + tieRowsScrollJS_.execJs("s") + "}"
       "}");
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

  WWidget *toResize = columnInfo(column).styleRule->templateWidget();
  toResize->resize(0, WLength::Auto);
  toResize->resize(width.toPixels(), WLength::Auto);

  WApplication *app = WApplication::instance();
  if (renderState_ < NeedRerenderHeader)
    app->doJavaScript(jsRef() + ".adjustColumns();");
}

WLength WTreeView::columnWidth(int column) const
{
  return columnInfo(column).width;
}

void WTreeView::setColumnAlignment(int column, AlignmentFlag alignment)
{
  columnInfo(column).alignment = alignment;

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

  WContainerWidget *wc = dynamic_cast<WContainerWidget *>(headerWidget(column));

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

void WTreeView::setRootIsDecorated(bool show)
{
  rootIsDecorated_ = show;
}

void WTreeView::setAlternatingRowColors(bool enable)
{
  if (alternatingRowColors_ != enable) {
    alternatingRowColors_ = enable;
    setRootNodeStyle();
  }
}

void WTreeView::setRootNodeStyle()
{
  if (!rootNode_)
    return;

  if (alternatingRowColors_)
    rootNode_->decorationStyle().setBackgroundImage
      (WApplication::resourcesUrl()
       + "themes/" + WApplication::instance()->cssTheme()
       + "/stripes/stripe-" + boost::lexical_cast<std::string>
       (static_cast<int>(rowHeight_.toPixels())) + "px.gif");
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
  WAbstractItemView::setModel(model);

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

  expandedSet_.clear();

  while (static_cast<int>(columns_.size()) > model->columnCount()) {
    delete columns_.back().styleRule;
    columns_.erase(columns_.begin() + columns_.size() - 1);
  }
}

void WTreeView::setRootIndex(const WModelIndex& rootIndex)
{
  if (rootIndex != rootIndex_) {
    rootIndex_ = rootIndex;

    if (model_)
      scheduleRerender(NeedRerenderData);
  }
}

void WTreeView::scheduleRerender(RenderState what)
{
  if (what == NeedRerender || what == NeedRerenderData) {
    delete rootNode_;
    rootNode_ = 0;
  }

  WAbstractItemView::scheduleRerender(what);
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
    case NeedRerenderData:
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

  WAbstractItemView::render();
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

  WContainerWidget *row = new WContainerWidget(headers_);
  row->setFloatSide(Right);

  if (column1Fixed_) {
    row->setStyleClass("Wt-tv-row headerrh background");
    row = new WContainerWidget(row);
    row->setStyleClass("Wt-tv-rowc headerrh");
  } else
    row->setStyleClass("Wt-tv-row");

  for (int i = 0; i < columnCount(); ++i) {
    WWidget *w = createHeaderWidget(app, i);

    if (i != 0) {
      w->setFloatSide(Left);
      row->addWidget(w);
    } else
      headers_->addWidget(w);
  }

  if (currentSortColumn_ != -1) {
    SortOrder order = columnInfo(currentSortColumn_).sortOrder;
    headerSortIconWidget(currentSortColumn_)
      ->setStyleClass(order == AscendingOrder
		      ? "Wt-tv-sh Wt-tv-sh-up"
		      : "Wt-tv-sh Wt-tv-sh-down");
  }

  app->doJavaScript(jsRef() + ".adjustColumns();");

  if (model_)
    modelHeaderDataChanged(Horizontal, 0, columnCount() - 1);
}

void WTreeView::enableAjax()
{
  rootNode_->clicked().connect(itemClickedJS_);
  rootNode_->doubleClicked().connect(itemDoubleClickedJS_);
  if (mouseWentDown_.isConnected() || dragEnabled_)
    rootNode_->mouseWentDown().connect(itemMouseDownJS_);
  if (mouseWentUp_.isConnected())
    rootNode_->mouseWentUp().connect(itemMouseUpJS_);

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

  if (WApplication::instance()->environment().ajax()) {
    rootNode_->clicked().connect(itemClickedJS_);
    rootNode_->doubleClicked().connect(itemDoubleClickedJS_);
    if (mouseWentDown_.isConnected() || dragEnabled_)
      rootNode_->mouseWentDown().connect(itemMouseDownJS_);
    if (mouseWentUp_.isConnected())
      rootNode_->mouseWentUp().connect(itemMouseUpJS_);
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
  } else if (type == "mouseup") {
    mouseWentUp_.emit(index, event);
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
		      createColumnInfo(i));

    if (renderState_ < NeedRerenderHeader) {
      if (start == 0)
	scheduleRerender(NeedRerenderHeader);
      else {
	app->doJavaScript(jsRef() + ".adjustColumns();");

	WContainerWidget *row = headerRow();

	for (int i = start; i < start + count; ++i) {
	  WWidget* w = createHeaderWidget(app, i);
	  w->setFloatSide(Left);
	  row->insertWidget(i - 1, w);
	}
      }
    }
  }

  if (renderState_ == NeedRerender || renderState_ == NeedRerenderData)
    return;

  if (start == 0)
    scheduleRerender(NeedRerenderData);
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
      app->doJavaScript(jsRef() + ".adjustColumns();");
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
    scheduleRerender(NeedRerenderData);
}

void WTreeView::modelColumnsRemoved(const WModelIndex& parent,
				    int start, int end)
{
  if (renderState_ == NeedRerender || renderState_ == NeedRerenderData)
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

  if (start <= currentSortColumn_ && currentSortColumn_ <= end)
    currentSortColumn_ = -1;
}

void WTreeView::modelRowsInserted(const WModelIndex& parent,
				  int start, int end)
{
  int count = end - start + 1;
  shiftModelIndexes(parent, start, count);

  if (renderState_ == NeedRerender || renderState_ == NeedRerenderData)
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

  if (renderState_ != NeedRerender || renderState_ != NeedRerenderData) {
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

	WTreeViewNode *node = s->node();
	s->setRows(s->rows() - removedHeight_); // could delete s ?
	node->adjustChildrenHeight(-removedHeight_);
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
  if (renderState_ == NeedRerender || renderState_ == NeedRerenderData)
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

  return dynamic_cast<WText *>(headerWidget(column)->find("sort"));
}

WWidget *WTreeView::headerWidget(int column, bool contentsOnly)
{
  WWidget *result = 0;

  if (column == 0)
    result = headers_->widget(headers_->count() - 1);
  else
    result = headerRow()->widget(column - 1);

  if (contentsOnly)
    return result->find("contents");
  else
    return result;
}

WContainerWidget *WTreeView::headerRow()
{
  WContainerWidget *row
    = dynamic_cast<WContainerWidget *>(headers_->widget(0));
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
  //assert(rootNode_->rowCount() == 1);

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

  //assert(rootNode_->rowCount() == 1);
}

int WTreeView::adjustRenderedNode(WTreeViewNode *node, int theNodeRow)
{
  //assert(rootNode_->rowCount() == 1);

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

	  // assert(rootNode_->rowCount() == 1);

	  WTreeViewNode *n = new WTreeViewNode(this, childIndex,
					       childHeight - 1,
					       i == childCount - 1, node);

	  // assert(rootNode_->rowCount() == 1);

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

  scheduleRerender(NeedRerenderData);
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

WAbstractItemView::ColumnInfo& WTreeView::columnInfo(int column) const
{
  while (column >= (int)columns_.size()) {
    ColumnInfo ci = createColumnInfo(column);
    columns_.push_back(ci);
  }

  return columns_[column];
}

WAbstractItemView::ColumnInfo WTreeView::createColumnInfo(const int column)
  const
{
  ColumnInfo ci = ColumnInfo(this,
			     ++nextColumnId_,
			     column);
  ci.styleRule = new WCssTemplateRule("#" + this->id()
				      + " ." + ci.styleClass());

  WApplication *app = WApplication::instance();
  if (column != 0) {
    ci.width = WLength(150);
    ci.styleRule->templateWidget()->resize(ci.width, WLength::Auto);
  } else
    app->styleSheet().addRule("#" + this->id() + " .Wt-tv-node"
			      + " ." + ci.styleClass(),
			      "width: auto;");

  app->styleSheet().addRule(ci.styleRule);
  return ci;
}

}

#endif // DOXYGEN_ONLY
