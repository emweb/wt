// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTREE_H_
#define WTREE_H_

#include <set>

#include <Wt/WCompositeWidget.h>

namespace Wt {

  namespace Impl {
    class SentinelTreeNode;
  }

  class WTreeNode;

/*! \class WTree Wt/WTree.h Wt/WTree.h
 *  \brief A widget that represents a navigatable tree
 *
 * %WTree provides a tree widget, and coordinates selection
 * functionality.
 *
 * Unlike the MVC-based WTreeView, the tree renders a widget
 * hierarchy, rather than a hierarhical standard model. This provides
 * extra flexibility (as any widget can be used as contents), at the
 * cost of server-side, client-side and bandwidth resources
 * (especially for large tree tables).
 *
 * The tree is implemented as a hierarchy of WTreeNode widgets.
 *
 * Selection is rendered by calling WTreeNode::renderSelected(bool). Only
 * tree nodes that are \link WTreeNode::setSelectable(bool) selectable\endlink
 * may participate in the selection.
 * 
 * \if cpp
 * Usage example:
 * \code
 * auto folderIcon = std::make_unique<Wt::WIconPair>(
 *                     "icons/yellow-folder-closed.png",
 *                     "icons/yellow-folder-open.png",
 *                     false);
 *
 * auto tree = std::make_unique<Wt::WTree>();
 * tree->setSelectionMode(Wt::SelectionMode::Single);
 *
 * auto rootPtr = std::make_unique<Wt::WTreeNode>("Tree root", std::move(folderIcon));
 * Wt::WTreeNode *root = rootPtr.get();
 * root->setStyleClass("example-tree");
 * tree->setTreeRoot(std::move(rootPtr));
 * root->label()->setTextFormat(Wt::TextFormat::Plain);
 * root->setImagePack("resources/");
 * root->setLoadPolicy(Wt::ContentLoading::NextLevel);
 * root->addChildNode(std::make_unique<Wt::WTreeNode>("one"));
 * root->addChildNode(std::make_unique<Wt::WTreeNode>("two"));
 *
 * auto three = std::make_unique<Wt::WTreeNode>("three");
 * three->addChildNode(std::make_unique<Wt::WTreeNode>("three: one"));
 * three->addChildNode(std::make_unique<Wt::WTreeNode>("three: two"));
 * root->addChildNode(std::move(three));
 *
 * root->expand();
 * \endcode
 * \endif
 *
 * A screenshot of the tree:
 * <TABLE border="0" align="center"> <TR> <TD> 
 * \image html WTree-default-1.png "An example WTree (default)"
 * </TD> <TD>
 * \image html WTree-polished-1.png "An example WTree (polished)"
 * </TD> </TR> </TABLE>
 *
 * \sa WTreeNode, WTreeTable, WTreeView
 */
class WT_API WTree : public WCompositeWidget
{
public:
  /*! \brief Creates a new tree.
   */
  WTree();

  /*! \brief Sets the tree root node.
   *
   * The initial value is \c 0.
   */
  void setTreeRoot(std::unique_ptr<WTreeNode> root);

  /*! \brief Returns the root node.
   *
   * \sa setTreeRoot()
   */
  WTreeNode *treeRoot() const { return treeRoot_; }

  /*! \brief Sets the selection mode.
   *
   * The default selection mode is Wt::SelectionMode::None.
   */
  void setSelectionMode(SelectionMode mode);

  /*! \brief Returns the selection mode.
   */
  SelectionMode selectionMode() const { return selectionMode_; } 

  /*! \brief Typedef for a set of WTreeNode's.
   */
  typedef std::set<WTreeNode *> WTreeNodeSet;

  /*! \brief Returns the set of selected tree nodes.
   */
  const WTreeNodeSet& selectedNodes() const { return selection_; }

  /*! \brief Sets a selection of tree nodes.
   */
  void select(const WTreeNodeSet& nodes);

  /*! \brief Selects or unselect the given <i>node</i>.
   */
  void select(WTreeNode *node, bool selected = true);

  /*! \brief Returns if the given <i>node</i> is currently selected.
   */
  bool isSelected(WTreeNode *node) const;

  /*! \brief Clears the current selection.
   */
  void clearSelection();

  /*! \brief %Signal that is emitted when the selection changes.
   */
  Signal<>& itemSelectionChanged() { return itemSelectionChanged_; }

private:
  WTreeNode *treeRoot_;
  Impl::SentinelTreeNode *sentinelRoot_;

  SelectionMode selectionMode_;
  WTreeNodeSet selection_;
  Signal<> itemSelectionChanged_;

  void onClick(WTreeNode *node, WMouseEvent event);

  void selectRange(WTreeNode *from, WTreeNode *to);
  void extendSelection(WTreeNode *node);

protected:
  void nodeRemoved(WTreeNode *node);
  void nodeAdded(WTreeNode * const node);

  friend class Impl::SentinelTreeNode;
};

}

#endif // WTREE_H_
