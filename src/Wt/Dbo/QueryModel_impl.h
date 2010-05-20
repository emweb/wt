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
int QueryModel<Result>::addColumn(const std::string& field)
{
  return addColumn(QueryColumn(field));
}

template <class Result>
int QueryModel<Result>::addColumn(const QueryColumn& column)
{
  columns_.push_back(column);

  columns_.back().displayFieldIdx_ = getFieldIndex(column.displayField_);
  if (!column.editField_.empty())
    columns_.back().editFieldIdx_ = getFieldIndex(column.editField_);

  return columns_.size() - 1;
}

template <class Result>
void QueryModel<Result>::addAllFieldsAsColumns()
{
  for (unsigned i = 0; i < fields_.size(); ++i) {
    if (fields_[i].qualifier().empty())
      addColumn(fields_[i].name());
    else
      addColumn(fields_[i].qualifier() + "." + fields_[i].name());
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
    cachedRowCount_ = query_.resultList().size();

    transaction.commit();
  }

  return cachedRowCount_;
}

template <class Result>
boost::any QueryModel<Result>::data(const WModelIndex& index, int role) const
{
  if (currentRow_ != index.row()) {
    const Result& result = resultRow(index.row());
    rowValues_.clear();
    query_result_traits<Result>::getValues(result, rowValues_);
    currentRow_ = index.row();
  }

  if (role == DisplayRole)
    return rowValues_[columns_[index.column()].displayFieldIdx_];
  else
    return boost::any();
}

template <class Result>
boost::any QueryModel<Result>::headerData(int section,
					 Orientation orientation,
					 int role) const
{
  if (orientation == Horizontal && role == DisplayRole) {
    return fields_[columns_[section].displayFieldIdx_].name();
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
  invalidateData();

  query_.orderBy(fields_[columns_[column].displayFieldIdx_].sql() + " "
		 + (order == AscendingOrder ? "asc" : "desc"));

  dataReloaded();
}

template <class Result>
const Result& QueryModel<Result>::resultRow(int row) const
{
  if (row < cacheStart_
      || row > cacheStart_ + static_cast<int>(cache_.size())) {
    cacheStart_ = std::max(row - batchSize_ / 4, 0);

    query_.offset(cacheStart_);
    query_.limit(batchSize_);

    Transaction transaction(query_.session());

    collection<Result> results = query_.resultList();
    cache_.insert(cache_.end(), results.begin(), results.end());   

    transaction.commit();
  }

  return cache_[row - cacheStart_];
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

  }
}

#endif // WT_DBO_QUERY_MODEL_IMPL_H_
