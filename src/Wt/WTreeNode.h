// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTREENODE_H_
#define WTREENODE_H_

#include <Wt/WCompositeWidget.h>
#include <Wt/WIconPair.h>

namespace Wt {

/*! \brief An enumeration for the policy to display the child count
 */
enum ChildCountPolicy {
  Disabled, //!< Do not display a child count
  Enabled,  //!< Always display a child count
  Lazy      //!< Only display a child count when the node is populated
};

/*! \class WTreeNode Wt/WTreeNode.h Wt/WTreeNode.h
 *  \brief A single node in a tree.
 *
 * A tree list is constructed by combining several tree node objects
 * in a tree hierarchy, by passing the parent tree node as the last
 * argument in the child node constructor, or by using addChildNode(),
 * to add a child to its parent.
 *
 * Each tree node has a label, and optionally a label icon pair. The
 * icon pair offers the capability to show a different icon depending
 * on the state of the node (expanded or collapsed). When the node has
 * any children, a child count may be displayed next to the label
 * using setChildCountPolicy().
 *
 * Expanding a tree node it will collapse all its children, so that a user
 * may collapse/expand a node as a short-cut to collapsing all children.
 *
 * The treenode provides several policies to communicate the current
 * contents of the tree to the client (if possible):
 * <ul>
 *   <li>ContentLoading::Eager: the entire tree is transmitted to the client,
 *     and all tree navigation requires no further communication.</li>
 *   <li>ContentLoading::Lazy: only the minimum is transmitted to the
 *     client. When expanding a node for the first time, only then it is
 *     transmitted to the client, and this may thus have some latency.</li>
 *   <li>ContentLoading::NextLevel: all leafs of visible children are
 *     transmitted, but not their children. This provides a good trade-off
 *     between bandwith use and interactivity, since expanding any
 *     tree node will happen instantly, and at the same time trigger some
 *     communication in the back-ground to load the next level of
 *     invisible nodes.</li>
 * </ul>
 *
 * The default policy is ContentLoading::Lazy. Another load policy
 * may be specified using setLoadPolicy() on the root node and before
 * adding any children. The load policy is inherited by all children
 * in the tree.
 *
 * There are a few scenarios where it makes sense to specialize the
 * %WTreeNode class. One scenario is create a tree that is populated
 * dynamically while browsing. For this purpose you should reimplement the
 * populate() method, whose default implementation does nothing. This
 * method is called when 'loading' the node. The exact moment for loading
 * a treenode depends on the LoadPolicy.
 *
 * A second scenario that is if you want to customize the look of the
 * tree label (see labelArea()) or if you want to modify or augment
 * the event collapse/expand event handling (see doExpand() and
 * doCollapse()).
 *
 * See WTree for a usage example.
 *
 * \sa WTree, WTreeTableNode
 */
class WT_API WTreeNode : public WCompositeWidget
{
public:
  /*! \brief Creates a tree node with the given label.
   *
   * The labelIcon, if specified, will appear just before the label and
   * its state reflect the expand/collapse state of the node.
   *
   * The node is initialized to be collapsed.
   */
  WTreeNode(const WString& labelText,
	    std::unique_ptr<WIconPair> labelIcon = nullptr);

  /*! \brief Destructor.
   */
  ~WTreeNode();

  /*! \brief Returns the tree.
   *
   * By default if this node has no parent the result will be 0.
   */
  virtual WTree *tree() const;

  /*! \brief Returns the label.
   */
  WText *label() const { return labelText_; }

  /*! \brief Returns the label icon.
   */
  WIconPair *labelIcon() const { return labelIcon_; }

  /*! \brief Sets the label icon.
   */
  void setLabelIcon(std::unique_ptr<WIconPair> labelIcon);

  /*! \brief Inserts a child node.
   *
   * Inserts the node \p node at index \p index.
   */
  virtual void insertChildNode(int index,
			       std::unique_ptr<WTreeNode> node);

  /*! \brief Adds a child node.
   *
   * Equivalent to:
   * \code
   * insertChildNode(childNodes().size(), std::move(node));
   * \endcode
   *
   * \sa insertChildNode()
   */
  WTreeNode *addChildNode(std::unique_ptr<WTreeNode> node);

  /*! \brief Removes a child node.
   */
  std::unique_ptr<WTreeNode> removeChildNode(WTreeNode *node);

  /*! \brief Returns the list of children.
   */
  std::vector<WTreeNode *> childNodes() const;

  /*! \brief Returns the number of children that should be displayed.
   *
   * This is used to display the count in the count label. The default
   * implementation simply returns childNodes().size().
   */
  virtual int displayedChildCount() const;

  /*! \brief Configures how and when the child count should be displayed
   *
   * By default, no child count indication is disabled (this is the behaviour
   * since 2.1.1). Use this method to enable child count indications.
   *
   * The child count policy is inherited by all children in the tree.
   */
  void setChildCountPolicy(ChildCountPolicy policy);

  /*! \brief Returns the child count policy
   *
   * \sa setChildCountPolicy(ChildCountPolicy policy)
   */
  ChildCountPolicy childCountPolicy() const { return childCountPolicy_; }

  /*! \brief Sets the load policy for this tree.
   *
   * This may only be set on the root of a tree, and before adding
   * any children.
   */
  void setLoadPolicy(ContentLoading loadPolicy);  

  /*! \brief Returns whether this node is expanded.
   */
  bool isExpanded() const;

  /*! \brief Allows this node to be selected.
   *
   * By default, all nodes may be selected.
   *
   * \sa isSelectable(), WTree::select(WTreeNode *, bool)
   */
  virtual void setSelectable(bool selectable) override;

  /*! \brief Returns if this node may be selected.
   *
   * \sa setSelectable(bool)
   */
  virtual bool isSelectable() const { return selectable_; }

  /*! \brief Returns the parent node
   *
   * \sa childNodes()
   */
  WTreeNode *parentNode() const { return parentNode_; }

  /*! \brief Sets the visibility of the node itself
   *
   * If \c false, then the node itself is not displayed, but only its children.
   * This is typically used to hide the root node of a tree.
   */
  void setNodeVisible(bool visible);

  /*! \brief Sets whether this node's children are decorated.
   *
   * By default, node's children have expand/collapse and other lines
   * to display their linkage and offspring.
   *
   * By setting \p decorated to \c false, you can hide the decorations for the 
   * node's children.
   */
  void setChildrenDecorated(bool decorated);
  
  /*! \brief Sets whether this node is interactive.
   * 
   * Interactive nodes can be clicked upon and will populate a list of children 
   * when clicked. By disabling the interactivity, a node will not react to a 
   * click event. 
   */
  void setInteractive(bool interactive);

  /*! \brief Expands this node.
   *
   * Besides the actual expansion of the node, this may also trigger
   * the loading and population of the node children, or of the children's
   * children.
   *
   * \sa collapse()
   * \sa doExpand()
   */
  void expand();

  /*! \brief Collapses this node.
   *
   * \sa expand()
   * \sa doCollapse()
   */
  void collapse();

  /*! \brief %Signal emitted when the node is expanded by the user.
   *
   * \sa collapsed()
   */
  EventSignal<WMouseEvent>& expanded();

  /*! \brief %Signal emitted when the node is collapsed by the user.
   *
   * \sa expanded()
   */
  EventSignal<WMouseEvent>& collapsed();

  /*! \brief %Signal that is emitted when the node is added or removed from
   *         the selection
   *
   * \sa WTree::itemSelectionChanged
   */
  Signal<bool>& selected() { return selected_; }

#ifndef WT_TARGET_JAVA
  virtual bool hasParent() const override;
#endif //WT_TARGET_JAVA

protected:
  /*! \brief Creates a tree node with empty labelArea().
   *
   * This tree node has no label or labelicon, and is therefore ideally
   * suited to provide a custom look.
   */
  WTreeNode();

  /*! \brief Accesses the container widget that holds the label area.
   *
   * Use this to customize how the label should look like.
   */
  WContainerWidget *labelArea() const;

  /*! \brief Populates the node dynamically on loading.
   *
   * Reimplement this method if you want to populate the widget dynamically,
   * as the tree is being browsed and therefore loaded. This is only
   * usefull with LazyLoading or NextLevelLoading strategies.
   */
  virtual void populate();

  /*! \brief Returns whether this node has already been populated.
   *
   * \sa populate()
   */
  bool populated() const { return populated_; }

  /*! \brief Returns whether this node can be expanded.
   *
   * The default implementation populates the node if necessary, and then
   * checks if there are any child nodes.
   *
   * You may wish to reimplement this method if you reimplement populate(),
   * and you have a quick default for determining whether a node may be
   * expanded (which does not require populating the node).
   *
   * \sa populate()
   */
  virtual bool expandable();

  /*! \brief Renders the node to be selected.
   *
   * The default implementation changes the style class of the labelArea()
   * to "selected".
   */
  virtual void renderSelected(bool selected);

  /*! \brief Reacts to the removal of a descendant node.
   *
   * Reimplement this method if you wish to react on the removal of a
   * descendant node. The default implementation simply propagates the
   * event to the parent.
   */
  virtual void descendantRemoved(WTreeNode *node);

  /*! \brief Reacts to the addition of a descendant node.
   *
   * Reimplement this method if you wish to react on the addition of a
   * descendant node. The default implementation simply propagates the
   * event to the parent.
   */
  virtual void descendantAdded(WTreeNode *node);

  /*! \brief The actual expand.
   *
   * This method, which is implemented as a stateless slot, performs the
   * actual expansion of the node.
   *
   * You may want to reimplement this function (and undoDoExpand()) if you
   * wish to do additional things on node expansion.
   *
   * \sa doCollapse()
   * \sa expand()
   */
  virtual void doExpand();

  /*! \brief The actual collapse.
   *
   * This method, which is implemented as a stateless slot, performs the
   * actual collapse of the node.
   *
   * You may want to reimplement this function (and undoDoCollapse()) if you
   * wish to do additional things on node expansion.
   *
   * \sa doExpand()
   * \sa collapse()
   */
  virtual void doCollapse();

  /*! \brief Undo method for doCollapse() stateless implementation.
   *
   * \sa doCollapse()
   */
  virtual void undoDoExpand();

  /*! \brief Undo method for doCollapse() stateless implementation.
   *
   * \sa doExpand()
   */
  virtual void undoDoCollapse();

  /*! \brief Accesses the icon pair that allows expansion of the tree node.
   */
  WIconPair *expandIcon() const { return expandIcon_; }

  WTemplate *impl() const { return layout_; }

private:
  bool collapsed_, selectable_, visible_, childrenDecorated_;
  WTreeNode *parentNode_;
  std::vector<std::unique_ptr<WTreeNode> > notLoadedChildren_;
  ContentLoading loadPolicy_;
  ChildCountPolicy childCountPolicy_;

  WTemplate *layout_;
  WIconPair *expandIcon_;
  WText *noExpandIcon_;
  WIconPair *labelIcon_;
  WText	*labelText_, *childCountLabel_;

  bool childrenLoaded_, populated_, interactive_;

  Signal<bool> selected_;

  void init(const WString &labelText, std::unique_ptr<WIconPair> labelIcon);
  WContainerWidget *childContainer() const;
  int childCount() const;
  void loadChildren();
  void loadGrandChildren();
  void create();
  void update();
  bool isLastChildNode() const;
  void updateChildren(bool recursive = false);
  bool wasCollapsed_;
  bool doPopulate();

  static const char* imagePlus_;
  static const char* imageMin_;
  static const char* imagePlusRtl_;
  static const char* imageMinRtl_;

  Wt::Signals::connection clickedConnection_;
  friend class WTree;
};

}

#endif // WTREENODE_H_
