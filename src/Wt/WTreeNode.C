/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Utils.h"

#include "Wt/WApplication"
#include "Wt/WCssDecorationStyle"
#include "Wt/WEnvironment"
#include "Wt/WIconPair"
#include "Wt/WImage"
#include "Wt/WTable"
#include "Wt/WTableCell"
#include "Wt/WText"
#include "Wt/WTreeNode"

namespace Wt {

const char *WTreeNode::imagePlus_ = "nav-plus.gif";
const char *WTreeNode::imageMin_ = "nav-minus.gif";

WTreeNode::WTreeNode(const WString& labelText,
		     WIconPair *labelIcon, WTreeNode *parent)
  : collapsed_(true),
    selectable_(true),
    visible_(true),
    childrenDecorated_(true),
    parentNode_(0),
    childCountPolicy_(Disabled),
    labelIcon_(labelIcon),
    labelText_(new WText(labelText)),
    childrenLoaded_(false),
    populated_(false),
    interactive_(true),
    selected_(this)
{
  create();

  if (parent)
    parent->addChildNode(this);
}

WTreeNode::WTreeNode(WTreeNode *parent)
  : collapsed_(true),
    selectable_(true),
    visible_(true),
    childrenDecorated_(true),
    parentNode_(0),
    childCountPolicy_(Disabled),
    labelIcon_(0),
    labelText_(0),
    childrenLoaded_(false),
    populated_(false),
    interactive_(true),
    selected_(this)
{
  create();

  if (parent)
    parent->addChildNode(this);
}

EventSignal<WMouseEvent>& WTreeNode::expanded()
{
  return expandIcon_->icon1Clicked();
}

EventSignal<WMouseEvent>& WTreeNode::collapsed()
{
  return expandIcon_->icon2Clicked();
}

int WTreeNode::displayedChildCount() const
{
  return childNodes_.size();
}

void WTreeNode::setSelectable(bool selectable)
{
  selectable_ = selectable;
}

void WTreeNode::setInteractive(bool interactive)
{
  interactive_ = interactive;
}

WTableCell *WTreeNode::labelArea()
{
  return layout_->elementAt(0, 1);
}

WTreeNode::~WTreeNode()
{
  for (unsigned i = 0; i < childNodes_.size(); ++i)
    delete childNodes_[i];

  // also delete these two as only one of them is inserted in the
  // widget hierarchy at any time
  delete noExpandIcon_;
  delete expandIcon_;
}

void WTreeNode::populate()
{ }

bool WTreeNode::doPopulate()
{
  if (!populated_) {
    populated_ = true;
    populate();

    return true;
  } else
    return false;
}

bool WTreeNode::expandable()
{
  if (interactive_) {
    doPopulate();

    return !childNodes_.empty();
  } else
    return false;
}

void WTreeNode::setImagePack(const std::string& url)
{ }

std::string WTreeNode::imagePack() const
{
  return "";
}

void WTreeNode::setNodeVisible(bool visible)
{
  visible_ = visible;
  updateChildren(false);
}

void WTreeNode::setChildrenDecorated(bool decorated)
{
  childrenDecorated_ = decorated;
  updateChildren(false);
}

void WTreeNode::create()
{
  setImplementation(layout_ = new WTable());

  setStyleClass("Wt-tree");
  layout_->setSelectable(false);

  implementStateless(&WTreeNode::doExpand, &WTreeNode::undoDoExpand);
  implementStateless(&WTreeNode::doCollapse, &WTreeNode::undoDoCollapse);

  WApplication *app = WApplication::instance();

  expandIcon_
    = new WIconPair(WApplication::resourcesUrl() + "themes/" + app->cssTheme()
		    + "/" + imagePlus_,
		    WApplication::resourcesUrl() + "themes/" + app->cssTheme()
		    + "/" + imageMin_);
  noExpandIcon_ = new WText();
  noExpandIcon_->setStyleClass("Wt-noexpand");

  layout_->rowAt(1)->hide();

  if (labelText_)
    // "treenodelabel" is for backwards compatibility with Wt < 3.1.1
    labelText_->setStyleClass("Wt-label treenodelabel");

  childCountLabel_ = 0;

  layout_->elementAt(0, 0)->setStyleClass("Wt-trunk");
  layout_->elementAt(0, 0)->addWidget(noExpandIcon_);

  if (labelIcon_) {
    layout_->elementAt(0, 1)->addWidget(labelIcon_);
    labelIcon_->setVerticalAlignment(AlignMiddle);
  }

  if (labelText_)
    layout_->elementAt(0, 1)->addWidget(labelText_);

  layout_->elementAt(0, 0)->setContentAlignment(AlignLeft | AlignTop);
  layout_->elementAt(0, 1)->setContentAlignment(AlignLeft | AlignMiddle);

  layout_->rowAt(0)->setStyleClass("Wt-node");

  childrenLoaded_ = false;

  setLoadPolicy(LazyLoading);
}

void WTreeNode::setChildCountPolicy(ChildCountPolicy policy)
{
  if (policy != Disabled && !childCountLabel_) {
    childCountLabel_ = new WText();
    childCountLabel_->setMargin(WLength(7), Left);
    // "treenodechildcount" is for backwards compatibility.
    childCountLabel_->setStyleClass("Wt-childcount treenodechildcount");

    layout_->elementAt(0, 1)->addWidget(childCountLabel_);
  }

  childCountPolicy_ = policy;

  if (childCountPolicy_ == Enabled) {
    WTreeNode *parent = parentNode();
 
    if (parent && parent->isExpanded())
      if (doPopulate())
	update();
  }

  if (childCountPolicy_ != Disabled) {
    for (unsigned i = 0; i < childNodes_.size(); ++i)
      childNodes_[i]->setChildCountPolicy(childCountPolicy_);
  }
}

void WTreeNode::setLoadPolicy(LoadPolicy loadPolicy)
{
  loadPolicy_ = loadPolicy;

  switch (loadPolicy) {
  case PreLoading:
    loadChildren();

    break;
  case NextLevelLoading:
    if (isExpanded()) {
      loadChildren();
      loadGrandChildren();
    } else {
      WTreeNode *parent = parentNode();
      if (parent && parent->isExpanded())
	loadChildren();

      expandIcon_
	->icon1Clicked().connect(this, &WTreeNode::loadGrandChildren);
    }
    break;
  case LazyLoading:
    if (isExpanded())
      loadChildren();
    else {
      if (childCountPolicy_ == Enabled) {
	WTreeNode *parent = parentNode();
	if (parent && parent->isExpanded())
	  doPopulate();
      }

      expandIcon_->icon1Clicked().connect(this, &WTreeNode::expand);
    }
  }

  if (loadPolicy_ != LazyLoading) {
    for (unsigned i = 0; i < childNodes_.size(); ++i)
      childNodes_[i]->setLoadPolicy(loadPolicy_);
  }
}

void WTreeNode::loadChildren()
{
  if (!childrenLoaded_) {
    doPopulate();

    for (unsigned i = 0; i < childNodes_.size(); ++i)
      layout_->elementAt(1, 1)->addWidget(childNodes_[i]);

    expandIcon_->icon1Clicked().connect(this, &WTreeNode::doExpand);
    expandIcon_->icon2Clicked().connect(this, &WTreeNode::doCollapse);

    resetLearnedSlots();

    childrenLoaded_ = true;
  }
}

void WTreeNode::loadGrandChildren()
{
  for (unsigned i = 0; i < childNodes_.size(); ++i)
    childNodes_[i]->loadChildren();
}

bool WTreeNode::isLastChildNode() const
{
  WTreeNode *parent = parentNode();

  if (parent)
    return parent->childNodes_.back() == this;
  else
    return true;
}

void WTreeNode::descendantAdded(WTreeNode *node)
{
  WTreeNode *parent = parentNode();

  if (parent)
    parent->descendantAdded(node);  
}

void WTreeNode::descendantRemoved(WTreeNode *node)
{
  WTreeNode *parent = parentNode();

  if (parent)
    parent->descendantRemoved(node);
}

void WTreeNode::addChildNode(WTreeNode *node)
{
  insertChildNode(childNodes_.size(), node);
}

void WTreeNode::insertChildNode(int index, WTreeNode *node)
{
  childNodes_.insert(childNodes_.begin() + index, node);
  node->parentNode_ = this;

  if (childrenLoaded_)
    layout_->elementAt(1, 1)->insertWidget(index, node);
  else
    node->setParent(0); // because node->hasParent() has Changed

  descendantAdded(node);

  if (loadPolicy_ != node->loadPolicy_)
    node->setLoadPolicy(loadPolicy_);

  if (childCountPolicy_ != node->childCountPolicy_)
    node->setChildCountPolicy(childCountPolicy_);

  /*
   * If newly inserted node is last, then previous last node needs to
   * be updated.
   */
  if (index == (int)childNodes_.size() - 1 && childNodes_.size() > 1)
    childNodes_[childNodes_.size() - 2]->update();

  node->update();
  update();
  resetLearnedSlots();
}

void WTreeNode::removeChildNode(WTreeNode *node)
{
  Utils::erase(childNodes_, node);
  node->parentNode_ = 0;

  if (childrenLoaded_)
    layout_->elementAt(1, 1)->removeWidget(node);

  descendantRemoved(node);

  updateChildren();
}

void WTreeNode::updateChildren(bool recursive)
{
  for (unsigned i = 0; i < childNodes_.size(); ++i)
    if (recursive)
      childNodes_[i]->updateChildren(recursive);
    else
      childNodes_[i]->update();

  update();

  resetLearnedSlots();
}

bool WTreeNode::isExpanded() const
{
  return !collapsed_;
}

void WTreeNode::expand()
{
  if (!isExpanded()) {
    if (!childrenLoaded_) {
      loadChildren();
    }

    /*
     * Happens if expandable() for an unpopulated node returned true,
     * but after populate(), there were no children: update the node to
     * reflect that in fact this node cannot be expanded after all
     */
    if (parentNode() && childNodes_.empty()) {
      parentNode()->resetLearnedSlots();
      update();
      return;
    }

    if (loadPolicy_ == NextLevelLoading)
      loadGrandChildren();

    doExpand();

    updateChildren();
  }
}

void WTreeNode::collapse()
{
  if (isExpanded())
    doCollapse();
}

void WTreeNode::doExpand()
{
  wasCollapsed_ = !isExpanded();
  collapsed_ = false;

  if (!childNodes_.empty()) {
    expandIcon_->setState(1);
    layout_->rowAt(1)->show();

    if (labelIcon_)
      labelIcon_->setState(1);
  }

  /*
   * collapse all children
   */
  for (unsigned i = 0; i < childNodes_.size(); ++i)
    childNodes_[i]->doCollapse();
}

void WTreeNode::doCollapse()
{
  wasCollapsed_ = !isExpanded();
  collapsed_ = true;

  expandIcon_->setState(0);
  layout_->rowAt(1)->hide();

  if (labelIcon_)
    labelIcon_->setState(0);
}

void WTreeNode::undoDoCollapse()
{
  if (!wasCollapsed_) {
    // re-expand
    expandIcon_->setState(1);
    layout_->rowAt(1)->show();
    if (labelIcon_)
      labelIcon_->setState(1);
    collapsed_ = false;
  }
}

void WTreeNode::undoDoExpand()
{
  if (wasCollapsed_) {
    // re-collapse
    expandIcon_->setState(0);
    layout_->rowAt(1)->hide();
    if (labelIcon_)
      labelIcon_->setState(0);

    collapsed_ = true;
  }

  /*
   * undo collapse of children
   */
  for (unsigned i = 0; i < childNodes_.size(); ++i)
    childNodes_[i]->undoDoCollapse();  
}

void WTreeNode::setLabelIcon(WIconPair *labelIcon)
{
  delete labelIcon_;
  labelIcon_ = labelIcon;

  if (labelIcon_) {
    if (labelText_)
      layout_->elementAt(0, 1)->insertBefore(labelIcon_, labelText_);
    else
      layout_->elementAt(0, 1)->addWidget(labelIcon_);

    labelIcon_->setState(isExpanded() ? 1 : 0);
  }
}

void WTreeNode::renderSelected(bool isSelected)
{
  layout_->rowAt(0)->setStyleClass(isSelected ? "Wt-selected selected" : "");
  selected().emit(isSelected);
}

void WTreeNode::update()
{
  bool isLast = isLastChildNode();

  if (!visible_) {
    layout_->rowAt(0)->hide();
    expandIcon_->hide();
    layout_->elementAt(0, 0)->resize(0, WLength::Auto);
    layout_->elementAt(1, 0)->resize(0, WLength::Auto);
  }

  WTreeNode *parent = parentNode();
  if (parent && !parent->childrenDecorated_) {
    layout_->elementAt(0, 0)->hide();
    layout_->elementAt(1, 0)->hide();
  }

  if (expandIcon_->state() != (isExpanded() ? 1 : 0))
    expandIcon_->setState(isExpanded() ? 1 : 0);
  if (layout_->rowAt(1)->isHidden() != !isExpanded())
    layout_->rowAt(1)->setHidden(!isExpanded());
  if (labelIcon_ && (labelIcon_->state() != (isExpanded() ? 1 : 0)))
    labelIcon_->setState(isExpanded() ? 1 : 0);

  if (isLast) {
    layout_->elementAt(0, 0)->setStyleClass("Wt-end");
    layout_->elementAt(1, 0)->setStyleClass("");
  } else {
    layout_->elementAt(0, 0)->setStyleClass("Wt-trunk");
    layout_->elementAt(1, 0)->setStyleClass("Wt-trunk");
  }

  if (!parentNode() || parentNode()->isExpanded()) {
    if (childCountPolicy_ == Enabled && !populated_)
      doPopulate();

    if (!expandable()) {
      if (noExpandIcon_->parent() == 0) {
	layout_->elementAt(0, 0)->addWidget(noExpandIcon_);
	layout_->elementAt(0, 0)->removeWidget(expandIcon_);
      }
    } else {
      if (expandIcon_->parent() == 0) {
	layout_->elementAt(0, 0)->addWidget(expandIcon_);
	layout_->elementAt(0, 0)->removeWidget(noExpandIcon_);
      }
    }
  }

  if (childCountPolicy_ != Disabled && populated_ && childCountLabel_) {
    int n = displayedChildCount();
    if (n)
      childCountLabel_->setText
	(WString::fromUTF8("(" + boost::lexical_cast<std::string>(n) + ")"));
    else
      childCountLabel_->setText(WString());
  }
}

#ifndef WT_TARGET_JAVA
bool WTreeNode::hasParent() const
{
  if (parentNode_)
    return true;
  else
    return WCompositeWidget::hasParent();
}
#endif //WT_TARGET_JAVA

}
