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

#include <iostream>
#include <vector>
#include <string>
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

SetInfo::SetInfo(const char *aTableName,
		 RelationType aType,
		 const std::string& aJoinName,
		 const std::string& aJoinSelfId,
		 int someFkConstraints)
  : tableName(aTableName),
    joinName(aJoinName),
    joinSelfId(aJoinSelfId),
    flags(0),
    type(aType),
    fkConstraints(someFkConstraints)
{ }

Impl::MappingInfo::MappingInfo()
  : initialized_(false)
{ }

MappingInfo::~MappingInfo()
{ }

void MappingInfo::init(Session& session)
{ 
  throw Exception("Not to be done.");
}

void MappingInfo::dropTable(Session& session,
				     std::set<std::string>& tablesDropped)
{
  throw Exception("Not to be done.");
}

void MappingInfo::rereadAll()
{ 
  throw Exception("Not to be done.");
}

MetaDboBase *MappingInfo::create(Session& session)
{
  throw Exception("Not to be done.");
}

void MappingInfo::load(Session& session, MetaDboBase *obj)
{
  throw Exception("Not to be done.");
}

MetaDboBase *MappingInfo::load(Session& session, SqlStatement *statement,
			       int& column)
{
  throw Exception("Not to be done.");
}

std::string MappingInfo::primaryKeys() const
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

    } // end namespace Impl

Session::JoinId::JoinId(const std::string& aJoinIdName,
			const std::string& aTableIdName,
			const std::string& aSqlType)
  : joinIdName(aJoinIdName),
    tableIdName(aTableIdName),
    sqlType(aSqlType)
{ }

Session::Session()
  : schemaInitialized_(false),
    //useRowsFromTo_(false),
    requireSubqueryAlias_(false),
    connection_(0),
    connectionPool_(0),
    transaction_(0),
    flushMode_(Auto)
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
    throw Exception("Operation requires an active transaction");

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
  MetaDboBaseSet::nth_index<1>::type& setIndex = dirtyObjects_.get<1>();

  if (setIndex.erase(obj) > 0)
    obj->decRef();

  // FIXME what about Transaction.objects_ ?
}

Call Session::execute(const std::string& sql)
{
  initSchema();

  if (!transaction_)
    throw Exception("Dbo execute(): no active transaction");

  return Call(*this, sql);
}

void Session::initSchema() const
{
  if (schemaInitialized_)
    return;

  Session *self = const_cast<Session *>(this);
  self->schemaInitialized_ = true;

  Transaction t(*self);

  SqlConnection *conn = self->connection(false);
  longlongType_ = sql_value_traits<long long>::type(conn, 0);
  intType_ = sql_value_traits<int>::type(conn, 0);
  haveSupportUpdateCascade_ = conn->supportUpdateCascade();
  limitQueryMethod_ = conn->limitQueryMethod();
  requireSubqueryAlias_ = conn->requireSubqueryAlias();

  for (ClassRegistry::const_iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    i->second->init(*self);

  for (ClassRegistry::const_iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    self->resolveJoinIds(i->second);

  for (ClassRegistry::const_iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    self->prepareStatements(i->second);

  t.commit();
}

void Session::prepareStatements(Impl::MappingInfo *mapping)
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

  SqlConnection *conn;
  if (transaction_)
    conn = transaction_->connection_;
  else
    conn = useConnection();

  if (mapping->surrogateIdFieldName) {
    sql << conn->autoincrementInsertSuffix(mapping->surrogateIdFieldName);
  }

  if (!transaction_)
    returnConnection(conn);

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

    if (firstField)
      throw Exception("Table " + std::string(mapping->tableName)
		      + " is missing a natural id defined with Wt::Dbo::id()");
  } else
    idCondition
      += std::string() + "\"" + mapping->surrogateIdFieldName + "\" = ?";

  mapping->idCondition = idCondition;

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
    const Impl::SetInfo& info = mapping->sets[i];

    sql.str("");

    Impl::MappingInfo *otherMapping = getMapping(info.tableName);

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
    std::string other;

    for (unsigned i = 0; i < otherMapping->fields.size(); ++i) {
      if (!firstField)
	sql << ", ";
      firstField = false;

      const FieldInfo& field = otherMapping->fields[i];
      sql << "\"" << field.name() << "\"";

      if (field.isForeignKey()
	  && field.foreignKeyTable() == mapping->tableName) {
	if (field.foreignKeyName() == info.joinName) {
	  if (!fkConditions.empty())
	    fkConditions += " and ";
	  fkConditions += std::string("\"") + field.name() + "\" = ?";
	} else {
	  if (!other.empty())
	    other += " and ";

	  other += "'" + field.foreignKeyName() + "'";
	}
      }
    }

    sql << " from \"" << Impl::quoteSchemaDot(otherMapping->tableName);
    
    switch (info.type) {
    case ManyToOne:
      // where joinfield_id(s) = ?

      if (fkConditions.empty()) {
	std::string msg = std::string()
	  + "Relation mismatch for table '" + mapping->tableName
	  + "': no matching belongsTo() found in table '"
	  + otherMapping->tableName + "' with name '" + info.joinName
	  + "'";

	if (!other.empty())
	  msg += ", but did find with name " + other + "?";

	throw Exception(msg);
      }

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
	= getJoinIds(otherMapping, info.joinOtherId, info.flags & Impl::SetInfo::LiteralOtherId);

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
	= getJoinIds(mapping, info.joinSelfId, info.flags & Impl::SetInfo::LiteralSelfId);

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

void Session::executeSql(std::vector<std::string>& sql, std::ostream *sout)
{
  for (unsigned i = 0; i < sql.size(); i++)
    if (sout)
      *sout << sql[i] << ";\n";
    else
      connection(true)->executeSql(sql[i]);
}

void Session::executeSql(std::stringstream& sql, std::ostream *sout)
{
  if (sout)
    *sout << sql.str() << ";\n";
  else
    connection(true)->executeSql(sql.str());
}

std::string Session::constraintName(const char *tableName,
                           std::string foreignKeyName)
{
  std::stringstream ans;
  ans << "\"fk_"<<tableName << "_" << foreignKeyName << "\"";
  return ans.str();
}


/*
void Session::mergeDuplicates(Impl::MappingInfo *mapping)
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

void Session::resolveJoinIds(Impl::MappingInfo *mapping)
{
  for (unsigned i = 0; i < mapping->sets.size(); ++i) {
    Impl::SetInfo& set = mapping->sets[i];

    if (set.type == ManyToMany) {
      Impl::MappingInfo *other = getMapping(set.tableName);

      for (unsigned j = 0; j < other->sets.size(); ++j) {
	const Impl::SetInfo& otherSet = other->sets[j];

	if (otherSet.joinName == set.joinName) {
	  // second check make sure we find the other id if Many-To-Many between
	  // same table
	  if (mapping != other || i != j) {
	    set.joinOtherId = otherSet.joinSelfId;
	    set.otherFkConstraints = otherSet.fkConstraints;
	    if (otherSet.flags & Impl::SetInfo::LiteralSelfId)
	      set.flags |= Impl::SetInfo::LiteralOtherId;
	    break;
	  }
	}
      }
    }
  }
}

std::string Session::tableCreationSql()
{
  initSchema();

  std::stringstream sout;

  Transaction t(*this);

  std::set<std::string> tablesCreated;

  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    createTable(i->second, tablesCreated, &sout, false);

  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    createRelations(i->second, tablesCreated, &sout);

  t.commit();  

  return sout.str();
}

void Session::createTables()
{
  initSchema();

  Transaction t(*this);

  std::set<std::string> tablesCreated;

  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    createTable(i->second, tablesCreated, 0, false);

  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    createRelations(i->second, tablesCreated, 0);

  t.commit();
}

void Session::createTable(Impl::MappingInfo *mapping,
			  std::set<std::string>& tablesCreated,
                          std::ostream *sout,
                          bool createConstraints)
{
  if (tablesCreated.count(mapping->tableName) != 0)
    return;

  tablesCreated.insert(mapping->tableName);

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
      if (field.isForeignKey() && !(field.fkConstraints() & Impl::FKNotNull)) {
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

    if (field.isForeignKey() && 
	(createConstraints || !connection(false)->supportAlterTable())) {
      if (!firstField)
	sql << ",\n";

      unsigned firstI = i;
      i = findLastForeignKeyField(mapping, field, firstI);
      sql << "  " << constraintString(mapping, field, firstI, i);

      createTable(mapping, tablesCreated, sout, false);
    } else
      ++i;
  }

  sql << "\n)";

  executeSql(sql, sout);

  if (mapping->surrogateIdFieldName) {
    std::string tableName = Impl::quoteSchemaDot(mapping->tableName);
    std::string idFieldName = mapping->surrogateIdFieldName;

    std::vector<std::string> sql = 
      connection(false)->autoincrementCreateSequenceSql(tableName,
							idFieldName);

    executeSql(sql, sout);
  }
}

void Session::createRelations(Impl::MappingInfo *mapping,
			      std::set<std::string>& tablesCreated,
			      std::ostream *sout)
{
  for (unsigned i = 0; i < mapping->sets.size(); ++i) {
    const Impl::SetInfo& set = mapping->sets[i];

    if (set.type == ManyToMany) {
      if (tablesCreated.count(set.joinName) == 0) {
	Impl::MappingInfo *other = getMapping(set.tableName);

	createJoinTable(set.joinName, mapping, other,
			set.joinSelfId, set.joinOtherId,
			set.fkConstraints, set.otherFkConstraints,
			set.flags & Impl::SetInfo::LiteralSelfId,
			set.flags & Impl::SetInfo::LiteralOtherId,
			tablesCreated, sout);
      }
    }
  }

  if (connection(false)->supportAlterTable()){ //backend condition
    for (unsigned i = 0; i < mapping->fields.size();) {
      const FieldInfo& field = mapping->fields[i];
      if (field.isForeignKey()){
        std::stringstream sql;

	std::string table = Impl::quoteSchemaDot(mapping->tableName);

        sql << "alter table \"" << table << "\""
            << " add ";

        unsigned firstI = i;
        i = findLastForeignKeyField(mapping, field, firstI);
        sql << constraintString(mapping, field, firstI, i);

        executeSql(sql, sout);

      } else
        ++i;
    }
  }
}

//constraint fk_... foreign key ( ..., .. , .. ) references (..)
std::string Session::constraintString(Impl::MappingInfo *mapping,
                                      const FieldInfo& field,
                                      unsigned fromIndex,
                                      unsigned toIndex)
{
  std::stringstream sql;

  sql << "constraint \"fk_"
      << mapping->tableName << "_" << field.foreignKeyName() << "\""
      << " foreign key (\"" << field.name() << "\"";

  for(unsigned i = fromIndex + 1; i < toIndex; ++i){
    const FieldInfo& nextField = mapping->fields[i];
    sql << ", \"" << nextField.name() << "\"";
  }

  Impl::MappingInfo *otherMapping = getMapping(field.foreignKeyTable().c_str());

  sql << ") references \"" << Impl::quoteSchemaDot(field.foreignKeyTable())
      << "\" (" << otherMapping->primaryKeys() << ")";

  if (field.fkConstraints() & Impl::FKOnUpdateCascade
      && haveSupportUpdateCascade_)
    sql << " on update cascade";
  else if (field.fkConstraints() & Impl::FKOnUpdateSetNull
	   && haveSupportUpdateCascade_)
    sql << " on update set null";

  if (field.fkConstraints() & Impl::FKOnDeleteCascade)
    sql << " on delete cascade";
  else if (field.fkConstraints() & Impl::FKOnDeleteSetNull)
    sql << " on delete set null";

  if (connection(false)->supportDeferrableFKConstraint()) //backend condition
    sql << " deferrable initially deferred";

  return sql.str();
}

unsigned Session::findLastForeignKeyField(Impl::MappingInfo *mapping,
                                 const FieldInfo& field,
                                 unsigned index)
{
  while (index < mapping->fields.size()) {
    const FieldInfo& nextField = mapping->fields[index];
    if (nextField.foreignKeyName() == field.foreignKeyName()) {
      ++index;
    } else
      break;
  }

  return index;
}

void Session::createJoinTable(const std::string& joinName,
			      Impl::MappingInfo *mapping1,
			      Impl::MappingInfo *mapping2,
			      const std::string& joinId1,
			      const std::string& joinId2,
			      int fkConstraints1, int fkConstraints2,
			      bool literalJoinId1, bool literalJoinId2,
			      std::set<std::string>& tablesCreated,
			      std::ostream *sout)
{
  Impl::MappingInfo joinTableMapping;

  joinTableMapping.tableName = joinName.c_str();
  joinTableMapping.versionFieldName = 0;
  joinTableMapping.surrogateIdFieldName = 0;

  addJoinTableFields(joinTableMapping, mapping1, joinId1, "key1",
		     fkConstraints1, literalJoinId1);
  addJoinTableFields(joinTableMapping, mapping2, joinId2, "key2",
		     fkConstraints2, literalJoinId2);

  createTable(&joinTableMapping, tablesCreated, sout, true);

  createJoinIndex(joinTableMapping, mapping1, joinId1, "key1", sout);
  createJoinIndex(joinTableMapping, mapping2, joinId2, "key2", sout);
}

void Session::createJoinIndex(Impl::MappingInfo& joinTableMapping,
			      Impl::MappingInfo *mapping,
			      const std::string& joinId,
			      const std::string& foreignKeyName,
			      std::ostream *sout)
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

  executeSql(sql, sout);
}

std::vector<Session::JoinId> 
Session::getJoinIds(Impl::MappingInfo *mapping, const std::string& joinId, bool literalJoinId)
{
  std::vector<Session::JoinId> result;

  std::string foreignKeyName;
  if (joinId.empty())
    foreignKeyName = std::string(mapping->tableName);
  else
    foreignKeyName = joinId;

  if (mapping->surrogateIdFieldName) {
    std::string idName;

    if (literalJoinId)
      idName = joinId;
    else
      idName = foreignKeyName
	+ "_" + mapping->surrogateIdFieldName;

    result.push_back
      (JoinId(idName, mapping->surrogateIdFieldName, longlongType_));

  } else {
    int nbNaturalIdFields = 0;
    for (unsigned i = 0; i < mapping->fields.size(); ++i) {
      const FieldInfo& field = mapping->fields[i];

      if (field.isNaturalIdField()) {
	++nbNaturalIdFields;
	std::string idName;
	if (literalJoinId) {
	  // NOTE: there should be only one natural id field in this case!
	  idName = joinId;
	} else {
	  idName = foreignKeyName + "_" + field.name();
	}
	result.push_back(JoinId(idName, field.name(), field.sqlType()));
      }
    }
    if (literalJoinId && nbNaturalIdFields != 1) {
      throw Exception(std::string("The literal join id >") + joinId + " was used,"
		      " but there are " + boost::lexical_cast<std::string>(nbNaturalIdFields) +
		      " natural id fields. There may only be one natural id field.");
    }
  }

  return result;
}

void Session::addJoinTableFields(Impl::MappingInfo& result,
				 Impl::MappingInfo *mapping,
				 const std::string& joinId,
				 const std::string& keyName,
				 int fkConstraints,
				 bool literalJoinId)
{
  std::vector<JoinId> joinIds = getJoinIds(mapping, joinId, literalJoinId);

  for (unsigned i = 0; i < joinIds.size(); ++i)
    result.fields.push_back
      (FieldInfo(joinIds[i].joinIdName, &typeid(long long),
		 joinIds[i].sqlType,
		 mapping->tableName, keyName,
		 FieldInfo::NaturalId | FieldInfo::ForeignKey,
		 fkConstraints));
}

void Session::dropTables()
{
  initSchema();

  if (connectionPool_)
    connectionPool_->prepareForDropTables();
  else
    connection_->prepareForDropTables();

  Transaction t(*this);

  flush();

  //remove constraints first.
  if (connection(false)->supportAlterTable()){
    for (ClassRegistry::iterator i = classRegistry_.begin();
         i != classRegistry_.end(); ++i){
      Impl::MappingInfo *mapping = i->second;
      //find the constraint.
      //ALTER TABLE products DROP CONSTRAINT some_name
      for (unsigned j = 0; j < mapping->fields.size(); ++j) {
        const FieldInfo& field = mapping->fields[j];
        if (field.isForeignKey()){
          std::stringstream sql;
	  std::string table = Impl::quoteSchemaDot(mapping->tableName);

          sql << "alter table \"" << table << "\""
              << " drop "
              << connection(false)->alterTableConstraintString() << " "
              << constraintName(mapping->tableName, field.foreignKeyName());

          j = findLastForeignKeyField(mapping, field, j);

	  executeSql(sql, 0);
        }
      }
    }
  }

  std::set<std::string> tablesDropped;
  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    i->second->dropTable(*this, tablesDropped);

  t.commit();
}

Impl::MappingInfo *Session::getMapping(const char *tableName) const
{
  TableRegistry::const_iterator i = tableRegistry_.find(tableName);

  if (i != tableRegistry_.end())
    return i->second;
  else
    return 0;
}

void Session::needsFlush(MetaDboBase *obj)
{
  typedef MetaDboBaseSet::nth_index<1>::type Set;
  Set& setIndex = dirtyObjects_.get<1>();

  std::pair<Set::iterator, bool> inserted = setIndex.insert(obj);

  if (inserted.second) {
    // was a new entry
    obj->incRef();
  }

  // If it's a delete, move it to the back
  //
  // In fact, this might be wrong: we need to consider dependencies
  // (constraints) that depend on this object: foreign keys generated
  // by 'belongsTo()' referencing this object: the objects that hold
  // these foreign keys may need to be updated (or deleted!) before
  // this object is deleted, one thus needs to take care of the order in which
  // objects are being deleted
  if (obj->isDeleted()) {
    // was an existing entry, move to back
    typedef MetaDboBaseSet::nth_index<0>::type List;
    List& listIndex = dirtyObjects_.get<0>();

    List::iterator i = dirtyObjects_.project<0>(inserted.first);

    listIndex.splice(listIndex.end(), listIndex, i);
  }
}

void Session::flush()
{
  for (unsigned i=0; i < objectsToAdd_.size(); i++)
    needsFlush(objectsToAdd_[i]);

  objectsToAdd_.clear();

  while (!dirtyObjects_.empty()) {
    MetaDboBaseSet::iterator i = dirtyObjects_.begin();
    MetaDboBase *dbo = *i;
    dbo->flush();
    dirtyObjects_.erase(i);
    dbo->decRef();
  }
}

void Session::rereadAll(const char *tableName)
{
  for (ClassRegistry::iterator i = classRegistry_.begin();
       i != classRegistry_.end(); ++i)
    if (!tableName || std::string(tableName) == i->second->tableName)
      i->second->rereadAll();
}

void Session::discardUnflushed()
{
  objectsToAdd_.clear();
  rereadAll();
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

  Impl::MappingInfo *mapping = getMapping(tableName);
  if (!mapping)
    throw Exception(std::string("Table ") + tableName + " was not mapped.");

  if (mapping->surrogateIdFieldName)
    result.push_back(FieldInfo(mapping->surrogateIdFieldName,
			       &typeid(long long),
			       longlongType_,
			       FieldInfo::SurrogateId |
			       FieldInfo::NeedsQuotes));

  if (mapping->versionFieldName)
    result.push_back(FieldInfo(mapping->versionFieldName, &typeid(int),
			       intType_,
			       FieldInfo::Version | FieldInfo::NeedsQuotes));

  result.insert(result.end(), mapping->fields.begin(), mapping->fields.end());
}

MetaDboBase *Session::createDbo(Impl::MappingInfo *mapping)
{
  return mapping->create(*this);
}

void Session::load(MetaDboBase *dbo)
{
  Impl::MappingInfo *mapping = dbo->getMapping();
  mapping->load(*this, dbo);
}
  }
}
