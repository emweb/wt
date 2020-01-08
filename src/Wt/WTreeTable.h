// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTREETABLE_H_
#define WTREETABLE_H_

#include <Wt/WCompositeWidget.h>

namespace Wt {

class WTree;
class WTreeTableNode;
class WText;

/*! \class WTreeTable Wt/WTreeTable.h Wt/WTreeTable.h
 *  \brief A table with a navigatable tree in the first column.
 *
 * A %WTreeTable implements a tree table, where additional data
 * associated is associated with tree items, which are organized in
 * columns.
 *
 * Unlike the MVC-based WTreeView widget, the tree renders a widget
 * hierarchy, rather than a hierarhical standard model. This provides
 * extra flexibility (as any widget can be used as contents), at the
 * cost of server-side, client-side and bandwidth resources
 * (especially for large tree tables).
 *
 * The actual data is organized and provided by WTreeTableNode widgets.
 *
 * To use the tree table, you need <b>first</b> to call addColumn() to
 * specify the additional data columns. Next, you need to set the tree
 * root using setTreeRoot() and bind additional information (text or
 * other widgets) in each node using
 * WTreeTableNode::setColumnWidget(). Thus, you cannot change the
 * number of columns once the tree root has been set.
 *
 * The table cannot be given a height using CSS style rules, instead you
 * must use layout managers, or use resize().
 *
 * \if cpp
 * Usage example:
 * \code
 * auto treeTable = std::make_unique<Wt::WTreeTable>();
 * treeTable->resize(650, 300);
 *
 * // Add 3 extra columns
 * treeTable->addColumn("Yuppie Factor", 125);
 * treeTable->addColumn("# Holidays", 125);
 * treeTable->addColumn("Favorite Item", 125);
 *
 * // Create and set the root node
 * auto rootPtr = std::make_unique<Wt::WTreeTableNode>("All Personnel");
 * Wt::WTreeTableNode *root = rootPtr.get();
 * root->setImagePack("resources/");
 * treeTable->setTreeRoot(std::move(rootPtr), "Emweb Organigram");
 *
 * // Populate the tree with data nodes.
 * auto node1 = std::make_unique<Wt::WTreeTableNode>("Upper Management");
 * std::unique_ptr<Wt::WTreeTableNode> node2;
 * node2 = std::make_unique<Wt::WTreeTableNode>("Chief Anything Officer");
 * node2->setColumnWidget(1, std::make_unique<Wt::WText>("-2.8"));
 * node2->setColumnWidget(2, std::make_unique<Wt::WText>("20"));
 * node2->setColumnWidget(3, std::make_unique<Wt::WText>("Scepter"));
 * node1->addChildNode(std::move(node2));
 *
 * node2 = std::make_unique<Wt::WTreeTableNode>("Vice President of Parties");
 * node2->setColumnWidget(1, std::make_unique<Wt::WText>("13.57"));
 * node2->setColumnWidget(2, std::make_unique<Wt::WText>("365"));
 * node2->setColumnWidget(3, std::make_unique<Wt::WText>("Flag"));
 * node1->addChildNode(std::move(node2));
 *
 * root->addChildNode(std::move(node1));
 *
 * root->expand();
 * \endcode
 * \endif
 *
 * A screenshot of the treetable:
 * \image html WTreeTable-default-1.png "An example WTreeTable (default)"
 * \image html WTreeTable-polished-1.png "An example WTreeTable (polished)"
 *
 * \sa WTreeTableNode, WTreeView
 */
class WT_API WTreeTable : public WCompositeWidget
{
public:
  /*! \brief Creates a new tree table.
   *
   * The treeRoot() is \c 0. The table should first be properly dimensioned
   * using addColumn() calls, and then data using setTreeRoot().
   */
  WTreeTable();

  /*! \brief Adds an extra column.
   *
   * Add an extra column, specifying the column header and a column
   * width. The extra columns are numbered from 1 as column 0 contains
   * the tree itself. The header for column 0 (the tree itself) is
   * specified in setTreeRoot(), and the width of column 0 takes the
   * remaining available width.
   */
  void addColumn(const WString& header, const WLength& width);

  /*! \brief Returns the number of columns in this table.
   *
   * Returns the number of columns in the table, including in the count
   * column 0 (which contains the tree).
   *
   * \sa addColumn()
   */
  int columnCount() const { return (int)columnWidths_.size(); }

  /*! \brief Sets the tree root.
   *
   * Sets the data for the tree table, and specify the header for the
   * first column.
   *
   * The initial \p root is nullptr.
   *
   * \sa treeRoot(), setTree()
   */
  void setTreeRoot(std::unique_ptr<WTreeTableNode> root, const WString& header);

  /*! \brief Returns the tree root.
   */
  WTreeTableNode *treeRoot();

  /*! \brief Sets the tree which provides the data for the tree table.
   *
   * \sa setTreeRoot().
   */
  void setTree(std::unique_ptr<WTree> tree, const WString& header);

  /*! \brief Returns the tree that provides the data this table.
   *
   * \sa setTree().
   */
  WTree *tree() const { return tree_; }

  /*! \brief Returns the column width for the given column.
   *
   * The width of the first column (with index 0), containing the
   * tree, is implied by the width set for the table minus the width
   * of all other columns.
   *
   * \sa addColumn(), setTreeRoot()
   */
  WLength columnWidth(int column) const { return columnWidths_[column]; }

  /*! \brief Returns the header for the given column.
   *
   * \sa addColumn(), setTreeRoot()
   */
  WText *header(int column) const;

  /*! \brief Returns the header widget.
   *
   * This is the widget that contains the column headers.
   */
  WWidget *headerWidget() const;

protected:
  virtual void render(WFlags<RenderFlag> flags) override;

private:
  WContainerWidget *impl_;
  WContainerWidget *headers_;
  WContainerWidget *headerContainer_;
  WTree *tree_;

  std::vector<WLength> columnWidths_;

  void defineJavaScript();
};

}

#endif // WTREETABLE_H_
