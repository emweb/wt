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
				    const std::string& aJoinName,
				    const std::string& aJoinSelfId,
				    const std::string& aJoinOtherId)
  : tableName(aTableName),
    joinName(aJoinName),
    joinSelfId(aJoinSelfId),
    joinOtherId(aJoinOtherId),
    type(aType)
{ }

PrepareStatements::PrepareStatements(Session& session, const char *tableName,
				     std::vector<FieldInfo>& fields)
  : session_(session),
    tableName_(tableName),
    fields_(fields)
{ }

std::vector<std::string> PrepareStatements::getSelfSql() const
{
  std::vector<std::string> result;

  std::stringstream sql;

  std::string table = Impl::quoteSchemaDot(tableName_);

  sql << "insert into \"" << table << "\" (\"version";
  for (unsigned i = 0; i < fields_.size(); ++i)
    sql << "\", \"" << fields_[i].name();
  sql << "\") values (?";
  for (unsigned i = 0; i < fields_.size(); ++i)
    sql << ", ?";

  SqlConnection *conn = session_.useConnection();
  sql << ")" << conn->autoincrementInsertSuffix();
  session_.returnConnection(conn);

  result.push_back(sql.str()); // 0

  sql.str("");

  sql << "update \"" << table << "\" set \"version\" = ?";
  for (unsigned i = 0; i < fields_.size(); ++i)
    sql << ", \"" << fields_[i].name() << "\" = ?";
  sql << " where \"id\" = ? and \"version\" = ?";
  result.push_back(sql.str()); // 1

  sql.str("");

  sql << "delete from \"" << table
      << "\" where \"id\" = ?";
  result.push_back(sql.str()); // 2

  sql.str("");

  sql << "delete from \"" << table
      << "\" where \"id\" = ? and \"version\" = ?";
  result.push_back(sql.str()); // 3

  sql.str("");

  sql << "select \"version";
  for (unsigned i = 0; i < fields_.size(); ++i)
    sql << "\", \"" << fields_[i].name();
  sql << "\" from \"" << table << "\" where \"id\" = ?";
  result.push_back(sql.str()); // 4

  return result;
}

void PrepareStatements::addCollectionsSql(std::vector<std::string>& statements)
  const
{
  for (unsigned i = 0; i < sets_.size(); ++i) {
    const SetInfo& info = sets_[i];

    std::string ssql = *session_.getStatementSql(info.tableName,
						 Session::SqlSelectById);
    std::stringstream sql;

    switch (info.type) {
    case ManyToOne:
      // 'id" = ?' -> 'joinField_id" = ?'
      ssql.erase(ssql.length() - 7);
      ssql += info.joinName + "_id\" = ?";

      // 'select version' -> 'select id, version'
      ssql.insert(7, "\"id\", ");

      statements.push_back(ssql);
      break;
    case ManyToMany:
      // (1) select for collection

      std::string join_self_id = info.joinSelfId.empty()
	? std::string(tableName_) + "_id" : info.joinSelfId;
      std::string join_other_id = info.joinOtherId.empty()
	? std::string(info.tableName) + "_id" : info.joinOtherId;

      std::string joinName = Impl::quoteSchemaDot(info.joinName);
      std::string tableName = Impl::quoteSchemaDot(info.tableName);

      // ' where "id" = ?'
      //   -> ' join "joinName" on "joinName"."tableName_id" = "this"."id"'
      ssql.erase(ssql.length() - 15);

      // 'select version' -> 'select id, version'
      ssql.insert(7, "\"id\", ");

      sql << ssql << " join \"" << joinName
	  << "\" on \"" << joinName << "\".\"" << join_other_id
	  << "\" = \"" << tableName << "\".\"id\" "
	  << "where \"" << joinName << "\".\""
	  << join_self_id << "\" = ?";

      statements.push_back(sql.str());

      // (2) insert into collection

      sql.str("");

      sql << "insert into \"" << joinName
	  << "\" (\"" << join_self_id << "\", \"" << join_other_id
	  << "\") values (?, ?)";

      statements.push_back(sql.str());

      // (3) delete from collections

      sql.str("");

      sql << "delete from \"" << joinName
	  << "\" where \"" << join_self_id << "\" = ? and \""
	  << join_other_id << "\" = ?";

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

  sql_ << "create table \"" << Impl::quoteSchemaDot(tableName) << "\" (\n"
       << "  id " << session.connection()->autoincrementType()
       << " primary key " << session.connection()->autoincrementSql() << ",\n"
       << "  version integer not null";
}

void CreateSchema::exec()
{
  sql_ << "\n)";
  session_.connection()->executeSql(sql_.str());
}

void CreateSchema::createJoinTable(const std::string& joinName,
				   const char *table1, const char *table2,
				   const std::string& joinId1,
				   const std::string& joinId2)
{
  if (tablesCreated_.count(joinName) > 0)
    return;

  sql_.str("");

  tablesCreated_.insert(joinName);

  std::string table1_id = joinId1.empty()
    ? std::string(table1) + "_id" : joinId1;
  std::string table2_id = joinId2.empty()
    ? std::string(table2) + "_id" : joinId2;

  std::string jn = Impl::quoteSchemaDot(joinName);

  sql_ << "create table \"" << jn << "\" (\n"
       << "  \"" << table1_id << "\" integer references \""
       << Impl::quoteSchemaDot(table1) << "\"(\"id\"),\n"
       << "  \"" << table2_id << "\" integer references \""
       << Impl::quoteSchemaDot(table2) << "\"(\"id\"),\n"
       << "  primary key(\"" << table1_id << "\", \"" << table2_id << "\")\n)";

  session_.connection()->executeSql(sql_.str());

  sql_.str("");

  sql_ << "create index \"" << joinName << "_" << table1
       << "\" on \"" << jn << "\" (\"" << table1_id << "\")";

  session_.connection()->executeSql(sql_.str());

  sql_.str("");

  sql_ << "create index \"" << joinName << "_" << table2
       << "\" on \"" << jn << "\" (\"" << table2_id << "\")";

  session_.connection()->executeSql(sql_.str());
}

bool CreateSchema::isReading() const { return false; }
bool CreateSchema::isWriting() const { return false; }
bool CreateSchema::isSchema() const { return true; }

DropSchema::DropSchema(Session& session, const char *tableName,
		       std::set<std::string>& tablesDropped)
  : session_(session),
    tableName_(tableName),
    tablesDropped_(tablesDropped)
{
  tablesDropped_.insert(tableName);
}

void DropSchema::drop(const std::string& table)
{
  tablesDropped_.insert(table);
  session_.connection()
    ->executeSql("drop table \"" + Impl::quoteSchemaDot(table) + "\"");
}

bool DropSchema::isReading() const { return false; }
bool DropSchema::isWriting() const { return false; }
bool DropSchema::isSchema() const { return true; }

SaveDbAction::SaveDbAction(MetaDboBase& dbo)
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

LoadDbAction::LoadDbAction(MetaDboBase& dbo, SqlStatement *statement,
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

  statement_->done();
}

bool LoadDbAction::isReading() const { return true; }
bool LoadDbAction::isWriting() const { return false; }
bool LoadDbAction::isSchema() const { return false; }

TransactionDoneAction::TransactionDoneAction(Session *session,
					     MetaDboBase& dbo, bool success)
  : session_(session),
    success_(success),
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

GetManyToManyJoinIdAction
::GetManyToManyJoinIdAction(const std::string& joinName,
			    const std::string& notId)
  : joinName_(joinName),
    result_(notId),
    found_(false)
{ }

bool GetManyToManyJoinIdAction::isReading() const { return false; }
bool GetManyToManyJoinIdAction::isWriting() const { return false; }
bool GetManyToManyJoinIdAction::isSchema() const { return true; }

ToAnysAction::ToAnysAction(std::vector<boost::any>& result)
  : result_(result)
{ }

bool ToAnysAction::isReading() const { return true; }
bool ToAnysAction::isWriting() const { return false; }
bool ToAnysAction::isSchema() const { return false; }

  }
}
