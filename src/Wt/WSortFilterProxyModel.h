// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSORT_FILTER_PROXY_MODEL_H_
#define WSORT_FILTER_PROXY_MODEL_H_

#include <Wt/WAbstractProxyModel.h>
#include <regex>

namespace Wt {

/*! \class WSortFilterProxyModel Wt/WSortFilterProxyModel.h Wt/WSortFilterProxyModel.h
 *  \brief A proxy model for %Wt's item models that provides filtering
 *         and/or sorting.
 *
 * This proxy model does not store data itself, but presents data from
 * a source model, after filtering rows. It also allows sorting of the
 * source model data, without actually altering the source model. This
 * may be convenient when the source model does not support sorting
 * (i.e. does not reimplement WAbstractProxyModel::sort()), or you do
 * not want to reorder the underlying model since that affects all views
 * on the model.
 *
 * To use the proxy model to filter data, you use the methods
 * setFilterKeyColumn(), setFilterRegExp() and setFilterRole() to
 * specify a filtering operation based on the values of a single
 * column. If this filtering mechanism is too limiting, you can
 * provide specialized filtering by reimplementing the
 * filterAcceptRow() method.
 *
 * Sorting is provided by reimplementing the standard
 * WAbstractItemModel::sort() method. In this way, a view class such
 * as WTreeView may resort the model as indicated by the user. Use
 * setSortRole() to indicate on what data role sorting should be done,
 * or reimplement the lessThan() method to provide a specialized
 * sorting method.
 *
 * By default, the proxy does not automatically refilter and resort
 * when the original model changes. Data changes or row additions to
 * the source model are not automatically reflected in the proxy
 * model, but to maintain integrity, row removals in the source model
 * are always reflected in the proxy model. You can enable the model
 * to always refilter and resort when the underlying model changes
 * using setDynamicSortFilter().
 *
 * Usage example:
 * \if cpp
 * \code
 * // model is the source model
 * std::shared_ptr<Wt::WAbstractItemModel> model = ...
 *
 * // we setup a proxy to filter the source model
 * auto proxy = std::make_shared<Wt::WSortFilterProxyModel>();
 * proxy->setSourceModel(model);
 * proxy->setDynamicSortFilter(true);
 * proxy->setFilterKeyColumn(0);
 * proxy->setFilterRole(Wt::ItemDataRole::User);
 * proxy->setFilterRegExp("Wt::.*");
 *
 * // configure a view to use the proxy model instead of the source model
 * Wt::WTreeView *view = addWidget(std::make_unique<Wt::WTreeView>());
 * view->setModel(proxy);
 * ...
 * \endcode
 * \elseif java
 * \code
 * // model is the source model
 *  WAbstractItemModel model = ...
 * 
 * // we setup a proxy to filter the source model
 * WSortFilterProxyModel proxy = new WSortFilterProxyModel(this);
 * proxy.setSourceModel(model);
 * proxy.setDynamicSortFilter(true);
 * proxy.setFilterKeyColumn(0);
 * proxy.setFilterRole(ItemDataRole.User);
 * proxy.setFilterRegExp(".*");
 *		 
 * // configure a view to use the proxy model instead of the source model
 * WTreeView view = new WTreeView(this);
 * view.setModel(proxy);
 * ...
 * \endcode
 * \endif
 *
 * \ingroup modelview
 */
class WT_API WSortFilterProxyModel : public WAbstractProxyModel
{
public:
  /*! \brief Constructor.
   */
  WSortFilterProxyModel();

  /*! \brief Destructor.
   */
  virtual ~WSortFilterProxyModel();

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
   * All signals of the source model are forwarded to the
   * proxy model.
   */
  virtual void setSourceModel
    (const std::shared_ptr<WAbstractItemModel>& sourceModel) override;

  /*! \brief Specify the column on which the filtering is applied.
   *
   * This configures the column on which the filterRegExp() is applied.
   *
   * The default value is 0.
   *
   * \sa setFilterRegExp(), setFilterRole()
   */
  void setFilterKeyColumn(int column);

  /*! \brief Return the column on which the filtering is applied.
   *
   * \sa setFilterKeyColumn()
   */
  int filterKeyColumn() const { return filterKeyColumn_; }

  /*! \brief Specify a regular expression for filtering.
   *
   * This configures the regular expression used for filtering on
   * filterKeyColumn().
   *
   * The default value is an empty expression, which disables
   * filtering.
   *
   * \sa setFilterKeyColumn(), setFilterRole()
   */
  void setFilterRegExp(std::unique_ptr<std::regex> pattern);

  /*! \brief Return the regular expression used for filtering.
   *
   * \sa setFilterRegExp()
   */
  std::regex *filterRegExp() const;

  /*! \brief Specify the data role used for filtering.
   *
   * This configures the data role used for filtering on
   * filterKeyColumn().
   *
   * The default value is \link Wt::ItemDataRole::Display ItemDataRole::Display\endlink.
   *
   * \sa setFilterKeyColumn(), setFilterRegExp()
   */
  void setFilterRole(ItemDataRole role);

  /*! \brief Return the data role used for filtering.
   *
   * \sa setFilterRole()
   */
  ItemDataRole filterRole() const { return filterRole_; }

  /*! \brief Specify the data role used used for sorting.
   *
   * This configures the data role used for sorting.
   *
   * The default value is \link Wt::ItemDataRole::Display ItemDataRole::Display\endlink.
   *
   * \sa lessThan()
   */
  void setSortRole(ItemDataRole role);

  /*! \brief Return the data role used for sorting.
   *
   * \sa setSortRole()
   */
  ItemDataRole sortRole() const { return sortRole_; }

  /*! \brief Returns the current sort column.
   *
   * When sort() has not been called, the model is unsorted, and this
   * method returns -1.
   *
   * \sa sort()
   */
  int sortColumn() const { return sortKeyColumn_; }

  /*! \brief Returns the current sort order.
   *
   * \sa sort()
   */
  SortOrder sortOrder() const { return sortOrder_; }

  /*! \brief Configure the proxy to dynamically track changes in the
   *         source model.
   *
   * When \p enable is \c true, the proxy will re-filter and re-sort
   * the model when changes happen to the source model.
   *
   * \note This may be ackward when editing through the proxy model,
   * since changing some data may rearrange the model and thus
   * invalidate model indexes. Therefore it is usually less
   * complicated to manipulate directly the source model instead.
   *
   * \sa lessThan()
   */
  void setDynamicSortFilter(bool enable);

  /*! \brief Returns whether this proxy dynmically filters and sorts.
   *
   * \sa setDynamicSortFilter()
   */
  bool dynamicSortFilter() const { return dynamic_; }

  /*! \brief Invalidates the current filter.
   *
   * This refilters and resorts the model, and is useful only if you
   * have reimplemented filterAcceptRow() and/or lessThan()
   */
  void invalidate();

  virtual int columnCount(const WModelIndex& parent = WModelIndex())
    const override;
  virtual int rowCount(const WModelIndex& parent = WModelIndex())
    const override;

  virtual WModelIndex parent(const WModelIndex& index) const override;
  virtual WModelIndex index(int row, int column,
			    const WModelIndex& parent = WModelIndex())
    const override;

  virtual bool setHeaderData(int section, Orientation orientation,
			     const cpp17::any& value,
                             ItemDataRole role = ItemDataRole::Edit) override;
  virtual cpp17::any headerData(int section,
			     Orientation orientation = Orientation::Horizontal,
                         ItemDataRole role = ItemDataRole::Display) const override;
  virtual WFlags<HeaderFlag> headerFlags
    (int section, Orientation orientation = Orientation::Horizontal)
    const override;

  /*! \brief Inserts a number rows.
   *
   * The rows are inserted in the source model, and if successful,
   * also in the proxy model regardless of whether they are matched by
   * the current filter. They are inserted at the indicated row,
   * regardless of whether this is the correct place according to the
   * defined sorting.
   *
   * As soon as you set data for the column on which the filtering is
   * active, or which affects the sorting, the row may be filtered out
   * or change position when dynamic sorting/filtering is
   * enabled. Therefore, it is usually a good idea to temporarily
   * disable the dynamic sort/filtering behaviour while inserting new
   * row(s) of data.
   */
  virtual bool insertRows(int row, int count,
			  const WModelIndex& parent = WModelIndex()) override;

  /*! \brief Removes a number rows.
   *
   * The rows are removed from the source model.
   */
  virtual bool removeRows(int row, int count,
			  const WModelIndex& parent = WModelIndex()) override;

  virtual void sort(int column, SortOrder order = SortOrder::Ascending)
    override;

protected:
  /*! \brief Returns whether a source row is accepted by the filter.
   *
   * The default implementation uses filterKeyColumn(), filterRole()
   * and filterRegExp().
   *
   * You may want to reimplement this method to provide specialized
   * filtering.
   */
  virtual bool filterAcceptRow(int sourceRow, const WModelIndex& sourceParent)
    const;

#ifndef WT_TARGET_JAVA
  /*! \brief Compares two indexes.
   *
   * The default implementation uses sortRole() and an ordering using
   * the operator< when the data is of the same type or compares
   * lexicographically otherwise.
   *
   * You may want to reimplement this method to provide specialized
   * sorting.
   */
  virtual bool lessThan(const WModelIndex& lhs, const WModelIndex& rhs)
    const;
#endif

#ifdef WT_TARGET_JAVA
  /*! \brief Compares two indexes.
   *
   * The default implementation uses sortRole() and an ordering using
   * the operator< when the data is of the same type or compares
   * lexicographically otherwise.
   *
   * You may want to reimplement this method to provide specialized
   * sorting.
   */
  virtual int compare(const WModelIndex& lhs, const WModelIndex& rhs) const;
#endif

private:
  /*
   * For every proxy parent, we keep the following info:
   */
  struct Item : public BaseItem {
    // maps source rows to proxy rows
    std::vector<int> sourceRowMap_;
    // maps proxy rows to source rows
    std::vector<int> proxyRowMap_;

    Item(const WModelIndex& sourceIndex) : BaseItem(sourceIndex) { }
    virtual ~Item();
  };

  struct Compare W_JAVA_COMPARATOR(int) {
    Compare(const WSortFilterProxyModel *aModel, Item *anItem)
      : model(aModel), item(anItem) { }

#ifndef WT_TARGET_JAVA
    bool operator()(int sourceRow1, int sourceRow2) const;

    bool lessThan(int sourceRow1, int sourceRow2) const;
#endif // WT_TARGET_JAVA

    int compare(int sourceRow1, int sourceRow2) const;

    const WSortFilterProxyModel *model;
    Item *item;
  };

  std::unique_ptr<std::regex> regex_;
  int filterKeyColumn_;
  ItemDataRole filterRole_;
  int sortKeyColumn_;
  ItemDataRole sortRole_;
  SortOrder sortOrder_;
  bool dynamic_, inserting_;

  std::vector<Wt::Signals::connection> modelConnections_;
  mutable ItemMap mappedIndexes_;
  mutable Item* mappedRootItem_;

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

  Item *itemFromSourceIndex(const WModelIndex& sourceIndex) const;
  Item *parentItemFromIndex(const WModelIndex& index) const;
  Item *itemFromIndex(const WModelIndex& index) const;
  void resetMappings();
  void updateItem(Item *item) const;
  void rebuildSourceRowMap(Item *item) const;

  int mappedInsertionPoint(int sourceRow, Item *item) const;
#ifndef WT_TARGET_JAVA
  int compare(const WModelIndex& lhs, const WModelIndex& rhs) const;
#endif
};

}

#endif // WSORT_FILTER_PROXY_MODEL_H_
