// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTREETABLENODE_H_
#define WTREETABLENODE_H_

#include <Wt/WTreeNode.h>

namespace Wt {

class WTreeTable;

/*! \class WTreeTableNode Wt/WTreeTableNode.h Wt/WTreeTableNode.h
 *  \brief A specialized tree node which allows additional data to be
 *         associated with each node.
 *
 * Additional data for each column can be set using setColumnWidget().
 *
 * \sa WTreeNode, WTreeTable
 */
class WT_API WTreeTableNode : public WTreeNode
{
public:
  /*! \brief Creates a new tree table node.
   *
   * \sa WTreeNode::WTreeNode()
   */
  WTreeTableNode(const WString& labelText,
		 std::unique_ptr<WIconPair> labelIcon = nullptr);

  /*! \brief Sets a widget to be displayed in the given column for this node.
   *
   * Columns are counted starting from 0 for the tree list itself, and 1
   * for the first additional column.
   *
   * The node label (in column 0) is not considered a column widget.
   * To set a custom widget in column 0, you can add a widget to the
   * labelArea().
   */
  void setColumnWidget(int column, std::unique_ptr<WWidget> item);

  /*! \brief Returns the widget set for a column.
   *
   * Returns the widget set previously using setColumnWidget(), or \c 0
   * if no widget was previously set.
   */
  WWidget *columnWidget(int column);

  /*! \brief Returns the table for this node.
   *
   * \sa WTreeTableNode::setTable()
   */
  WTreeTable *table() const { return table_; }

  virtual void insertChildNode(int index,
			       std::unique_ptr<WTreeNode> node) override;

protected:

  /*! \brief Sets the table for this node.
   *
   * This method is called when the node is inserted, directly, or indirectly
   * into a table.
   *
   * You may want to reimplement this method if you wish to customize the
   * behaviour of the node depending on table properties. For example to only
   * associate data with the node when the tree list is actually used inside
   * a table.
   *
   * \sa WTreeTableNode::table()
   */
  virtual void setTable(WTreeTable *table);

private:
  WTreeTable *table_;
  WContainerWidget *row_;

  struct ColumnWidget {
    WWidget *widget;
    bool     isSet;

    ColumnWidget(WWidget *aWidget, bool set)
      : widget(aWidget), isSet(set) { }
  };

  std::vector<ColumnWidget> columnWidgets_;

  /*
   * the number of columns, besides the the tree itself
   */
  void createExtraColumns(int numColumns);

  /*
   * The width for the column, counting from 1
   */
  WLength columnWidth(int column);

  friend class WTreeTable;
};

}

#endif // WTREETABLENODE_H_
