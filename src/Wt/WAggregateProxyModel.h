// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WAGGREGATE_PROXY_MODEL_H_
#define WAGGREGATE_PROXY_MODEL_H_

#include <Wt/WAbstractProxyModel.h>

namespace Wt {

class WRegExp;

/*! \class WAggregateProxyModel Wt/WAggregateProxyModel.h Wt/WAggregateProxyModel.h
 *  \brief A proxy model for %Wt's item models that provides column aggregation.
 *
 * This proxy model does not store data itself, but presents data from
 * a source model, and presents methods to organize columns using aggregation,
 * through which a user may navigate (usually to obtain more detailed results
 * related to a single aggregate quantity).
 *
 * To use this proxy model, you should provide a source model using
 * setSourceModel(), and define column ranges that can be aggregated
 * using addAggregate().
 *
 * \if cpp
 * Example:
 * \code
    int COLS = 18;
    int ROWS = 20;

    // set up the source model
    model_ = std::make_shared<WStandardItemModel>(0, COLS);

    std::string columns[] = {
      "Supplier",
      "2004",
        "Q1",
          "January",
          "February",
          "March",
        "Q2",
          "April",
          "May",
          "June",
        "Q3",
          "July",
          "August",
          "September",
        "Q4",
          "October",
          "November",
          "December"
    };

    for (unsigned i = 0; i < COLS; ++i) {
      model_->setHeaderData(i, Orientation::Horizontal, columns[i]);
    }

    for (unsigned i = 0; i < ROWS; ++i) {
      model_->insertRow(i);
      for (unsigned j = 0; j < COLS; ++j)
	model_->setData(i, j, "col " + std::to_string(j));
    }

    // set up the proxy model
    auto proxy = std::make_shared<WAggregateProxyModel>();
    proxy->setSourceModel(model_);

    proxy->addAggregate(1, 2, 17);
    proxy->addAggregate(2, 3, 5);
    proxy->addAggregate(6, 7, 9);
    proxy->addAggregate(10, 11, 13);
    proxy->addAggregate(14, 15, 17);

    proxy->expandColumn(1); // expand 2004 -> Q1 Q2 Q3 Q4

    // define a view
    treeView_ = root()->addWidget(std::make_unique<WTreeView>());
    treeView_->setColumnBorder(Wt::StandardColor::Black);
    treeView_->setModel(proxy);
    treeView_->setColumnWidth(0, 160);
    treeView_->setColumnResizeEnabled(true);
    treeView_->setSortingEnabled(true);
\endcode
 * \endif
 *
 * This example would render like this:
 *
 * \image html WAggregateProxyModel-1.png "A WTreeView using a WAggregateProxyModel"
 *
 * \note This model does not support dynamic changes to the column
 * definition of the source model (i.e. insertions or deletions of
 * source model columns).
 *
 * \ingroup modelview
 */
class WT_API WAggregateProxyModel : public WAbstractProxyModel
{
public:
  /*! \brief Constructor.
   *
   * Sets up the proxy without aggregation functionality.
   */
  WAggregateProxyModel();

  /*! \brief Destructor.
   */
  virtual ~WAggregateProxyModel();

  /*! \brief Adds a new column aggregation definition.
   *
   * The \p parentColumn is the column index in the source model that
   * acts as an aggregate for columns \p firstColumn to \p
   * lastColumn. \p parentColumn must border the range defined by
   * \p firstColumn to \p lastColumn:
   * \code
   * parentColumn == firstColumn - 1 || parentColumn == lastColumn + 1 
   * \endcode
   *
   * Note that column parameters reference column indexes in the
   * source model.
   *
   * Aggregation definitions can be nested, but should be strictly
   * hierarchical.
   *
   * The aggregate column will initially be collapsed.
   *
   * Only one aggregate can be defined per \p parentColumn.
   */
  void addAggregate(int parentColumn, int firstColumn, int lastColumn);

  virtual WModelIndex mapFromSource(const WModelIndex& sourceIndex)
    const override;
  virtual WModelIndex mapToSource(const WModelIndex& proxyIndex)
    const override;

  virtual void setSourceModel(const std::shared_ptr<WAbstractItemModel>&
			      sourceModel) override;

  virtual void expandColumn(int column) override;
  virtual void collapseColumn(int column) override;

  virtual int columnCount(const WModelIndex& parent = WModelIndex())
    const override;
  virtual int rowCount(const WModelIndex& parent = WModelIndex())
    const override;

  virtual WFlags<HeaderFlag> headerFlags
    (int section, Orientation orientation = Orientation::Horizontal)
    const override;

  virtual bool setHeaderData
    (int section, Orientation orientation, const cpp17::any& value,
     ItemDataRole role = ItemDataRole::Edit) override;
  virtual cpp17::any headerData
    (int section, Orientation orientation = Orientation::Horizontal,
     ItemDataRole role = ItemDataRole::Display) const override;

  virtual WModelIndex parent(const WModelIndex& index) const override;
  virtual WModelIndex index(int row, int column,
			    const WModelIndex& parent = WModelIndex())
    const override;

  virtual void sort(int column, 
		    SortOrder order = SortOrder::Ascending) override;

private:
  struct Aggregate {
    int parentSrc_;
    int firstChildSrc_, lastChildSrc_;
    int level_;

    bool collapsed_;

    std::vector<Aggregate> nestedAggregates_;

    Aggregate();
    Aggregate(int parentColumn, int firstColumn, int lastColumn);
    bool contains(const Aggregate& other) const;
    bool contains(int column) const;
    Aggregate *add(const Aggregate& other);
    int mapFromSource(int sourceColumn) const;
    int mapToSource(int column) const;
    bool before(const Aggregate& other) const;
    bool after(int column) const;
    bool before(int column) const;
    int collapsedCount() const;
    Aggregate *findAggregate(int parentColumn);
    const Aggregate *findAggregate(int parentColumn) const;
    const Aggregate *findEnclosingAggregate(int column) const;
    int firstVisibleNotBefore(int column) const;
    int lastVisibleNotAfter(int column) const;
  };

  Aggregate topLevel_;

  std::vector<Wt::Signals::connection> modelConnections_;

  void expand(Aggregate& aggregate);
  void collapse(Aggregate& aggregate);

  void propagateBeginRemove(const WModelIndex& proxyIndex,
			    int start, int end);
  void propagateEndRemove(const WModelIndex& proxyIndex,
			  int start, int end);
  void propagateBeginInsert(const WModelIndex& proxyIndex,
			    int start, int end);
  void propagateEndInsert(const WModelIndex& proxyIndex,
			  int start, int end);

  int lastVisibleSourceNotAfter(int sourceColumn) const;
  int firstVisibleSourceNotBefore(int sourceColumn) const;
  
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
};

}

#endif // WAGGREGATE_PROXY_MODEL_H_
