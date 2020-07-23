// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_QUERY_MODEL_IMPL_H_
#define WT_DBO_QUERY_MODEL_IMPL_H_

#include <Wt/Dbo/QueryModel.h>
#include <Wt/Dbo/QueryColumn.h>

#include <string>

namespace Wt {
  namespace Dbo {

template <class Result>
QueryModel<Result>::QueryModel()
  : batchSize_(40),
    cachedRowCount_(-1),
    cacheStart_(-1),
    currentRow_(-1)
{ }

template <class Result>
void QueryModel<Result>::setQuery(const Query<Result>& query,
				  bool keepColumns)
{
  queryLimit_ = query.limit();
  queryOffset_ = query.offset();

  if (!keepColumns) {
    query_ = query;
    fields_ = query_.fields();
    columns_.clear();
    sortOrderBy_.clear();
    reset();
  } else {
    invalidateData();
    query_ = query;
    fields_ = query_.fields();
    if (!sortOrderBy_.empty()) {
      query_.orderBy(sortOrderBy_);
    }
    dataReloaded();
  }
}

template <class Result>
Query<Result> QueryModel<Result>::query() const
{
  Query<Result> result = query_;
  result.limit(queryLimit_);
  result.offset(queryOffset_);
  return result;
}

template <class Result>
void QueryModel<Result>::setBatchSize(int count)
{
  batchSize_ = count;
}

template <class Result>
int QueryModel<Result>::addColumn(const std::string& field,
				  const WString& header,
				  WFlags<ItemFlag> flags)
{
  return addColumn(QueryColumn(field, header, flags));  
}

template <class Result>
int QueryModel<Result>::addColumn(const std::string& field,
				  WFlags<ItemFlag> flags)
{
  return addColumn(QueryColumn(field, WString::fromUTF8(field), flags));
}

template <class Result>
int QueryModel<Result>::addColumn(const QueryColumn& column)
{
  columns_.push_back(column);
  columns_.back().fieldIdx_ = getFieldIndex(column.field_);

  return static_cast<int>(columns_.size() - 1);
}

template <class Result>
void QueryModel<Result>::addAllFieldsAsColumns()
{
  for (unsigned i = 0; i < fields_.size(); ++i) {
    WFlags<ItemFlag> flags = ItemFlag::Selectable;

    if (fields_[i].isMutable())
      flags |= ItemFlag::Editable;

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

  return static_cast<int>(columns_.size());
}

template <class Result>
int QueryModel<Result>::rowCount(const WModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  if (cachedRowCount_ == -1) {
    if (batchSize_)
      cacheRow(0);

    if (cachedRowCount_ == -1) {
      Transaction transaction(query_.session());

      query_.limit(queryLimit_);
      query_.offset(queryOffset_);

      Query<Result> unorderedQuery(query_);
      unorderedQuery.orderBy("");
      cachedRowCount_ = static_cast<int>(unorderedQuery.resultList().size());

      transaction.commit();
    }
  }

  return cachedRowCount_;
}

template <class Result>
WFlags<ItemFlag> QueryModel<Result>::flags(const WModelIndex& index) const
{
  return columns_[index.column()].flags_;
}

template <class Result>
cpp17::any QueryModel<Result>::data(const WModelIndex& index, ItemDataRole role) const
{
  setCurrentRow(index.row());

  if (role == ItemDataRole::Display || role == ItemDataRole::Edit)
    return rowValues_[columns_[index.column()].fieldIdx_];
  else
    return cpp17::any();
}

template <class Result>
void QueryModel<Result>::setCurrentRow(int row) const
{
  if (currentRow_ != row) {
    Transaction transaction(query_.session());

    const Result& result = resultRow(row);
    rowValues_.clear();
    query_result_traits<Result>::getValues(result, rowValues_);

    currentRow_ = row;

    transaction.commit();
  }
}

template <class Result>
bool QueryModel<Result>::setData(const WModelIndex& index,
                                 const cpp17::any& value, ItemDataRole role)
{
  if (role == ItemDataRole::Edit) {
    {
      Transaction transaction(query_.session());

      Result& result = resultRow(index.row());

      int column = columns_[index.column()].fieldIdx_;

      const FieldInfo& field = fields()[column];

      cpp17::any dbValue = Wt::convertAnyToAny(value, *field.type());

      query_result_traits<Result>::setValue(result, column, dbValue);

      transaction.commit();
    }

    invalidateRow(index.row());

    return true;
  } else
    return false;
}

template <class Result>
void QueryModel<Result>::invalidateData()
{
  layoutAboutToBeChanged().emit();

  cachedRowCount_ = cacheStart_ = currentRow_ = -1;
  cache_.clear();
  rowValues_.clear();
  stableIds_.clear();
}

template <class Result>
void QueryModel<Result>::dataReloaded()
{
  layoutChanged().emit();  
}

template <class Result>
void QueryModel<Result>::sort(int column, SortOrder order)
{
  /*
   * This should not change the row count
   */
  int rc = cachedRowCount_;

  invalidateData();

  sortOrderBy_ = createOrderBy(column, order);
  query_.orderBy(sortOrderBy_);

  cachedRowCount_ = rc;
  dataReloaded();
}

template <class Result>
std::string QueryModel<Result>::createOrderBy(int column, SortOrder order)
{
  return fieldInfo(column).sql() + " "
    + (order == SortOrder::Ascending ? "asc" : "desc");
}

template <class Result>
Result QueryModel<Result>::stableResultRow(int row) const
{
  StableResultIdMap::const_iterator i = stableIds_.find(row);

  if (i != stableIds_.end())
    return resultById(i->second);
  else
    return resultRow(row);
}

template <class Result>
Result& QueryModel<Result>::resultRow(int row)
{
  cacheRow(row);

  if (row >= cacheStart_ + static_cast<int>(cache_.size()))
    throw Exception("QueryModel: geometry inconsistent with database: "
                    "row (= " + std::to_string(row) + ") >= "
                    "cacheStart_ (= " + std::to_string(cacheStart_) + ") + "
                    "cache_.size() (= " + std::to_string(cache_.size()) + ")");

  return cache_[row - cacheStart_];
}

template <class Result>
int QueryModel<Result>::indexOf(const Result& result) const
{
  for (int i = 0; i < rowCount(); ++i) {
    if (resultRow(i) == result)
      return i;
  }

  return -1;
}

template <class Result>
void QueryModel<Result>::cacheRow(int row) const
{
  if (row < cacheStart_
      || row >= cacheStart_ + static_cast<int>(cache_.size())) {
    cacheStart_ = std::max(row - batchSize_ / 4, 0);

    int qOffset = cacheStart_;
    if (queryOffset_ > 0)
      qOffset += queryOffset_;
    query_.offset(qOffset);

    int qLimit = batchSize_;
    if (queryLimit_ > 0)
      qLimit = std::min(batchSize_, queryLimit_ - cacheStart_);
    query_.limit(qLimit);

    Transaction transaction(query_.session());

    collection<Result> results = query_.resultList();
    cache_.clear();
    cache_.insert(cache_.end(), results.begin(), results.end());   

    for (unsigned i = 0; i < cache_.size(); ++i) {
      long long id = resultId(cache_[i]);
      if (id != -1)
	stableIds_[cacheStart_ + i] = id;
    }
    if (static_cast<int>(cache_.size()) < qLimit
        && qOffset == 0 && cachedRowCount_ == -1)
      cachedRowCount_ = cache_.size();

    transaction.commit();
  }
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
const FieldInfo &QueryModel<Result>::fieldInfo(int column) const
{
  return fields_[columns_[column].fieldIdx_];
}

template <class Result>
const std::string &QueryModel<Result>::fieldName(int column) const
{
  return columns_[column].field_;
}

template <class Result>
WFlags<ItemFlag> QueryModel<Result>::columnFlags(int column) const
{
  return columns_[column].flags_;
}

template <class Result>
void QueryModel<Result>::setColumnFlags(int column, WFlags<ItemFlag> flags)
{
  columns_[column].flags_ = flags;
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

  for (int i = 0; i < count; ++i) {
    Result r = createRow();
    addRow(r);

    /*
     * Insert also into cache, this avoids a useless insert+query
     * when insertion is followed by setData() calls.
     */
    if (cacheStart_ != -1
	&& cacheStart_ + (int)cache_.size() == row + i)
      cache_.push_back(r);
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
  
  for (int i = 0; i < count; ++i) {
    deleteRow(resultRow(row));
    cache_.erase(cache_.begin() + (row - cacheStart_));
  }

  cachedRowCount_ -= count;

  endRemoveRows();

  return true;
}

template <class Result>
bool QueryModel<Result>::setHeaderData(int section, Orientation orientation,
				       const cpp17::any& value,
                                       ItemDataRole role)
{
  if (orientation == Orientation::Horizontal) {
    if (role == ItemDataRole::Edit)
      role = ItemDataRole::Display;

    columns_[section].headerData_[role] = value;

    headerDataChanged().emit(orientation, section, section);

    return true;
  } else
    return WAbstractTableModel::setHeaderData(section, orientation,
					      value, role);
}

template <class Result>
cpp17::any QueryModel<Result>::headerData(int section, Orientation orientation,
                                          ItemDataRole role) const
{
  if (orientation == Orientation::Horizontal) {
    if (role == ItemDataRole::Level)
      return WAbstractTableModel::headerData(section, orientation, role);

    QueryColumn::HeaderData::const_iterator i
      = columns_[section].headerData_.find(role);

    if (i != columns_[section].headerData_.end())
      return i->second;
    else
      return cpp17::any();
  } else
    return WAbstractTableModel::headerData(section, orientation, role);
}

template <class Result>
long long QueryModel<Result>::resultId(const Result& result) const
{
  return query_result_traits<Result>::id(result);
}

template <class Result>
Result QueryModel<Result>::resultById(long long id) const
{
  Transaction transaction(query_.session());

  return query_result_traits<Result>::findById(query_.session(), id);
}

template <class Result>
void *QueryModel<Result>::toRawIndex(const WModelIndex& index) const
{
  if (index.isValid()) {
    long long id = resultId(resultRow(index.row()));

    if (id >= 0)
      return reinterpret_cast<void *>(id + 1);
    else
      return nullptr;
  } else
    return nullptr;
}

template <class Result>
WModelIndex QueryModel<Result>::fromRawIndex(void *rawIndex) const
{
  if (rawIndex) {
    long long id = reinterpret_cast<long long>(rawIndex) - 1;

    for (int row = 0; row < std::min(rowCount(), batchSize_); ++row) {
      const Result& result = resultRow(row);

      if (resultId(result) == id)
	return index(row, 0);
    }
  }

  return WModelIndex();
}

  }
}

#endif // WT_DBO_QUERY_MODEL_IMPL_H_
