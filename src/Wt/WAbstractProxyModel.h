// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WABSTRACT_PROXY_MODEL_H_
#define WABSTRACT_PROXY_MODEL_H_

#include <Wt/WAbstractItemModel.h>

namespace Wt {

/*! \class WAbstractProxyModel Wt/WAbstractProxyModel.h Wt/WAbstractProxyModel.h
 *  \brief An abstract proxy model for %Wt's item models.
 *
 * A proxy model does not store data, but presents data from a source
 * model in another way. It may provide filtering, sorting, or other
 * computed changes to the source model. A proxy model may be a fully
 * functional model, that also allows modification of the underlying
 * model.
 *
 * This abstract proxy model may be used as a starting point for
 * implementing a custom proxy model, when WSortFilterProxyModel is
 * not adequate. It implements data access and manipulation using the
 * a virtual mapping method (mapToSource()) to access and manipulate
 * the underlying sourceModel().
 *
 * \ingroup modelview
 */
class WT_API WAbstractProxyModel : public WAbstractItemModel
{
public:
  /*! \brief Constructor.
   */
  WAbstractProxyModel();

  /*! \brief Maps a source model index to the proxy model.
   *
   * This method returns a model index in the proxy model that
   * corresponds to the model index \p sourceIndex in the source
   * model. This method must only be implemented for source model
   * indexes that are mapped and thus are the result of mapToSource().
   *
   * \sa mapToSource()
   */
  virtual WModelIndex mapFromSource(const WModelIndex& sourceIndex) const = 0;

  /*! \brief Maps a proxy model index to the source model.
   *
   * This method returns a model index in the source model that
   * corresponds to the proxy model index \p proxyIndex.
   *
   * \sa mapFromSource()
   */
  virtual WModelIndex mapToSource(const WModelIndex& proxyIndex) const = 0;

  /*! \brief Sets the source model.
   *
   * The source model provides the actual data for the proxy
   * model.
   *
   * Ownership of the source model is <i>not</i> transferred.
   *
   * Note that the source model's signals are not forwarded to
   * the proxy model by default, but some specializations,
   * like WBatchEditProxyModel and WSortFilterProxyModel do.
   * If you want to reimplement data() with no changes to row
   * or column indices, consider the use of WIdentityProxyModel.
   */
  virtual void setSourceModel(const std::shared_ptr<WAbstractItemModel>&
			      sourceModel);

  /*! \brief Returns the source model.
   *
   * \sa setSourceModel()
   */
  std::shared_ptr<WAbstractItemModel> sourceModel() const {
    return sourceModel_;
  }

  using WAbstractItemModel::setData;
  using WAbstractItemModel::data;
  using WAbstractItemModel::setHeaderData;

  /*! \brief Returns the data at a specific model index.
   *
   * The default proxy implementation translates the index to the
   * source model, and calls sourceModel()->data() with this index.
   */
  virtual cpp17::any data(const WModelIndex& index, 
                       ItemDataRole role = ItemDataRole::Display) const override;

  /*! \brief Returns the row or column header data.
   *
   * The default proxy implementation constructs a dummy
   * WModelIndex with the row set to 0 and column set to \c section
   * if the orientation is Wt::Orientation::Horizontal, or with the
   * row set to \c section and the column set to 0 if the orientation
   * is Wt::Orientation::Vertical.
   *
   * The resulting section that is forwarded to sourceModel()->headerData()
   * depends on how the WModelIndex is transformed with mapToSource().
   */
  virtual cpp17::any headerData(int section,
			     Orientation orientation = Orientation::Horizontal,
                             ItemDataRole role = ItemDataRole::Display)
    const override;

  /*! \brief Sets the data at the given model index.
   *
   * The default proxy implementation calls sourceModel()->setData(mapToSource(index), value, role)
   */
  virtual bool setData(const WModelIndex& index, const cpp17::any& value,
                       ItemDataRole role = ItemDataRole::Edit) override;

  /*! \brief Sets the data at the given model index.
   *
   * The default proxy implementation calls sourceModel()->setData(mapToSource(index), values)
   */
  virtual bool setItemData(const WModelIndex& index, const DataMap& values)
    override;

  /*! \brief Returns the flags for an item.
   *
   * The default proxy implementation calls sourceModel()->flags(mapToSource(index))
   */
  virtual WFlags<ItemFlag> flags(const WModelIndex& index) const override;

  /*! \brief Returns the flags for a header.
   *
   * The default proxy implementation constructs a dummy
   * WModelIndex with the row set to 0 and column set to \c section
   * if the orientation is Wt::Orientation::Horizontal, or with the
   * row set to \c section and the column set to 0 if the orientation
   * is Wt::Orientation::Vertical.
   *
   * The resulting section that is forwarded to sourceModel()->headerFlags()
   * depends on how the WModelIndex is transformed with mapToSource().
   */
  virtual WFlags<HeaderFlag> headerFlags
    (int section, Orientation orientation = Orientation::Horizontal)
    const override;

  /*! \brief Inserts one or more columns.
   *
   * The default proxy implementation calls sourceModel()->insertColumns(column, count, parent)
   */
  virtual bool insertColumns(int column, int count,
			     const WModelIndex& parent = WModelIndex())
    override;

  /*! \brief Removes columns.
   *
   * The default proxy implementation calls sourceModel()->removeColumns(column, count, parent)
   */
  virtual bool removeColumns(int column, int count,
			     const WModelIndex& parent = WModelIndex())
    override;

  /*! \brief Returns a mime-type for dragging a set of indexes.
   *
   * The default proxy implementation calls sourceModel()->mimeType()
   */
  virtual std::string mimeType() const override;

  /*! \brief Returns a list of mime-types that could be accepted for a
   *         drop event.
   *
   * The default proxy implementation calls sourceModel()->acceptDropMimeTypes()
   */
  virtual std::vector<std::string> acceptDropMimeTypes() const override;

  /*! \brief Handles a drop event.
   *
   * The default proxy implementation maps the given row and parent
   * to the row and parent in the source model, and forwards the
   * dropEvent call to the source model.
   */
  virtual void dropEvent(const WDropEvent& e, DropAction action,
			 int row, int column, const WModelIndex& parent)
    override;

  /*! \brief Converts a model index to a raw pointer that remains valid
   *         while the model's layout is changed.
   *
   * The default proxy implementation calls sourceModel()->toRawIndex(mapToSource(index))
   */
  virtual void *toRawIndex(const WModelIndex& index) const override;

  /*! \brief Converts a raw pointer to a model index.
   *
   * The default proxy implementation calls mapFromSource(sourceModel()->fromRawIndex(rawIndex))
   */
  virtual WModelIndex fromRawIndex(void *rawIndex) const override;

protected:
  /*! \brief Create a source model index.
   *
   * This is a utility function that allows you to create indexes in
   * the source model. In this way, you can reuse the internal pointers of
   * the source model in proxy model indexes, and convert a proxy model index
   * back to the source model index using this method.
   */
  WModelIndex createSourceIndex(int row, int column, void *ptr) const;

  /*! \brief A base class for an item modeling a source index parent.
   *
   * Many mplementations of a proxy model will need to maintain a data
   * structure per source model indexes, where they relate source rows or
   * columns to proxy rows or columns, per hierarchical parent.
   *
   * It may be convenient to start from this item class as a base
   * class so that shiftModelIndexes() can be used to update this data
   * structure when the source model adds or removes rows.
   *
   * You will typically use your derived class of this item as the
   * internal pointer for proxy model indexes: a proxy model index
   * will have an item as internal pointer whose sourceIndex_ corresponds
   * to the source equivalent of the proxy model index parent.
   *
   * \sa createIndex()
   */
  struct WT_API BaseItem {
    /*! \brief The source model index.
     *
     * The source model index for this item.
     */
    WModelIndex sourceIndex_;

    /*! \brief Create a BaseItem.
     */
    BaseItem(const WModelIndex& sourceIndex) : sourceIndex_(sourceIndex) { }
    virtual ~BaseItem();
  };

  /*! \brief A map for items.
   *
   * \sa BaseItem
   */
#ifndef WT_TARGET_JAVA
  typedef std::map<WModelIndex, BaseItem *> ItemMap;
#else
  typedef std::treemap<WModelIndex, BaseItem *> ItemMap;
#endif // WT_TARGET_JAVA

  /*! \brief Utility methods to shift items in an item map.
   *
   * You can use this method to adjust an item map after the source
   * model has inserted or removed rows. When removing rows (count < 0),
   * items may possibly be removed and deleted.
   */
  void startShiftModelIndexes(const WModelIndex& sourceParent, int start, int count,
			      ItemMap& items);
  void endShiftModelIndexes(const WModelIndex& sourceParent, int start, int count,
			    ItemMap& items);

private:
  std::vector<BaseItem *> itemsToShift_;
  std::shared_ptr<WAbstractItemModel> sourceModel_;
};

}

#endif // WABSTRACT_PROXY_MODEL_H_
