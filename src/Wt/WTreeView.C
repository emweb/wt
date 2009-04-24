/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cmath>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include "Wt/WAbstractItemModel"
#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WCheckBox"
#include "Wt/WEnvironment"
#include "Wt/WIconPair"
#include "Wt/WItemSelectionModel"
#include "Wt/WImage"
#include "Wt/WTable"
#include "Wt/WTableCell"
#include "Wt/WText"
#include "Wt/WTreeView"
#include "Wt/WVBoxLayout"
#include "Wt/WWebWidget"

#include "Utils.h"

/*
  TODO:

  nice to have:
   - stateless slot implementations
   - keyboard navigation ?

  bugs:
   - reconnect checkedChange signals when shifting rows in a node

  editing
*/

#ifndef DOXYGEN_ONLY

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

class RowSpacer : public Wt::WWebWidget
{
public:
  RowSpacer(Wt::WTreeViewNode *node, int height)
    : node_(node),
      height_(0)
  {
    setRows(height);
    setInline(false);
    setStyleClass("spacer");
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

  bool                childrenLoaded_;
  WIconPair          *expandIcon_;
  WImage             *noExpandIcon_;

  void loadChildren();

  WModelIndex childIndex(int column);

  WAnchor   *anchorWidget(int column);
  WCheckBox *checkBox(int column, bool autoCreate, bool triState = false);
  WImage    *iconWidget(int column, bool autoCreate = false);
  WText     *textWidget(int column);
  WWidget   *widget(int column);
  void       setWidget(int column, WWidget *w, bool replace);

  void checkChanged(int column);
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
    expandIcon_(0),
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
    elementAt(0, 1)->setStyleClass("c1 rh");

    updateGraphics(isLast, view_->model()->rowCount(index_) == 0);
    insertColumns(0, view_->columnCount());

    selfHeight = 1;

    if (view_->selectionBehavior() == SelectRows && view_->isSelected(index_))
      renderSelected(true, 0);

  } else
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

    bool haveCheckBox = false;

    if (child.flags() & ItemIsUserCheckable) {
      boost::any checkedData = child.data(CheckStateRole);
      CheckState state = checkedData.empty() ? Unchecked
	: (checkedData.type() == typeid(bool) ?
	   (boost::any_cast<bool>(checkedData) ? Checked : Unchecked)
	   : (checkedData.type() == typeid(CheckState) ?
	      boost::any_cast<CheckState>(checkedData) : Unchecked));
      checkBox(i, true, child.flags() & ItemIsTristate)->setCheckState(state);
      haveCheckBox = true;
    } else
      delete checkBox(i, false);

    std::string internalPath = asString(child.data(InternalPathRole)).toUTF8();
    std::string url = asString(child.data(UrlRole)).toUTF8();

    if (!internalPath.empty() || !url.empty()) {
      WAnchor *a = anchorWidget(i);

      if (!internalPath.empty())
	a->setRefInternalPath(internalPath);
      else
	a->setRef(url);
    }

    WText *t = textWidget(i);

    WString label = asString(child.data(), view_->columnFormat(i));
    if (label.empty() && haveCheckBox)
      label = " ";
    t->setText(label);

    std::string iconUrl = asString(child.data(DecorationRole)).toUTF8();
    if (!iconUrl.empty()) {
      iconWidget(i, true)->setImageRef(iconUrl);
    } else if (iconUrl.empty()) {
      delete iconWidget(i, false);
    }

    WWidget *w = widget(i);
    WString tooltip = asString(child.data(ToolTipRole));
    w->setToolTip(tooltip);

    if (i != 0) {
      WT_USTRING sc = asString(child.data(StyleClassRole));

      if (view_->selectionBehavior() == SelectItems && view_->isSelected(child))
	sc += WT_USTRING::fromUTF8(" selected");

      if (!sc.empty()) {
	w->setStyleClass(w->styleClass() + " " + sc);
      }
    }

    if (child.flags() & ItemIsDropEnabled)
      w->setAttributeValue("drop", WString::fromUTF8("true"));
    else
      w->setAttributeValue("drop", WString::fromUTF8("false"));
  }
}

void WTreeViewNode::updateGraphics(bool isLast, bool isEmpty)
{
  if (index_.parent() == view_->rootIndex() && !view_->rootIsDecorated()) {
    delete expandIcon_;
    expandIcon_ = 0;
    delete noExpandIcon_;
    noExpandIcon_ = 0;
    elementAt(0, 0)->setStyleClass("c0");
    elementAt(1, 0)->setStyleClass("c0");

    return;
  }

  if (!isEmpty) {
    if (!expandIcon_) {
      delete noExpandIcon_;
      noExpandIcon_ = 0;
      expandIcon_ = new WIconPair(view_->imagePack() + "nav-plus.gif",
				  view_->imagePack() + "nav-minus.gif");
      elementAt(0, 0)->addWidget(expandIcon_);

      expandIcon_->icon1Clicked().connect(SLOT(this, WTreeViewNode::doExpand));
      expandIcon_->icon2Clicked().connect(SLOT(this, WTreeViewNode::doCollapse));

      if (isExpanded())
	expandIcon_->setState(1);
    }
  } else {
    if (!noExpandIcon_) {
      delete expandIcon_;
      expandIcon_ = 0;
      noExpandIcon_ = new WImage(view_->imagePack() + "line-middle.gif");
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

    row->setStyleClass("Wt-tv-row rh");
    tc->insertWidget(0, row);
  }

  for (int i = 0; i < view_->columnCount(); ++i) {
    WText *w = new WText();
    w->setWordWrap(true);
    setWidget(i, w, false);
  }

  update(0, view_->columnCount() - 1);
}

void WTreeViewNode::removeColumns(int column, int count)
{
  insertColumns(0, 0);
}

bool WTreeViewNode::isLast() const
{
  return index_.row() == view_->model()->rowCount(index_.parent()) - 1;
}

WModelIndex WTreeViewNode::childIndex(int column)
{
  return view_->model()->index(index_.row(), column, index_.parent());
}

/*
 * Possible layouts:
 *  1) WText
 * or
 *  2) WContainerWidget ([WCheckbox] [WImage] [inv] WText)
 * or
 *  3) WAnchor ([WImage] [inv] WText)
 * or
 *  4) WContainerWidget ([WCheckbox] WAnchor ([Image] [inv] WText))
 */

WText *WTreeViewNode::textWidget(int column)
{
  WWidget *w = widget(column);

  /*
   * Case 1
   */
  WText *result = dynamic_cast<WText *>(w);
  if (result)
    return result;

  /* Cases 2-3 */
  WContainerWidget *wc = dynamic_cast<WContainerWidget *>(w);

  result = dynamic_cast<WText *>(wc->widget(wc->count() - 1));
  if (result)
    return result;

  /* Case 4 */
  wc = dynamic_cast<WContainerWidget *>(wc->widget(wc->count() - 1));

  return dynamic_cast<WText *>(wc->widget(wc->count() - 1));
}

WImage *WTreeViewNode::iconWidget(int column, bool autoCreate)
{
  WWidget *w = widget(column);

  /*
   * Case 1
   */
  WText *result = dynamic_cast<WText *>(w);
  if (result)
    if (autoCreate) {
      WContainerWidget *wc = new WContainerWidget();
      setWidget(column, wc, true);

      WImage *image = new WImage();
      image->setStyleClass("icon");
      wc->addWidget(image);

      // IE does not want to center vertically without this:
      if (wApp->environment().agentIsIE()) {
	WImage *inv = new WImage(wApp->onePixelGifUrl());
	inv->setStyleClass("rh w0 icon");
	wc->addWidget(inv);
      }

      wc->addWidget(w);

      return image;
    } else
      return 0;

  /* Cases 2-4 */
  WContainerWidget *wc = dynamic_cast<WContainerWidget *>(w);

  for (int i = 0; i < wc->count(); ++i) {
    WImage *image = dynamic_cast<WImage *>(wc->widget(i));
    if (image)
      return image;

    WAnchor *anchor = dynamic_cast<WAnchor *>(wc->widget(i));
    if (anchor) {
      wc = anchor;
      i = -1;
    }
  }

  if (autoCreate) {
    WImage *image = new WImage();
    image->setStyleClass("icon");
    wc->insertWidget(wc->count() - 1, image);

    // IE does not want to center vertically without this:
    if (wApp->environment().agentIsIE()) {
      WImage *inv = new WImage(wApp->onePixelGifUrl());
      inv->setStyleClass("rh w0 icon");
      wc->insertWidget(wc->count() - 1, inv);
    }

    return image;
  } else
    return 0;
}

WAnchor *WTreeViewNode::anchorWidget(int column)
{
  WWidget *w = widget(column);
  
  /*
   * Case 3
   */
  WAnchor *result = dynamic_cast<WAnchor *>(w);
  if (result)
    return result;
  else {
    /*
     * Case 1
     */
    WText *text = dynamic_cast<WText *>(w);
    if (text) {
      WAnchor *a = new WAnchor();
      setWidget(column, a, true);

      a->addWidget(w);

      return a;
    }

    /*
     * Case 4
     */
    WContainerWidget *wc = dynamic_cast<WContainerWidget *>(w);
    WWidget *lw = wc->widget(wc->count() - 1);

    WAnchor *a = dynamic_cast<WAnchor *>(lw);
    if (a)
      return a;

    /*
     * Case 2
     */
    a = new WAnchor();
    int firstToMove = 0;

    WCheckBox *cb = dynamic_cast<WCheckBox *>(wc->widget(0));
    if (cb)
      firstToMove = 1;

    wc->insertWidget(firstToMove, a);

    while (wc->count() > firstToMove + 1) { 
      WWidget *c = wc->widget(firstToMove + 1);
      wc->removeWidget(c);
      a->addWidget(c);
    }

    return a;
  }
}

WCheckBox *WTreeViewNode::checkBox(int column, bool autoCreate, bool triState)
{
  WWidget *w = widget(column);

  WText *t = dynamic_cast<WText *>(w);
  WAnchor *a = dynamic_cast<WAnchor *>(w);
  WContainerWidget *wc = dynamic_cast<WContainerWidget *>(w);

  /*
   * Case 1 or 3
   */
  if (t || a)
    if (autoCreate) {
      wc = new WContainerWidget();
      setWidget(column, wc, true);
      wc->addWidget(w);
    } else
      return 0;

  WCheckBox *cb = dynamic_cast<WCheckBox *>(wc->widget(0));

  if (!cb && autoCreate) {
    cb = new WCheckBox(false, 0);
    wc->insertWidget(0, cb);

    view_->checkedChangeMapper_->mapConnect
      (cb->changed(), WTreeView::CheckedInfo(childIndex(column), cb));
  }

  if (cb)
    cb->setTristate(triState);

  return cb;
}

void WTreeViewNode::setWidget(int column, WWidget *newW, bool replace)
{
  WTableCell *tc = elementAt(0, 1);

  WWidget *current = replace ? widget(column) : 0;

  if (current) {
    newW->setStyleClass(current->styleClass());
    current->setStyleClass("");
  } else
    newW->setStyleClass("Wt-tv-c rh " + view_->columnStyleClass(column));

  if (column == 0) {
    if (current)
      tc->removeWidget(current);

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

  if (column == 0)
    return tc->widget(tc->count() - 1);
  else {
    WContainerWidget *row = dynamic_cast<WContainerWidget *>(tc->widget(0));
    if (view_->column1Fixed_)
      row = dynamic_cast<WContainerWidget *>(row->widget(0));

    return row->widget(column - 1);
  }
}

void WTreeViewNode::doExpand()
{
  if (isExpanded())
    return;

  loadChildren();

  if (expandIcon_)
    expandIcon_->setState(1);

  view_->expandedSet_.insert(index_);

  rowAt(1)->show();
  if (parentNode())
    parentNode()->adjustChildrenHeight(childrenHeight_);

  view_->adjustRenderedNode(this, renderedRow());
  view_->needAdjustToViewport();

  view_->expanded_.emit(index_);
}

void WTreeViewNode::doCollapse()
{
  if (!isExpanded())
    return;

  if (expandIcon_)
    expandIcon_->setState(0);

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

  for (WTreeViewNode *c = nextChildNode(0); c; c = nextChildNode(c)) {
    if (c->index_.row() > modelRow)
      return topSpacer();
    else if (c->index_.row() == modelRow)
      return c;
  }

  return bottomSpacer();
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
    rowAt(0)->setStyleClass(selected ? "selected" : "");
  else {
    WWidget *w = widget(column);
    if (selected)
      w->setStyleClass(Utils::addWord(w->styleClass().toUTF8(), "selected"));
    else
      w->setStyleClass(Utils::eraseWord(w->styleClass().toUTF8(), "selected"));
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
    sorting(view->sorting_)
{
  styleRule = new WCssTemplateRule("#" + view->formName()
				   + " ." + this->styleClass());
  if (column) {
    width = WLength(150);
    styleRule->templateWidget()->resize(width, WLength::Auto);
  }

  app->styleSheet().addRule(styleRule);
}

std::string WTreeView::ColumnInfo::styleClass() const
{
  return "Wt-tv-c" + boost::lexical_cast<std::string>(id);
}

WTreeView::WTreeView(WContainerWidget *parent)
  : WCompositeWidget(0),
    model_(0),
    selectionModel_(new WItemSelectionModel(0, this)),
    rowHeight_(20),
    headerHeight_(20),
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

  checkedChangeMapper_ = new WSignalMapper<CheckedInfo>(this);
  checkedChangeMapper_->mapped().connect(SLOT(this, WTreeView::onCheckedChange));

  clickedForSortMapper_ = new WSignalMapper<int>(this);
  clickedForSortMapper_->mapped().connect(SLOT(this,
					     WTreeView::toggleSortColumn));

  if (!app->environment().ajax()) {
    clickedMapper_ = new WSignalMapper<WModelIndex>(this);
    clickedMapper_->mapped().connect(SLOT(this, WTreeView::handleClick));
  }

  itemEvent_.connect(SLOT(this, WTreeView::onItemEvent));

  setStyleClass("Wt-treeview unselectable");
  setAttributeValue("onselectstart", "return false;");

  imagePack_ = WApplication::resourcesUrl();

  const char *CSS_RULES_NAME = "Wt::WTreeView";

  if (!app->styleSheet().isDefined(CSS_RULES_NAME)) {
    app->styleSheet().addRule
      (".Wt-treeview",
       "font-family: verdana,helvetica,tahoma,sans-serif;"
       "font-size: 10pt;"
       "cursor: default;", CSS_RULES_NAME);

    app->styleSheet().addRule
      (".Wt-treeview .spacer",
       "background: url(" + imagePack_ + "loading.png);");

    /* header */
    app->styleSheet().addRule
      (".Wt-treeview .header-div",
       "-moz-user-select: none;"
       "-khtml-user-select: none;"
       "background-color: #EEEEEE;"
       "user-select: none;"
       "overflow: hidden;"
       "width: 100%;");

    app->styleSheet().addRule
      (".Wt-treeview .header .Wt-label",
       std::string() + "white-space: normal;"
       "font-weight: bold;"
       "text-overflow: ellipsis;"
       + (app->environment().agentIsIE() ? "zoom: 1;" : "") + 
       "overflow: hidden;");


    /* nodes */
    app->styleSheet().addRule
      (".Wt-treeview .Wt-trunk",
       "background: url(" + imagePack_ + "line-trunk.gif) repeat-y;");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-end",
       "background: url(" + imagePack_ + "tv-line-last.gif) no-repeat 0 center;");

    app->styleSheet().addRule
      (".Wt-treeview table", "width: 100%");

    app->styleSheet().addRule
      (".Wt-treeview td.c1", "width: 100%");

    app->styleSheet().addRule
      (".Wt-treeview td.c0",
       "width: 1px; vertical-align: middle");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-row", "float: right; overflow: hidden;");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-c",
       "display: block; float: left;"
       "padding: 0px 3px;"
       "text-overflow: ellipsis;"
       "overflow: hidden;"
       "white-space: nowrap;");

    app->styleSheet().addRule
      (".Wt-treeview img.icon",
       "margin-right: 3px; vertical-align: middle");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-node img.w0",
       "width: 0px");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-node .c0 img",
       "margin-right: 0px;");

    /* resize handles */
    app->styleSheet().addRule
      (".Wt-treeview div.Wt-tv-rh",
       "float: right; width: 4px; cursor: col-resize;"
       "padding-left: 0px;");

    if (app->environment().agentIsIE())
      app->styleSheet().addRule
	(".Wt-treeview .header .Wt-tv-c",
	 "padding: 0px;"
	 "padding-left: 6px;");
    else
      app->styleSheet().addRule
	(".Wt-treeview .header .Wt-tv-c",
	 "padding: 0px;"
	 "margin-left: 6px;");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-rh:hover",
       "background-color: #DDDDDD;");

    app->styleSheet().addRule
      (".Wt-treeview div.Wt-tv-rhc0",
       "float: left; width: 4px;");

    /* sort handles */
    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-sh",
       "float: right; width: 16px; margin-top: 6px;");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-sh-nrh",
       "float: right; width: 16px; margin-top: 6px; margin-right: 4px");

    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-shc0", "float: left;");

    /* selection */
    app->styleSheet().addRule
      (".Wt-treeview .selected",
       "background-color: #FFFFAA;");

    /* item drag & drop */
    app->styleSheet().addRule
      (".Wt-treeview .drop-site",
       "background-color: #EEEEEE;"
       "outline: 1px dotted black;");

    /* borders */
    app->styleSheet().addRule
      (".Wt-treeview .Wt-tv-row .Wt-tv-c",
       "border-right: 1px solid;");
    app->styleSheet().addRule
      (".Wt-treeview .header .Wt-tv-row, "
       ".Wt-treeview .Wt-tv-node .Wt-tv-row",
       "border-left: 1px solid;");

    setColumnBorder(white /* WColor(0xD0, 0xD0, 0xD0) */);

    /* bottom scrollbar */
    if (app->environment().agentIsWebKit() || app->environment().agentIsOpera())
      app->styleSheet().addRule
	(".Wt-treeview .Wt-tv-rowc", "position: relative;");

    if (app->environment().agentIsIE())
      app->styleSheet().addRule
	(".Wt-treeview .Wt-scroll", "overflow-x: scroll; height: 16px;");
    else
      app->styleSheet().addRule
	(".Wt-treeview .Wt-scroll", "overflow: scroll; height: 16px;");
    app->styleSheet().addRule
      (".Wt-treeview .Wt-scroll div", "height: 1px;");
  }

  app->styleSheet().addRule("#" + formName() + " .cwidth", "height: 1px;");

  /* item drag & drop */
  app->styleSheet().addRule
    ("#" + formName() + "dw",
     "width: 32px; height: 32px;"
     "background: url(" + imagePack_ + "items-not-ok.gif);");

  app->styleSheet().addRule
    ("#" + formName() + "dw.valid-drop",
     "width: 32px; height: 32px;"
     "background: url(" + imagePack_ + "items-ok.gif);");

  rowHeightRule_ = new WCssTemplateRule("#" + formName() + " .rh");
  app->styleSheet().addRule(rowHeightRule_);

  rowWidthRule_ = new WCssTemplateRule("#" + formName() + " .Wt-tv-row");
  app->styleSheet().addRule(rowWidthRule_);

  rowContentsWidthRule_ = new WCssTemplateRule("#"+ formName() +" .Wt-tv-rowc");
  app->styleSheet().addRule(rowContentsWidthRule_);

  c0WidthRule_ = new WCssTemplateRule("#" + formName() + " .c0w");
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
  headerContainer_->setStyleClass("header cwidth");
  headers_ = new WContainerWidget(headerContainer_);
  headers_->setStyleClass("header-div headerrh");
  headers_->setAttributeValue("unselectable", "on");

  headerHeightRule_ = new WCssTemplateRule("#" + formName() + " .headerrh");
  app->styleSheet().addRule(headerHeightRule_);
  setHeaderHeight(headerHeight_);

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
     ""  "if (t.className === 'selected')"
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
     ""  "var parent = obj.parentNode,"
     ""      "diffx = Math.max(nowxy.x - lastx, -parent.offsetWidth),"
     ""      "c = parent.className.split(' ')[2];"
     ""  "if (c) {"
     ""    "var r = WT.getCssRule('#" + formName() + " .' + c),"
     ""        "tw = WT.pxself(r, 'width');"
     ""    "if (tw == 0) tw = parent.offsetWidth;" 
     ""    "r.style.width = (tw + diffx) + 'px';"
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
       "" WT_CLASS ".getCssRule('#" + formName() + " .Wt-tv-rowc').style.left"
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

  app->addAutoJavaScript
    ("{var e=" + contentsContainer_->jsRef() + ";"
     "var s=" + jsRef() + ";"
     "var WT=" WT_CLASS ";"
     "if (e) {"
     """var tw=s.offsetWidth-WT.px(s, 'borderLeftWidth')"
     ""       "-WT.px(s, 'borderRightWidth');"
     ""
     """if (tw > 200) {" // XXX: IE's incremental rendering foobars completely
     ""  "var h= " + headers_->jsRef() + ", "
     ""      "hh= h.firstChild, "
     ""      "t= " + contents_->jsRef() + ".firstChild, "
     ""      "r= WT.getCssRule('#" + formName() + " .cwidth'), "
     ""      "vscroll=e.scrollHeight > e.offsetHeight,"
     ""      "contentstoo=(r.style.width == h.style.width);"
     ""  "if (vscroll) {"
     ""    "r.style.width=(tw - 17) + 'px';"
     ""  "} else {"
     ""    "r.style.width=tw + 'px';"
     ""  "}"
     ""  "e.style.width=tw + 'px';"
     ""  "h.style.width=t.offsetWidth + 'px';"
     ""  "if (s.className.indexOf('column1') != -1) {"
     ""    "var r=WT.getCssRule('#" + formName() + " .c0w'),"
     ""        "hh=h.firstChild,"
     ""        "w=tw - WT.pxself(r, 'width') - (vscroll ? 17 : 0);"
     ""    "WT.getCssRule('#" + formName() + " .Wt-tv-row').style.width"
     ""       "= w + 'px';"
     ""    "var extra = " WT_CLASS ".hasTag(hh.childNodes[1], 'IMG') ? 21 : 6;"
     ""    "hh.style.width= (w + extra) + 'px';"
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

  renderState_ = RenderOk;

  if (parent)
    parent->addWidget(this);
}

void WTreeView::refresh()
{
  WApplication *app = WApplication::instance();

  std::string columnsWidth = std::string() +
    "var WT=" WT_CLASS ","
    ""  "t=" + contents_->jsRef() + ".firstChild,"
    ""  "h=" + headers_->jsRef() + ","
    ""  "hh=h.firstChild,"
    ""  "hc=hh.firstChild" + (column1Fixed_ ? ".firstChild" : "") + ","
    ""  "totalw=0,"
    ""  "extra=" + (column1Fixed_ ? "1" : "4")
               + " + (WT.hasTag(hh.childNodes[1], 'IMG') ? 17 : 6);"
    "if(" + jsRef() + ".offsetWidth == 0) return;"
    "for (var i=0, length=hc.childNodes.length; i < length; ++i) {"
    """var cl = hc.childNodes[i].className.split(' ')[2],"
    ""    "r = WT.getCssRule('#" + formName() + " .' + cl);"
    """totalw += WT.pxself(r, 'width') + 7;" // 2 x 3px (padding) + 1px (border)
    "}"
    "var cw = WT.pxself(hh, 'width'),"
    ""  "hdiff = c ? (cw == 0 ? 0 : (totalw - (cw - extra))) : diffx;";
  if (!column1Fixed_)
    columnsWidth +=
      "t.style.width = (t.offsetWidth + hdiff) + 'px';"
      "h.style.width = t.offsetWidth + 'px';"
      "hh.style.width = (totalw + extra) + 'px';";
  else
    columnsWidth +=
      "var r = WT.getCssRule('#" + formName() + " '"
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
     ""    "object.className = 'valid-drop';"
     ""    "self.dropEl = item.el;"
     ""    "self.dropEl.classNameOrig = self.dropEl.className;"
     ""    "self.dropEl.className = self.dropEl.className + ' drop-site';"
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

    setStyleClass("Wt-treeview column1 unselectable");
    WContainerWidget *rootWrap
      = dynamic_cast<WContainerWidget *>(contents_->widget(0));
    rootWrap->resize(WLength(100, WLength::Percentage), WLength::Auto);
    rootWrap->setOverflow(WContainerWidget::OverflowHidden);

    // needed for IE, otherwise row expands automatically to content width
    rowWidthRule_->templateWidget()->resize(0, WLength::Auto);

    WContainerWidget *scrollBarContainer = new WContainerWidget();
    scrollBarContainer->setStyleClass("cwidth");
    scrollBarContainer->resize(WLength::Auto, 16);
    WContainerWidget *scrollBarC = new WContainerWidget(scrollBarContainer);
    scrollBarC->setStyleClass("Wt-tv-row Wt-scroll");
    scrollBarC->scrolled().connect(tieRowsScrollJS_);
    WContainerWidget *scrollBar = new WContainerWidget(scrollBarC);
    scrollBar->setStyleClass("Wt-tv-rowc");
    WApplication *app = WApplication::instance();
    if (app->environment().agentIsWebKit() || app->environment().agentIsOpera())
      scrollBar->setAttributeValue("style", "left: 0px;");
    impl_->layout()->addWidget(scrollBarContainer);
  }
}

void WTreeView::load()
{
  WCompositeWidget::load();
}

WTreeView::~WTreeView()
{ 
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

void WTreeView::setColumnFormat(int column, const WT_USTRING& format)
{
  columnInfo(column).format = format;
}

WT_USTRING WTreeView::columnFormat(int column) const
{
  return columnInfo(column).format;
}

void WTreeView::setColumnWidth(int column, const WLength& width)
{
  columnInfo(column).width = width;

  if (column != 0)
    columnInfo(column).styleRule->templateWidget()
      ->resize(width, WLength::Auto);
  else
    c0WidthRule_->templateWidget()
      ->resize(width.toPixels(), WLength::Auto);
}

WLength WTreeView::columnWidth(int column) const
{
  return columnInfo(column).width;
}

void WTreeView::setColumnAlignment(int column, AlignmentFlag alignment)
{
  static const char *cssText[] = { "left", "right", "center", "justify" };
  // in jwt branch we will be able to set alignment

  WWidget *w = columnInfo(column).styleRule->templateWidget();

  if (column != 0)
    w->setAttributeValue("style",
			 std::string("text-align: ") + cssText[alignment - 1]);
  else
    if (alignment == AlignRight) {
#ifndef WT_TARGET_JAVA
      w->setFloatSide(Right);
#else
      w->setAttributeValue("style", "float: right;");
#endif // WT_TARGET_JAVA
    }
}

AlignmentFlag WTreeView::columnAlignment(int column) const
{
  return columnInfo(column).alignment;
}

void WTreeView::setHeaderAlignment(int column, AlignmentFlag alignment)
{
  columnInfo(column).headerAlignment = alignment;

  if (renderState_ == NeedRerender)
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
    = new WCssTextRule(".Wt-treeview .Wt-tv-row .Wt-tv-c, "
		       ".Wt-treeview .header .Wt-tv-row, "
		       ".Wt-treeview .Wt-tv-node .Wt-tv-row",
		       "border-color: " + color.cssText());
  WApplication::instance()->styleSheet().addRule(borderColorRule_);
}

void WTreeView::setColumnResizeEnabled(bool enabled)
{
  if (enabled != columnResize_) {
    columnResize_ = enabled;
    rerenderHeader();
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
      (imagePack_ + "stripes/stripe-"
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
    needAdjustToViewport();
}

void WTreeView::setHeaderHeight(const WLength& height, bool multiLine)
{
  headerHeight_ = height;
  multiLineHeader_ = multiLine;

  headerHeightRule_->templateWidget()->resize(WLength::Auto, headerHeight_);
  if (!multiLineHeader_)
    headerHeightRule_->templateWidget()->setLineHeight(headerHeight_);
  else
    headerHeightRule_->templateWidget()->setLineHeight(WLength::Auto);

  headers_->resize(headers_->width(), headerHeight_);
  headerContainer_->resize(WLength::Auto, headerHeight_);

  if (renderState_ == NeedRerender)
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

    needAdjustToViewport();
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

  renderState_ = NeedRerender;
  askRerender();
}

void WTreeView::setRootIndex(const WModelIndex& rootIndex)
{
  if (rootIndex != rootIndex_) {
    rootIndex_ = rootIndex;

    if (model_) {
      renderState_ = std::max(renderState_, NeedRerenderTree);
      askRerender();
    }
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

void WTreeView::render()
{
  while (renderState_ != RenderOk) {
    RenderState s = renderState_;
    renderState_ = RenderOk;

    switch (s) {
    case NeedRerender:
      initLayoutJavaScript();
      rerenderHeader();
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

  WCompositeWidget::render();
}

void WTreeView::needAdjustToViewport()
{
  renderState_ = std::max(renderState_, NeedAdjustViewPort);
  askRerender();
}

void WTreeView::rerenderHeader()
{
  for (int i = 0; i < columnCount(); ++i) {
    WWidget *w = columnInfo(i).extraHeaderWidget;
    if (!w)
      columnInfo(i).extraHeaderWidget = createExtraHeaderWidget(i);
    else
      dynamic_cast<WContainerWidget *>(w->parent())->removeWidget(w);
  }

  headers_->clear();

  /* cols 1.. */
  WContainerWidget *rowc = new WContainerWidget(headers_);
#ifndef WT_TARGET_JAVA
  rowc->setFloatSide(Right);
#else
  rowc->setAttributeValue("style", "float: right;");
#endif // WT_TARGET_JAVA
  WContainerWidget *row = new WContainerWidget(rowc);
  row->setStyleClass("Wt-tv-row headerrh");
  row->setAttributeValue("unselectable", "on");

  if (column1Fixed_) {
    row = new WContainerWidget(row);
    row->setStyleClass("Wt-tv-rowc");
  }

  /* sort and resize handles for col 0 */
  if (columnInfo(0).sorting) {
    WImage *sortIcon = new WImage(rowc);
      sortIcon->setStyleClass(columnResize_ ? "Wt-tv-sh Wt-tv-shc0"
			      : "Wt-tv-sh-nrh Wt-tv-shc0");
    sortIcon->setImageRef(imagePack_ + "sort-arrow-none.gif");
    clickedForSortMapper_->mapConnect(sortIcon->clicked(), 0);
  }

  if (columnResize_) {
    WContainerWidget *resizeHandle = new WContainerWidget(rowc);
    resizeHandle->setStyleClass("Wt-tv-rh headerrh Wt-tv-rhc0");
    resizeHandle->mouseWentDown().connect(resizeHandleMDownJS_);
    resizeHandle->mouseWentUp().connect(resizeHandleMUpJS_);
    resizeHandle->mouseMoved().connect(resizeHandleMMovedJS_);
  }

  WApplication *app = WApplication::instance();
  for (int i = 1; i < columnCount(); ++i) {
    WWidget *w = createHeaderWidget(app, i);
    row->addWidget(w);
  }

  /* col 0 */
  WText *t = new WText("&nbsp;");
  if (columnCount() > 0)
    if (!multiLineHeader_)
      t->setStyleClass(columnStyleClass(0) + " headerrh Wt-label");
    else
      t->setStyleClass(columnStyleClass(0) + " Wt-label");
  t->setInline(false);
  t->setAttributeValue("unselectable", "on");
  t->setAttributeValue("style", "float: none; margin: 0px auto;"
		       "padding-left: 6px;");

  if (columnInfo(0).extraHeaderWidget) {
    WContainerWidget *c = new WContainerWidget(headers_);
    c->setInline(true); // For IE7
    c->addWidget(t);
    c->addWidget(columnInfo(0).extraHeaderWidget);
  } else
    headers_->addWidget(t);

  if (model_)
    modelHeaderDataChanged(Horizontal, 0, columnCount() - 1);
}

WWidget *WTreeView::createHeaderWidget(WApplication *app, int column)
{
  ColumnInfo& info = columnInfo(column);
  WContainerWidget *w = new WContainerWidget();
  w->setStyleClass("Wt-tv-c headerrh " + info.styleClass());
  w->setContentAlignment(info.headerAlignment);

  if (columnResize_) {
    WContainerWidget *resizeHandle = new WContainerWidget(w);
    resizeHandle->setStyleClass("Wt-tv-rh headerrh");
    resizeHandle->mouseWentDown().connect(resizeHandleMDownJS_);
    resizeHandle->mouseWentUp().connect(resizeHandleMUpJS_);
    resizeHandle->mouseMoved().connect(resizeHandleMMovedJS_);
  }

  if (info.sorting) {
    WImage *sortIcon = new WImage(w);
    sortIcon->setStyleClass(columnResize_ ? "Wt-tv-sh" : "Wt-tv-sh-nrh");
    sortIcon->setImageRef(imagePack_ + "sort-arrow-none.gif");
    clickedForSortMapper_->mapConnect(sortIcon->clicked(), info.id);
  }

  WText *t = new WText("&nbsp;", w);
  t->setStyleClass("Wt-label");
  t->setInline(false);
  t->setAttributeValue("unselectable", "on");
  if (multiLineHeader_ || app->environment().agentIsIE())
    t->setWordWrap(true);
  else
    t->setWordWrap(false);

  if (columnInfo(column).extraHeaderWidget)
    w->addWidget(columnInfo(column).extraHeaderWidget);

  return w;
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
    if (mouseWentDown_.isConnected())
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

  needAdjustToViewport();
}

void WTreeView::onCheckedChange(CheckedInfo info)
{
  if (info.checkBox->isTristate())
    model_->setData(info.index, boost::any(info.checkBox->checkState()),
		    CheckStateRole);
  else
    model_->setData(info.index, boost::any(info.checkBox->isChecked()),
		    CheckStateRole);
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
    if (i->second->formName() == nodeId) {
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
      acceptDrops(acceptMimeTypes[i], "drop-site");
    else
      stopAcceptDrops(acceptMimeTypes[i]);
}

void WTreeView::setDragEnabled(bool enable)
{
  if (dragEnabled_ != enable) {
    dragEnabled_ = enable;

    if (enable) {
      dragWidget_ = new WText(headerContainer_);
      dragWidget_->setId(formName() + "dw");
      dragWidget_->setInline(false);
      dragWidget_->hide();
      setAttributeValue("dwid", dragWidget_->formName());

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

    if (start == 0)
      rerenderHeader();
    else {
      WContainerWidget *row = headerRow();

      for (int i = start; i < start + count; ++i)
	row->insertWidget(i - 1, createHeaderWidget(app, i));
    }
  }

  if (start == 0) {
    renderState_ = std::max(renderState_, NeedRerenderTree);
    askRerender();
  } else {
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

void WTreeView::modelColumnsRemoved(const WModelIndex& parent,
				    int start, int end)
{
  int count = end - start + 1;
  if (!parent.isValid()) {
    columns_.erase(columns_.begin() + start, columns_.begin() + start + count);

    if (start == 0)
      rerenderHeader();
    else {
      for (int i = start; i < start + count; ++i)
	delete headerWidget(start);
    }
  }

  if (start == 0) {
    renderState_ = std::max(renderState_, NeedRerenderTree);
    askRerender();
  } else {
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
	    = firstRenderedRow_ + validRowCount_
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
  if (orientation == Horizontal) {
    for (int i = start; i <= end; ++i) {
      WString label = asString(model_->headerData(i));
      headerTextWidget(i)->setText(label);
    }
  }
}

WImage *WTreeView::headerSortIconWidget(int column)
{
  if (!columnInfo(column).sorting)
    return 0;

  if (column == 0) {
    WContainerWidget *row
      = dynamic_cast<WContainerWidget *>(headers_->widget(0));

    return dynamic_cast<WImage *>(row->widget(1));
  } else {
    WContainerWidget *w
      = dynamic_cast<WContainerWidget *>(headerWidget(column));

    return dynamic_cast<WImage *>(w->widget(columnResize_ ? 1 : 0));
  }
}

WText *WTreeView::headerTextWidget(int column)
{
  WWidget *w = headerWidget(column);

  WText *result = dynamic_cast<WText *>(w); // for column 0 without extra
  if (result)
    return result;
  else {
    // for columns 1 - n or column 0 with extra widget
    WContainerWidget *wc = dynamic_cast<WContainerWidget *>(w);
    int fromLast = columnInfo(column).extraHeaderWidget ? 1 : 0;
    return dynamic_cast<WText *>(wc->widget(wc->count() - 1 - fromLast));
  }
}

WWidget *WTreeView::headerWidget(int column)
{
  if (column == 0)
    return headers_->widget(headers_->count() - 1);
  else {
    return headerRow()->widget(column - 1);
  }
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

  renderState_ = std::max(renderState_, NeedAdjustViewPort);
  askRerender();
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
}

int WTreeView::adjustRenderedNode(WTreeViewNode *node, int theNodeRow)
{
  WModelIndex index = node->modelIndex();

  if (index != rootIndex_)
    ++theNodeRow;

  // if a node has children loaded but is not currently expanded, then still
  // adjust it, but do not increase nodeRow
  int dummyNodeRow = theNodeRow;

  int *nodeRow;
  if (!isExpanded(index))
    if (node->childrenLoaded())
      nodeRow = &dummyNodeRow;
    else
      return theNodeRow;
  else
    nodeRow = &theNodeRow;

  if (node->isAllSpacer()) {
    if (*nodeRow + node->childrenHeight() > firstRenderedRow_
	&& *nodeRow < firstRenderedRow_ + validRowCount_) {
      // replace spacer by some nodes
      int childCount = model_->rowCount(index);

      bool firstNode = true;
      int rowStubs = 0;

      for (int i = 0; i < childCount; ++i) {
	WModelIndex childIndex = model_->index(i, 0, index);

	int childHeight = subTreeHeight(childIndex);

	if (*nodeRow <= firstRenderedRow_ + validRowCount_
	    && *nodeRow + childHeight > firstRenderedRow_) {
	  if (firstNode) {
	    firstNode = false;
	    node->setTopSpacerHeight(rowStubs);
	    rowStubs = 0;
	  }

	  WTreeViewNode *n = new WTreeViewNode(this, childIndex,
					       childHeight - 1,
					       i == childCount - 1, node);
	  node->childContainer()->addWidget(n);

	  int nestedNodeRow = *nodeRow;
	  nestedNodeRow = adjustRenderedNode(n, nestedNodeRow);
	  assert(nestedNodeRow == *nodeRow + childHeight);
	} else {
	  rowStubs += childHeight;
	}
	*nodeRow += childHeight;
      }
      node->setBottomSpacerHeight(rowStubs);
    } else
      *nodeRow += node->childrenHeight();
  } else {
    // get a reference to the first existing child, which we'll recursively
    // adjust later
    int topSpacerHeight = node->topSpacerHeight();
    int nestedNodeRow = *nodeRow + topSpacerHeight;
    WTreeViewNode *child = node->nextChildNode(0);

    int childCount = model_->rowCount(index);
    while (topSpacerHeight != 0
	   && *nodeRow + topSpacerHeight > firstRenderedRow_) {
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

      nestedNodeRow = *nodeRow + topSpacerHeight - childHeight;
      nestedNodeRow = adjustRenderedNode(n, nestedNodeRow);
      assert(nestedNodeRow == *nodeRow + topSpacerHeight);

      topSpacerHeight -= childHeight;
      node->addTopSpacerHeight(-childHeight);
    }

    for (; child; child=node->nextChildNode(child))
      nestedNodeRow = adjustRenderedNode(child, nestedNodeRow);

    int nch = node->childrenHeight();
    int bottomSpacerStart = nch - node->bottomSpacerHeight();

    while (node->bottomSpacerHeight() != 0
	   && *nodeRow + bottomSpacerStart
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

      nestedNodeRow = *nodeRow + bottomSpacerStart;
      nestedNodeRow = adjustRenderedNode(n, nestedNodeRow);
      assert(nestedNodeRow == *nodeRow + bottomSpacerStart + childHeight);

      node->addBottomSpacerHeight(-childHeight);
      bottomSpacerStart += childHeight;
    }

    *nodeRow += nch;
  }

  return theNodeRow;
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
      set.erase(it);
    } else if (count < 0) {
      // delete indexes that are about to be deleted, if they are within
      // the range of deleted indexes
      do {
	if (p.parent() == parent
	    && p.row() >= start
	    && p.row() < start - count) {
	  set.erase(it);
	  break;
	} else
	  p = p.parent();
      } while (p != parent);
    }

#ifndef WT_TARGET_JAVA
    it = n;
#endif
  }

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
      ->setImageRef(imagePack_ + "sort-arrow-none.gif");

  currentSortColumn_ = column;
  columnInfo(column).sortOrder = order;

  if (renderState_ != NeedRerender)
    headerSortIconWidget(currentSortColumn_)
      ->setImageRef(imagePack_ + (order == AscendingOrder ? "sort-arrow-up.gif"
				  : "sort-arrow-down.gif"));

  model_->sort(column, order);
}

void WTreeView::setSortingEnabled(bool enabled)
{
  sorting_ = enabled;
  for (int i = 0; i < columnCount(); ++i)
    columnInfo(i).sorting = enabled;

  rerenderHeader();
}

void WTreeView::setSortingEnabled(int column, bool enabled)
{
  columnInfo(column).sorting = enabled;

  rerenderHeader();
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

  renderState_ = std::max(renderState_, NeedRerenderTree);
  askRerender();
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
  WModelIndexSet nodes = selectionModel_->selection_;

  for (WModelIndexSet::iterator i = nodes.begin(); i != nodes.end(); ++i)
    internalSelect(*i, Deselect);
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
