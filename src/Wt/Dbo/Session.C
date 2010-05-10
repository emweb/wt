// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/Call"
#include "Wt/Dbo/Exception"
#include "Wt/Dbo/Session"
#include "Wt/Dbo/SqlConnection"
#include "Wt/Dbo/SqlConnectionPool"
#include "Wt/Dbo/SqlStatement"

#include <cassert>
#include <iostream>
#include <boost/lexical_cast.hpp>

namespace Wt {
  namespace Dbo {
    namespace Impl {

std::string& replace(std::string& s, char c, const std::string& r)
{
  std::string::size_type p = 0;

  while ((p = s.find(c, p)) != std::string::npos) {
    s.replace(p, 1, r);
    p += r.length();
  }

  return s;
}

std::string quoteSchemaDot(const std::string& table) {
  std::string result = table;
  replace(result, '.', "\".\"");
  return result;
}

    } // end namespace Impl

Session::Session()
  : connection_(0),
    connectionPool_(0),
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

void Session::setConnectionPool(SqlConnectionPool& pool)
{
  connectionPool_ = &pool;
}

SqlConnection *Session::connection()
{
  if (!transaction_)
    throw std::logic_error("Operation requires an active transaction");

  return transaction_->connection_;
}

SqlConnection *Session::useConnection()
{
  if (connectionPool_)
    return connectionPool_->getConnection();
  else
    return connection_;
}

void Session::returnConnection(SqlConnection *connection)
{
  if (connectionPool_)
    connectionPool_->returnConnection(connection);
}

void Session::prune(MetaDboBase *obj)
{
  if (dirtyObjects_.erase(obj) > 0)
    obj->decRef();
}

Call Session::execute(const std::string& sql)
{
  if (!transaction_)
    throw std::logic_error("Dbo execute(): no active transaction");

  return Call(*this, sql);
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

void Session::dropTables()
{
  Transaction t(*this);

  flush();

  std::set<std::string> tablesDropped;
  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    i->second->dropTable(*this, tablesDropped);

  t.commit();
}

void Session::needsFlush(MetaDboBase *obj)
{
  if (dirtyObjects_.insert(obj).second)
    obj->incRef();
}

void Session::flush()
{
  while (!dirtyObjects_.empty()) {
    MetaDboBaseSet::iterator i = dirtyObjects_.begin();
    MetaDboBase *dbo = *i;
    dirtyObjects_.erase(i);
    dbo->flush();
    dbo->decRef();
  }
}

std::string Session::statementId(const char *tableName, int statementIdx)
{  
  return std::string(tableName) + ":"
    + boost::lexical_cast<std::string>(statementIdx);
}

SqlStatement *Session::getStatement(const std::string& id)
{
  return connection()->getStatement(id);
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
    result = prepareStatement(id, *getStatementSql(tableName, statementIdx));

  return result;
}

const std::string *Session::getStatementSql(const char *tableName,
					    int statementIdx)
{
  for (ClassRegistry::const_iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i) {
    if (i->second->tableName == tableName) {
      if (i->second->statements.empty())
	i->second->prepareStatements(*this);
      return &i->second->statements[statementIdx];
    }
  }

  assert(false);

  return 0;
}

SqlStatement *Session::prepareStatement(const std::string& id,
					const std::string& sql)
{
  SqlConnection *conn = connection();
  SqlStatement *result = conn->prepareStatement(sql);
  conn->saveStatement(id, result);
  result->use();

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

void Session::getFields(const char *tableName,
			std::vector<FieldInfo>& result)
{
  result.push_back(FieldInfo("id", &typeid(long long), FieldInfo::Id));
  result.push_back(FieldInfo("version", &typeid(int), FieldInfo::Version));

  TableRegistry::const_iterator i = tableRegistry_.find(tableName);
  if (i != tableRegistry_.end()) {
    ClassMappingInfo *info = i->second;
    if (info->statements.empty())
      info->prepareStatements(*this);
    result.insert(result.end(), info->fields.begin(), info->fields.end());
  } else
    throw std::logic_error(std::string("Table ") + tableName
			   + " was not mapped.");
}

  }
}
