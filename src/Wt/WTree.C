/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WIconPair.h"
#include "Wt/WTemplate.h"
#include "Wt/WText.h"
#include "Wt/WTree.h"
#include "Wt/WTreeNode.h"
#include "WebUtils.h"

namespace {
  using namespace Wt;

  /*
   * Get the ancestors of n as a list, starting from the tree root,
   * and including n.
   *
   * Returns false if node n is currently not visible because one of its
   * ancestors is not expanded.
   */
  bool getAncestors(WTreeNode *n, std::vector<WTreeNode *>& ancestors)
  {
    WTreeNode *p = n->parentNode();
    if (p) {
      if (!p->isExpanded())
	return false;
      else
	if (!getAncestors(p, ancestors))
	  return false;
    }

    ancestors.push_back(n);
    return true;
  }

  /*
   * Compares two nodes and returns the one that is rendered higher in
   * the list. Returns 0 if one of the nodes is currently not rendered
   * (invisible).
   */
  WTreeNode *firstNode(WTreeNode *n1, WTreeNode *n2)
  {
    std::vector<WTreeNode *> ancestors1, ancestors2;

    bool visible1 = n1 ? getAncestors(n1, ancestors1) : true;
    bool visible2 = n2 ? getAncestors(n2, ancestors2) : true;

    if (!visible1 || !visible2)
      return nullptr;

    if (!n1)
      return n2;

    if (!n2)
      return n1;

    for (unsigned i = 1; i < std::min(ancestors1.size(), ancestors2.size());
	 ++i) {
      if (ancestors1[i] != ancestors2[i]) {
	WTreeNode *parent = ancestors1[i-1];
	int i1 = Utils::indexOf(parent->childNodes(), ancestors1[i]);
	int i2 = Utils::indexOf(parent->childNodes(), ancestors2[i]);

	if (i1 < i2)
	  return n1;
	else
	  return n2;
      }
    }

    if (ancestors1.size() > ancestors2.size())
      return n2;
    else
      return n1;
  }
}

namespace Wt {

  namespace Impl {
    class SentinelTreeNode final : public WTreeNode
    {
    public:
      SentinelTreeNode(WTree *tree)
	: WTreeNode(""),
	  tree_(tree)
      {
	addStyleClass("Wt-sentinel");
	setNodeVisible(false);
	expand();
     }
      
     virtual WTree *tree() const override { return tree_; }

    protected:
      virtual void descendantRemoved(WTreeNode *node) override
      {
	tree_->nodeRemoved(node);
      }

      virtual void descendantAdded(WTreeNode *node) override
      {
	tree_->nodeAdded(node);
      }

    private:
      WTree *tree_;
    };
  }

WTree::WTree()
  : WCompositeWidget(),
    treeRoot_(nullptr),
    selectionMode_(SelectionMode::None)
{
  setImplementation(std::unique_ptr<Impl::SentinelTreeNode>(sentinelRoot_ = new Impl::SentinelTreeNode(this)));
}

void WTree::setTreeRoot(std::unique_ptr<WTreeNode> node)
{
  if (treeRoot_)
    sentinelRoot_->removeChildNode(treeRoot_);
  treeRoot_ = node.get();
  sentinelRoot_->addChildNode(std::move(node));
}

void WTree::setSelectionMode(SelectionMode mode)
{
  if (mode != selectionMode_) {
    selectionMode_ = mode;
    clearSelection();
  }
}

void WTree::clearSelection()
{
  while (!selection_.empty()) {
    WTreeNode *n = *selection_.begin();
    select(n, false);
  }
}

void WTree::select(WTreeNode *node, bool selected)
{
  if (selectionMode_ == SelectionMode::Single && selected && 
      selection_.size() == 1 && Utils::first(selection_) == node)
    return; // node was already selected, avoid re-emission of signals

  if (selectionMode_ == SelectionMode::Single && selected)
    clearSelection();

  if (!selected || selectionMode_ != SelectionMode::None) {
    if (selected) {
      if (node->isSelectable()) {
	selection_.insert(node);
	node->renderSelected(selected);
      }
    } else {
      if (selection_.erase(node))
	node->renderSelected(false);
      else
	return; // node was not selected, avoid re-emission of signals
    }
  }

  itemSelectionChanged_.emit();
}

void WTree::select(const WTreeNodeSet& nodes)
{
  clearSelection();

  for (WTreeNodeSet::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
    select(*i);

  itemSelectionChanged_.emit();
}

void WTree::nodeRemoved(WTreeNode *node)
{
  select(node, false);

  node->clickedConnection_.disconnect();

  for (unsigned i = 0; i < node->childNodes().size(); ++i)
    nodeRemoved(node->childNodes()[i]);
}

void WTree::nodeAdded(WTreeNode * const node)
{
  if (node->isSelectable()) {
    WInteractWidget *w;

    if (WApplication::instance()->environment().ajax())
      w = node->impl();
    else
      w = node->label();

    node->clickedConnection_ = w->clicked().connect
      (this, std::bind(&WTree::onClick, this, node, std::placeholders::_1));
    w->clicked().preventPropagation();

    for (unsigned i = 0; i < node->childNodes().size(); ++i)
      nodeAdded(node->childNodes()[i]);
  }

  // This doesn't hurt but sounds like it is not something the library
  // should deal with ...
  if (!node->parentNode()->isSelectable() && isSelected(node->parentNode()))
    select(node->parentNode(), false);
}

void WTree::selectRange(WTreeNode *from, WTreeNode *to)
{
  clearSelection();

  WTreeNode *n = from;
  for (;;) {
    select(n);

    if (n == to)
      break;
    else {
      if (n->isExpanded() && !n->childNodes().empty())
	n = n->childNodes()[0];
      else {
	for (;;) {
	  const std::vector<WTreeNode *>& cs = n->parentNode()->childNodes();
	  int i = Utils::indexOf(cs, n);

	  i++;
	  if (i < static_cast<int>(cs.size())) {
	    n = cs[i];
	    break;
	  } else {
	    n = n->parentNode();
	  }
	}
      }
    }
  }
}

void WTree::extendSelection(WTreeNode *node)
{
  if (selection_.empty()) {
    select(node);
  } else {
    /*
     * Expand current selection. First we find the selection extremes.
     * If node is in between or below, we select from the top item to
     * node. If node is above, select everything from the bottom item
     * to node. Only visible nodes count.
     */
    WTreeNode *top = nullptr, *bottom = nullptr;

    for (WTreeNodeSet::const_iterator i = selection_.begin();
	 i != selection_.end(); ++i) {
      WTreeNode *s = *i;

      WTreeNode *f1 = firstNode(top, s);
      if (!f1)
	continue;

      top = f1;

      if (!bottom)
	bottom = s;
      else {
	WTreeNode *f2 = firstNode(bottom, s);
	if (f2 == bottom)
	  bottom = s;
      }
    }

    /*
     * All selected nodes are invisible: assume nothing was selected
     */
    if (!top) {
      clearSelection();
      select(node);
      return;
    }

    WTreeNode *f1 = firstNode(node, top);

    if (f1 == top)
      selectRange(top, node);
    else
      selectRange(node, bottom);
  }
}

void WTree::onClick(WTreeNode *node, WMouseEvent event)
{
  if (selectionMode_ == SelectionMode::None)
    return;

  if (selectionMode_ == SelectionMode::Extended) {
    if (event.modifiers().test(KeyboardModifier::Shift)) {
      extendSelection(node);
    } else {
      if (!(event.modifiers() & (KeyboardModifier::Control |
				 KeyboardModifier::Meta))) {
	if (isSelected(node))
	  return;
	else {
	  clearSelection();
	  select(node);
	}
      } else
	select(node, !isSelected(node));
    }
  } else
    select(node);
}

bool WTree::isSelected(WTreeNode *node) const
{
  WTreeNodeSet::const_iterator i = selection_.find(node);
  return i != selection_.end();
}

}
