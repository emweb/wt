// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_QUERY_IMPL_H_
#define WT_DBO_QUERY_IMPL_H_

#include <tuple>

#include <Wt/Dbo/Exception.h>
#include <Wt/Dbo/Field.h>
#include <Wt/Dbo/SqlStatement.h>
#include <Wt/Dbo/DbAction.h>

#include <Wt/Dbo/Field_impl.h>

#ifndef DOXYGEN_ONLY

namespace Wt {
  namespace Dbo {
    namespace Impl {

extern std::string WTDBO_API
completeQuerySelectSql(const std::string& sql,
		       const std::string& join,
		       const std::string& where,
		       const std::string& groupBy,
		       const std::string& having,
		       const std::string& orderBy,
		       int limit, int offset,
		       const std::vector<FieldInfo>& fields,
		       LimitQuery useRowsFromTo);

extern std::string WTDBO_API
createQuerySelectSql(const std::string& from,
		     const std::string& join,
		     const std::string& where,
		     const std::string& groupBy,
		     const std::string& having,
		     const std::string& orderBy,
		     int limit, int offset,
		     const std::vector<FieldInfo>& fields,
		     LimitQuery useRowsFromTo);

extern std::string WTDBO_API
createQueryCountSql(const std::string& query,
                    bool requireSubqueryAlias);

extern void WTDBO_API
substituteFields(const SelectFieldList& list,
		 const std::vector<FieldInfo>& fs,
		 std::string& sql,
                 int& offset);

extern void WTDBO_API 
parseSql(const std::string& sql, SelectFieldLists& fieldLists);

template <class Result>
QueryBase<Result>::QueryBase()
  : session_(nullptr)
{ }

template <class Result>
QueryBase<Result>::QueryBase(Session& session, const std::string& sql)
  : session_(&session),
    sql_(sql)
{
  parseSql(sql_, selectFieldLists_);
}

template <class Result>
QueryBase<Result>::QueryBase(Session& session, const std::string& table,
			     const std::string& where)
  : session_(&session)
{
  sql_ = "from " + table + ' ' + where;
}

template <class Result>
QueryBase<Result>& QueryBase<Result>::operator=(const QueryBase<Result>& other)
{
  session_ = other.session_;
  sql_ = other.sql_;
  selectFieldLists_ = other.selectFieldLists_;

  return *this;
}

template <class Result>
std::vector<FieldInfo> QueryBase<Result>::fields() const
{
  std::vector<FieldInfo> result;

  if (selectFieldLists_.empty())
    query_result_traits<Result>::getFields(*session_, 0, result);
  else {
    /*
     * We'll build only the aliases from the first selection list
     * (this matters only for compound selects
     */
    fieldsForSelect(selectFieldLists_[0], result);
  }

  return result;
}

template <class Result>
std::pair<SqlStatement *, SqlStatement *>
QueryBase<Result>::statements(const std::string& join,
			      const std::string& where,
			      const std::string& groupBy,
			      const std::string& having,
			      const std::string& orderBy,
			      int limit, int offset) const
{
  SqlStatement *statement, *countStatement;

  if (selectFieldLists_.empty()) {
    /*
     * sql_ is "from ..."
     */
    std::string sql;

    std::vector<FieldInfo> fs = this->fields();
    sql = Impl::createQuerySelectSql(sql_, join, where, groupBy, having, orderBy,
                                     limit, offset, fs,
                                     this->session_->limitQueryMethod_);
    statement = this->session_->getOrPrepareStatement(sql);

    sql = Impl::createQueryCountSql(sql, this->session_->requireSubqueryAlias_);

    countStatement = this->session_->getOrPrepareStatement(sql);
  } else {
    /*
     * sql_ is complete "[with ...] select ..."
     */
    std::string sql = sql_;
    int sql_offset = 0;

    std::vector<FieldInfo> fs;
    for (unsigned i = 0; i < selectFieldLists_.size(); ++i) {
      const SelectFieldList& list = selectFieldLists_[i];

      fs.clear();
      this->fieldsForSelect(list, fs);

      Impl::substituteFields(list, fs, sql, sql_offset);
    }

    sql = Impl::completeQuerySelectSql(sql, join, where, groupBy, having, orderBy,
                                       limit, offset, fs,
                                       this->session_->limitQueryMethod_);

    statement = this->session_->getOrPrepareStatement(sql);

    sql = Impl::createQueryCountSql(sql, this->session_->requireSubqueryAlias_);

    countStatement = this->session_->getOrPrepareStatement(sql);
  }

  return std::make_pair(statement, countStatement);
}

template <class Result>
void QueryBase<Result>::fieldsForSelect(const SelectFieldList& list,
					std::vector<FieldInfo>& result) const
{
  std::vector<std::string> aliases;
  for (unsigned i = 0; i < list.size(); ++i) {
    const SelectField& field = list[i];
    aliases.push_back(sql_.substr(field.begin, field.end - field.begin));
  }

  query_result_traits<Result>::getFields(*session_, &aliases, result);
  if (!aliases.empty())
    throw Exception("Session::query(): too many aliases for result");
}

template <class Result>
Session& QueryBase<Result>::session() const
{
  return *session_;
}

template <class Result>
Result QueryBase<Result>::singleResult(const collection<Result>& results) const
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
Query<Result, DirectBinding>::Query()
  : statement_(nullptr),
    countStatement_(nullptr)
{ }

template <class Result>
Query<Result, DirectBinding>::Query(Session& session, const std::string& sql)
  : Impl::QueryBase<Result>(session, sql),
    statement_(nullptr),
    countStatement_(nullptr)
{
  prepareStatements();
}

template <class Result>
Query<Result, DirectBinding>::Query(Session& session, const std::string& table,
				    const std::string& where)
  : Impl::QueryBase<Result>(session, table, where),
    statement_(nullptr),
    countStatement_(nullptr)
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
void Query<Result, DirectBinding>::reset()
{
  column_ = 0;
  this->statement_->reset();
  this->countStatement_->reset();
}

template <class Result>
Result Query<Result, DirectBinding>::resultValue() const
{
  return this->singleResult(resultList());
}

template <class Result>
collection<Result> Query<Result, DirectBinding>::resultList() const
{
  if (!this->session_)
    return collection<Result>();

  if (!statement_)
    throw std::logic_error("Query<Result, DirectBinding>::resultList() "
			   "may be called only once");

  SqlStatement *s = this->statement_, *cs = this->countStatement_;
  this->statement_ = this->countStatement_ = nullptr;

  return collection<Result>(this->session_, s, cs);
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
  if (!this->session_)
    return;

  this->session_->flush();

  std::tie(this->statement_, this->countStatement_) 
    = this->statements(std::string(), std::string(), std::string(), std::string(),
		       std::string(), -1, -1);

  column_ = 0;
}

namespace Impl {
  template <typename T>
  void Parameter<T>::bind(SaveBaseAction& binder)
  {
    field(binder, v_, "parameter");
  }

  template <typename T>
  Parameter<T> *Parameter<T>::clone() const
  {
    return new Parameter<T>(v_);
  }
}



template <class Result>
Query<Result, DynamicBinding>::Query()
{ }

template <class Result>
Query<Result, DynamicBinding>::Query(Session& session, const std::string& sql)
  : Impl::QueryBase<Result>(session, sql)
{ }

template <class Result>
Query<Result, DynamicBinding>::Query(Session& session,
				     const std::string& table,
				     const std::string& where)
  : Impl::QueryBase<Result>(session, table, where)
{ }

template <class Result>
Query<Result, DynamicBinding>
::Query(const Query<Result, DynamicBinding>& other)
  : AbstractQuery(other),
    Impl::QueryBase<Result>(other)
{ }

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::operator=
(const Query<Result, DynamicBinding>& other)
{
  Impl::QueryBase<Result>::operator=(other);
  AbstractQuery::operator=(other);
  return *this;
}

template <class Result>
Query<Result, DynamicBinding>::~Query()
{
  reset();
}

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::join(const std::string& other)
{
  AbstractQuery::join(other);

  return *this;
}

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::leftJoin(const std::string& other)
{
  AbstractQuery::leftJoin(other);

  return *this;
}

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::rightJoin(const std::string& other)
{
  AbstractQuery::rightJoin(other);

  return *this;
}

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::where(const std::string& where)
{
  AbstractQuery::where(where);

  return *this;
}

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::orderBy(const std::string& orderBy)
{
  AbstractQuery::orderBy(orderBy);

  return *this;
}

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::groupBy(const std::string& groupBy)
{
  AbstractQuery::groupBy(groupBy);

  return *this;
}

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::having(const std::string& having)
{
  AbstractQuery::having(having);

  return *this;
}
template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::offset(int offset)
{
  AbstractQuery::offset(offset);

  return *this;
}

template <class Result>
Query<Result, DynamicBinding>&
Query<Result, DynamicBinding>::limit(int limit)
{
  AbstractQuery::limit(limit);

  return *this;
}

template <class Result>
Result Query<Result, DynamicBinding>::resultValue() const
{
  return this->singleResult(resultList());
}

template <class Result>
collection<Result> Query<Result, DynamicBinding>::resultList() const
{
  if (!this->session_)
    return collection<Result>();

  this->session_->flush();

  SqlStatement *statement, *countStatement;

  std::tie(statement, countStatement)
    = this->statements(join_, where_, groupBy_, having_, orderBy_, limit_, offset_);

  bindParameters(this->session_, statement);
  bindParameters(this->session_, countStatement);

  return collection<Result>(this->session_, statement, countStatement);
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

  }
}

#endif // DOXYGEN_ONLY

#endif // WT_DBO_QUERY_IMPL_H_
