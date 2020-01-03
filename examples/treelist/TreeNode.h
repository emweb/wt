// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef TREENODE_H_
#define TREENODE_H_

#include <Wt/WCompositeWidget.h>
#include <Wt/WText.h>

class IconPair;

using namespace Wt;

namespace Wt {
  class WTable;
  class WImage;
}

/**
 * @addtogroup treelist
 */
/*@{*/

/*! \brief Example implementation of a single tree list node.
 *
 * <i>This is an example of a basic treelist implementation. As of
 * version 1.1.8, a more flexible treenode implementation is included as
 * part of the library: WTreeNode.</i>
 *
 * A tree list is constructed by nesting TreeNode objects in a tree
 * hierarchy.
 *
 * A TreeNode has a label, and optionally a two-state label icon, which
 * defines a different image depending on the state of the node (expanded
 * or collapsed). When the node has any children, a child count is also
 * indicated.
 *
 * Next to the icons, two style classes determine the look of a TreeNode:
 * the label has style "treenodelabel", and the child count has as style
 * "treenodechildcount".
 *
 * Use CSS nested selectors to apply different styles to different treenodes.
 * For example, to style the treenode with style class "mynode":
 *
 * The behaviour of the tree node is to collapse all children when the
 * node is expanded (this is similar to how most tree node implementations
 * work).
 *
 * The widget uses a number of images which must be available in an
 * "icons/" folder (see the %Wt treelist examples).
 *
 * This widget is part of the %Wt treelist example.
 */
class TreeNode : public WCompositeWidget
{
public:
  /*! \brief Construct a tree node with the given label.
   *
   * The label is formatted in a WText with the given formatting.
   * The labelIcon (if not 0) will appear next to the label and its state
   * will reflect the expand/collapse state of the node.
   *
   * Optionally, a userContent widget may be associated with the node.
   * When expanded, this widget will be shown below the widget, but above
   * any of the children nodes.
   */
  TreeNode(const std::string labelText,
           TextFormat labelFormat,
           std::unique_ptr<IconPair> labelIcon);

  /*! \brief Adds a child node.
   */
  void addChildNode(std::unique_ptr<TreeNode> node);

  /*! \brief Removes a child node.
   */
  void removeChildNode(TreeNode *node, int index);

  /*! \brief Returns the list of children.
   */
  const std::vector<TreeNode *>& childNodes() const { return childNodes_; }

  /*! \brief Collapses this node.
   */
  void collapse();

  /*! \brief Expands this node.
   */
  void expand();

private:
  //! List of child nodes.
  std::vector<TreeNode *>   childNodes_;

  //! The parent node.
  TreeNode                 *parentNode_;

  //! Layout (2x2 table).
  WTable                   *layout_;

  //! The icon for expanding or collapsing.
  IconPair                 *expandIcon_;

  //! The single image shown instead of the expand/collapse icon when no children.
  WImage		   *noExpandIcon_;

  //! The icon next to the label.
  IconPair                 *labelIcon_;

  //! The children count '(x)' for x children.
  WText                    *childCountLabel_;

  //! The container in which the children are managed.
  WContainerWidget         *expandedContent_;

  //! Adjust the expand icon
  void adjustExpandIcon();

  //! Returns if is the last child within its parent (is rendered differently)
  bool isLastChildNode() const;

  //! Rerender when children have changed.
  void childNodesChanged();

  //! Was collapsed (for undo of prelearned collapse() and expand() slots.
  bool wasCollapsed_;

  //! Undo function for prelearning collapse()
  void undoCollapse();

  //! Undo function for prelearning expand()
  void undoExpand();

  //! Two sets of images, for a normal node, and for the last node.
  enum ImageIndex { Middle = 0, Last = 1 };

  static std::string imageLine_[];
  static std::string imagePlus_[];
  static std::string imageMin_[];
}; //

/*@}*/

#endif // WTREENODE_H_
