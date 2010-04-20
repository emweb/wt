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

#ifndef DOXYGEN_ONLY

namespace Wt {
  namespace Dbo {
    namespace Impl {

template <class Result>
QueryBase<Result>::QueryBase(Session& session, const std::string& select,
			     const std::string& from)
  : session_(session),
    select_(select),
    from_(from)
{ }

template <class Result>
Result QueryBase<Result>::singleResult(const collection<Result>& results)
  const
{
  typename collection<Result>::const_iterator i = results.begin();
  if (i == results.end())
    return Result();
  else {
    Result result = *i;
    ++i;
    if (i != results.end())
      throw NoUniqueResultException();
    return result;
  }
}
    }

template <class Result>
Query<Result, DirectBinding>::Query(Session& session, const std::string& select,
				   const std::string& from)
  : Impl::QueryBase<Result>(session, select, from),
    statement_(0),
    countStatement_(0)
{
  prepareStatements();
}

template <class Result>
Query<Result, DirectBinding>::~Query()
{
  if (statement_)
    statement_->done();
  if (countStatement_)
    countStatement_->done();
}

template <class Result>
template <typename T>
Query<Result, DirectBinding>&
Query<Result, DirectBinding>::bind(const T& value)
{
  sql_value_traits<T>::bind(value, this->statement_, column_, -1);
  sql_value_traits<T>::bind(value, this->countStatement_, column_, -1);

  ++column_;

  return *this;
}

template <class Result>
Result Query<Result, DirectBinding>::resultValue() const
{
  return singleResult(resultList());
}

template <class Result>
collection<Result> Query<Result, DirectBinding>::resultList() const
{
  if (!statement_)
    throw std::logic_error("Query<Result, DirectBinding>::resultList() "
			   "may be called only once");

  SqlStatement *s = this->statement_, *cs = this->countStatement_;
  this->statement_ = this->countStatement_ = 0;

  return collection<Result>(&this->session_, s, cs);
}

template <class Result>
Query<Result, DirectBinding>::operator Result () const
{
  return resultValue();
}

template <class Result>
Query<Result, DirectBinding>::operator collection<Result> () const
{
  return resultList();
}

template <class Result>
void Query<Result, DirectBinding>::prepareStatements() const
{
  this->session_.flush();

  std::string sql;

  sql = Impl::createQuerySql(Impl::Select, this->select_, this->from_);
  this->statement_ = this->session_.getOrPrepareStatement(sql);

  sql = Impl::createQuerySql(Impl::Count, this->select_, this->from_);
  this->countStatement_ = this->session_.getOrPrepareStatement(sql);

  column_ = 0;
}

namespace Impl {
  template <typename T>
  void Parameter<T>::bind(SqlStatement *statement, int column)
  {
    sql_value_traits<T>::bind(v_, statement, column, -1);
  }

  template <typename T>
  Parameter<T> *Parameter<T>::clone() const
  {
    return new Parameter<T>(v_);
  }
}

template <class Result>
Query<Result, DynamicBinding>::Query(Session& session,
				     const std::string& select,
				     const std::string& from)
  : Impl::QueryBase<Result>(session, select, from),
    offset_(-1),
    limit_(-1)
{ }

template <class Result>
Query<Result, DynamicBinding>
::Query(const Query<Result, DynamicBinding>& other)
  : Impl::QueryBase<Result>(other.session_, other.select_, other.from_),
    where_(other.where_),
    groupBy_(other.groupBy_),
    orderBy_(other.orderBy_),
    offset_(-1),
    limit_(-1)
{ 
  for (unsigned i = 0; i < other.parameters_.size(); ++i)
    parameters_.push_back(other.parameters_[i]->clone());
}

template <class Result>
Query<Result, DynamicBinding>::~Query()
{
  reset();
}

template <class Result>
template <typename T>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::bind(const T& value)
{
  parameters_.push_back(new Impl::Parameter<T>(value));

  return *this;
}

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::where(const std::string& where)
{
  if (!where_.empty())
    where_ += " and ";

  where_ += "(" + where + ")";

  return *this;
}

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::orderBy(const std::string& orderBy)
{
  orderBy_ = orderBy;

  return *this;
}

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::groupBy(const std::string& groupBy)
{
  groupBy_ = groupBy;

  return *this;
}

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::offset(int offset)
{
  offset_ = offset;

  return *this;
}

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::limit(int limit)
{
  limit_ = limit;

  return *this;
}

template <class Result>
Result Query<Result, DynamicBinding>::resultValue() const
{
  return singleResult(resultList());
}

template <class Result>
collection<Result> Query<Result, DynamicBinding>::resultList() const
{
  this->session_.flush();

  std::string sql;

  sql = Impl::createQuerySql(Impl::Select, this->select_, this->from_,
			     where_, groupBy_, orderBy_, offset_, limit_);
  SqlStatement *statement = this->session_.getOrPrepareStatement(sql);
  bindParameters(statement);

  sql = Impl::createQuerySql(Impl::Count, this->select_, this->from_,
			     where_, groupBy_, orderBy_, offset_, limit_);
  SqlStatement *countStatement = this->session_.getOrPrepareStatement(sql);
 
  bindParameters(countStatement);

  return collection<Result>(&this->session_, statement, countStatement);
}

template <class Result>
Query<Result, DynamicBinding>::operator Result () const
{
  return resultValue();
}

template <class Result>
Query<Result, DynamicBinding>::operator collection<Result> () const
{
  return resultList();
}

template <class Result>
void Query<Result, DynamicBinding>::bindParameters(SqlStatement *statement)
  const
{
  unsigned i = 0;
  for (; i < parameters_.size(); ++i)
    parameters_[i]->bind(statement, i);

  if (offset_ != -1)
    statement->bind(i++, offset_);

  if (limit_ != -1)
    statement->bind(i++, limit_);
}

template <class Result>
void Query<Result, DynamicBinding>::reset()
{
  for (unsigned i = 0; i < parameters_.size(); ++i)
    delete parameters_[i];

  parameters_.clear();
}

  }
}

#endif // DOXYGEN_ONLY

#endif // WT_DBO_QUERY_IMPL_H_
