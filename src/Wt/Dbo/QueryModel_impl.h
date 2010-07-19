// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_QUERY_MODEL_IMPL_H_
#define WT_DBO_QUERY_MODEL_IMPL_H_

#include <Wt/Dbo/QueryColumn>

namespace Wt {
  namespace Dbo {

template <class Result>
QueryModel<Result>::QueryModel(WObject *parent)
  : WAbstractTableModel(parent),
    editStrategy_(OnFieldChange),
    batchSize_(40),
    cachedRowCount_(-1),
    cacheStart_(-1),
    currentRow_(-1)
{ }

template <class Result>
void QueryModel<Result>::setQuery(const Query<Result>& query)
{
  query_ = query;
  fields_ = query_.fields();
  columns_.clear();

  reset();
}

template <class Result>
void QueryModel<Result>::setBatchSize(int count)
{
  batchSize_ = count;
}

template <class Result>
int QueryModel<Result>::addColumn(const std::string& field,
				  WFlags<ItemFlag> flags)
{
  return addColumn(QueryColumn(field, flags));
}

template <class Result>
int QueryModel<Result>::addColumn(const QueryColumn& column)
{
  columns_.push_back(column);
  columns_.back().fieldIdx_ = getFieldIndex(column.field_);

  return columns_.size() - 1;
}

template <class Result>
void QueryModel<Result>::addAllFieldsAsColumns()
{
  for (unsigned i = 0; i < fields_.size(); ++i) {
    WFlags<ItemFlag> flags = ItemIsSelectable;

    if (fields_[i].isMutable())
      flags |= ItemIsEditable;

    if (fields_[i].qualifier().empty())
      addColumn(fields_[i].name(), flags);
    else
      addColumn(fields_[i].qualifier() + "." + fields_[i].name(), flags);
  }
}

template <class Result>
int QueryModel<Result>::columnCount(const WModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  return columns_.size();
}

template <class Result>
int QueryModel<Result>::rowCount(const WModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  if (cachedRowCount_ == -1) {
    Transaction transaction(query_.session());

    query_.limit(-1);
    query_.offset(-1);
    cachedRowCount_
      = query_.resultList().size() + addedRows_.size() - removedRows_.size();

    transaction.commit();
  }

  return cachedRowCount_;
}

template <class Result>
WFlags<ItemFlag> QueryModel<Result>::flags(const WModelIndex& index) const
{
  return columns_[index.column()].flags_;
}

template <class Result>
boost::any QueryModel<Result>::data(const WModelIndex& index, int role) const
{
  setCurrentRow(index.row());

  if (role == DisplayRole || role == EditRole)
    return rowValues_[columns_[index.column()].fieldIdx_];
  else
    return boost::any();
}

template <class Result>
void QueryModel<Result>::setCurrentRow(int row) const
{
  if (currentRow_ != row) {
    const Result& result = resultRow(row);
    rowValues_.clear();
    query_result_traits<Result>::getValues(result, rowValues_);

    /*
     * Use edited values.
     */
    if (!editedValues_.empty()) {
      for (int col = 0; col < columnCount(); ++col) {
	ValueMap::const_iterator i = editedValues_.find(index(row, col));

	if (i != editedValues_.end())
	  rowValues_[col] = i->second;
      }
    }

    currentRow_ = row;
  }
}

template <class Result>
bool QueryModel<Result>::setData(const WModelIndex& index,
				 const boost::any& value, int role)
{
  if (role == EditRole) {
    if (editStrategy_ == OnFieldChange) {
      Result& result = resultRow(index.row());

      int column = columns_[index.column()].fieldIdx_;
      query_result_traits<Result>::setValue(result, column, value);

      invalidateRow(index.row());
    } else {
      editedValues_[index] = value;

      if (currentRow_ == index.row())
	rowValues_[columns_[index.column()].fieldIdx_] = value;

      dataChanged().emit(index, index);
    }

    return true;
  } else
    return false;
}


template <class Result>
void QueryModel<Result>::setEditStrategy(EditStrategy strategy)
{
  revertAll();

  editStrategy_ = strategy;
}

template <class Result>
bool QueryModel<Result>::isDirty() const
{
  return !editedValues_.empty()
    || !addedRows_.empty()
    || !removedRows_.empty();
}

template <class Result>
void QueryModel<Result>::submitAll()
{
  if (!isDirty())
    return;

  {
    Transaction t(query_.session());

    /*
     * Remove rows
     */
    for (unsigned i = 0; i < removedRows_.size(); ++i) {
      int row = removedRows_[i];
      
      deleteRow(adjustedResultRow(row));
      cache_.erase(cache_.begin() + (row - cacheStart_));
    }

    removedRows_.clear();

    /*
     * Add rows
     */
    for (unsigned i = 0; i < addedRows_.size(); ++i)
      addRow(addedRows_[i]);

    addedRows_.clear();

    /*
     * Modify rows
     */
    for (ValueMap::const_iterator i = editedValues_.begin();
	 i != editedValues_.end(); ++i) {
      Result& result = resultRow(i->first.row());
      int column = i->first.column();
      query_result_traits<Result>::setValue(result, column, i->second);
    }

    editedValues_.clear();
    editedRows_.clear();

    t.commit();
  }
}

template <class Result>
void QueryModel<Result>::revertAll()
{
  invalidateData();

  addedRows_.clear();
  editedValues_.clear();
  editedRows_.clear();

  dataReloaded();
}

template <class Result>
boost::any QueryModel<Result>::headerData(int section,
					 Orientation orientation,
					 int role) const
{
  if (orientation == Horizontal && role == DisplayRole) {
    return fields_[columns_[section].fieldIdx_].name();
  } else
    return WAbstractTableModel::headerData(section, orientation, role);
}

template <class Result>
void QueryModel<Result>::invalidateData()
{
  layoutAboutToBeChanged().emit();

  cachedRowCount_ = cacheStart_ = currentRow_ = -1;
  cache_.clear();
  rowValues_.clear();
}

template <class Result>
void QueryModel<Result>::dataReloaded()
{
  layoutChanged().emit();  
}

template <class Result>
void QueryModel<Result>::sort(int column, SortOrder order)
{
  if (isDirty())
    revertAll();

  /*
   * This should not change the row count
   */
  int rc = cachedRowCount_;

  invalidateData();

  query_.orderBy(fields_[columns_[column].fieldIdx_].sql() + " "
		 + (order == AscendingOrder ? "asc" : "desc"));

  cachedRowCount_ = rc;
  dataReloaded();
}

template <class Result>
Result& QueryModel<Result>::resultRow(int row)
{
  typename ResultMap::iterator i = editedRows_.find(row);
  if (i != editedRows_.end())
    return i->second;

  int firstAddedRow = rowCount() - addedRows_.size();
  if (row >= firstAddedRow)
    return addedRows_[row - firstAddedRow];

  return adjustedResultRow(adjustForRemoved(row));
}

template <class Result>
Result& QueryModel<Result>::adjustedResultRow(int adjustedRow)
{
  if (adjustedRow < cacheStart_
      || adjustedRow >= cacheStart_ + static_cast<int>(cache_.size())) {
    cacheStart_ = std::max(adjustedRow - batchSize_ / 4, 0);

    query_.offset(cacheStart_);
    query_.limit(batchSize_);

    Transaction transaction(query_.session());

    collection<Result> results = query_.resultList();
    cache_.clear();
    cache_.insert(cache_.end(), results.begin(), results.end());   

    transaction.commit();
  }

  return cache_[adjustedRow - cacheStart_];
}

template <class Result>
void QueryModel<Result>::setCachedResult(int row, const Result& result)
{
  editedRows_[adjustForRemoved(row)] = result;
  invalidateRow(row);
}

template <class Result>
void QueryModel<Result>::invalidateRow(int row)
{
  if (row == currentRow_)
    currentRow_ = -1;

  WModelIndex start = index(row, 0);
  WModelIndex end = index(row, columnCount() - 1);
  dataChanged().emit(start, end);
}

template <class Result>
const Result& QueryModel<Result>::resultRow(int row) const
{
  return const_cast<const Result&>
    (const_cast<QueryModel<Result> *>(this)->resultRow(row));
}

template <class Result>
void QueryModel<Result>::reload()
{
  invalidateData();
  dataReloaded();
}

template <class Result>
int QueryModel<Result>::getFieldIndex(const std::string& field)
{
  for (unsigned i = 0; i < fields_.size(); ++i) {
    if (fields_[i].name() == field)
      return i;
    if (!fields_[i].qualifier().empty())
      if (fields_[i].qualifier() + "." + fields_[i].name() == field)
	return i;
  }

  throw Exception("QueryModel: could not find field: '" + field + "'");
}

template <class Result>
const std::vector<FieldInfo>& QueryModel<Result>::fields() const
{
  return fields_;
}

template <class Result>
WFlags<ItemFlag> QueryModel<Result>::columnFlags(int column) const
{
  return columns_[column].flags_;
}

template <class Result>
void QueryModel<Result>::setColumnFlags(int column, WFlags<ItemFlag> flags)
{
  return columns_[column].flags_ = flags;
}

template <class Result>
Result QueryModel<Result>::createRow()
{
  return query_result_traits<Result>::create();
}

template <class Result>
void QueryModel<Result>::addRow(Result& result)
{
  query_result_traits<Result>::add(query_.session(), result);
}

template <class Result>
void QueryModel<Result>::deleteRow(Result& result)
{
  query_result_traits<Result>::remove(result);
}

template <class Result>
bool QueryModel<Result>::insertRows(int row, int count,
				    const WModelIndex& parent)
{
  if (row != rowCount())
    throw Exception("QueryModel: only supporting row insertion at end");

  beginInsertRows(parent, row, row + count - 1);

  if (editStrategy_ == OnFieldChange) {
    for (int i = 0; i < count; ++i) {
      Result r = createRow();
      addRow(r);

      /*
       * Insert also into cache, this avoids a useless insert+query
       * when insertion is followed by setData() calls.
       */
      if (cacheStart_ != -1
	  && cacheStart_ + (int)cache_.size() == adjustForRemoved(row) + i)
	cache_.push_back(r);
    }
  } else {
    for (int i = 0; i < count; ++i)
      addedRows_.push_back(createRow());
  }

  cachedRowCount_ += count;

  endInsertRows();

  return true;
}

template <class Result>
bool QueryModel<Result>::removeRows(int row, int count,
				    const WModelIndex& parent)
{
  beginRemoveRows(parent, row, row + count - 1);
  
  if (editStrategy_ == OnFieldChange) {
    for (int i = 0; i < count; ++i) {
      deleteRow(resultRow(row));
      cache_.erase(cache_.begin() + (row - cacheStart_));
    }

    cachedRowCount_ -= count;
  } else {
    for (int i = 0; i < count; ++i) {
      int firstAddedRow = rowCount() - addedRows_.size();
      if (row >= firstAddedRow)
	addedRows_.erase(addedRows_.begin() + (row - firstAddedRow));
      else
	removedRows_.push_back(row);
      --cachedRowCount_;
    }

    std::sort(removedRows_.begin(), removedRows_.end());
  }

  ValueMap shifted;
  for (ValueMap::iterator i = editedValues_.begin();
       i != editedValues_.end(); ++i) {
    shifted[index(i->first.row() + count, i->first.column())] = i->second;
  }
  editedValues_ = shifted;

  endRemoveRows();

  return true;
}

template <class Result>
int QueryModel<Result>::adjustForRemoved(int row) const
{
  // Upper bound of row: suppose five rows were removed, all at row 4
  //  removedRows_ = [4, 4, 4, 4, 4]
  //  then: adjustForRemoved(4) -> 4 + 5 = 9
  //        adjustForRemoved(3) -> 3 + 0 = 3
  //        adjustForRemoveD(5) -> 5 + 5 = 10
  int i = std::upper_bound(removedRows_.begin(), removedRows_.end(), row)
    - removedRows_.begin();

  return row + i;
}

  }
}

#endif // WT_DBO_QUERY_MODEL_IMPL_H_
