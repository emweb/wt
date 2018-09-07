/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/SqlConnection"
#include "Wt/Dbo/SqlStatement"
#include "Wt/Dbo/Exception"
#include "SqlConnection"
#include <boost/lexical_cast.hpp>

#include <cassert>
#include <iostream>

namespace Wt {
  namespace Dbo {

namespace {
  static const std::size_t WARN_NUM_STATEMENTS_THRESHOLD = 10;
}

SqlConnection::SqlConnection()
{ }

SqlConnection::SqlConnection(const SqlConnection& other)
  : properties_(other.properties_)
{ }

SqlConnection::~SqlConnection()
{
  assert(statementCache_.empty());
}

void SqlConnection::clearStatementCache()
{
  for (StatementMap::iterator i = statementCache_.begin();
       i != statementCache_.end(); ++i)
    delete i->second;

  statementCache_.clear();
}

void SqlConnection::executeSql(const std::string& sql)
{
  SqlStatement *s = prepareStatement(sql);
  s->execute();
  delete s;
}

void SqlConnection::executeSqlStateful(const std::string& sql)
{
  statefulSql_.push_back(sql);
  executeSql(sql);
}

SqlStatement *SqlConnection::getStatement(const std::string& id)
{
  std::pair<StatementMap::const_iterator, StatementMap::const_iterator> range =
      statementCache_.equal_range(id);
  const StatementMap::const_iterator &start = range.first;
  const StatementMap::const_iterator &end = range.second;
  SqlStatement *result = 0;
  for (StatementMap::const_iterator i = start; i != end; ++i) {
    result = i->second;
    if (result->use())
      return result;
  }
  if (result) {
    std::size_t count = statementCache_.count(id);
    if (count >= WARN_NUM_STATEMENTS_THRESHOLD) {
      std::cerr << "Warning: number of instances (" << (count + 1) << ") of prepared statement '"
                << id << "' for this "
                   "connection exceeds threshold (" << WARN_NUM_STATEMENTS_THRESHOLD << ")"
                   ". This could indicate a programming error.\n";
    }
    result = prepareStatement(result->sql());
    saveStatement(id, result);
  }
  return 0;
}

void SqlConnection::saveStatement(const std::string& id,
				  SqlStatement *statement)
{
  statementCache_.insert(std::make_pair(id, statement));
}

std::string SqlConnection::property(const std::string& name) const
{
  std::map<std::string, std::string>::const_iterator i = properties_.find(name);

  if (i != properties_.end())
    return i->second;
  else
    return std::string();
}

void SqlConnection::setProperty(const std::string& name,
				const std::string& value)
{
  properties_[name] = value;
}

bool SqlConnection::usesRowsFromTo() const
{
  return false;
}

LimitQuery SqlConnection::limitQueryMethod() const
{
  return Limit;
}

bool SqlConnection::supportAlterTable() const
{
  return false;
}

bool SqlConnection::supportDeferrableFKConstraint() const
{
  return false;
}

const char *SqlConnection::alterTableConstraintString() const
{
  return "constraint";
}

bool SqlConnection::showQueries() const
{
  return property("show-queries") == "true";
}

std::string SqlConnection::textType(int size) const
{
  if (size == -1)
    return "text";
  else{
    return "varchar(" + boost::lexical_cast<std::string>(size) + ")";
  }
}

std::string SqlConnection::longLongType() const
{
  return "bigint";
}

const char *SqlConnection::booleanType() const
{
  return "boolean";
}

bool SqlConnection::supportUpdateCascade() const
{
  return true;
}

bool SqlConnection::requireSubqueryAlias() const
{
  return false;
}

std::string SqlConnection::autoincrementInsertInfix(const std::string &) const
{
  return "";
}

void SqlConnection::prepareForDropTables()
{ }

std::vector<SqlStatement *> SqlConnection::getStatements() const
{
  std::vector<SqlStatement *> result;

  for (StatementMap::const_iterator i = statementCache_.begin();
       i != statementCache_.end(); ++i)
    result.push_back(i->second);

  return result;
}
  
  }
}
