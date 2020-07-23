// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_QUERY_MODEL_H_
#define WT_DBO_QUERY_MODEL_H_

#include <Wt/WAbstractTableModel.h>
#include <Wt/Dbo/Dbo.h>

namespace Wt {
  namespace Dbo {

class QueryColumn;

/*! \class QueryModel Wt/Dbo/QueryModel.h Wt/Dbo/QueryModel.h
 *  \brief A %Wt MVC Model to view/edit query results.
 *
 * The model fetches results from the query and presents the data in a
 * table. It supports sorting the underlying SQL query using
 * Query::orderBy().
 *
 * The default implementation of data() converts %Query results to
 * model data using query_result_traits<Result>::getValues(). You may
 * define your own data presentation using the underlying \p Result by
 * specializing data() and accessing data from resultRow().
 *
 * You may selectively add fields that you want to display using
 * addColumn(), or you can also add all columns based on the query
 * using addAllFieldsAsColumns().
 *
 * The model supports editing of the underlying data (even if the
 * underlying query fetches results from multiple tables!). Values in
 * columns that correspond to fields that have been mapped (and are
 * writable) in a Database Object can be edited. The default
 * implementation of setData() uses
 * query_result_traits<Result>::setValue() to manipulate the database
 * object, and thus uses the same write-behind properties as
 * ptr<C>::modify(). To customize editing, you can specialize
 * setData() and use resultRow() to modify the result object
 * directly.
 *
 * The model supports also inserting rows (only at the end), and
 * removing rows, which are reflected in object additions and removals
 * from the Session.
 *
 * Editing is directly to the underlying database objects (change,
 * insert and remove). Note that these changes will be flushed to the
 * database whenever a transaction is committed, or before a query is
 * run. The model will not explicitly create a transaction for the
 * modification, but since the model uses a query for reading data,
 * the change may be committed to the database depending on how the
 * model is loading data. Still, this implies that usually inserting a
 * row and setting its data happens within a single SQL
 * <tt>"insert"</tt> statement.
 *
 * To get good performance, the model keeps the following data cached:
 *  - rowCount()
 *  - a batch of data, controlled by setBatchSize()
 *
 * Moreover, the model will try to fetch a batch of data even if you
 * ask for the rowCount(), which may provide the row count as a
 * side-effect if the query returns less results than a single batch
 * size. If you do not want this behaviour (which might be the case if
 * you are only interested in rowCount(), not the actual data) then
 * you can avoid this behaviour by setting batchSize to 0.
 *
 * \ingroup dbo modelview
 */
template <class Result>
class QueryModel : public WAbstractTableModel
{
public:
  using WAbstractItemModel::data;
  using WAbstractItemModel::setData;

  /*! \brief Creates a new query model.
   *
   * You need to seed the model with a query using setQuery().
   */
  QueryModel();

  /*! \brief Sets the query.
   *
   * The \p query is used to query the database.
   *
   * Unless \p keepColumns is \c true, this resets the column list, so
   * you will need to (re-)add one or more columns using
   * addColumn().
   *
   * When keeping the current columns, a LayoutChange rather than a
   * Reset is emitted by the model, allowing views to keep their
   * column geometry as well.
   */
  void setQuery(const Query<Result>& query, bool keepColumns = false);

  /*! \brief Returns the query.
   *
   * \sa setQuery()
   */
  Query<Result> query() const;

  /*! \brief Adds a column.
   *
   * The \p field name may be a qualified or unqualified field
   * name. The list of available fields can be inspected using
   * fields().
   *
   * The \p header is used as Wt::ItemDataRole::Display for the column header
   * data.
   *
   * For the column items, flags() will returned the given \p
   * flags. For example, to indicate that a field is editable, you can
   * set the Wt::ItemFlag::Editable flag.
   *
   * \sa fields()
   */
  int addColumn(const std::string& field,
		const WString& header,
		WFlags<ItemFlag> flags = ItemFlag::Selectable);

  /*! \brief Adds a column.
   *
   * This is an overloaded function for convenience, which uses the
   * field name as the header value.
   *
   * \sa setHeaderData()
   */
  int addColumn(const std::string& field,
		WFlags<ItemFlag> flags = ItemFlag::Selectable);

  /*! \brief Sets column item flags.
   *
   * For items in column \p column, flags() will returned the given \p
   * flags. For example, to indicate that a field is editable, you can
   * set the Wt::ItemFlag::Editable flag.
   */
  void setColumnFlags(int column, WFlags<ItemFlag> flags);

  /*! \brief Returns column item flags.
   *
   * \sa setColumnFlags()
   */
  WFlags<ItemFlag> columnFlags(int column) const;

  // later
  int addColumn(const QueryColumn& column);

  /*! \brief Adds all the columns from the field list.
   *
   * All fields are added as columns. Fields that are mutable are marked
   * as editable columns.
   *
   * This is a convenient alternative to selectively adding columns
   * using addColumn().
   *
   * \sa fields()
   */
  void addAllFieldsAsColumns();

  /*! \brief Returns a stable result row.
   *
   * This return the same result row as was previously returned by
   * resultRow(), even if there were database inserts going on.
   *
   * This requires suitable implementations for resultId() and
   * resultById() functions.
   *
   * If the row wasn't fetched before (or since a reload()), then the
   * result of resultRow() is returned instead.
   */
  Result stableResultRow(int row) const;

  /*! \brief Returns a result row.
   *
   * This returns the result corresponding to a particular row, and
   * could be used to customize the model behaviour, e.g. by
   * specializing data() for certain columns.
   *
   * Returns a const reference to an entry in the result cache.
   */
  const Result& resultRow(int row) const;

  /*! \brief Returns a result row.
   *
   * This returns the result corresponding to a particular row, and
   * could be used to customize the model behaviour, e.g. by
   * specializing setData() for certain columns.
   *
   * Returns a reference to an entry in the result cache.
   *
   * \sa resultRow(int row) const
   */
  virtual Result& resultRow(int row);

  /*! \brief Returns the index of row.
   *
   * If the row isn't contained in the model, returns -1.
   */
  int indexOf(const Result& row) const;

  /*! \brief Rereads the data from the database.
   *
   * This invalidates the current (cached) data and informs views that
   * they should rerender. This emits the layoutAboutToBeChanged()
   * signal to inform the views that data and perhaps also row count was
   * changed.
   */
  void reload();

  /*! \brief Sets the batch size for fetching results.
   *
   * The model fetches results from the query in batch, and caches
   * these in memory to avoid repetitive querying of the database.
   *
   * The default batch size is 40.
   */
  void setBatchSize(int count);

  /*! \brief Returns the batch size for fetching results.
   *
   * \sa setBatchSize()
   */
  int batchSize() const { return batchSize_; }

  /*! \brief Returns the query field list.
   *
   * This returns the field list from the underlying query.
   */
  const std::vector<FieldInfo>& fields() const;

  /*! \brief Returns the FieldInfo structure for a column
   */
  const FieldInfo &fieldInfo(int column) const;

  /*! \brief Returns the field name for the a column
   */
  const std::string &fieldName(int column) const;

  /*! \brief Returns the number of columns.
   *
   * Returns the number of columns that have been added using
   * addColumn() or addAllFieldsAsColumns().
   *
   * Since the query model implements a flat table model, this returns
   * 0 when \p parent is valid.
   */
  virtual int columnCount(const WModelIndex& parent = WModelIndex()) const override;

  /*! \brief Returns the number of rows.
   *
   * Returns the number of rows returned from the underlying query.
   *
   * Since the query model implements a flat table model, this returns
   * 0 when \p parent is valid.
   */
  virtual int rowCount(const WModelIndex& parent = WModelIndex()) const override;

  /*! \brief Returns the flags for an item.
   *
   * Returns the flags set for the column using setColumnFlags().
   */
  virtual WFlags<ItemFlag> flags(const WModelIndex& index) const override;

  /*! \brief Returns the data for an item.
   *
   * Returns data of type Wt::ItemDataRole::Display or Wt::ItemDataRole::Edit based on the
   * field value corresponding to the index. If necessary, this
   * fetches a batch of results from the underlying database.
   *
   * The default implementation of data() converts %Query results to
   * model data using query_result_traits<Result>::getValues(). You may
   * define your own data presentation using the underlying \p Result by
   * specializing data() and accessing data from resultRow().
   */
  virtual cpp17::any data(const WModelIndex& index, 
                          ItemDataRole role = ItemDataRole::Display)
    const override;

  /*! \brief Sets data at the given model index.
   *
   * If \p role = Wt::ItemDataRole::Edit, sets the value for the field
   * corresponding to the index. All other editing is ignored.
   *
   * \sa setCachedResult()
   */
  virtual bool setData(const WModelIndex& index,
		       const cpp17::any& value,
                       ItemDataRole role = ItemDataRole::Edit) override;

  /*! \brief Sorts the model according to a particular column.
   *
   * This sorts the model by changing the query using
   * Query<BindStrategy>::orderBy().
   *
   * \sa createOrderBy()
   */
  virtual void sort(int column, 
		    SortOrder order = SortOrder::Ascending) override;

  /*! \brief Create specialized orderBy clause for sort
   *
   * The sort() method calls createOrderBy() to format a SQL
   * clause for use in Query<BindStrategy>::orderBy().
   * The default is to return a string that will sort on
   * a single column in the specified order.
   *
   * Customize this method if you need a more complex orderBy
   * clause, e.g. to break ties consistently.
   *
   * \sa sort()
   */
  virtual std::string createOrderBy(int column, SortOrder order);

  /*! \brief Inserts one or more rows.
   *
   * Row insertions are only supported at the end (\p row ==
   * rowCount()). For each added row, a new result is added to the
   * underlying database.
   */
  virtual bool insertRows(int row, int count,
			  const WModelIndex& parent = WModelIndex()) override;

  /*! \brief Removes one or more rows.
   *
   * For each removed row, the result is removed from the underlying database.
   */
  virtual bool removeRows(int row, int count,
			  const WModelIndex& parent = WModelIndex()) override;

  /*! \brief Sets header data for a column.
   *
   * The model will return this data in headerData(). Only column headers
   * are supported (orientation == Wt::Orientation::Horizontal).
   */
  virtual bool setHeaderData(int column, Orientation orientation,
			     const cpp17::any& value,
                             ItemDataRole role = ItemDataRole::Edit) override;

  /*! \brief Returns header data.
   *
   * \sa setHeaderData()
   */
  virtual cpp17::any headerData(int section,
			     Orientation orientation = Orientation::Horizontal,
                             ItemDataRole role = ItemDataRole::Display) const override;

  virtual void *toRawIndex(const WModelIndex& index) const override;
  virtual WModelIndex fromRawIndex(void *rawIndex) const override;

protected:
  /*! \brief Creates a new row.
   *
   * This method is called from within insertRows() to create a new
   * row.
   *
   * The default implementation uses query_result_traits<Result>::create().
   */
  virtual Result createRow();

  /*! \brief Adds a row to the session.
   *
   * This method is called from within insertRows() to add (and save)
   * a new result row to the Dbo session.
   *
   * The default implementation uses query_result_traits<Result>::add().
   */
  virtual void addRow(Result& result);

  /*! \brief Deletes a row from the session.
   *
   * This method is called from within removeRows() to remove (and
   * delete) a new result row from the Dbo session.
   *
   * The default implementation uses query_result_traits<Result>::remove().
   */
  virtual void deleteRow(Result& result);

  /*! \brief Returns a unique id for a result.
   *
   * This should, if possible, identify the result using a unique long long
   * id, and otherwise return -1.
   *
   * The default implementation uses query_result_traits<Result>::id().
   *
   * This is required for a working stableResultRow() implementation.
   *
   * \sa resultById()
   */
  virtual long long resultId(const Result& result) const;

  /*! \brief Returns a result by id.
   *
   * This should be the inverse of the resultId() function.
   *
   * The default implementation uses query_result_traits<Result>::findById().
   */
  virtual Result resultById(long long id) const;

private:
  void cacheRow(int row) const;

  typedef std::vector<cpp17::any> AnyList;
  typedef std::map<int, long long> StableResultIdMap;

  std::vector<QueryColumn> columns_;

  mutable Query<Result> query_;
  int queryLimit_, queryOffset_, batchSize_;
  mutable std::string sortOrderBy_;

  mutable int cachedRowCount_;
  mutable int cacheStart_;
  mutable std::vector<Result> cache_;

  mutable int currentRow_;
  mutable AnyList rowValues_;

  mutable StableResultIdMap stableIds_;

  std::vector<FieldInfo> fields_;

  int getFieldIndex(const std::string& field);

  void setCurrentRow(int row) const;
  void invalidateData();
  void invalidateRow(int row);
  void dataReloaded();
};

  }
}

#ifndef WT_DBO_QUERY_MODEL_IMPL_H_
#include <Wt/Dbo/QueryModel_impl.h>
#endif // WT_DBO_QUERY_MODEL_IMPL_H_

#endif // WT_DBO_QUERY_MODEL_H_
