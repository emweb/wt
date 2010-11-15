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
#include "Wt/Dbo/StdSqlTraits"

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

Session::JoinId::JoinId(const std::string& aJoinIdName,
			const std::string& aTableIdName,
			const std::string& aSqlType)
  : joinIdName(aJoinIdName),
    tableIdName(aTableIdName),
    sqlType(aSqlType)
{ }

Session::SetInfo::SetInfo(const char *aTableName,
			  RelationType aType,
			  const std::string& aJoinName,
			  const std::string& aJoinSelfId)
  : tableName(aTableName),
    joinName(aJoinName),
    joinSelfId(aJoinSelfId),
    type(aType)
{ }

Session::MappingInfo::MappingInfo()
  : initialized_(false)
{ }

Session::MappingInfo::~MappingInfo()
{ }

void Session::MappingInfo::init(Session& session)
{ 
  throw std::logic_error("Not to be done.");
}

void Session::MappingInfo::dropTable(Session& session,
				     std::set<std::string>& tablesDropped)
{
  throw std::logic_error("Not to be done.");
}

void Session::MappingInfo::rereadAll()
{ 
  throw std::logic_error("Not to be done.");
}

std::string Session::MappingInfo::primaryKeys() const
{
  if (surrogateIdFieldName)
    return std::string("\"") + surrogateIdFieldName + "\"";
  else {
    std::stringstream result;

    bool firstField = true;
    for (unsigned i = 0; i < fields.size(); ++i)
      if (fields[i].isIdField()) {
	if (!firstField)
	  result << ", ";
	result << "\"" << fields[i].name() << "\"";
	firstField = false;
      }

    return result.str();
  }
}

Session::Session()
  : schemaInitialized_(false),
    connection_(0),
    connectionPool_(0),
    transaction_(0)
{ }

Session::~Session()
{
  if (!dirtyObjects_.empty())
    std::cerr << "Warning: Wt::Dbo::Session exiting with "
	      << dirtyObjects_.size() << " dirty objects" << std::endl;

  while (!dirtyObjects_.empty()) {
    MetaDboBase *b = *dirtyObjects_.begin();
    b->decRef();
  }

  dirtyObjects_.clear();

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

SqlConnection *Session::connection(bool openTransaction)
{
  if (!transaction_)
    throw std::logic_error("Operation requires an active transaction");

  if (openTransaction)
    transaction_->open();

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

void Session::discardChanges(MetaDboBase *obj)
{
  if (dirtyObjects_.erase(obj) > 0)
    obj->decRef();
}

Call Session::execute(const std::string& sql)
{
  initSchema();

  if (!transaction_)
    throw std::logic_error("Dbo execute(): no active transaction");

  return Call(*this, sql);
}

void Session::initSchema() const
{
  if (schemaInitialized_)
    return;

  Session *self = const_cast<Session *>(this);
  self->schemaInitialized_ = true;

  Transaction t(*self);

  for (ClassRegistry::const_iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    i->second->init(*self);

  for (ClassRegistry::const_iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    self->resolveJoinIds(i->second);

  for (ClassRegistry::const_iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    self->prepareStatements(i->second);

  self->schemaInitialized_ = true;

  t.commit();
}

void Session::prepareStatements(MappingInfo *mapping)
{
  std::stringstream sql;

  std::string table = Impl::quoteSchemaDot(mapping->tableName);

  /*
   * SqlInsert
   */
  sql << "insert into \"" << table << "\" (";

  bool firstField = true;

  if (mapping->versionFieldName) {
    sql << "\"" << mapping->versionFieldName << "\"";
    firstField = false;
  }

  for (unsigned i = 0; i < mapping->fields.size(); ++i) {
    if (!firstField)
      sql << ", ";
    sql << "\"" << mapping->fields[i].name() << "\"";
    firstField = false;
  }

  sql << ") values (";

  firstField = true;
  if (mapping->versionFieldName) {
    sql << "?";
    firstField = false;
  }

  for (unsigned i = 0; i < mapping->fields.size(); ++i) {
    if (!firstField)
      sql << ", ";
    sql << "?";
    firstField = false;
  }

  sql << ")";

  if (mapping->surrogateIdFieldName) {
    SqlConnection *conn = useConnection();
    sql << conn->autoincrementInsertSuffix();
    returnConnection(conn);
  }

  mapping->statements.push_back(sql.str()); // SqlInsert

  /*
   * SqlUpdate
   */

  sql.str("");

  sql << "update \"" << table << "\" set ";

  firstField = true;

  if (mapping->versionFieldName) {
    sql << "\"" << mapping->versionFieldName << "\" = ?";
    firstField = false;
  }
  
  for (unsigned i = 0; i < mapping->fields.size(); ++i) {
    if (!firstField)
      sql << ", ";
    sql << "\"" << mapping->fields[i].name() << "\" = ?";

    firstField = false;
  }

  sql << " where ";

  std::string idCondition;

  if (!mapping->surrogateIdFieldName) {
    firstField = true;

    for (unsigned i = 0; i < mapping->fields.size(); ++i) {
      if (mapping->fields[i].isNaturalIdField()) {
	if (!firstField)
	  idCondition += " and ";
	idCondition += "\"" + mapping->fields[i].name() + "\" = ?";

	firstField = false;
      }
    }
  } else
    idCondition
      += std::string() + "\"" + mapping->surrogateIdFieldName + "\" = ?";

  sql << idCondition;

  if (mapping->versionFieldName)
    sql << " and \"" << mapping->versionFieldName << "\" = ?";

  mapping->statements.push_back(sql.str()); // SqlUpdate

  /*
   * SqlDelete
   */

  sql.str("");

  sql << "delete from \"" << table << "\" where " << idCondition;

  mapping->statements.push_back(sql.str()); // SqlDelete

  /*
   * SqlDeleteVersioned
   */
  if (mapping->versionFieldName)
    sql << " and \"" << mapping->versionFieldName << "\" = ?";

  mapping->statements.push_back(sql.str()); // SqlDeleteVersioned

  /*
   * SelectedById
   */

  sql.str("");

  sql << "select ";

  firstField = true;
  if (mapping->versionFieldName) {
    sql << "\"" << mapping->versionFieldName << "\"";
    firstField = false;
  }

  for (unsigned i = 0; i < mapping->fields.size(); ++i) {
    if (!firstField)
      sql << ", ";
    sql << "\"" << mapping->fields[i].name() << "\"";
    firstField = false;
  }

  sql << " from \"" << table << "\" where " << idCondition;

  mapping->statements.push_back(sql.str()); // SelectById

  /*
   * Collections SQL
   */
  for (unsigned i = 0; i < mapping->sets.size(); ++i) {
    const SetInfo& info = mapping->sets[i];

    sql.str("");

    MappingInfo *otherMapping = getMapping(info.tableName);

    // select [surrogate id,] version, ... from other

    sql << "select ";

    firstField = true;
    if (otherMapping->surrogateIdFieldName) {
      sql << "\"" << otherMapping->surrogateIdFieldName << "\"";
      firstField = false;
    }

    if (otherMapping->versionFieldName) {
      if (!firstField)
	sql << ", ";
      sql << "\"" << otherMapping->versionFieldName << "\"";
      firstField = false;
    }

    std::string fkConditions;

    for (unsigned i = 0; i < otherMapping->fields.size(); ++i) {
      if (!firstField)
	sql << ", ";
      firstField = false;

      const FieldInfo& field = otherMapping->fields[i];
      sql << "\"" << field.name() << "\"";

      if (field.isForeignKey()
	  && field.foreignKeyName() == info.joinName
	  && field.foreignKeyTable() == mapping->tableName) {
	if (!fkConditions.empty())
	  fkConditions += " and ";
	fkConditions += std::string("\"") + field.name() + "\" = ?";
      }
    }

    sql << " from \"" << Impl::quoteSchemaDot(otherMapping->tableName);
    
    switch (info.type) {
    case ManyToOne:
      // where joinfield_id(s) = ?

      sql << "\" where " << fkConditions;

      mapping->statements.push_back(sql.str());
      break;
    case ManyToMany:
      // (1) select for collection

      //     join "joinName" on "joinName"."joinId(s) = this."id(s)
      //     where joinfield_id(s) = ?

      std::string joinName = Impl::quoteSchemaDot(info.joinName);
      std::string tableName = Impl::quoteSchemaDot(info.tableName);

      sql << "\" join \"" << joinName
	  << "\" on ";

      std::vector<JoinId> otherJoinIds
	= getJoinIds(otherMapping, info.joinOtherId);

      if (otherJoinIds.size() > 1)
	sql << "(";

      for (unsigned i = 0; i < otherJoinIds.size(); ++i) {
	if (i != 0)
	  sql << " and ";
	sql << "\"" << joinName << "\".\"" << otherJoinIds[i].joinIdName
	    << "\" = \""
	    << tableName << "\".\"" << otherJoinIds[i].tableIdName << "\"";
      }

      if (otherJoinIds.size() > 1)
	sql << ")";

      sql << " where ";

      std::vector<JoinId> selfJoinIds
	= getJoinIds(mapping, info.joinSelfId);

      for (unsigned i = 0; i < selfJoinIds.size(); ++i) {
	if (i != 0)
	  sql << " and ";
	sql << "\"" << joinName << "\".\"" << selfJoinIds[i].joinIdName
	    << "\" = ?";
      }

      mapping->statements.push_back(sql.str());

      // (2) insert into collection

      sql.str("");

      sql << "insert into \"" << joinName
	  << "\" (";

      firstField = true;
      for (unsigned i = 0; i < selfJoinIds.size(); ++i) {
	if (!firstField)
	  sql << ", ";
	firstField = false;

	sql << "\"" << selfJoinIds[i].joinIdName << "\"";
      }

      for (unsigned i = 0; i < otherJoinIds.size(); ++i) {
	if (!firstField)
	  sql << ", ";
	firstField = false;

	sql << "\"" << otherJoinIds[i].joinIdName << "\"";
      }

      sql << ") values (";

      for (unsigned i = 0; i < selfJoinIds.size() + otherJoinIds.size(); ++i) {
	if (i != 0)
	  sql << ", ";
	sql << "?";
      }

      sql << ")";

      mapping->statements.push_back(sql.str());

      // (3) delete from collections

      sql.str("");

      sql << "delete from \"" << joinName << "\" where ";

      firstField = true;
      for (unsigned i = 0; i < selfJoinIds.size(); ++i) {
	if (!firstField)
	  sql << " and ";
	firstField = false;

	sql << "\"" << selfJoinIds[i].joinIdName << "\" = ?";
      }

      for (unsigned i = 0; i < otherJoinIds.size(); ++i) {
	if (!firstField)
	  sql << " and ";
	firstField = false;

	sql << "\"" << otherJoinIds[i].joinIdName << "\" = ?";
      }

      mapping->statements.push_back(sql.str());
    }
  }
}

/*
void Session::mergeDuplicates(MappingInfo *mapping)
{
  for (unsigned i = 0; i < mapping->fields.size(); ++i) {
    FieldInfo& f = mapping->fields[i];
    for (unsigned j = i + 1; j < mapping->fields.size(); ++j) {
      FieldInfo& f2 = mapping->fields[j];
      if (f.name() == f2.name()) {
	if (f.sqlType() != f2.sqlType())
	  throw Exception("Table: " + mapping->tableName + ": field '"
			  + f.name() + "' mapped multiple times");
			  "for " + mapping->tableName + "."
			  + set.joinName);
	  
      }
    }
  }
}
*/

void Session::resolveJoinIds(MappingInfo *mapping)
{
  for (unsigned i = 0; i < mapping->sets.size(); ++i) {
    SetInfo& set = mapping->sets[i];

    if (set.type == ManyToMany) {
      MappingInfo *other = getMapping(set.tableName);

      bool found = false;
      for (unsigned j = 0; j < other->sets.size(); ++j) {
	const SetInfo& otherSet = other->sets[j];

	if (otherSet.joinName == set.joinName) {
	  // second check make sure we find the other id if Many-To-Many between
	  // same table
	  if (mapping != other || i != j) {
	    set.joinOtherId = otherSet.joinSelfId;
	    found = true;
	    break;
	  }
	}
      }

      if (!found)
	throw Exception("Could not find corresponding Many-To-Many collection: "
			"for " + std::string(mapping->tableName) + "."
			+ set.joinName);
    }
  }
}

void Session::createTables()
{
  initSchema();

  Transaction t(*this);

  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    i->second->initialized_ = false; // to do ordered table creation
  
  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    createTable(i->second);

  std::set<std::string> joinTablesCreated;
  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    createRelations(i->second, joinTablesCreated);

  t.commit();
}

void Session::createTable(MappingInfo *mapping)
{
  if (mapping->initialized_)
    return;

  mapping->initialized_ = true;

  std::stringstream sql;

  sql << "create table \"" << Impl::quoteSchemaDot(mapping->tableName)
      << "\" (\n";

  bool firstField = true;

  // Auto-generated id
  if (mapping->surrogateIdFieldName) {
    sql << "  \"" << mapping->surrogateIdFieldName << "\" "
	<< connection(false)->autoincrementType()
	<< " primary key "
	<< connection(false)->autoincrementSql() << "";
    firstField = false;
  }

  // Optimistic locking version field
  if (mapping->versionFieldName) {
    if (!firstField)
      sql << ",\n";

    sql << "  \"" << mapping->versionFieldName << "\" "
	<< sql_value_traits<int>::type(0, 0);

    firstField = false;
  }

  std::string primaryKey;
  for (unsigned i = 0; i < mapping->fields.size(); ++i) {
    const FieldInfo& field = mapping->fields[i];

    if (!field.isVersionField()) {
      if (!firstField)
	sql << ",\n";

      std::string sqlType = field.sqlType();
      if (field.isForeignKey()) {
	if (sqlType.length() > 9
	    && sqlType.substr(sqlType.length() - 9) == " not null")
	  sqlType = sqlType.substr(0, sqlType.length() - 9);
      }

      sql << "  \"" << field.name() << "\" " << sqlType;

      firstField = false;

      if (field.isNaturalIdField()) {
	if (!primaryKey.empty())
	  primaryKey += ", ";
	primaryKey += "\"" + field.name() + "\"";
      }
    }
  }

  if (!primaryKey.empty()) {
    if (!firstField)
      sql << ",\n";

    sql << "  primary key (" << primaryKey << ")";
  }

  for (unsigned i = 0; i < mapping->fields.size();) {
    const FieldInfo& field = mapping->fields[i];

    if (field.isForeignKey()) {
      if (!firstField)
	sql << ",\n";

      sql << "  constraint \"fk_"
	  << mapping->tableName << "_" << field.foreignKeyName() << "\""
	  << " foreign key (\"" << field.name() << "\"";

      unsigned j = i + 1;
      while (j < mapping->fields.size()) {
	const FieldInfo& nextField = mapping->fields[j];
	if (nextField.foreignKeyName() == field.foreignKeyName()) {
	  sql << ", \"" << nextField.name() << "\"";
	  ++j;
	} else
	  break;
      }

      MappingInfo *otherMapping = getMapping(field.foreignKeyTable().c_str());

      if (!otherMapping->initialized_)
	createTable(otherMapping);

      sql << ") references \"" << Impl::quoteSchemaDot(field.foreignKeyTable())
	  << "\" (" << otherMapping->primaryKeys() << ")";

      i = j;
    } else
      ++i;
  }

  sql << "\n)\n";

  connection(true)->executeSql(sql.str());
}

void Session::createRelations(MappingInfo *mapping,
			      std::set<std::string>& joinTablesCreated)
{
  for (unsigned i = 0; i < mapping->sets.size(); ++i) {
    const SetInfo& set = mapping->sets[i];

    if (set.type == ManyToMany) {
      if (joinTablesCreated.count(set.joinName) == 0) {
	joinTablesCreated.insert(set.joinName);

	MappingInfo *other = getMapping(set.tableName);

	createJoinTable(set.joinName, mapping, other,
			set.joinSelfId, set.joinOtherId);
      }
    }
  }
}

void Session::createJoinTable(const std::string& joinName,
			      MappingInfo *mapping1, MappingInfo *mapping2,
			      const std::string& joinId1,
			      const std::string& joinId2)
{
  MappingInfo joinTableMapping;

  joinTableMapping.tableName = joinName.c_str();
  joinTableMapping.versionFieldName = 0;
  joinTableMapping.surrogateIdFieldName = 0;

  addJoinTableFields(joinTableMapping, mapping1, joinId1, "key1");
  addJoinTableFields(joinTableMapping, mapping2, joinId2, "key2");

  createTable(&joinTableMapping);

  std::set<std::string> dummy;
  createRelations(&joinTableMapping, dummy);

  createJoinIndex(joinTableMapping, mapping1, joinId1, "key1");
  createJoinIndex(joinTableMapping, mapping2, joinId2, "key2");
}

void Session::createJoinIndex(MappingInfo& joinTableMapping,
			      MappingInfo *mapping,
			      const std::string& joinId,
			      const std::string& foreignKeyName)
{
  std::stringstream sql;

  sql << "create index \"" << joinTableMapping.tableName << "_"
      << mapping->tableName;

  if (!joinId.empty())
    sql << "_" << joinId;

  sql << "\" on \"" << Impl::quoteSchemaDot(joinTableMapping.tableName)
      << "\" (";

  bool firstField = true;
  for (unsigned int i = 0; i < joinTableMapping.fields.size(); ++i) {
    const FieldInfo& f = joinTableMapping.fields[i];
    if (f.foreignKeyName() == foreignKeyName) {
      if (!firstField)
	sql << ", ";
      firstField = false;

      sql << "\"" << f.name() << "\"";
    }
  }

  sql << ")";

  connection(true)->executeSql(sql.str());
}

std::vector<Session::JoinId> 
Session::getJoinIds(MappingInfo *mapping, const std::string& joinId)
{
  std::vector<Session::JoinId> result;

  if (mapping->surrogateIdFieldName) {
    std::string idName;

    if (joinId.empty())
      idName = std::string(mapping->tableName)
	+ "_" + mapping->surrogateIdFieldName;
    else
      idName = joinId;

    result.push_back
      (JoinId(idName, mapping->surrogateIdFieldName,
	      sql_value_traits<long long>::type(0, 0)));

  } else {
    std::string foreignKeyName;

    if (joinId.empty())
      foreignKeyName = std::string(mapping->tableName);
    else
      foreignKeyName = joinId;

    for (unsigned i = 0; i < mapping->fields.size(); ++i) {
      const FieldInfo& field = mapping->fields[i];

      if (field.isNaturalIdField()) {
	std::string idName = foreignKeyName + "_" + field.name();
	result.push_back(JoinId(idName, field.name(), field.sqlType()));
      }
    }
  }

  return result;
}

void Session::addJoinTableFields(MappingInfo& result, MappingInfo *mapping,
				 const std::string& joinId,
				 const std::string& keyName)
{
  std::vector<JoinId> joinIds = getJoinIds(mapping, joinId);

  for (unsigned i = 0; i < joinIds.size(); ++i)
    result.fields.push_back
      (FieldInfo(joinIds[i].joinIdName, &typeid(long long),
		 joinIds[i].sqlType + " not null",
		 mapping->tableName, keyName,
		 FieldInfo::NaturalId | FieldInfo::ForeignKey));
}

void Session::dropTables()
{
  initSchema();

  Transaction t(*this);

  flush();

  std::set<std::string> tablesDropped;
  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    i->second->dropTable(*this, tablesDropped);

  t.commit();
}

Session::MappingInfo *Session::getMapping(const char *tableName) const
{
  TableRegistry::const_iterator i = tableRegistry_.find(tableName);

  if (i != tableRegistry_.end())
    return i->second;
  else
    return 0;
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
    dbo->flush();
    dirtyObjects_.erase(i);
    dbo->decRef();
  }
}

void Session::rereadAll()
{
  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    i->second->rereadAll();
}

std::string Session::statementId(const char *tableName, int statementIdx)
{  
  return std::string(tableName) + ":"
    + boost::lexical_cast<std::string>(statementIdx);
}

SqlStatement *Session::getStatement(const std::string& id)
{
  return connection(true)->getStatement(id);
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

const std::string&
Session::getStatementSql(const char *tableName, int statementIdx)
{
  return getMapping(tableName)->statements[statementIdx];
}

SqlStatement *Session::prepareStatement(const std::string& id,
					const std::string& sql)
{
  SqlConnection *conn = connection(false);
  SqlStatement *result = conn->prepareStatement(sql);
  conn->saveStatement(id, result);
  result->use();

  return result;
}

void Session::getFields(const char *tableName,
			std::vector<FieldInfo>& result)
{
  initSchema();

  MappingInfo *mapping = getMapping(tableName);
  if (!mapping)
    throw std::logic_error(std::string("Table ") + tableName
			   + " was not mapped.");

  if (mapping->surrogateIdFieldName)
    result.push_back(FieldInfo(mapping->surrogateIdFieldName,
			       &typeid(long long),
			       sql_value_traits<long long>::type(0, 0),
			       FieldInfo::SurrogateId));

  if (mapping->versionFieldName)
    result.push_back(FieldInfo(mapping->versionFieldName, &typeid(int),
			       sql_value_traits<int>::type(0, 0),
			       FieldInfo::Version));

  result.insert(result.end(), mapping->fields.begin(), mapping->fields.end());
}

  }
}
