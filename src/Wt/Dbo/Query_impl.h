// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_QUERY_IMPL_H_
#define WT_DBO_QUERY_IMPL_H_

#include <stdexcept>

#include <Wt/Dbo/Exception>
#include <Wt/Dbo/SqlStatement>

namespace Wt {
  namespace Dbo {

template <class Result>
Query<Result>::Query(Session& session, const std::string& select,
		     const std::string& from)
  : session_(session),
    select_(select),
    from_(from),
    statement_(0),
    countStatement_(0)
{ 
  prepareStatements();
}

template <class Result>
Query<Result>::~Query()
{ }

template <class Result>
template <typename T>
Query<Result>& Query<Result>::bind(const T& value)
{
  prepareStatements();

  sql_value_traits<T>::bind(value, statement_, column_);
  if (countStatement_)
    sql_value_traits<T>::bind(value, countStatement_, column_);

  ++column_;

  return *this;
}

template <class Result>
Result Query<Result>::resultValue() const
{
  collection<Result> list = resultList();
  typename collection<Result>::const_iterator i = list.begin();
  if (i == list.end())
    return Result();
  else {
    Result result = *i;
    ++i;
    if (i != list.end())
      throw NoUniqueResultException();
    return result;
  }
}

template <class Result>
collection<Result> Query<Result>::resultList() const
{
  prepareStatements();

  collection<Result> result;
  result.setStatement(statement_, countStatement_, &session_);
  return result;
}

template <class Result>
Query<Result>::operator Result () const
{
  return resultValue();
}

template <class Result>
Query<Result>::operator collection<Result> () const
{
  return resultList();
}

template <class Result>
void Query<Result>::prepareStatements() const
{
  session_.flush();

  if (!statement_) {
    std::string sql = createSql(Select);
    statement_ = session_.getOrPrepareStatement(sql);
    statement_->reset();
    column_ = 0;
  }

  if (!countStatement_) {
    std::string sql = createSql(Count);
    countStatement_ = session_.getOrPrepareStatement(sql);
    countStatement_->reset();
  }
}

template <class Result>
std::string Query<Result>::createSql(StatementKind kind) const
{
  return (kind == Count ? "select count(*)" : select_) + ' ' + from_;
}

  }
}

#endif // WT_DBO_QUERY_IMPL_H_
