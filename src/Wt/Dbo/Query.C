/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Query"

#include <iostream>
#include <string>

namespace Wt {
  namespace Dbo {
    namespace Impl {

ParameterBase::~ParameterBase()
{ }

std::string createQuerySql(StatementKind kind,
			   const std::string& select,
			   const std::string& from)
{
  if (kind == Select)
    return select + ' ' + from;
  else {
    /*
     * We cannot have " order by " in our query, but we cannot simply
     * junk evertything after " order by " since there may still be
     * parameters referenced in the " limit " or " offset " clause.
     */
    std::string result = "select count(*) " + from;

    std::size_t o = result.find(" order by ");
    if (o != std::string::npos) {
      std::size_t l = result.find(" limit ");
      std::size_t f = result.find(" offset ");

      std::size_t len;
      if (l != std::string::npos)
	len = l - o;
      else if (f != std::string::npos)
	len = f - o;
      else
	len = std::string::npos;

      result.erase(o, len);
    }

    return result;
  }
}

std::string createQuerySql(StatementKind kind, const std::string& select,
			   const std::string& from,
			   const std::string& where,
			   const std::string& groupBy,
			   const std::string& orderBy,
			   int offset, int limit)
{
  std::string result;

  if (kind == Select)
    result = select;
  else
    result = "select count(*) ";

  result += ' ' + from;

  if (!where.empty())
    result += " where " + where;

  if (!groupBy.empty())
    result += " group by " + groupBy;

  if (kind == Select && !orderBy.empty())
    result += " order by " + orderBy;

  if (offset != -1)
    result += " offset ?";

  if (limit != -1)
    result += " limit ?";

  return result;
}
    }
  }
}
				 
