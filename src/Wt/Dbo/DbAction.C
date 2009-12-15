/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>

#include "Wt/Dbo/DbAction"
#include "Wt/Dbo/Exception"
#include "Wt/Dbo/Session"
#include "Wt/Dbo/SqlConnection"
#include "Wt/Dbo/SqlStatement"

namespace Wt {
  namespace Dbo {

PrepareStatements::SetInfo::SetInfo(const char *aTableName,
				    RelationType aType,
				    const std::string& aJoinName)
  : tableName(aTableName),
    joinName(aJoinName),
    type(aType)
{ }

PrepareStatements::PrepareStatements(Session& session, const char *tableName)
  : session_(session),
    tableName_(tableName)
{ }

int PrepareStatements::columnCount() const
{
  return fields_.size() + 2; // + id, version
}

std::vector<std::string> PrepareStatements::getSelfSql() const
{
  std::vector<std::string> result;

  std::stringstream sql;

  sql << "insert into " << tableName_ << "(version";
  for (unsigned i = 0; i < fields_.size(); ++i)
    sql << ", " << fields_[i];
  sql << ") values (?";
  for (unsigned i = 0; i < fields_.size(); ++i)
    sql << ", ?";
  sql << ")";
  result.push_back(sql.str()); // 0

  sql.str("");

  sql << "update " << tableName_ << " set version = ?";
  for (unsigned i = 0; i < fields_.size(); ++i)
    sql << ", " << fields_[i] << " = ?";
  sql << " where id = ? and version = ?";
  result.push_back(sql.str()); // 1

  sql.str("");

  sql << "delete from " << tableName_ << " where id = ?";
  result.push_back(sql.str()); // 2

  sql.str("");

  sql << "delete from " << tableName_ << " where id = ? and version = ?";
  result.push_back(sql.str()); // 3

  sql.str("");

  sql << "select version";
  for (unsigned i = 0; i < fields_.size(); ++i)
    sql << ", " << fields_[i];
  sql << " from " << tableName_ << " where id = ?";
  result.push_back(sql.str()); // 4

  return result;
}

void PrepareStatements::addCollectionsSql(std::vector<std::string>& statements)
  const
{
  for (unsigned i = 0; i < sets_.size(); ++i) {
    const SetInfo& info = sets_[i];

    std::string ssql = session_.getStatementSql(info.tableName,
						Session::SqlSelectById);
    std::stringstream sql;

    switch (info.type) {
    case ManyToOne:
      // 'id = ?' -> 'joinField_id = ?'
      ssql.erase(ssql.length() - 6);
      ssql += info.joinName + "_id = ?";

      // 'select version' -> 'select id, version'
      ssql.insert(7, "id, ");

      statements.push_back(ssql);
      break;
    case ManyToMany:
      // (1) select for collection

      // ' where id = ?' -> ' join joinName on joinName.tableName_id = other.id'
      ssql.erase(ssql.length() - 13);

      // 'select version' -> 'select id, version'
      ssql.insert(7, "id, ");

      sql << ssql << " join " << info.joinName
	  << " on " << info.joinName << "." << info.tableName << "_id = "
	  << info.tableName << ".id "
	  << "where " << info.joinName << "." << tableName_ << "_id = ?";

      statements.push_back(sql.str());

      // (2) insert into collection

      sql.str("");

      sql << "insert into " << info.joinName
	  << "(" << tableName_ << "_id, " << info.tableName
	  << "_id) values (?, ?)";

      statements.push_back(sql.str());

      // (3) delete from collections

      sql.str("");

      sql << "delete from " << info.joinName
	  << " where " << tableName_ << "_id = ? and "
	  << info.tableName << "_id = ?";

      statements.push_back(sql.str());
    }
  }
}

bool PrepareStatements::isReading() const { return false; }
bool PrepareStatements::isWriting() const { return false; }
bool PrepareStatements::isSchema() const { return true; }

CreateSchema::CreateSchema(Session& session, const char *tableName,
			   std::set<std::string>& tablesCreated)
  : session_(session),
    tableName_(tableName),
    tablesCreated_(tablesCreated),
    needSetsPass_(false)
{
  tablesCreated_.insert(tableName);

  sql_ << "create table " << tableName << "(\n"
       << "  id integer primary key autoincrement,\n"
       << "  version integer not null";
}

void CreateSchema::exec()
{
  sql_ << "\n)";

  session_.connection_->executeSql(sql_.str());
}

void CreateSchema::createJoinTable(const std::string& joinName,
				   const char *table1,
				   const char *table2)
{
  sql_.str("");

  tablesCreated_.insert(joinName);

  sql_ << "create table " << joinName << "(\n"
       << "  " << table1 << "_id integer references " << table1 << "(id),\n"
       << "  " << table2 << "_id integer references " << table2 << "(id),\n"
       << "  primary key(" << table1 << "_id, " << table2 << "_id)\n)";

  session_.connection_->executeSql(sql_.str());
}

bool CreateSchema::isReading() const { return false; }
bool CreateSchema::isWriting() const { return false; }
bool CreateSchema::isSchema() const { return true; }

SaveDbAction::SaveDbAction(DboBase& dbo)
  : dummy_(0),
    loadSets_(dbo, 0, dummy_),
    dbo_(dbo),
    statement_(0)
{ }

void SaveDbAction::startDependencyPass()
{
  pass_ = Dependencies;
}

void SaveDbAction::startSelfPass()
{
  pass_ = Self;
  needSetsPass_ = false;

  statement_->reset();
  column_ = 0;
  statement_->bind(column_++, dbo_.version() + 1);
}

void SaveDbAction::exec()
{
  if (!isInsert_) {
    statement_->bind(column_++, dbo_.id());

    // when saved in the transaction, we will be at version() + 1
    statement_->bind(column_++, dbo_.version()
		     + (dbo_.savedInTransaction() ? 1 : 0));
  }

  statement_->execute();

  if (isInsert_)
    dbo_.setId(statement_->insertedId());
  else {
    int modifiedCount = statement_->affectedRowCount();
    if (modifiedCount != 1)
      throw StaleObjectException(dbo_.id(), dbo_.version());
  }
}

void SaveDbAction::startSetsPass()
{
  pass_ = Sets;
}

bool SaveDbAction::isReading() const { return false; }
bool SaveDbAction::isWriting() const { return true; }
bool SaveDbAction::isSchema() const { return false; }

LoadDbAction::LoadDbAction(DboBase& dbo, SqlStatement *statement,
			   int& column)
  : dbo_(dbo),
    statement_(statement),
    column_(column)
{
  setStatementIdx_ = 0;
}

void LoadDbAction::exec()
{
  statement_->reset();
  statement_->bind(0, dbo_.id());
  statement_->execute();

  if (!statement_->nextRow())
    throw ObjectNotFoundException(dbo_.id());
}

void LoadDbAction::setTableName(const char *tableName)
{
  tableName_ = tableName;
}

void LoadDbAction::start()
{
  int version;
  statement_->getResult(column_++, &version);
  dbo_.setVersion(version);
}

void LoadDbAction::done()
{
  if (statement_->nextRow())
    throw Exception("Dbo load: multiple rows for id "
		    + boost::lexical_cast<std::string>(dbo_.id()) + " ??");
}

bool LoadDbAction::isReading() const { return true; }
bool LoadDbAction::isWriting() const { return false; }
bool LoadDbAction::isSchema() const { return false; }

TransactionDoneAction::TransactionDoneAction(DboBase& dbo, bool success)
  : success_(success),
    dummy_(0),
    undoLoadSets_(dbo, 0, dummy_)
{ }

bool TransactionDoneAction::isReading() const { return true; }
bool TransactionDoneAction::isWriting() const { return false; }
bool TransactionDoneAction::isSchema() const { return false; }

SessionAddAction::SessionAddAction(Session *session)
  : session_(session)
{ }

bool SessionAddAction::isReading() const { return true; }
bool SessionAddAction::isWriting() const { return false; }
bool SessionAddAction::isSchema() const { return false; }

  }
}
