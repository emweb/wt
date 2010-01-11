// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/Exception"
#include "Wt/Dbo/Session"
#include "Wt/Dbo/SqlConnection"
#include "Wt/Dbo/SqlStatement"

#include <cassert>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace Wt {
  namespace Dbo {

const int Session::SqlInsert = 0;
const int Session::SqlUpdate = 1;
const int Session::SqlDelete = 2;
const int Session::SqlDeleteVersioned = 3;
const int Session::SqlSelectById = 4;
const int Session::FirstSqlSelectSet = 5;

Session::Session()
  : connection_(0),
    transaction_(0)
{ }

Session::~Session()
{
  if (!dirtyObjects_.empty())
    std::cerr << "Warning: Wt::Dbo::Session exiting with "
	      << dirtyObjects_.size() << " dirty objects" << std::endl;

  for (MetaDboBaseSet::iterator i = dirtyObjects_.begin(); 
       i != dirtyObjects_.end(); ++i)
    (*i)->decRef();

  dirtyObjects_.clear();

  for (Registry::iterator i = registry_.begin(); i != registry_.end(); ++i) {
    i->second->setState(MetaDboBase::Orphaned);
  }

  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    delete i->second;
}

void Session::setConnection(SqlConnection& connection)
{
  connection_ = &connection;
}

SqlConnection *Session::useConnection()
{
  return connection_;
}

void Session::returnConnection(SqlConnection *connection)
{
}

void Session::prune(MetaDboBase *obj)
{
  if (dirtyObjects_.erase(obj) > 0)
    obj->decRef();
}

void Session::createTables()
{
  Transaction t(*this);

  std::set<std::string> tablesCreated;

  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    i->second->createTable(*this, tablesCreated);

  t.commit();
}

void Session::needsFlush(MetaDboBase *obj)
{
  if (dirtyObjects_.insert(obj).second)
    obj->incRef();
}

void Session::flush()
{
  for (MetaDboBaseSet::iterator i = dirtyObjects_.begin();
       i != dirtyObjects_.end(); ++i) {
    (*i)->flush();
    (*i)->decRef();
  }

  dirtyObjects_.clear();
}

std::string Session::statementId(const char *tableName, int statementIdx)
{  
  return std::string(tableName) + ":"
    + boost::lexical_cast<std::string>(statementIdx);
}

SqlStatement *Session::getStatement(const std::string& id)
{
  return transaction_->connection_->getStatement(id);
}

SqlStatement *Session::getOrPrepareStatement(const std::string& sql)
{
  SqlStatement *s = getStatement(sql);
  if (!s)
    s = prepareStatement(sql, sql);

  return s;
}

SqlStatement *Session::getStatement(const char *tableName, int statementIdx)
{
  std::string id = statementId(tableName, statementIdx);
  SqlStatement *result = getStatement(id);

  if (!result)
    result = prepareStatement(id, getStatementSql(tableName, statementIdx));

  return result;
}

std::string Session::getStatementSql(const char *tableName, int statementIdx)
{
  for (ClassRegistry::const_iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    if (i->second->tableName == tableName) {
      if (i->second->statements.empty())
	i->second->prepareStatements(*this);
      return i->second->statements[statementIdx];
    }

  assert(false);

  return std::string();
}

SqlStatement *Session::prepareStatement(const std::string& id,
					const std::string& sql)
{
  SqlConnection *conn = transaction_->connection_;
  SqlStatement *result = conn->prepareStatement(sql);
  conn->saveStatement(id, result);
  return result;
}

void Session::doDelete(SqlStatement *statement, long long id,
		       bool withVersion, int version)
{
  statement->reset();
  statement->bind(0, id);

  if (withVersion)
    statement->bind(1, version);

  statement->execute();

  if (withVersion) {
    statement->nextRow();
    int modifiedCount = statement->affectedRowCount();
    if (modifiedCount != 1)
      throw StaleObjectException(id, version);
  }
}

Session::ClassMappingInfo::~ClassMappingInfo()
{ }

static std::string& replace(std::string& s, const std::string& k,
			    const std::string& r)
{
  std::string::size_type p = 0;

  while ((p = s.find(k, p)) != std::string::npos) {
    s.replace(p, k.length(), r);
    p += r.length();
  }

  return s;
}

void Session::parseSql(const std::string& sql,
		       std::vector<std::string>& aliases,
		       std::string& rest)
{
  std::size_t selectPos = sql.find("select ");
  if (selectPos != 0)
    throw std::logic_error("Session::query(): query should start with 'select '"
			   " (sql='" + sql + "'");

  std::size_t fromPos = sql.find(" from ");
  if (fromPos == std::string::npos)
    throw std::logic_error("Session::query(): expected ' from ' in query"
			   " (sql='" + sql + "'");

  std::string aliasStr = sql.substr(7, fromPos - 7);
  boost::split(aliases, aliasStr, boost::is_any_of(","));
  rest = sql.substr(fromPos);
}

std::string Session::getColumns(const char *tableName,
				std::vector<std::string> *aliases)
{
  std::string columns = getStatementSql(tableName, SqlSelectById);

  std::size_t f = columns.find(" from ");
  columns.erase(f);

  // 'select version' -> 'id, version'
  columns.insert(7, "\"id\", ");
  columns.erase(0, 7);

  if (aliases) {
    if (aliases->empty())
      throw std::logic_error("Session::query(): not enough aliases for result");

    std::string alias = aliases->front();
    aliases->erase(aliases->begin());

    replace(columns, ", ", ", " + alias + ".");
    columns = alias + "." + columns;
  }

  return columns;
}
  }
}
