/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/SqlConnection.h"
#include "Wt/Dbo/SqlStatement.h"
#include "Wt/Dbo/Exception.h"
#include "SqlConnection.h"

#include <cassert>

namespace Wt {
  namespace Dbo {

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

SqlStatement *SqlConnection::getStatement(const std::string& id) const
{
  StatementMap::const_iterator i = statementCache_.find(id);
  if (i != statementCache_.end()) {
    SqlStatement *result = i->second.get();
    /*
     * Later, if already in use, manage reentrant use by cloning the statement
     * and adding it to a linked list in the statementCache_
     */
    if (!result->use())
      throw Exception("A collection for '" + id + "' is already in use."
		      " Reentrant statement use is not yet implemented."); 

    return result;
  } else
    return 0;
}

void SqlConnection::saveStatement(const std::string& id,
				  std::unique_ptr<SqlStatement> statement)
{
  statementCache_[id] = std::move(statement);
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
