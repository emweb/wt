// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_SQL_TRAITS_IMPL_H_
#define WT_DBO_SQL_TRAITS_IMPL_H_

#include <Wt/Dbo/SqlStatement>

namespace Wt {
  namespace Dbo {

template <typename V, class Enable>
void sql_value_traits<V, Enable>::bind(const char *v, SqlStatement *statement,
				       int column)
{
  statement->bind(column, v);
}

template <typename Result>
std::string sql_result_traits<Result>
::getColumns(Session& session, std::vector<std::string> *aliases)
{
  if (!aliases || aliases->empty())
    throw std::logic_error("Session::query(): not enough aliases for results");

  std::string result = aliases->front();
  aliases->erase(aliases->begin());
  return result;
}

template <typename Result>
Result sql_result_traits<Result>::loadValues(Session& session,
					     SqlStatement& statement,
					     int& column)
{
  Result result;
  sql_value_traits<Result>::read(result, &statement, column++);
  return result;
}

  }
}

#endif // WT_DBO_SQL_TRAITS_IMPL_H_
