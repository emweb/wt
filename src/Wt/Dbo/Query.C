/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Query.h"
#include "Query_impl.h"
#include "SqlTraits.h"
#include "StdSqlTraits.h"
#include "ptr.h"

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
    // We don't need to add aliases in this case, normally
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

std::string addLimitQuery(const std::string& sql, const std::string &orderBy, int limit, int offset,
			  LimitQuery limitQueryMethod)
{
  std::string result = sql;

  switch (limitQueryMethod) {
  case LimitQuery::Limit:
    if (limit != -1)
      result += " limit ?";

    if (offset != -1)
      result += " offset ?";

    break;

  case LimitQuery::RowsFromTo:
    if (limit != -1 || offset != -1) {
      result += " rows ? to ?";
    }

    break;

  case LimitQuery::Rownum:
    if (limit != -1 && offset == -1)
      result = " select * from ( " + result + " ) where rownum <= ?";
    else if (limit != -1 && offset != -1)
      result = " select * from ( select row_.*, rownum rownum2 from ( " +
	result + " ) row_ where rownum <= ?) where rownum2 > ?";

  case LimitQuery::OffsetFetch:
    if (offset != -1 || limit != -1) {
      if (orderBy.empty())
        result += " order by (select null)";
      if (offset != -1)
        result += " offset (?) rows";
      else
        result += " offset 0 rows";
    }

    if (limit != -1)
      result += " fetch first (?) rows only";

    // SQL Server is a special snowflake that requires an offset
    // parameter when order by is used in subqueries.
    if (!orderBy.empty() && offset == -1 && limit == -1)
      result += " offset 0 rows";

    break;
  
  case LimitQuery::NotSupported:
    break;
  }

  return result;
}
std::string completeQuerySelectSql(const std::string& sql,
				   const std::string& join,
				   const std::string& where,
				   const std::string& groupBy,
				   const std::string& having,
				   const std::string& orderBy,
				   int limit, int offset,
				   const std::vector<FieldInfo>& fields,
				   LimitQuery limitQueryMethod)
{
  std::string result = sql + join;

  if (!where.empty())
    result += " where " + where;

  if (!groupBy.empty())
    addGroupBy(result, groupBy, fields);

  if (!having.empty())
    result += " having " + having;

  if (!orderBy.empty())
    result += " order by " + orderBy;

  return addLimitQuery(result, orderBy, limit, offset, limitQueryMethod);
}

std::string createQuerySelectSql(const std::string& from,
				 const std::string& join,
				 const std::string& where,
				 const std::string& groupBy,
				 const std::string& having,
				 const std::string& orderBy,
				 int limit, int offset,
				 const std::vector<FieldInfo>& fields,
				 LimitQuery limitQueryMethod)
{
  std::string result = "select " + selectColumns(fields) + ' ' + from + join;

  if (!where.empty())
    result += " where " + where;

  if (!groupBy.empty())
    addGroupBy(result, groupBy, fields);

  if (!having.empty())
    result += " having " + having;

  if (!orderBy.empty())
    result += " order by " + orderBy;

  return addLimitQuery(result, orderBy, limit, offset, limitQueryMethod);
}

std::string createQueryCountSql(const std::string& query,
				bool requireSubqueryAlias)
{
  if (requireSubqueryAlias)
    return "select count(1) from (" + query + ") dbocount";
  else
    return "select count(1) from (" + query + ")";
}

void substituteFields(const SelectFieldList& list,
		      const std::vector<FieldInfo>& fs,
		      std::string& sql,
                      int& offset)
{
  for (unsigned i = 0, j = 0; j < list.size(); ++j) {
    if (fs[i].isFirstDboField()) {
      std::string dboFields;

      for (;;) {
	if (!dboFields.empty())
	  dboFields += ", ";

	dboFields += fs[i].sql();
        dboFields += " as col" + std::to_string(i);

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
    } else {
      if (!fs[i].isAliasedName()) {
        int start = list[j].end + offset;
        std::string col = " as col" + std::to_string(i);
        sql.insert(start, col);
        offset += col.size();
      }
      ++i;
    }
  }
}

    }

AbstractQuery& AbstractQuery::join(const std::string& other)
{
  join_ += " join " + other;

  return *this;
}

AbstractQuery& AbstractQuery::leftJoin(const std::string& other)
{
  join_ += " left join " + other;

  return *this;
}

AbstractQuery& AbstractQuery::rightJoin(const std::string& other)
{
  join_ += " right join " + other;

  return *this;
}

AbstractQuery& AbstractQuery::where(const std::string& where)
{
  if (!where.empty()) {
    if (!where_.empty())
      where_ += " and ";

    where_ += "(" + where + ")";
  }

  return *this;
}

AbstractQuery& AbstractQuery::orWhere(const std::string& where)
{
  if (!where.empty()) {
    if (!where_.empty())
      where_ = "(" + where_ + ") or ";

    where_ += "(" + where + ")";
  }

  return *this;
}

AbstractQuery& AbstractQuery::orderBy(const std::string& orderBy)
{
  orderBy_ = orderBy;

  return *this;
}

AbstractQuery& AbstractQuery::groupBy(const std::string& groupBy)
{
  groupBy_ = groupBy;

  return *this;
}

AbstractQuery& AbstractQuery::having(const std::string& having)
{
  if (!having.empty()) {
    if (!having_.empty())
      having_ += " and ";

    having_ += "(" + having + ")";
  }

  return *this;
}

AbstractQuery& AbstractQuery::offset(int offset)
{
  offset_ = offset;

  return *this;
}

int AbstractQuery::offset() const
{
  return offset_;
}

AbstractQuery& AbstractQuery::limit(int limit)
{
  limit_ = limit;

  return *this;
}

int AbstractQuery::limit() const
{
  return limit_;
}

void AbstractQuery::reset()
{
  for (unsigned i = 0; i < parameters_.size(); ++i)
    delete parameters_[i];

  parameters_.clear();
}

AbstractQuery::AbstractQuery()
  : limit_(-1),
    offset_(-1)
{ }

AbstractQuery::~AbstractQuery()
{ }

AbstractQuery::AbstractQuery(const AbstractQuery& other)
  : join_(other.join_),
    where_(other.where_),
    groupBy_(other.groupBy_),
    having_(other.having_),
    orderBy_(other.orderBy_),
    limit_(other.limit_),
    offset_(other.offset_)
{
  for (unsigned i = 0; i < other.parameters_.size(); ++i)
    parameters_.push_back(other.parameters_[i]->clone());
}

AbstractQuery& AbstractQuery::operator=(const AbstractQuery& other)
{
  join_ = other.join_;
  where_ = other.where_;
  groupBy_ = other.groupBy_;
  having_ = other.having_;
  orderBy_ = other.orderBy_;
  limit_ = other.limit_;
  offset_ = other.offset_;

  reset();

  for (unsigned i = 0; i < other.parameters_.size(); ++i)
    parameters_.push_back(other.parameters_[i]->clone());

  return *this;
}

void AbstractQuery::bindParameters(Session *session, SqlStatement *statement) const
{
  SaveBaseAction binder(session, statement, 0);

  for (unsigned i = 0; i < parameters_.size(); ++i)
    parameters_[i]->bind(binder);

  switch (session->limitQueryMethod_) {
  case LimitQuery::Limit:
    if (limit_ != -1) {
      int v = limit_;
      field(binder, v, "limit");
    }

    if (offset_ != -1) {
      int v = offset_;
      field(binder, v, "offset");
    }

    break;

  case LimitQuery::RowsFromTo:
    if (limit_ != -1 || offset_ != -1) {
      int from = offset_ == -1 ? 1 : offset_ + 1;
      field(binder, from, "from");

      int to = (limit_ == -1) ? (1 << 30) : (from + limit_ - 1);
      field(binder, to, "to");
    }

    break;

  case LimitQuery::Rownum:
    if (limit_ != -1){
      int v = limit_;
      field(binder, v, "rownum");
    }

    if (offset_ != -1){
      int v = offset_;
      field(binder, v, "rownum2");
    }

    break;

  case LimitQuery::OffsetFetch:
    if (offset_ != -1) {
      int v = offset_;
      field(binder, v, "offset");
    }

    if (limit_ != -1) {
      int v = limit_;
      field(binder, v, "limit");
    }

    break;

  case LimitQuery::NotSupported:
    break;
  }
}

  }
}

