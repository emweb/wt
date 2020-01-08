// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WBATCH_EDIT_PROXY_MODEL_H_
#define WBATCH_EDIT_PROXY_MODEL_H_

#include <Wt/WAbstractProxyModel.h>

namespace Wt {

/*! \class WBatchEditProxyModel Wt/WBatchEditProxyModel.h Wt/WBatchEditProxyModel.h
 *  \brief A proxy model for %Wt's item models that provides batch editing.
 *
 * This proxy model presents data from a source model, and caches any
 * editing operation without affecting the underlying source model,
 * until commitAll() or revertAll() is called. In this way, you can
 * commit all the editing in batch to the underlying source model,
 * only when the user confirms the changes.
 *
 * All editing operations are supported:
 *  - changing data (setData())
 *  - inserting and removing rows (insertRows() and removeRows())
 *  - inserting and removing columns (insertColumns() and removeColumns())
 *
 * The model supports both simple tabular models, as well as
 * hierarchical (tree-like / treetable-like) models, with children
 * under items in the first column.
 *
 * Default values for a newly inserted row can be set using
 * setNewRowData() and flags for its items using setNewRowFlags().
 *
 * \ingroup modelview
 */
class WT_API WBatchEditProxyModel : public WAbstractProxyModel
{
public:
  /*! \brief Constructor.
   */
  WBatchEditProxyModel();

  /*! \brief Destructor.
   */
  virtual ~WBatchEditProxyModel();

  /*! \brief Returns whether changes have not yet been committed.
   *
   * Returns whether have been made to the proxy model, which could be
   * committed using commitAll() or reverted using revertAll().
   */
  bool isDirty() const;

  /*! \brief Commits changes.
   *
   * This commits all changes to the source model.
   *
   * \sa revertAll()
   */
  void commitAll();

  /*! \brief Reverts changes.
   *
   * This reverts all changes.
   *
   * \sa commitAll()
   */
  void revertAll();

  /*! \brief Sets default data for a newly inserted row.
   *
   * You can use this method to initialize data for a newly inserted
   * row.
   */
  void setNewRowData(int column, const cpp17::any& data,
                     ItemDataRole role = ItemDataRole::Display);

  /*! \brief Sets the item flags for items in a newly inserted row.
   *
   * By default, flags() will return ItemFlag::Selectable.
   */
  void setNewRowFlags(int column, WFlags<ItemFlag> flags);

  /*! \brief Configures data used to indicate a modified item.
   *
   * This sets \p data for item data role \p role to be returned by
   * data() for an item that is dirty (e.g. because it belongs to a
   * newly inserted row/column, or because new data has been set for
   * it.
   *
   * When \p role is Wt::ItemDataRole::StyleClass, the style class is appended
   * to any style already returned by the source model or set by
   * setNewRowData().
   *
   * By default there is no dirty indication.
   */
  void setDirtyIndication(ItemDataRole role, const cpp17::any& data);

  virtual WModelIndex mapFromSource(const WModelIndex& sourceIndex)
    const override;

  virtual WModelIndex mapToSource(const WModelIndex& proxyIndex)
    const override;

  /*! \brief Sets the source model.
   *
   * The source model provides the actual data for the proxy
   * model.
   *
   * Ownership of the source model is <i>not</i> transferred.
   *
   * All signals of the source model are propagated to the
   * proxy model.
   */
  virtual void setSourceModel
    (const std::shared_ptr<WAbstractItemModel>& sourceModel) override;

  virtual int columnCount(const WModelIndex& parent = WModelIndex())
    const override;
  virtual int rowCount(const WModelIndex& parent = WModelIndex())
    const override;

  virtual WModelIndex parent(const WModelIndex& index) const override;
  virtual WModelIndex index(int row, int column,
			    const WModelIndex& parent = WModelIndex())
    const override;

  using WAbstractItemModel::setData;
  using WAbstractItemModel::data;
  using WAbstractItemModel::setHeaderData;

  virtual cpp17::any data(const WModelIndex& index, ItemDataRole role = ItemDataRole::Display)
    const override;

  /*! \brief Sets item data.
   *
   * The default implementation will copy Wt::ItemDataRole::Edit data to
   * Wt::ItemDataRole::Display. You may want to specialize the model to provide
   * a more specialized editing behaviour.
   */
  virtual bool setData(const WModelIndex& index, const cpp17::any& value,
                       ItemDataRole role = ItemDataRole::Edit) override;

  virtual WFlags<ItemFlag> flags(const WModelIndex& index) const override;

  virtual cpp17::any headerData(int section,
			     Orientation orientation = Orientation::Horizontal,
                             ItemDataRole role = ItemDataRole::Display) const override;

  virtual bool insertRows(int row, int count,
			  const WModelIndex& parent = WModelIndex()) override;

  virtual bool removeRows(int row, int count,
			  const WModelIndex& parent = WModelIndex()) override;

  virtual bool insertColumns(int column, int count,
			     const WModelIndex& parent = WModelIndex())
    override;

  virtual bool removeColumns(int column, int count,
			     const WModelIndex& parent = WModelIndex())
    override;

  virtual void sort(int column, 
		    SortOrder order = SortOrder::Ascending) override;

private:
  struct Cell {
    int row;
    int column;

    bool operator< (const Cell& other) const;

    Cell(int r, int c) : row(r), column(c) { }
  };

  typedef std::map<Cell, DataMap> ValueMap;

  /*
   * For every proxy parent, we keep the following info:
   */
  struct Item : public BaseItem {
    Item *insertedParent_;    // !=0 when parent was inserted

    ValueMap editedValues_;
    std::vector<int> removedRows_;      // sorted, proxy rows
    std::vector<int> insertedRows_;     // sorted, proxy rows
    std::vector<Item *> insertedItems_; // indexed by index in insertedRows_

    std::vector<int> removedColumns_;   // sorted, proxy columns
    std::vector<int> insertedColumns_;  // sorted, proxy columns

    Item(const WModelIndex& sourceIndex);
    Item(Item *insertedParent);
    virtual ~Item();
  };

  bool submitting_;
  std::map<int, DataMap> newRowData_;
  std::map<int, WFlags<ItemFlag> > newRowFlags_;
  ItemDataRole dirtyIndicationRole_;
  cpp17::any dirtyIndicationData_;

  std::vector<Wt::Signals::connection> modelConnections_;
  mutable ItemMap mappedIndexes_;

  void sourceColumnsAboutToBeInserted(const WModelIndex& parent,
				      int start, int end);
  void sourceColumnsInserted(const WModelIndex& parent, int start, int end);

  void sourceColumnsAboutToBeRemoved(const WModelIndex& parent,
				     int start, int end);
  void sourceColumnsRemoved(const WModelIndex& parent, int start, int end);

  void sourceRowsAboutToBeInserted(const WModelIndex& parent,
				   int start, int end);
  void sourceRowsInserted(const WModelIndex& parent, int start, int end);

  void sourceRowsAboutToBeRemoved(const WModelIndex& parent,
				  int start, int end);
  void sourceRowsRemoved(const WModelIndex& parent, int start, int end);

  void sourceDataChanged(const WModelIndex& topLeft,
			 const WModelIndex& bottomRight);

  void sourceHeaderDataChanged(Orientation orientation, int start, int end);

  void sourceLayoutAboutToBeChanged();
  void sourceLayoutChanged();

  void sourceModelReset();

  Item *itemFromSourceIndex(const WModelIndex& sourceIndex,
			    bool autoCreate = true) const;
  Item *itemFromInsertedRow(Item *parentItem, const WModelIndex& index,
			    bool autoCreate = true) const;
  Item *parentItemFromIndex(const WModelIndex& index) const;
  Item *itemFromIndex(const WModelIndex& index, bool autoCreate = true) const;
  bool isRemoved(const WModelIndex& sourceIndex) const;
  int adjustedProxyRow(Item *item, int sourceRow) const;
  int adjustedSourceRow(Item *item, int proxyRow) const;
  int adjustedProxyColumn(Item *item, int sourceColumn) const;
  int adjustedSourceColumn(Item *item, int proxyColumn) const;
  int adjustedProxyIndex(int sourceIndex,
			 const std::vector<int>& ins,
			 const std::vector<int>& rem) const;
  int adjustedSourceIndex(int proxyIndex,
			  const std::vector<int>& ins,
			  const std::vector<int>& rem) const;

  void insertIndexes(Item *item,
		     std::vector<int>& ins, std::vector<Item *> *rowItems,
		     int index, int count);
  void removeIndexes(Item *item,
		     std::vector<int>& ins, std::vector<int>& rem,
		     std::vector<Item *>* rowItems,
		     int index, int count);

  void deleteItemsUnder(Item *item, int row);
  void shift(std::vector<int>& v, int row, int count);
  void shiftRows(ValueMap& v, int row, int count);
  void shiftRows(Item *item, int row, int count);
  void shiftColumns(ValueMap& v, int column, int count);
  void shiftColumns(Item *item, int column, int count);

  void resetMappings();

  cpp17::any indicateDirty(ItemDataRole role, const cpp17::any& value) const;
};

}

#endif // WBATCH_EDIT_PROXY_MODEL_H_
