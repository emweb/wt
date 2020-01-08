// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSTANDARD_ITEM_MODEL_H_
#define WSTANDARD_ITEM_MODEL_H_

#include <Wt/WAbstractItemModel.h>

namespace Wt {

class WStandardItem;

/*! \class WStandardItemModel Wt/WStandardItemModel.h Wt/WStandardItemModel.h
 *  \brief A standard data model, which stores its data in memory.
 *
 * The standard item model supports all features of
 * WAbstractItemModel, and can thus be used to represent tables, trees
 * and tree tables.
 *
 * The data itself are organized in WStandardItem objects. There is
 * one invisible root object (invisibleRootItem()) that holds the
 * toplevel data. Most methods in this class that access or manipulate
 * data internally operate on this root item.
 *
 * If you want to use the model as a table, then you can use
 * WStandardItemModel(int, int) to set the initial table
 * size, and use the item() and setItem() methods to set data. You can
 * change the geometry by inserting rows (insertRow()) or columns
 * (insertColumn()) or removing rows (removeRow()) or columns
 * (removeColumn()).
 *
 * If you want to use the model as a tree (or tree table), then you
 * can use the default constructor to start with an empty tree, and
 * use the WStandardItem API on invisibleRootItem() to manipulate the
 * tree root. When you are building a tree, the column count at each
 * node is 1. When you are building a tree table, you can add
 * additional columns of data for each internal node. Only the items
 * in the first column have children that result in a hierarchical
 * tree structure.
 *
 * When using the model with a view class, you can use the
 * itemFromIndex() and indexFromItem() models to translate between
 * model indexes (that are used by the view class) and standard items.
 * 
 * \if cpp
 * Usage example for tabular data:
 * \code
 * int rows = 5;
 * int columns = 4;
 *
 * std::shared_ptr<Wt::WStandardItemModel> model =
 *   std::make_shared<Wt::WStandardItemModel>(rows, columns);
 *
 * for (int row = 0; row < rows; ++row) {
 *   for (int column = 0; column < columns; ++column) {
 *     auto item = std::make_unique<Wt::WStandardItem>();
 *     item->setText("Item " + std::to_string(row)
 *                   + ", " + std::to_string(column));
 *     model->setItem(row, column, std::move(item));
 *   }
 * }
 * \endcode
 *
 * Usage example for tree-like data:
 * \code
 * int topLevelRows = 5;
 * int secondLevelRows = 7;
 *
 * auto model = std::make_shared<Wt::WStandardItemModel>();
 * Wt::WStandardItem *root = model->invisibleRootItem();
 *
 * for (int row = 0; row < topLevelRows; ++row) {
 *   auto topLevel = std::make_unique<Wt::WStandardItem>();
 *   topLevel->setText("Item " + std::to_string(row));
 *   for (int row2 = 0; row2 < secondLevelRows; ++row2) {
 *     auto item = std::make_unique<Wt::WStandardItem>();
 *     item->setText("Item " + std::to_string(row)
 *                   + ": " + std::to_string(row2));
 *     topLevel->appendRow(std::move(item));
 *   }
 *   root->appendRow(std::move(topLevel));
 * }
 * \endcode
 * \endif 
 *
 * \ingroup modelview
 */
class WT_API WStandardItemModel : public WAbstractItemModel
{
public:
  /*! \brief Creates a new standard item model.
   */
  WStandardItemModel();

  /*! \brief Creates a new standard item model with an initial geometry.
   *
   * Creates a standard item model with a geometry of
   * <i>rows</i> x \p columns. All items are set to \c 0.
   */
  WStandardItemModel(int rows, int columns);

  /*! \brief Destructor.
   */
  ~WStandardItemModel();

  /*! \brief Erases all data in the model.
   *
   * After clearing the model, rowCount() and columnCount() are 0.
   */
  void clear();

  /*! \brief Returns the invisible root item.
   *
   * The invisible root item is a special item that is not rendered
   * itself, but holds the top level data.
   */
  WStandardItem *invisibleRootItem() const {
    return invisibleRootItem_.get();
  }

  /*! \brief Returns the model index for a particular item.
   *
   * If the \p item is the invisibleRootItem(), then an invalid
   * index is returned.
   *
   * \sa itemFromIndex()
   */
  WModelIndex indexFromItem(const WStandardItem *item) const;

  /*! \brief Returns the standard item that corresponds to a model index.
   *
   * If the index is an invalid index, then the invisibleRootItem() is
   * returned.
   *
   * \sa indexFromItem()
   */
  WStandardItem *itemFromIndex(const WModelIndex& index) const;

  /*! \brief Adds a single column of top level items.
   *
   * Appends a single column of top level \p items. If necessary,
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

  /*! \brief Inserts a single column of top level items.
   *
   * Inserts a single column of top level \p items at column
   * \p column. If necessary, the row count is increased.
   *
   * Equivalent to:
   * \code
   * invisibleRootItem()->insertColumn(column, std::move(items));
   * \endcode
   *
   * \sa WStandardItem::insertColumn()
   */
  void insertColumn(int column,
		    std::vector<std::unique_ptr<WStandardItem> > items);

  /*! \brief Adds a single row of top level items.
   *
   * Appends a single row of top level \p items. If necessary,
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

  using WAbstractItemModel::insertRow;
  using WAbstractItemModel::insertColumn;

  /*! \brief Inserts a single row of top level items.
   *
   * Inserts a single row of top level \p items at row
   * \p row. If necessary, the column count is increased.
   *
   * Equivalent to:
   * \code
   * invisibleRootItem()->insertRow(row, std::move(items));
   * \endcode
   *
   * \sa WStandardItem::insertRow()
   */
  void insertRow(int row, std::vector<std::unique_ptr<WStandardItem> > items);

  /*! \brief Appends a single row containing a single item.
   *
   * Appends a single toplevel row, with a single item.
   *
   * Equivalent to:
   * \code
   * insertRow(rowCount(), std::move(item));
   * \endcode
   *
   * \sa WStandardItem::insertRow(int, std::unique_ptr<WStandardItem>)
   */
  void appendRow(std::unique_ptr<WStandardItem> item);

  /*! \brief Inserts a single row containing a single item.
   *
   * Inserts a single toplevel row, with a single item.
   *
   * Equivalent to:
   * \code
   * invisibleRootItem()->insertRow(row, std::move(item));
   * \endcode
   *
   * \sa WStandardItem::insertRow(int, std::unique_ptr<WStandardItem>)
   */
  void insertRow(int row, std::unique_ptr<WStandardItem> item);

  /*! \brief Returns a toplevel item.
   *
   * Returns the top level at at (<i>row</i>, \p column). This may
   * be 0 if no item was set previously at that position, or if the
   * indicated position is out of bounds.
   *
   * Equivalent to:
   * \code
   * invisibleRootItem()->child(row, column);
   * \endcode
   *
   * \sa WStandardItem::child()
   */
  WStandardItem *item(int row, int column = 0) const;

  /*! \brief Sets a toplevel item.
   *
   * Sets the top level at at (<i>row</i>, \p column). If
   * necessary, the number of rows or columns is increased.
   *
   * If an item was previously set for that position, it is deleted
   * first.
   *
   * Equivalent to:
   * \code
   * invisibleRootItem()->setChild(row, column, std::move(item));
   * \endcode
   *
   * \sa WStandardItem::setChild(int, int, std::unique_ptr<WStandardItem>)
   */
  void setItem(int row, int column, std::unique_ptr<WStandardItem> item);

  /*! \brief Returns the item prototype.
   *
   * \sa setItemPrototype()
   */
  WStandardItem *itemPrototype() const;

  /*! \brief Sets the item prototype.
   *
   * Set the item that is cloned when an item needs to be created
   * because the model is manipulated through its WAbstractItemModel
   * API. For example, this may be needed when a view sets data at a
   * position for which no item was previously set and thus created.
   *
   * The new item is created based on this prototype by using
   * WStandardItem::clone().
   *
   * The default prototype is WStandardItem().
   *
   * \sa setItemPrototype()
   */
  void setItemPrototype(std::unique_ptr<WStandardItem> item);

  /*! \brief Takes a column out of the model.
   *
   * Removes a column from the model, and returns the items that it
   * contained.
   *
   * Equivalent to:
   * \code
   * invisibleRootItem()->takeColumn(column);
   * \endcode
   *
   * \sa WStandardItem::takeColumn(), WStandardItem::takeRow()
   */
  std::vector<std::unique_ptr<WStandardItem> > takeColumn(int column);

  /*! \brief Takes a row out of the model.
   *
   * Removes a row from the model, and returns the items that it
   * contained.
   *
   * Equivalent to:
   * \code
   * invisibleRootItem()->takeRow(row);
   * \endcode
   *
   * \sa WStandardItem::takeRow(), takeColumn()
   */
  std::vector<std::unique_ptr<WStandardItem> > takeRow(int row);

  /*! \brief Takes an item out of the model.
   *
   * Removes an item from the model, and returns it.
   *
   * Equivalent to:
   * \code
   * invisibleRootItem()->takeItem(row, column);
   * \endcode
   *
   * \sa takeItem(), WStandardItem::takeRow(), WStandardItem::takeColumn()
   */
  std::unique_ptr<WStandardItem> takeItem(int row, int column = 0);

  /*! \brief Sets header flags.
   *
   * By default, no flags are set.
   */
  void setHeaderFlags(int section, Orientation orientation,
		      WFlags<HeaderFlag> flags);


#ifndef DOXYGEN_ONLY
  using WAbstractItemModel::setData;
  using WAbstractItemModel::data;
  using WAbstractItemModel::setHeaderData;

  virtual WFlags<HeaderFlag> headerFlags
    (int section, Orientation orientation = Orientation::Horizontal) 
    const override;
  virtual WFlags<ItemFlag> flags(const WModelIndex& index) const override;

  virtual WModelIndex parent(const WModelIndex& index) const override;

  virtual cpp17::any data(const WModelIndex& index,
                       ItemDataRole role = ItemDataRole::Display)
    const override;
  virtual cpp17::any headerData(int section,
			     Orientation orientation = Orientation::Horizontal,
                             ItemDataRole role = ItemDataRole::Display)
    const override;

  virtual WModelIndex index(int row, int column,
			    const WModelIndex& parent = WModelIndex())
    const override;

  virtual int columnCount(const WModelIndex& parent = WModelIndex())
    const override;

  virtual int rowCount(const WModelIndex& parent = WModelIndex())
    const override;

  virtual bool insertColumns(int column, int count,
			     const WModelIndex& parent = WModelIndex())
    override;
  virtual bool insertRows(int row, int count,
			  const WModelIndex& parent = WModelIndex())
    override;
  virtual bool removeColumns(int column, int count,
			     const WModelIndex& parent = WModelIndex())
    override;
  virtual bool removeRows(int row, int count,
			  const WModelIndex& parent = WModelIndex())
    override;
  virtual bool setData(const WModelIndex& index, const cpp17::any& value,
                       ItemDataRole role = ItemDataRole::Edit)
    override;
  virtual bool setHeaderData(int section, Orientation orientation,
			     const cpp17::any& value,
                             ItemDataRole role = ItemDataRole::Edit)
    override;

  virtual void *toRawIndex(const WModelIndex& index) const override;
  virtual WModelIndex fromRawIndex(void *rawIndex) const override;

  virtual void dropEvent(const WDropEvent& e, DropAction action,
			 int row, int column, const WModelIndex& parent) override;

#endif // DOXYGEN_ONLY

  /*! \brief Set the role used to sort the model.
   *
   * The default role is \link Wt::ItemDataRole::Display ItemDataRole::Display\endlink.
   *
   * \sa sort().
   */
  void setSortRole(ItemDataRole role);

  /*! \brief Returns the role used to sort the model.
   *
   * \sa setSortRole()
   */
  ItemDataRole sortRole() const { return sortRole_; }

  virtual void sort(int column, SortOrder order = SortOrder::Ascending)
    override;

  /*! \brief %Signal emitted when an item is changed.
   *
   * This signal is emitted whenever data for an item has changed. The
   * item that has changed is passed as the first parameter.
   *
   * \sa WStandardItem::setData()
   */
  Signal<WStandardItem *>& itemChanged() { return itemChanged_; }

protected:
#ifndef DOXYGEN_ONLY
  void beginInsertColumns(const WModelIndex& parent, int first, int last);
  void beginInsertRows(const WModelIndex& parent, int first, int last);
  void beginRemoveColumns(const WModelIndex& parent, int first, int last);
  void beginRemoveRows(const WModelIndex& parent, int first, int last);
#endif // DOXYGEN_ONLY

private:
  typedef std::map<ItemDataRole, cpp17::any> HeaderData;
  ItemDataRole sortRole_;

  std::vector<HeaderData> columnHeaderData_, rowHeaderData_;
  std::vector<WFlags<HeaderFlag> > columnHeaderFlags_, rowHeaderFlags_;
  std::unique_ptr<WStandardItem> invisibleRootItem_, itemPrototype_;

  Signal<WStandardItem *> itemChanged_;

  void init();
  WStandardItem *itemFromIndex(const WModelIndex& index, bool lazyCreate)
    const;
  void insertHeaderData(std::vector<HeaderData>& headerData,
			std::vector<WFlags<HeaderFlag> >& fl,
			WStandardItem *item, int index,	int count);
  void removeHeaderData(std::vector<HeaderData>& headerData,
			std::vector<WFlags<HeaderFlag> >& fl,
			WStandardItem *item, int index, int count);

  friend class WStandardItem;
};

}

#endif // WSTANDARD_ITEM_MODEL_H_
