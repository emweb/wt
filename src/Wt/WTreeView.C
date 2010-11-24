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
#include "Wt/WPushButton"
#include "Wt/WTable"
#include "Wt/WTableCell"
#include "Wt/WText"
#include "Wt/WTreeView"
#include "Wt/WVBoxLayout"
#include "Wt/WWebWidget"

#include "EscapeOStream.h"
#include "JavaScriptLoader.h"
#include "Utils.h"

#ifndef WT_DEBUG_JS
#include "js/WTreeView.min.js"
#endif

/*
  TODO:

  nice to have:
   - stateless slot implementations
   - keyboard navigation ?
*/

#ifndef DOXYGEN_ONLY

// Widest scrollbar found ? My Gnome Firefox has this
#define SCROLLBAR_WIDTH_TEXT "22"
#define SCROLLBAR_WIDTH      22

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

    if (WApplication::instance()->environment().ajax()) {
      clicked().connect(*config_->toggleJS_);

      for (unsigned i = 0; i < config_->states().size(); ++i)
	signals_.push_back(new JSignal<>(this, "t-" + config_->states()[i]));
    } else {
      clicked().connect(this, &ToggleButton::handleClick);
      for (unsigned i = 0; i < config_->states().size(); ++i)
	signals_.push_back(new Signal<>(this));
    }
  }

  virtual ~ToggleButton() {
    for (unsigned i = 0; i < signals_.size(); ++i)
      delete signals_[i];
  }

  SignalBase& signal(int i) { return *signals_[i]; }

  void setState(int i)
  {
    setStyleClass(config_->states()[i]);
  }

private:
  std::vector<SignalBase *> signals_;
  ToggleButtonConfig       *config_;

  void handleClick() {
    for (unsigned i = 0; i < config_->states().size(); ++i)
      if (config_->states()[i] == styleClass().toUTF8()) {
	(dynamic_cast<Signal<> *>(signals_[i]))->emit();
	break;
      }
  }
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

  WWidget *widget(int column);

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

  if (view_->isEditing()) {
    WModelIndex parent = index_.parent();

    int thisNodeCount = view_->model()->columnCount(parent);

    for (int i = 0; i < thisNodeCount; ++i) {
      WModelIndex child = childIndex(i);

      if (view_->isEditing(child)) {
	boost::any editState = view_->itemDelegate(i)->editState(widget(i));
	view_->setEditState(child, editState);
	view_->setEditorWidget(child, 0);
      }
    }
  }
}

void WTreeViewNode::update(int firstColumn, int lastColumn)
{
  WModelIndex parent = index_.parent();

  int thisNodeCount = view_->model()->columnCount(parent);

  for (int i = firstColumn; i <= lastColumn; ++i) {
    WModelIndex child = i < thisNodeCount ? childIndex(i) : WModelIndex();

    WWidget *w = widget(i);

    WFlags<ViewItemRenderFlag> renderFlags = 0;
    if (view_->selectionBehavior() == SelectItems && view_->isSelected(child))
      renderFlags |= RenderSelected;

    if (view_->isEditing(child)) {
      renderFlags |= RenderEditing;
      if (view_->hasEditFocus(child))
	renderFlags |= RenderFocused;
    }

    if (!view_->isValid(child)) {
      renderFlags |= RenderInvalid;
    }
    
    w = view_->itemDelegate(i)->update(w, child, renderFlags);

    if (renderFlags & RenderEditing)
      view_->setEditorWidget(child, w);

    if (!w->parent()) {
      setWidget(i, w);

      /*
       * If we are creating a new editor, then reset its current edit
       * state.
       */
      if (renderFlags & RenderEditing) {
	boost::any state = view_->editState(child);
	if (!state.empty())
	  view_->itemDelegate(i)->setEditState(w, state);
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

      expandButton_->signal(0).connect(this, &WTreeViewNode::doExpand);
      expandButton_->signal(1).connect(this, &WTreeViewNode::doCollapse);

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
  SStream s;

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
      view_->clickedMapper_->mapConnect1(wi->clicked(), childIndex(column));
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
    else
      view_->pageChanged().emit();
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
  if (c->count() == 0 || !(result = dynamic_cast<RowSpacer *>(c->widget(0)))) {
    if (!create)
      return 0;
    else {
      result = new RowSpacer(this, 0);
      c->insertWidget(0, result);
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
      || !(result = dynamic_cast<RowSpacer *>(c->widget(c->count() - 1)))) {
    if (!create)
      return 0;
    else {
      result = new RowSpacer(this, 0);
      c->addWidget(result);
    }
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
      w->addStyleClass(WT_USTRING::fromUTF8("Wt-selected"));
    else
      w->removeStyleClass(WT_USTRING::fromUTF8("Wt-selected"));
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
    renderedNodesAdded_(false),
    rootNode_(0),
    borderColorRule_(0),
    rootIsDecorated_(true),
    column1Fixed_(false),
    collapsed_(this),
    expanded_(this),
    viewportTop_(0),
    viewportHeight_(30),
    nodeLoad_(0),
    headerContainer_(0),
    contentsContainer_(0),
    scrollBarC_(0),
    itemEvent_(impl_, "itemEvent")
{
  setSelectable(false);

  expandConfig_ = new ToggleButtonConfig(this);
  expandConfig_->addState("Wt-expand");
  expandConfig_->addState("Wt-collapse");
  expandConfig_->generate();

  itemEvent_.connect(this, &WTreeView::onItemEvent);

  setStyleClass("Wt-treeview");

  const char *CSS_RULES_NAME = "Wt::WTreeView";

  WApplication *app = WApplication::instance();

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
      (".Wt-treeview .Wt-tv-c",
       "padding: 0px 3px;");

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
       "margin-right: 0px; border-right: 1px solid white;");

    /* sort handles */
    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-sh", std::string() +
       "float: right; width: 16px; height: 16px; padding-bottom: 6px;"
       "cursor: pointer; cursor:hand;");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-sh-nrh", std::string() + 
       "float: right; width: 16px; height: 16px;"
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

  app->styleSheet().addRule("#" + id() + " .cwidth", "");

  rowHeightRule_ = new WCssTemplateRule("#" + id() + " .rh");
  app->styleSheet().addRule(rowHeightRule_);

  rowWidthRule_ = new WCssTemplateRule("#" + id() + " .Wt-tv-row");
  app->styleSheet().addRule(rowWidthRule_);

  rowContentsWidthRule_ = new WCssTemplateRule("#" + id() +" .Wt-tv-rowc");
  app->styleSheet().addRule(rowContentsWidthRule_);

  app->addAutoJavaScript
    ("{var obj = $('#" + id() + "').data('obj');"
     "if (obj) obj.autoJavaScript();}");

  if (parent)
    parent->addWidget(this);

  setup();
}

void WTreeView::setup()
{
  WApplication *app = WApplication::instance();

  impl_->clear();

  rootNode_ = 0;

  /*
   * Setup main layout
   */
  headers_ = new WContainerWidget();
  headers_->setStyleClass("Wt-headerdiv headerrh");

  contents_ = new WContainerWidget();
  WContainerWidget *wrapRoot = new WContainerWidget();
  contents_->addWidget(wrapRoot);

  if (app->environment().agentIsIE()) {
    wrapRoot->setAttributeValue("style", "zoom: 1");
    contents_->setAttributeValue("style", "zoom: 1");
  }

  if (app->environment().ajax()) {
    impl_->setPositionScheme(Relative);

    WVBoxLayout *layout = new WVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    headerContainer_ = new WContainerWidget();
    headerContainer_->setOverflow(WContainerWidget::OverflowHidden);
    headerContainer_->setStyleClass("Wt-header headerrh cwidth");
    headerContainer_->addWidget(headers_);

    contentsContainer_ = new WContainerWidget();
    contentsContainer_->setStyleClass("cwidth");
    contentsContainer_->setOverflow(WContainerWidget::OverflowAuto);
    contentsContainer_->scrolled().connect(this, &WTreeView::onViewportChange);
    contentsContainer_->scrolled().connect
      ("function(obj, event) {"
       "" + headerContainer_->jsRef() + ".scrollLeft=obj.scrollLeft;"
       /* the following is a workaround for IE7 */
       """var t = " + contents_->jsRef() + ".firstChild;"
       """var h = " + headers_->jsRef() + ";"
       """h.style.width = (t.offsetWidth - 1) + 'px';"
       """h.style.width = t.offsetWidth + 'px';"
       "}");
    contentsContainer_->addWidget(contents_);

    layout->addWidget(headerContainer_);
    layout->addWidget(contentsContainer_, 1);

    impl_->setLayout(layout);
  } else {
    contentsContainer_ = new WContainerWidget();
    contentsContainer_->addWidget(contents_);
    contentsContainer_->setOverflow(WContainerWidget::OverflowHidden);

    impl_->setPositionScheme(Relative);
    contentsContainer_->setPositionScheme(Relative);
    contents_->setPositionScheme(Relative);

    impl_->addWidget(headers_);
    impl_->addWidget(contentsContainer_);

    viewportHeight_ = 1000;

    resize(width(), height());
  }

  setRowHeight(rowHeight());
}

void WTreeView::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  if (!app->environment().ajax())
    return;

  const char *THIS_JS = "js/WTreeView.js";

  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WTreeView", wtjs1);
    app->setJavaScriptLoaded(THIS_JS);
  }

  app->doJavaScript("new " WT_CLASS ".WTreeView("
		    + app->javaScriptClass() + "," + jsRef() + ","
		    + contentsContainer_->jsRef() + ","
		    + headerContainer_->jsRef() + ","
		    + (column1Fixed_ ? "true" : "false") + ");");
}

void WTreeView::setColumn1Fixed(bool fixed)
{
  WApplication *app = WApplication::instance();

  // This kills progressive enhancement too
  if (!app->environment().ajax())
    return;

  if (fixed && !column1Fixed_) {
    column1Fixed_ = fixed;

    setStyleClass("Wt-treeview column1");
    WContainerWidget *rootWrap
      = dynamic_cast<WContainerWidget *>(contents_->widget(0));
    rootWrap->resize(WLength(100, WLength::Percentage), WLength::Auto);
    rootWrap->setOverflow(WContainerWidget::OverflowHidden);

    // needed for IE, otherwise row expands automatically to content width
    rowWidthRule_->templateWidget()->resize(0, WLength::Auto);

    bool useStyleLeft
      = app->environment().agentIsWebKit()
      || app->environment().agentIsOpera();

    if (useStyleLeft)
      tieRowsScrollJS_.setJavaScript
	("function(obj, event) {"
	 "" WT_CLASS ".getCssRule('#" + id() + " .Wt-tv-rowc').style.left"
	 ""  "= -obj.scrollLeft + 'px';"
	 "}");
    else {
      /*
       * this is for some reason very very slow in webkit, and with
       * application/xml on Firefox (jQuery suffers)
       */
      tieRowsScrollJS_.setJavaScript
	("function(obj, event) {"
	 "obj.parentNode.style.width = "
	 "" WT_CLASS ".getCssRule('#" + id() + " .cwidth').style.width;"
	 "$('#" + id() + " .Wt-tv-rowc').parent().scrollLeft(obj.scrollLeft);"
	 "}");
    }

    WContainerWidget *scrollBarContainer = new WContainerWidget();
    scrollBarContainer->setStyleClass("cwidth");
    scrollBarContainer->resize(WLength::Auto, SCROLLBAR_WIDTH);
    scrollBarC_ = new WContainerWidget(scrollBarContainer);
    scrollBarC_->setStyleClass("Wt-tv-row Wt-scroll");
    scrollBarC_->scrolled().connect(tieRowsScrollJS_);

    if (app->environment().agentIsIE()) {
      scrollBarContainer->setPositionScheme(Relative);
      scrollBarC_->setAttributeValue("style", "right: 0px");
      // and still it doesn't work properly...
    }

    WContainerWidget *scrollBar = new WContainerWidget(scrollBarC_);
    scrollBar->setStyleClass("Wt-tv-rowc");
    if (useStyleLeft)
      scrollBar->setAttributeValue("style", "left: 0px;");
    impl_->layout()->addWidget(scrollBarContainer);
  }
}

WTreeView::~WTreeView()
{ 
  delete expandConfig_;
  delete rowHeightRule_;

  impl_->clear();
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

  if (app->environment().ajax() && renderState_ < NeedRerenderHeader)
    app->doJavaScript("$('#" + id() + "').data('obj').adjustColumns();");

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
  WAbstractItemView::setAlternatingRowColors(enable);
  setRootNodeStyle();
}

void WTreeView::setRootNodeStyle()
{
  if (!rootNode_)
    return;

  if (alternatingRowColors())
    rootNode_->decorationStyle().setBackgroundImage
      (WApplication::resourcesUrl()
       + "themes/" + WApplication::instance()->cssTheme()
       + "/stripes/stripe-" + boost::lexical_cast<std::string>
       (static_cast<int>(rowHeight().toPixels())) + "px.gif");
   else
     rootNode_->decorationStyle().setBackgroundImage("");
 }

void WTreeView::setRowHeight(const WLength& rowHeight)
{
  WAbstractItemView::setRowHeight(rowHeight);

  rowHeightRule_->templateWidget()->resize(WLength::Auto, rowHeight);
  rowHeightRule_->templateWidget()->setLineHeight(rowHeight);

  if (!WApplication::instance()->environment().ajax() && !height().isAuto())
    viewportHeight_ = static_cast<int>(contentsContainer_->height().toPixels()
				       / rowHeight.toPixels());

  setRootNodeStyle();

  for (NodeMap::const_iterator i = renderedNodes_.begin();
       i != renderedNodes_.end(); ++i)
    i->second->rerenderSpacers();

  if (rootNode_)
    scheduleRerender(NeedAdjustViewPort);
}

void WTreeView::setHeaderHeight(const WLength& height, bool multiLine)
{
  WAbstractItemView::setHeaderHeight(height, multiLine);

  if (renderState_ >= NeedRerenderHeader)
    return;

  // XX: cannot do it for column 0!
  if (!WApplication::instance()->environment().agentIsIE())
    for (int i = 1; i < columnCount(); ++i)
      headerTextWidget(i)->setWordWrap(multiLine);
}

void WTreeView::resize(const WLength& width, const WLength& height)
{
  WApplication *app = WApplication::instance();
  WLength w = app->environment().ajax() ? WLength::Auto : width;

  contentsContainer_->resize(w, WLength::Auto);
  
  if (headerContainer_)
    headerContainer_->resize(w, WLength::Auto);

  if (!height.isAuto()) {
    if (!app->environment().ajax()) {
      if (impl_->count() < 3)
	impl_->addWidget(createPageNavigationBar());

      double navigationBarHeight = 25;
      double headerHeight = this->headerHeight().toPixels();

      contentsContainer_->resize(width, height.toPixels()
				 - navigationBarHeight - headerHeight);
      viewportHeight_
	= static_cast<int>(contentsContainer_->height().toPixels()
			   / rowHeight().toPixels());
    } else
      viewportHeight_ = static_cast<int>
	(std::ceil(height.toPixels() / rowHeight().toPixels()));
  } else {
    if (app->environment().ajax())
      viewportHeight_ = 30;

    scheduleRerender(NeedAdjustViewPort);
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

void WTreeView::setModel(WAbstractItemModel *model)
{
  WAbstractItemView::setModel(model);

  /* connect slots to new model */
  modelConnections_.push_back(model->columnsInserted().connect
			      (this, &WTreeView::modelColumnsInserted));
  modelConnections_.push_back(model->columnsAboutToBeRemoved().connect
			      (this, &WTreeView::modelColumnsAboutToBeRemoved));
  modelConnections_.push_back(model->columnsRemoved().connect
			      (this, &WTreeView::modelColumnsRemoved));
  modelConnections_.push_back(model->rowsInserted().connect
			      (this, &WTreeView::modelRowsInserted));
  modelConnections_.push_back(model->rowsAboutToBeRemoved().connect
			      (this, &WTreeView::modelRowsAboutToBeRemoved));
  modelConnections_.push_back(model->rowsRemoved().connect
			      (this, &WTreeView::modelRowsRemoved));
  modelConnections_.push_back(model->dataChanged().connect
			      (this, &WTreeView::modelDataChanged));
  modelConnections_.push_back(model->headerDataChanged().connect
			      (this, &WTreeView::modelHeaderDataChanged));
  modelConnections_.push_back(model->layoutAboutToBeChanged().connect
			      (this, &WTreeView::modelLayoutAboutToBeChanged));
  modelConnections_.push_back(model->layoutChanged().connect
			      (this, &WTreeView::modelLayoutChanged));
  modelConnections_.push_back(model->modelReset().connect
			      (this, &WTreeView::modelReset));

  expandedSet_.clear();

  while (static_cast<int>(columns_.size()) > model->columnCount()) {
    delete columns_.back().styleRule;
    columns_.erase(columns_.begin() + columns_.size() - 1);
  }

  pageChanged().emit();
}

void WTreeView::scheduleRerender(RenderState what)
{
  if (what == NeedRerender || what == NeedRerenderData) {
    delete rootNode_;
    rootNode_ = 0;
  }

  WAbstractItemView::scheduleRerender(what);
}

void WTreeView::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull)
    defineJavaScript();

  while (renderState_ != RenderOk) {
    RenderState s = renderState_;
    renderState_ = RenderOk;

    switch (s) {
    case NeedRerender:
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


  if (column1Fixed_ && renderedNodesAdded_) {
    WApplication::instance()->doJavaScript
      ("{var s=" + scrollBarC_->jsRef() + ";"
       """if (s) {" + tieRowsScrollJS_.execJs("s") + "}"
       "}");
    renderedNodesAdded_ = false;
  }

  WAbstractItemView::render(flags);
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

  if (app->environment().ajax())
    app->doJavaScript("$('#" + id() + "').data('obj').adjustColumns();");

  if (model())
    modelHeaderDataChanged(Horizontal, 0, columnCount() - 1);
}

void WTreeView::enableAjax()
{
  setup();
  defineJavaScript();

  rerenderHeader();
  rerenderTree();

  WCompositeWidget::enableAjax();
}

void WTreeView::rerenderTree()
{
  WContainerWidget *wrapRoot
    = dynamic_cast<WContainerWidget *>(contents_->widget(0));

  wrapRoot->clear();

  firstRenderedRow_ = calcOptimalFirstRenderedRow();
  validRowCount_ = 0;

  rootNode_ = new WTreeViewNode(this, rootIndex(), -1, true, 0);
  rootNode_->resize(WLength(100, WLength::Percentage), 1);

  if (WApplication::instance()->environment().ajax()) {
    connectObjJS(rootNode_->clicked(), "click");
    connectObjJS(rootNode_->doubleClicked(), "dblClick");
    if (mouseWentDown().isConnected() || dragEnabled_)
      connectObjJS(rootNode_->mouseWentDown(), "mouseDown");
    if (mouseWentUp().isConnected())
      connectObjJS(rootNode_->mouseWentUp(), "mouseUp");
  }

  setRootNodeStyle();

  wrapRoot->addWidget(rootNode_);

  pageChanged().emit();

  adjustToViewport();
}

void WTreeView::onViewportChange(WScrollEvent e)
{
  viewportTop_ = static_cast<int>
    (std::floor(e.scrollY() / rowHeight().toPixels()));

  viewportHeight_ = static_cast<int>
    (std::ceil(e.viewportHeight() / rowHeight().toPixels()));

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

  WModelIndex index = model()->index(c0index.row(), column, c0index.parent());

  if (type == "clicked") {
    handleClick(index, event);
  } else if (type == "dblclicked") {
    handleDoubleClick(index, event);
  } else if (type == "mousedown") {
    mouseWentDown().emit(index, event);
  } else if (type == "mouseup") {
    mouseWentUp().emit(index, event);
  } else if (type == "drop") {
    WDropEvent e(WApplication::instance()->decodeObject(extra1), extra2, event);
    dropEvent(e, index);
  }
}

int WTreeView::subTreeHeight(const WModelIndex& index,
			     int lowerBound, int upperBound)
{
  int result = 0;

  if (index != rootIndex())
    ++result;

  if (result >= upperBound)
    return result;

  if (isExpanded(index)) {
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

void WTreeView::setCollapsed(const WModelIndex& index)
{
  expandedSet_.erase(index);

  bool selectionHasChanged = false;
  WModelIndexSet& selection = selectionModel()->selection_;

  for (WModelIndexSet::iterator it = selection.lower_bound(index);
       it != selection.end();) {    
    /*
     * The following is needed because internalSelect(Deselect) will remove
     * the iterated element
     */
#ifndef WT_TARGET_JAVA
    WModelIndexSet::iterator n = it;
    ++n;
#endif

    WModelIndex i = *it;
    if (i == index) {
    } else if (WModelIndex::isAncestor(i, index)) {
      if (internalSelect(i, Deselect))
	selectionHasChanged = true;
    } else
      break;

#ifndef WT_TARGET_JAVA
    it = n;
#endif
  }

  if (selectionHasChanged)
    selectionChanged().emit();
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
      columns_.insert(columns_.begin() + i, createColumnInfo(i));

    if (renderState_ < NeedRerenderHeader) {
      if (start == 0)
	scheduleRerender(NeedRerenderHeader);
      else {
	if (app->environment().ajax())
	  app->doJavaScript("$('#" + id() + "').data('obj').adjustColumns();");

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
    WTreeViewNode *node = nodeForIndex(parent);
    if (node)
      for (WTreeViewNode *c = node->nextChildNode(0); c;
	   c = node->nextChildNode(c))
	c->insertColumns(start, count);
  }
}

void WTreeView::modelColumnsAboutToBeRemoved(const WModelIndex& parent,
					     int start, int end)
{
  int count = end - start + 1;
  if (!parent.isValid()) {
    if (renderState_ < NeedRerenderHeader) {
      WApplication *app = wApp;
      if (app->environment().ajax())
	app->doJavaScript("$('#" + id() + "').data('obj').adjustColumns();");
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
    WTreeViewNode *node = nodeForIndex(parent);
    if (node)
      for (WTreeViewNode *c = node->nextChildNode(0); c;
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

  if (renderState_ == NeedRerender || renderState_ == NeedRerenderData)
    return;

  WWidget *parentWidget = widgetForIndex(parent);

  if (parentWidget) {
    WTreeViewNode *parentNode = dynamic_cast<WTreeViewNode *>(parentWidget);

    if (parentNode) {
      if (parentNode->childrenLoaded()) {
	WWidget *startWidget = 0;

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
	  renderedRowsChanged(renderedRow(model()->index(start, 0, parent),
					  parentNode->topSpacer(),
					  renderLowerBound(),
					  renderUpperBound()),
			      count);

	} else if (startWidget && startWidget == parentNode->bottomSpacer()) {
	  parentNode->addBottomSpacerHeight(count);
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

	  int nodesToAdd = std::min(count, maxRenderHeight);

	  WTreeViewNode *first = 0;
	  for (int i = 0; i < nodesToAdd; ++i) {
	    WTreeViewNode *n
	      = new WTreeViewNode(this, model()->index(start + i, 0, parent), -1,
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
	  if (end == model()->rowCount(parent) - 1 && start >= 1) {
	    WTreeViewNode *n = dynamic_cast<WTreeViewNode *>
	      (parentNode->widgetForModelRow(start - 1));

	    if (n)
	      n->updateGraphics(false,
				model()->rowCount(n->modelIndex()) == 0);
	  }
	}
      } /* else:
	   children not loaded -- so we do not need to bother
	 */

      // Update graphics for parent when first children have een added
      if (model()->rowCount(parent) == count)
	parentNode->updateGraphics(parentNode->isLast(), false);
    } else {
      RowSpacer *s = dynamic_cast<RowSpacer *>(parentWidget);

      s->setRows(s->rows() + count);
      s->node()->adjustChildrenHeight(count);

      renderedRowsChanged(renderedRow(model()->index(start, 0, parent), s,
				      renderLowerBound(), renderUpperBound()),
			  count);
    }
  } else {
    /* else:
       parentWidget is 0: it means it is not even part of any spacer.
       FIXME: the parent could still be rendered but (somehow) collapsed ?
     */
  }
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
	      WModelIndex childIndex = model()->index(i, 0, parent);

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

	      delete node;
	    }
	  }

	  parentNode->normalizeSpacers();

	  // Update graphics for last node in parent, if we are removing rows
	  // at the back. This is not affected by widgetForModelRow() returning
	  // accurate information of rows just deleted and indexes not yet
	  // shifted
	  if (end == model()->rowCount(parent) - 1 && start >= 1) {
	    WTreeViewNode *n = dynamic_cast<WTreeViewNode *>
	      (parentNode->widgetForModelRow(start - 1));

	    if (n)
	      n->updateGraphics(true, model()->rowCount(n->modelIndex()) == 0);
	  }
	} /* else:
	     children not loaded -- so we do not need to bother
	  */

	// Update graphics for parent when all rows have been removed
	if (model()->rowCount(parent) == count)
	  parentNode->updateGraphics(parentNode->isLast(), true);
      } else {
	RowSpacer *s = dynamic_cast<RowSpacer *>(parentWidget);

	for (int i = start; i <= end; ++i) {
	  WModelIndex childIndex = model()->index(i, 0, parent);
	  int childHeight = subTreeHeight(childIndex);
	  removedHeight_ += childHeight;

	  if (i == start)
	    firstRemovedRow_ = renderedRow(childIndex, s);
	}

	WTreeViewNode *node = s->node();
	s->setRows(s->rows() - removedHeight_); // could delete s ?
	node->adjustChildrenHeight(-removedHeight_);
      }
    } else {
      /*
	parentWidget is 0: it means it is not even part of any spacer.
	FIXME: but it could still be rendered, yet (somehow) collapsed ?
      */
    }
  }
}

void WTreeView::modelRowsRemoved(const WModelIndex& parent,
				 int start, int end)
{
  int count = end - start + 1;

  if (renderState_ != NeedRerender || renderState_ != NeedRerenderData) {
    WWidget *parentWidget = widgetForIndex(parent);
    if (parentWidget) {
      WTreeViewNode *parentNode = dynamic_cast<WTreeViewNode *>(parentWidget);
      if (parentNode) {
	if (parentNode->childrenLoaded()) {
	  parentNode->adjustChildrenHeight(-removedHeight_);
	  parentNode->shiftModelIndexes(start, -count);
	}
      }
    }
  }

  shiftModelIndexes(parent, start, -count);

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
	  WModelIndex index = model()->index(r, 0, parent);

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
  } else {
    /*
      parent is not displayed
      FIXME: but it could still be rendered, yet (somehow) not expanded ?
    */
  }
}

void WTreeView::modelHeaderDataChanged(Orientation orientation,
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

WWidget *WTreeView::headerWidget(int column, bool contentsOnly)
{
  WWidget *result = 0;

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
      result += subTreeHeight(model()->index(r, 0, parent), 0,
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

    int childCount = model()->rowCount(index);
    while (topSpacerHeight != 0
	   && nodeRow + topSpacerHeight > firstRenderedRow_) {
      // eat from top spacer and replace with actual nodes
      WTreeViewNode *n
	= dynamic_cast<WTreeViewNode *>(node->childContainer()->widget(1));

      assert(n);

      WModelIndex childIndex
	= model()->index(n->modelIndex().row() - 1, 0, index);

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
	= model()->index(n->modelIndex().row() + 1, 0, index);

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

void WTreeView::shiftModelIndexes(const WModelIndex& parent,
				  int start, int count)
{
  shiftModelIndexes(parent, start, count, model(), expandedSet_);

  int removed = shiftModelIndexes(parent, start, count, model(),
				  selectionModel()->selection_);

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
  if (selectionBehavior() == SelectRows && index.column() != 0)
    return internalSelect(model()->index(index.row(), 0, index.parent()),
			  option);

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
    WModelIndex column0Index = model()->index(index.row(), 0, index.parent());
    NodeMap::const_iterator i = renderedNodes_.find(column0Index);
    return i != renderedNodes_.end() ? i->second : 0;
  }
}

void WTreeView::selectRange(const WModelIndex& first, const WModelIndex& last)
{
  WModelIndex index = first;
  for (;;) {
    for (int c = first.column(); c <= last.column(); ++c) {
      WModelIndex ic = model()->index(index.row(), c, index.parent());
      internalSelect(ic, Select);

      if (ic == last)
	return;
    }

    WModelIndex indexc0
      = index.column() == 0 ? index
      : model()->index(index.row(), 0, index.parent());

    if (isExpanded(indexc0) && model()->rowCount(indexc0) > 0)
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

    WApplication *app = WApplication::instance();
    app->styleSheet().addRule("#" + this->id() + " .Wt-tv-node"
			      " ." + ci.styleClass(),
			      "width: auto;"
			      "text-overflow: ellipsis;"
			      "overflow: hidden");
  }

  return ci;
}

void WTreeView::setCurrentPage(int page)
{
  viewportTop_ = page * viewportHeight_;

  contents_->setOffsets(-viewportTop_ * rowHeight().toPixels(), Top);

  pageChanged().emit();

  scheduleRerender(NeedAdjustViewPort);
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

}

#endif // DOXYGEN_ONLY
