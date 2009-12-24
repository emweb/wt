/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/StdSqlTraits"
#include "Wt/Dbo/SqlStatement"

#ifndef DOXYGEN_ONLY

namespace Wt {
  namespace Dbo {

const char *sql_value_traits<std::string>::type()
{
  return "text not null";
}

void sql_value_traits<std::string>::bind(const std::string& v,
					 SqlStatement *statement, int column)
{
  statement->bind(column, v);
}

bool sql_value_traits<std::string>::read(std::string& v,
					 SqlStatement *statement, int column)
{
  if (!statement->getResult(column, &v)) {
    v.clear();
    return true;
  } else
    return false;
}

const char *sql_value_traits<long long>::type()
{
  return "integer not null";
}

void sql_value_traits<long long>::bind(long long v,
				       SqlStatement *statement, int column)
{
  statement->bind(column, v);
}

bool sql_value_traits<long long>::read(long long& v,
				       SqlStatement *statement, int column)
{
  if (!statement->getResult(column, &v))
    return true;
  else
    return false;
}

const char *sql_value_traits<int>::type()
{
  return "integer not null";
}

void sql_value_traits<int>::bind(int v, SqlStatement *statement, int column)
{
  statement->bind(column, v);
}

bool sql_value_traits<int>::read(int& v, SqlStatement *statement, int column)
{
  if (!statement->getResult(column, &v))
    return true;
  else
    return false;
}

  }
}

#endif
