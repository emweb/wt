// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_SQL_TRAITS_IMPL_H_
#define WT_DBO_SQL_TRAITS_IMPL_H_

#include <Wt/Dbo/SqlStatement.h>

namespace Wt {
  namespace Dbo {

template <typename V, class Enable>
void sql_value_traits<V, Enable>::bind(const char *v, SqlStatement *statement,
				       int column, int size)
{
  statement->bind(column, v);
}

template <typename Result>
void query_result_traits<Result>::getFields(Session& session,
					  std::vector<std::string> *aliases,
					  std::vector<FieldInfo>& result)
{
  /* Adds an immutable single value field */

  if (!aliases || aliases->empty())
    throw std::logic_error("Session::query(): not enough aliases for results");
  std::string name = aliases->front();
  aliases->erase(aliases->begin());

  std::string sqlType = "??"; // FIXME, get from session ?

  int flags = 0;
  std::string::const_iterator as = Impl::ifind_last_as(name);
  if (as != name.end()) {
    flags = flags | FieldFlags::AliasedName;
    name = name.substr(as - name.begin());
  }

  result.push_back(FieldInfo(name, &typeid(Result), sqlType, flags));
}

template <typename Result>
Result query_result_traits<Result>::load(Session& session,
					 SqlStatement& statement,
					 int& column)
{
  Result result;
  sql_value_traits<Result>::read(result, &statement, column++, -1);
  return result;
}

template <typename Result>
void query_result_traits<Result>::getValues(const Result& result,
					    std::vector<cpp17::any>& values)
{
  values.push_back(result);
}

template <typename Result>
void query_result_traits<Result>::setValue(Result& result,
					   int& index, const cpp17::any& value)
{
  if (index == 0)
    result = cpp17::any_cast<Result>(value);
  --index;
}

template <typename Result>
Result query_result_traits<Result>::create()
{
  return Result();
}

template <typename Result>
void query_result_traits<Result>::add(Session& session, Result& result)
{
}

template <typename Result>
void query_result_traits<Result>::remove(Result& result)
{
}

template <typename Result>
long long query_result_traits<Result>::id(const Result& result)
{
  return -1;
}

template <typename Result>
Result query_result_traits<Result>::findById(Session& session, long long id)
{
  return Result();
}

  }
}

#endif // WT_DBO_SQL_TRAITS_IMPL_H_
