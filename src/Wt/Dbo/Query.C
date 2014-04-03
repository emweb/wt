/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Query"
#include "Query_impl.h"
#include "SqlTraits"
#include "ptr"

#include <string>
#include <boost/algorithm/string.hpp>

namespace Wt {
  namespace Dbo {
    namespace Impl {

std::size_t ifind(const std::string& s, const std::string& needle)
{
  boost::iterator_range<std::string::const_iterator> i
    = boost::ifind_first(s, needle);
  if (!i)
    return std::string::npos;
  else
    return i.begin() - s.begin();
}

std::string selectColumns(const std::vector<FieldInfo>& fields) {
  std::string result;

  for (unsigned i = 0; i < fields.size(); ++i) {
    const FieldInfo& field = fields[i];
    if (!result.empty())
      result += ", ";

    result += field.sql();
  }

  return result;
}

ParameterBase::~ParameterBase()
{ }

void addGroupBy(std::string& result, const std::string& groupBy,
		const std::vector<FieldInfo>& fields)
{		      
  std::vector<std::string> groupByFields;
  boost::split(groupByFields, groupBy, boost::is_any_of(","));

  for (unsigned i = 0; i < groupByFields.size(); ++i) {
    boost::trim(groupByFields[i]);

    std::string g;
    for (unsigned j = 0; j < fields.size(); ++j)
      if (fields[j].qualifier() == groupByFields[i]) {
	if (!g.empty())
	  g += ", ";
	g += fields[j].sql();
      }

    if (!g.empty())
      groupByFields[i] = g;
  }

  result += " group by ";
  for (unsigned i = 0; i < groupByFields.size(); ++i) {
    if (i != 0)
      result += ", ";
    result += groupByFields[i];
  }
}
std::string addLimitQuery(const std::string& sql, int limit, int offset,
			  LimitQuery limitQueryMethod)
{
  std::string result = sql;
  
  if (limitQueryMethod == Limit) {
    if (limit != -1)
      result += " limit ?";

    if (offset != -1)
      result += " offset ?";
  }
  else if (( limitQueryMethod == RowsFromTo )&&
    ( limit != -1 || offset != -1) ){
    result += " rows ? to ?";
  }
  else { // useRowsFromTo == Rownum
    if (limit != -1 && offset == -1)
      result = " select * from ( " + result + " ) where rownum <= ?";
    else if (limit != -1 && offset != -1)
      result = " select * from ( select row_.*, rownum rownum2 from ( " +
	result + " ) row_ where rownum <= ?) where rownum2 > ?";
  }

  return result;
}
std::string completeQuerySelectSql(const std::string& sql,
				   const std::string& where,
				   const std::string& groupBy,
				   const std::string& orderBy,
				   int limit, int offset,
				   const std::vector<FieldInfo>& fields,
				   LimitQuery limitQueryMethod)
{
  std::string result = sql;

  if (!where.empty())
    result += " where " + where;

  if (!groupBy.empty())
    addGroupBy(result, groupBy, fields);

  if (!orderBy.empty())
    result += " order by " + orderBy;

  return addLimitQuery(result, limit, offset, limitQueryMethod);
}

std::string createQuerySelectSql(const std::string& from,
				 const std::string& where,
				 const std::string& groupBy,
				 const std::string& orderBy,
				 int limit, int offset,
				 const std::vector<FieldInfo>& fields,
				 LimitQuery limitQueryMethod)
{
  std::string result = "select " + selectColumns(fields) + ' ' + from;

  if (!where.empty())
    result += " where " + where;

  if (!groupBy.empty())
    addGroupBy(result, groupBy, fields);

  if (!orderBy.empty())
    result += " order by " + orderBy;

  return addLimitQuery(result, limit, offset, limitQueryMethod);
}

std::string createWrappedQueryCountSql(const std::string& query,
				       bool requireSubqueryAlias)
{
  if (requireSubqueryAlias)
    return "select count(1) from (" + query + ") dbocount";
  else
    return "select count(1) from (" + query + ")";
}

std::string createQueryCountSql(const std::string& query,
				const std::string& from,
				const std::string& where,
				const std::string& groupBy,
				const std::string& orderBy,
				int limit, int offset,
				LimitQuery limitQueryMethod,
				bool requireSubqueryAlias)
{
  /*
   * If there is a " group by ", then we cannot simply substitute
   * count(1), that still gives multiple results for each group.
   *
   * We cannot have " order by " in our query (e.g. on PostgreSQL)
   * except when ordering by the count (e.g. when we have a group by),
   * but we cannot simply junk evertything after " order by " since
   * there may still be parameters referenced in the " limit " or "
   * offset " clause.
   *
   * The Internet consensus is that wrapping like this is not really
   * a performance loss so we do not take any risk here.
   *
   * Also, we cannot count like this when we have a limit or offset
   * parameter.
   */
  if (!groupBy.empty() || ifind(from, "group by") != std::string::npos
      || !orderBy.empty() || ifind(from, "order by") != std::string::npos
      || limit != -1 || offset != -1)
    return createWrappedQueryCountSql(query, requireSubqueryAlias);
  else {
    std::string result = "select count(1) " + from;

    if (!where.empty())
      result += " where " + where;

    return addLimitQuery(result, limit, offset, limitQueryMethod);
  }
}

void substituteFields(const SelectFieldList& list,
		      const std::vector<FieldInfo>& fs,
		      std::string& sql,
		      int offset)
{
  for (unsigned i = 0, j = 0; j < list.size(); ++j) {
    if (fs[i].isFirstDboField()) {
      std::string dboFields;

      for (;;) {
	if (!dboFields.empty())
	  dboFields += ", ";

	dboFields += fs[i].sql();

	++i;
	if (i >= fs.size()
	    || fs[i].qualifier().empty()
	    || fs[i].isFirstDboField())
	  break;
      }

      int start = list[j].begin + offset;
      int count = list[j].end - list[j].begin;

      sql.replace(start, count, dboFields);

      offset += (dboFields.length() - (list[j].end - list[j].begin));
    } else
      ++i;
  }
}

    }
  }
}
				 
