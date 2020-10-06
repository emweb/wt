/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "WebUtils.h"

#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WEnvironment.h"
#include "Wt/WIconPair.h"
#include "Wt/WText.h"
#include "Wt/WTemplate.h"
#include "Wt/WTheme.h"
#include "Wt/WTree.h"
#include "Wt/WTreeNode.h"

namespace Wt {

const char *WTreeNode::imagePlus_ = "nav-plus.gif";
const char *WTreeNode::imageMin_ = "nav-minus.gif";
const char *WTreeNode::imagePlusRtl_ = "nav-plus-rtl.gif";
const char *WTreeNode::imageMinRtl_ = "nav-minus-rtl.gif";

WTreeNode::WTreeNode(const WString& labelText,
		     std::unique_ptr<WIconPair> labelIcon)
  : collapsed_(true),
    selectable_(true),
    visible_(true),
    childrenDecorated_(true),
    parentNode_(nullptr),
    childCountPolicy_(Disabled),
    labelIcon_(labelIcon.get()),
    childrenLoaded_(false),
    populated_(false),
    interactive_(true)
{
  init(labelText, std::move(labelIcon));
}

WTreeNode::WTreeNode()
  : collapsed_(true),
    selectable_(true),
    visible_(true),
    childrenDecorated_(true),
    parentNode_(nullptr),
    childCountPolicy_(Disabled),
    labelIcon_(nullptr),
    childrenLoaded_(false),
    populated_(false),
    interactive_(true)
{
  init(WString::Empty, nullptr);
}

void WTreeNode::init(const WString &labelText, std::unique_ptr<WIconPair> labelIcon)
{
  setImplementation(std::unique_ptr<WTemplate>(layout_ = new WTemplate(tr("Wt.WTreeNode.template"))));
  setStyleClass("Wt-tree");
  layout_->setSelectable(false);

  layout_->bindEmpty("cols-row");
  layout_->bindEmpty("trunk-class");

  implementStateless(&WTreeNode::doExpand, &WTreeNode::undoDoExpand);
  implementStateless(&WTreeNode::doCollapse, &WTreeNode::undoDoCollapse);

  WApplication *app = WApplication::instance();

  /*
   * Children
   */
  WContainerWidget *children 
    = layout_->bindWidget("children", std::make_unique<WContainerWidget>());
  children->setList(true);
  children->hide();

  /*
   * Expand icon
   */
  if (WApplication::instance()->layoutDirection() == 
      LayoutDirection::RightToLeft)
    expandIcon_ = layout_->bindWidget
      ("expand",
       std::make_unique<WIconPair>
       (app->theme()->resourcesUrl() + imagePlusRtl_,
	app->theme()->resourcesUrl() + imageMinRtl_));
  else
    expandIcon_ = layout_->bindWidget
      ("expand",
       std::make_unique<WIconPair>
       (app->theme()->resourcesUrl() + imagePlus_,
	app->theme()->resourcesUrl() + imageMin_));

  expandIcon_->setStyleClass("Wt-ctrl Wt-expand");
  expandIcon_->hide();

  noExpandIcon_ = layout_->bindWidget("no-expand",
				      std::make_unique<WText>());
  noExpandIcon_->setStyleClass("Wt-ctrl Wt-noexpand");
  addStyleClass("Wt-trunk");

  /*
   * Label
   */
  layout_->bindWidget("label-area", std::make_unique<WContainerWidget>());

  childCountLabel_ = nullptr;

  if (labelIcon_) {
    labelArea()->addWidget(std::move(labelIcon));
    labelIcon_->setVerticalAlignment(AlignmentFlag::Middle);
  }

  labelText_ = labelArea()->addWidget(std::make_unique<WText>(labelText));
  labelText_->setStyleClass("Wt-label");

  childrenLoaded_ = false;

  setLoadPolicy(ContentLoading::Lazy);
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
  return childCount();
}

int WTreeNode::childCount() const
{
  return childContainer()->count() + notLoadedChildren_.size();
}

std::vector<WTreeNode *> WTreeNode::childNodes() const
{
  std::vector<WTreeNode *> result;
  result.reserve(childCount());

  for (int i = 0; i < childContainer()->count(); ++i)
    result.push_back(dynamic_cast<WTreeNode *>(childContainer()->widget(i)));

  for (const auto& i : notLoadedChildren_)
    result.push_back(i.get());

  return result;
}


void WTreeNode::setSelectable(bool selectable)
{
  selectable_ = selectable;
}

void WTreeNode::setInteractive(bool interactive)
{
  interactive_ = interactive;
}

WContainerWidget *WTreeNode::labelArea() const
{
  return layout_->resolve<WContainerWidget *>("label-area");
}

WContainerWidget *WTreeNode::childContainer() const
{
  return layout_->resolve<WContainerWidget *>("children");
}

WTreeNode::~WTreeNode()
{ }

WTree *WTreeNode::tree() const
{
  return parentNode_ ? parentNode_->tree() : nullptr;
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

    return childCount() > 0;
  } else
    return false;
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

void WTreeNode::setChildCountPolicy(ChildCountPolicy policy)
{
  if (policy != Disabled && !childCountLabel_) {
    childCountLabel_ = labelArea()->addWidget(std::make_unique<WText>());
    childCountLabel_->setMargin(WLength(7), Side::Left);
    childCountLabel_->setStyleClass("Wt-childcount");
  }

  childCountPolicy_ = policy;

  if (childCountPolicy_ == Enabled) {
    WTreeNode *parent = parentNode();
 
    if (parent && parent->isExpanded())
      if (doPopulate())
	update();
  }

  if (childCountPolicy_ != Disabled) {
    auto children = childNodes();
    for (auto& c : children)
      c->setChildCountPolicy(childCountPolicy_);
  }
}

void WTreeNode::setLoadPolicy(ContentLoading loadPolicy)
{
  loadPolicy_ = loadPolicy;

  switch (loadPolicy) {
  case ContentLoading::Eager:
    loadChildren();

    break;
  case ContentLoading::NextLevel:
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
  case ContentLoading::Lazy:
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

  if (loadPolicy_ != ContentLoading::Lazy) {
    auto children = childNodes();
    for (auto& c : children)
      c->setLoadPolicy(loadPolicy_);
  }
}

void WTreeNode::loadChildren()
{
  if (!childrenLoaded_) {
    doPopulate();

    for (unsigned i = 0; i < notLoadedChildren_.size(); ++i)
      childContainer()->addWidget(std::move(notLoadedChildren_[i]));
    notLoadedChildren_.clear();

    expandIcon_->icon1Clicked().connect(this, &WTreeNode::doExpand);
    expandIcon_->icon2Clicked().connect(this, &WTreeNode::doCollapse);

    resetLearnedSlots();

    childrenLoaded_ = true;
  }
}

void WTreeNode::loadGrandChildren()
{
  auto children = childNodes();
  for (auto& c : children)
    c->loadChildren();
}

bool WTreeNode::isLastChildNode() const
{
  WTreeNode *parent = parentNode();

  if (parent)
    return parent->childNodes().back() == this;
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

WTreeNode *WTreeNode::addChildNode(std::unique_ptr<WTreeNode> node)
{
  auto result = node.get();
  insertChildNode(childCount(), std::move(node));
  return result;
}

void WTreeNode::insertChildNode(int index, std::unique_ptr<WTreeNode> node)
{
  node->parentNode_ = this;

  WTreeNode *added = node.get();

  if (childrenLoaded_)
    childContainer()->insertWidget(index, std::move(node));
  else
    notLoadedChildren_.insert(notLoadedChildren_.begin() + index,
			      std::move(node));

  descendantAdded(added);

  if (loadPolicy_ != added->loadPolicy_)
    added->setLoadPolicy(loadPolicy_);

  if (childCountPolicy_ != added->childCountPolicy_)
    added->setChildCountPolicy(childCountPolicy_);

  /*
   * If newly inserted node is last, then previous last node needs to
   * be updated.
   */
  if (index == (int)childCount() - 1 && childCount() > 1)
    childNodes()[childCount() - 2]->update();

  added->update();
  update();
  resetLearnedSlots();
}

std::unique_ptr<WTreeNode> WTreeNode::removeChildNode(WTreeNode *node)
{
  node->parentNode_ = nullptr;

  std::unique_ptr<WTreeNode> result;
  if (childrenLoaded_)
    result = childContainer()->removeWidget(node);
  else
    result = Utils::take(notLoadedChildren_, node);

  descendantRemoved(node);

  updateChildren();

  return result;
}

void WTreeNode::updateChildren(bool recursive)
{
  auto children = childNodes();
  for (auto& c : children)
    if (recursive)
      c->updateChildren(recursive);
    else
      c->update();

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
    if (parentNode() && childCount() == 0) {
      parentNode()->resetLearnedSlots();
      update();
    }

    if (loadPolicy_ == ContentLoading::NextLevel)
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

  expandIcon_->setState(1);

  childContainer()->show();

  if (labelIcon_)
    labelIcon_->setState(1);

  /*
   * collapse all children
   */
  auto children = childNodes();
  for (auto& c : children)
    c->doCollapse();
}

void WTreeNode::doCollapse()
{
  wasCollapsed_ = !isExpanded();
  collapsed_ = true;

  expandIcon_->setState(0);
  childContainer()->hide();

  if (labelIcon_)
    labelIcon_->setState(0);
}

void WTreeNode::undoDoCollapse()
{
  if (!wasCollapsed_) {
    // re-expand
    expandIcon_->setState(1);
    childContainer()->show();
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
    childContainer()->hide();
    if (labelIcon_)
      labelIcon_->setState(0);

    collapsed_ = true;
  }

  /*
   * undo collapse of children
   */
  auto children = childNodes();
  for (auto& c : children)
    c->undoDoCollapse();  
}

void WTreeNode::setLabelIcon(std::unique_ptr<WIconPair> labelIcon)
{
  if (labelIcon_)
    labelIcon_->removeFromParent();

  labelIcon_ = labelIcon.get();

  if (labelIcon) {
    if (labelText_)
      labelArea()->insertBefore(std::move(labelIcon), labelText_);
    else
      labelArea()->addWidget(std::move(labelIcon));

    labelIcon_->setState(isExpanded() ? 1 : 0);
  }
}

void WTreeNode::renderSelected(bool isSelected)
{
  layout_->bindString("selected", isSelected ?
		      WApplication::instance()->theme()->activeClass() : "");
  selected().emit(isSelected);
}

void WTreeNode::update()
{
  bool isLast = isLastChildNode();

  if (!visible_) {
    layout_->bindString("selected", "Wt-root");
    childContainer()->addStyleClass("Wt-root");
  } else {
    if (tree()) {
      const WTree::WTreeNodeSet &s = tree()->selectedNodes();
      if (s.find(this) != s.end())
        layout_->bindString("selected", WApplication::instance()->theme()->activeClass());
      else
        layout_->bindEmpty("selected");
    } else
      layout_->bindEmpty("selected");
    childContainer()->removeStyleClass("Wt-root");
  }

  WTreeNode *parent = parentNode();
  if (parent && !parent->childrenDecorated_) {
    // FIXME
  }

  if (expandIcon_->state() != (isExpanded() ? 1 : 0))
    expandIcon_->setState(isExpanded() ? 1 : 0);
  if (childContainer()->isHidden() != !isExpanded())
    childContainer()->setHidden(!isExpanded());
  if (labelIcon_ && (labelIcon_->state() != (isExpanded() ? 1 : 0)))
    labelIcon_->setState(isExpanded() ? 1 : 0);

  toggleStyleClass("Wt-trunk", !isLast);
  layout_->bindString("trunk-class", isLast ? "Wt-end" : "Wt-trunk");

  if (!parentNode() || parentNode()->isExpanded()) {
    if (childCountPolicy_ == Enabled && !populated_)
      doPopulate();
    
    expandIcon_->setHidden(!expandable());
    noExpandIcon_->setHidden(expandable());
  }

  if (childCountPolicy_ != Disabled && populated_ && childCountLabel_) {
    int n = displayedChildCount();
    if (n)
      childCountLabel_->setText
	(WString::fromUTF8("(" + std::to_string(n) + ")"));
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
