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
    batchSize_(40),
    cachedRowCount_(-1),
    cacheStart_(-1),
    currentRow_(-1)
{ }

template <class Result>
void QueryModel<Result>::setQuery(const Query<Result>& query,
				  bool keepColumns)
{
  if (!keepColumns) {
    query_ = query;
    fields_ = query_.fields();
    columns_.clear();
    reset();
  } else {
    invalidateData();
    query_ = query;
    fields_ = query_.fields();
    dataReloaded();
  }
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

  return static_cast<int>(columns_.size());
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
    cachedRowCount_ = static_cast<int>(query_.resultList().size());

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

    currentRow_ = row;
  }
}

template <class Result>
bool QueryModel<Result>::setData(const WModelIndex& index,
				 const boost::any& value, int role)
{
  if (role == EditRole) {
    Transaction transaction(query_.session());

    Result& result = resultRow(index.row());

    int column = columns_[index.column()].fieldIdx_;

    const FieldInfo& field = fields()[column];

    boost::any dbValue = Wt::convertAnyToAny(value, *field.type());

    query_result_traits<Result>::setValue(result, column, dbValue);

    invalidateRow(index.row());

    transaction.commit();

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

  query_.orderBy(fields_[columns_[column].fieldIdx_].sql() + " "
		 + (order == AscendingOrder ? "asc" : "desc"));

  cachedRowCount_ = rc;
  dataReloaded();
}

template <class Result>
Result& QueryModel<Result>::resultRow(int row)
{
  if (row < cacheStart_
      || row >= cacheStart_ + static_cast<int>(cache_.size())) {
    cacheStart_ = std::max(row - batchSize_ / 4, 0);

    query_.offset(cacheStart_);
    query_.limit(batchSize_);

    Transaction transaction(query_.session());

    collection<Result> results = query_.resultList();
    cache_.clear();
    cache_.insert(cache_.end(), results.begin(), results.end());   

    if (row >= cacheStart_ + static_cast<int>(cache_.size()))
      throw Exception("QueryModel: geometry inconsistent with database");

    transaction.commit();
  }

  return cache_[row - cacheStart_];
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
const FieldInfo &QueryModel<Result>::fieldInfo(int column)
{
  return fields_[columns_[column].fieldIdx_];
}

template <class Result>
const std::string &QueryModel<Result>::fieldName(int column)
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
				       const boost::any& value,
				       int role)
{
  if (orientation == Horizontal) {
    if (role == EditRole)
      role = DisplayRole;

    columns_[section].headerData_[role] = value;

    return true;
  } else
    return WAbstractTableModel::setHeaderData(section, orientation,
					      value, role);
}

template <class Result>
boost::any QueryModel<Result>::headerData(int section, Orientation orientation,
					  int role) const
{
  if (orientation == Horizontal) {
    QueryColumn::HeaderData::const_iterator i
      = columns_[section].headerData_.find(role);

    if (i != columns_[section].headerData_.end())
      return i->second;
    else
      return boost::any();
  } else
    return WAbstractTableModel::headerData(section, orientation, role);
}

  }
}

#endif // WT_DBO_QUERY_MODEL_IMPL_H_
