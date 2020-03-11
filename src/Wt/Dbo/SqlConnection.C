/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/Exception.h"
#include "Wt/Dbo/Logger.h"
#include "Wt/Dbo/SqlConnection.h"
#include "Wt/Dbo/SqlStatement.h"
#include "Wt/Dbo/StringStream.h"

#include <cassert>
#include <iostream>

namespace Wt {
  namespace Dbo {

LOGGER("Dbo.SqlConnection");

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
  statementCache_.clear();
}

void SqlConnection::executeSql(const std::string& sql)
{
  std::unique_ptr<SqlStatement> s = prepareStatement(sql);
  s->execute();
}

void SqlConnection::executeSqlStateful(const std::string& sql)
{
  statefulSql_.push_back(sql);
  executeSql(sql);
}

SqlStatement *SqlConnection::getStatement(const std::string& id)
{
  StatementMap::const_iterator start;
  StatementMap::const_iterator end;
  std::tie(start, end) = statementCache_.equal_range(id);
  SqlStatement *result = nullptr;
  for (auto i = start; i != end; ++i) {
    result = i->second.get();
    if (result->use())
      return result;
  }
  if (result) {
    auto count = statementCache_.count(id);
    if (count >= WARN_NUM_STATEMENTS_THRESHOLD) {
      LOG_WARN("Warning: number of instances (" << (count + 1) << ") of prepared statement '"
               << id << "' for this "
                  "connection exceeds threshold (" << WARN_NUM_STATEMENTS_THRESHOLD << ")"
                  ". This could indicate a programming error.");
    }
    auto stmt = prepareStatement(result->sql());
    result = stmt.get();
    saveStatement(id, std::move(stmt));
  }
  return nullptr;
}

void SqlConnection::saveStatement(const std::string& id,
				  std::unique_ptr<SqlStatement> statement)
{
  statementCache_.emplace(id, std::move(statement));
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
  return LimitQuery::Limit;
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
    return "varchar(" + std::to_string(size) + ")";
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
    result.push_back(i->second.get());

  return result;
}
  
  }
}
