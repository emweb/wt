// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSTANDARD_ITEM_H_
#define WSTANDARD_ITEM_H_

#include <map>
#include <string>
#include <vector>
#include <Wt/WModelIndex.h>
#include <Wt/WGlobal.h>

namespace Wt {

class WStandardItemModel;
class WString;

/*! \class WStandardItem Wt/WStandardItem.h Wt/WStandardItem.h
 *  \brief An item in a WStandardItemModel.
 *
 * The item provides access to various data properties: \link
 * setText() text\endlink, \link setIcon() icon\endlink, \link
 * setStyleClass() CSS style class\endlink, \link setToolTip() tool
 * tip\endlink, and \link setChecked() check state\endlink, and
 * data flags (setFlags() and setCheckable()).
 *
 * An item may contain a table of children items: the initial geometry
 * may be specified in the constructor, or using the methods
 * setRowCount() and setModelCount(). Unspecified items are 0. You can
 * set or inspect children items using the setChild() and child()
 * methods.
 *
 * It is possible to reimplement this class and specialize the methods
 * for data acess (setData() and data()), or provide custom sorting
 * functionality by reimplementing 
 * \if cpp
 * operator<().
 * \elseif java
 * compare().
 * \endif
 *
 * \ingroup modelview
 */
class WT_API WStandardItem
{
public:
  /*! \brief Creates an empty standard item.
   */
  WStandardItem();

  /*! \brief Creates an item with a text.
   *
   * \sa setText()
   */
  WStandardItem(const WString& text);

  /*! \brief Creates an item with an icon and a text.
   *
   * \sa setText(), setIcon()
   */
  WStandardItem(const std::string& iconUri, const WString& text);

  /*! \brief Creates an item with an initial geometry.
   *
   * \sa setRowCount(), setColumnCount()
   */
  WStandardItem(int rows, int columns = 1);

  /*! \brief Destructor.
   */
  virtual ~WStandardItem();

  /*! \brief Sets the text.
   *
   * The text is stored as \link Wt::ItemDataRole::Display ItemDataRole::Display\endlink data.
   *
   * The default text is empty ("").
   *
   * \sa text(), setData()
   */
  void setText(const WString& text);

  /*! \brief Returns the text.
   *
   * \sa setText()
   */
  WString text() const;

  /*! \brief Sets the icon url.
   *
   * The icon is stored as \link Wt::ItemDataRole::Decoration
   * ItemDataRole::Decoration\endlink data.
   *
   * The default icon url is empty ("").
   *
   * \sa icon(), setData()
   */
  void setIcon(const std::string& uri);

  /*! \brief Returns the icon url.
   *
   * \sa setIcon()
   */
  std::string icon() const;

  /*! \brief Sets the CSS style class.
   *
   * The style class is stored as \link Wt::ItemDataRole::StyleClass
   * ItemDataRole::StyleClass\endlink data.
   *
   * The default style class is empty ("").
   *
   * \sa styleClass(), setData()
   */
  void setStyleClass(const WString& styleClass);

  /*! \brief Returns the item style class.
   *
   * \sa setStyleClass()
   */
  WString styleClass() const;

  /*! \brief Sets a tool tip.
   *
   * The tool tip is stored as \link Wt::ItemDataRole::ToolTip ItemDataRole::ToolTip\endlink data.
   *
   * The default tool tip is empty ("").
   *
   * \sa toolTip(), setData()
   */
  void setToolTip(const WString& toolTip);

  /*! \brief Returns the tool tip.
   *
   * \sa setToolTip()
   */
  WString toolTip() const;

  /*! \brief Sets a link.
   *
   * The link is stored as \link Wt::ItemDataRole::Link ItemDataRole::Link\endlink data.
   *
   * \sa setData()
   */
  void setLink(const WLink& link);

  /*! \brief Returns a link.
   *
   * \sa setLink()
   */
  WLink link() const;

  /*! \brief Checks or unchecks the item.
   *
   * The value is stored as \link Wt::ItemDataRole::CheckState
   * ItemDataRole::CheckState\endlink data.
   *
   * By default, an item is not checked.
   *
   * Note: the checkbox will only be enabled if the item is checkable
   *       (see setCheckable()).
   *
   * If the item is tri-state, you may consider using setCheckState() instead
   * which supports also setting the third Wt::CheckState::PartiallyChecked state.
   *
   * \sa setCheckable(), setCheckState()
   */
  void setChecked(bool checked);

  /*! \brief Returns whether the item is checked.
   *
   * \sa setChecked()
   */
  bool isChecked() const;

  /*! \brief Sets the check state.
   *
   * Like setChecked(), this sets the check state, but allows also setting
   * the Wt::CheckState::PartiallyChecked state when the item is tri-state checkable.
   *
   * The value is stored as \link Wt::ItemDataRole::CheckState ItemDataRole::CheckState\endlink
   * data.
   *
   * \sa setCheckable(), setData()
   */
  void setCheckState(CheckState checked);

  /*! \brief Returns the item's check state.
   *
   * \sa setCheckState()
   */
  CheckState checkState() const;

  /*! \brief Sets the flags.
   *
   * The default flag value is \link Wt::ItemFlag::Selectable
   * ItemFlag::Selectable\endlink.
   *
   * \sa ItemFlag, flags(), setCheckable()
   */
  void setFlags(WFlags<ItemFlag> flags);

  /*! \brief Returns the flags.
   *
   * \sa setFlags()
   */
  WFlags<ItemFlag> flags() const;

  /*! \brief Makes the item checkable.
   *
   * Adds \link Wt::ItemFlag::UserCheckable ItemFlag::UserCheckable
   * \endlink to the item's flags.
   *
   * \sa setFlags(), setChecked()
   */
  void setCheckable(bool checkable);

  /*! \brief Returns whether the item is checkable.
   *
   * \sa setCheckable()
   */
  bool isCheckable() const;

  /*! \brief Makes the item tri-state checkable.
   *
   * When \p tristate is \c true, the item is checkable with three
   * states: Wt::CheckState::Unchecked, Wt::CheckState::Checked, and Wt::CheckState::PartiallyChecked.
   *
   * This requires that the item is also checkable (see setCheckable())
   *
   * \sa setCheckable()
   */
  void setTristate(bool tristate);

  /*! \brief Returns whether the item is tri-state checkable.
   *
   * \sa setTristate()
   */
  bool isTristate() const;

  void setEditable(bool editable);
  bool isEditable() const;

  /*! \brief Sets item data.
   *
   * Sets item data for the given role.
   *
   * \sa data()
   */
  virtual void setData(const cpp17::any& data, ItemDataRole role = ItemDataRole::User);

  /*! \brief Returns item data.
   *
   * Returns item data for the given role.
   *
   * \sa data()
   */
  virtual cpp17::any data(ItemDataRole role = ItemDataRole::User) const;

  /*! \brief Returns whether the item has any children.
   *
   * This is a convenience method and checks whether rowCount() and
   * columnCount() differ both from 0.
   *
   * \sa rowCount(), columnCount()
   */
  bool hasChildren() const;

  /*! \brief Sets the row count.
   *
   * If \p rows is bigger than the current row count, empty rows
   * are appended.
   *
   * If \p rows is smaller than the current row count, rows are
   * deleted at the end.
   *
   * \note If \p rows > 0, and columnCount() == 0, columnCount
   * is first increased to 1 using setColumnCount(1).
   *
   * \sa setColumnCount(), rowCount()
   */
  void setRowCount(int rows);

  /*! \brief Returns the row count.
   *
   * \sa setRowCount()
   */
  int rowCount() const;

  /*! \brief Sets the column count.
   *
   * If \p columns is bigger than the current column count, empty
   * columns are appended.
   *
   * If \p columns is smaller than the current column count,
   * columns are deleted at the end.
   *
   * \sa setRowCount(), columnCount()
   */
  void setColumnCount(int columns);

  /*! \brief Returns the column count.
   *
   * \sa setRowCount()
   */
  int columnCount() const;

  /*! \brief Add a single column of items.
   *
   * Appends a single column of \p items. If necessary,
   * the row count is increased.
   *
   * Equivalent to:
   * \code
   * insertColumn(columnCount(), std::move(items));
   * \endcode
   *
   * \sa insertColumn(), appendRow()
   */
  void appendColumn(std::vector<std::unique_ptr<WStandardItem> > items);

  /*! \brief Inserts a single column of items.
   *
   * Inserts a single column of \p items at column
   * \p column. If necessary, the row count is increased.
   * 
   * \sa WStandardItem::insertRow()
   */
  void insertColumn(int column,
		    std::vector<std::unique_ptr<WStandardItem> > items);

  /*! \brief Add a single row of items.
   *
   * Appends a single row of \p items. If necessary,
   * the column count is increased.
   *
   * Equivalent to:
   * \code
   * insertRow(rowCount(), std::move(items));
   * \endcode
   *
   * \sa insertRow(), appendColumn()
   */
  void appendRow(std::vector<std::unique_ptr<WStandardItem> > items);

  /*! \brief Inserts a single row of items.
   *
   * Inserts a single row of <i>items</i> at row \p row. If
   * necessary, the column count is increased.
   * 
   * \sa insertColumn()
   */
  void insertRow(int row, std::vector<std::unique_ptr<WStandardItem> > items);

  /*! \brief Inserts a number of empty columns.
   *
   * Inserts <i>count</i> empty columns at position \p column.
   * 
   * \sa insertRows()
   */
  void insertColumns(int column, int count);

  /*! \brief Inserts a number of empty rows.
   *
   * Inserts <i>count</i> empty rows at position \p row.
   * 
   * \sa insertColumns()
   */
  void insertRows(int row, int count);

  /*! \brief Appends a row containing one item.
   *
   * This is a convenience method for nodes with a single column (for
   * example for tree nodes). This adds a row with a single item, and
   * is equivalent to:
   *
   * \code
   * insertRow(rowCount(), std::move(item));
   * \endcode
   * 
   * \sa insertRow(int, std::unique_ptr<WStandardItem>)
   */
  void appendRow(std::unique_ptr<WStandardItem> item);

  /*! \brief Inserts a row containing one item.
   *
   * This is a convenience method for nodes with a single column (for
   * example for tree nodes). This inserts a row with a single item,
   * and is equivalent to:
   *
   * \if cpp
   * \code
   * std::vector<std::unique_ptr<WStandardItem>> r;
   * r.emplace_back(std::move(item));
   * insertRow(row, std::move(r));
   * \endcode
   * \elseif java
   * \code 
   * List<WStandardItem> r;
   * r.add(item);
   * insertRow(row, r);
   * \endcode
   * \endif
   * 
   * \sa insertRow(int, const std::vector<WStandardItem *>&)
   */
  void insertRow(int row, std::unique_ptr<WStandardItem> item);

  /*! \brief Appends multiple rows containing one item.
   *
   * This is a convenience method for nodes with a single column (for
   * example for tree nodes). This adds a number of rows, each
   * containing a single item, and is equivalent to:
   *
   * \code
   * insertRows(rowCount(), std::move(items));
   * \endcode
   * 
   * \sa insertRows(int, const std::vector<WStandardItem *>&)
   */
  void appendRows(std::vector<std::unique_ptr<WStandardItem> > items);

  /*! \brief Inserts multiple rows containing one item.
   *
   * This is a convenience method for nodes with a single column (for
   * example for tree nodes). This inserts a number of rows at row
   * \p row, each containing a single item, and is equivalent to:
   *
   * \if cpp
   * \code
   * for (unsigned i = 0; i < items.size(); ++i)
   *   insertRow(row + i, std::move(items[i]));
   * \endcode
   * \endif
   * 
   * \sa insertRow(int, std::unique_ptr<WStandardItem>)
   */
  void insertRows(int row,
		  std::vector<std::unique_ptr<WStandardItem> > items);

  /*! \brief Sets a child item.
   *
   * Sets a child item <i>item</i> at position (\p row,
   * \p column).
   *
   * If necessary, the rowCount() and/or the columnCount() is increased.
   *
   * \sa child().
   */
  void setChild(int row, int column,
		std::unique_ptr<WStandardItem> item);

  /*! \brief Sets a child item.
   *
   * This is a convenience method for nodes with a single column
   * (e.g. tree nodes), and is equivalent to:
   * \code
   * setChild(row, 0, std::move(item));
   * \endcode
   *
   * \sa setChild(int, int, WStandardItem *).
   */
  void setChild(int row, std::unique_ptr<WStandardItem> item);

  /*! \brief Returns a child item.
   *
   * Returns the child item at position (<i>row</i>, \p column).
   * This may be \c 0 if an item was not previously set, or if the
   * position is out of bounds.
   *
   * \sa setChild(int, int, WStandardItem *).
   */
  WStandardItem *child(int row, int column = 0) const;

  /*! \brief Takes a child out of the item.
   *
   * Returns the child item at position (<i>row</i>, \p column),
   * and removes it (by setting \c 0 instead).
   *
   * \sa child(), setChild(int, int, WStandardItem *)
   */
  std::unique_ptr<WStandardItem> takeChild(int row, int column);

  /*! \brief Takes a column of children out of the item.
   *
   * Returns the column \p column, and removes the
   * column from the model (reducing the column count by
   * one). Ownership of all items is transferred to the caller.
   *
   * \sa takeRow(), removeColumn()
   */
  std::vector<std::unique_ptr<WStandardItem> > takeColumn(int column);

  /*! \brief Takes a row of children out of the item.
   *
   * Returns the row \p row, and removes the row from the
   * model (reducing the row count by one). Ownership of all items is
   * transferred to the caller.
   *
   * \sa takeColumn(), removeRow()
   */
  std::vector<std::unique_ptr<WStandardItem> > takeRow(int row);

  /*! \brief Removes a single column.
   *
   * Removes the column \p column from the model (reducing the
   * column count by one). Is equivalent to:
   * \code
   * removeColumns(column, 1);
   * \endcode
   *
   * \sa removeColumns(), takeColumn()
   */
  void removeColumn(int column);

  /*! \brief Removes a number of columns.
   *
   * Removes \p count columns from the model (reducing the
   * column count by \p count).
   *
   * \sa removeColumn(), removeRows()
   */
  void removeColumns(int column, int count);

  /*! \brief Removes a single row.
   *
   * Removes the row \p row from the model (reducing the
   * row count by one). Is equivalent to:
   * \code
   * removeRows(row, 1);
   * \endcode
   *
   * \sa removeRows(), takeRow()
   */
  void removeRow(int row);

  /*! \brief Removes a number of rows.
   *
   * Removes \p count rows from the model (reducing the
   * row count by \p count).
   *
   * \sa removeRow(), removeColumns()
   */
  void removeRows(int row, int count);

  /*! \brief Returns the model index for this item.
   *
   * \sa WStandardItemModel::indexFromItem()
   */
  WModelIndex index() const;

  /*! \brief Returns the model.
   *
   * This is the model that this item belongs to, or 0 if the item is
   * not associated with a model.
   */
  WStandardItemModel *model() const { return model_; }

  /*! \brief Returns the parent item.
   *
   * Returns the parent item.
   *
   * \sa setChild()
   */
  WStandardItem *parent() const { return parent_; }

  /*! \brief Returns the row index.
   *
   * Returns the row index of this item in the parent.
   *
   * \sa column()
   */
  int row() const { return row_; }

  /*! \brief Returns the column index.
   *
   * Returns the column index of this item in the parent.
   *
   * \sa column()
   */
  int column() const { return column_; }

  /*! \brief Returns a clone of this item.
   *
   * \sa WStandardItemModel::setItemPrototype()
   */
  virtual std::unique_ptr<WStandardItem> clone() const;

  /*! \brief Compares the item with another item.
   *
   * This is used during sorting (from sortChildren()), and returns
   * which of the two items is the lesser, based on their data.
   *
   * The default implementation compares the data based on the value
   * corresponding to the WStandardItemModel::sortRole().
   *
   * \sa sortChildren(), WStandardItemModel::setSortRole()
   */
  virtual bool operator< (const WStandardItem& other) const;

  /*! \brief Sorts the children according to a given column and sort order.
   *
   * Children of this item, and all children items are sorted
   * recursively. Existing model indexes will be invalidated by
   * the operation (will point to other items).
   *
   * The WStandardItemModel::layoutAboutToBeChanged and
   * WStandardItemModel::layoutChanged signals are emitted before and
   * after the operation so that you get a chance to invalidate or
   * update model indexes.
   * 
   * \if cpp
   * \sa operator<(), WStandardItemModel::setSortRole()
   * \elseif java
   * \sa compare(), WStandardItemModel::setSortRole()
   * \endif
   */
  virtual void sortChildren(int column, SortOrder order);

protected:
  /*! \brief Set the model for this WStandardItem and its children.
   *
   * You may override this method if you want to change its behaviour.
   */
  virtual void setModel(WStandardItemModel *model);

private:
#ifndef WT_TARGET_JAVA
  typedef std::map<ItemDataRole, cpp17::any> DataMap;
#else
  typedef std::treemap<ItemDataRole, cpp17::any> DataMap;
#endif
  typedef std::vector<std::unique_ptr<WStandardItem> > Column;
  typedef std::vector<Column> ColumnList;

  /*! \brief Compares the item with another item.
   *
   * This is used during sorting (from sortChildren()), and returns
   * which of the two items is the lesser, based on their data.
   *
   * The default implementation compares the data based on the value
   * corresponding to the WStandardItemModel::sortRole().
   *
   * \sa sortChildren(), WStandardItemModel::setSortRole()
   */
  int compare(const WStandardItem& other) const;

  WStandardItemModel *model_;
  WStandardItem *parent_;
  int row_, column_;

  DataMap          data_;
  WFlags<ItemFlag> flags_;

  std::unique_ptr<ColumnList> columns_;

  void signalModelDataChange();
  void adoptChild(int row, int column, WStandardItem *item);
  void orphanChild(WStandardItem *item);
  void recursiveSortChildren(int column, SortOrder order);
  void renumberColumns(int column);
  void renumberRows(int row);

  friend class WStandardItemModel;
};

}

#endif // WSTANDARD_ITEM_H_
