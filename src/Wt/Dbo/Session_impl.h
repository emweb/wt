// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_SESSION_IMPL_H_
#define WT_DBO_SESSION_IMPL_H_

#include <iostream>
#include <stdexcept>

#include <Wt/Dbo/SqlConnection>
#include <Wt/Dbo/Query>

namespace Wt {
  namespace Dbo {

template <class C>
void Session::mapClass(const char *tableName)
{
  ClassMapping<C> *mapping = new ClassMapping<C>();
  mapping->tableName = tableName;

  classRegistry_[&typeid(C)] = mapping;
}

template <class C>
SqlStatement *Session::getStatement(int statementIdx)
{
  ClassRegistry::iterator i = classRegistry_.find(&typeid(C));
  ClassMappingInfo *mapping = i->second;

  std::string id = statementId(mapping->tableName, statementIdx);

  SqlStatement *result = getStatement(id);

  if (!result) {
    // not yet prepared in this connection
    if (mapping->statements.empty())
      mapping->prepareStatements(*this);

    result = prepareStatement(id, mapping->statements[statementIdx]);
  }

  return result;
}

template <class C>
void Session::prepareStatements()
{
  ClassRegistry::iterator i = classRegistry_.find(&typeid(C));
  ClassMappingInfo *mapping = i->second;

  if (mapping->statements.empty())
      mapping->prepareStatements(*this);
}

template <class C>
std::string Session::manyToManyJoinId(const std::string& joinName,
				      const std::string& notId)
{
  GetManyToManyJoinIdAction action(joinName, notId);

  C dummy;
  action.visit(dummy);

  if (action.found())
    return action.result();
  else
    throw std::logic_error("No reverse mapping found for "
			   "many-to-many relation '" + joinName + '\'');
}

template <class C>
const char *Session::tableName() const
{
  ClassRegistry::const_iterator i = classRegistry_.find(&typeid(C));
  if (i != classRegistry_.end())
    return i->second->tableName;
  else
    throw std::logic_error(std::string("Class ")
			   + typeid(C).name() + " was not mapped.");
}

template <class C>
ptr<C> Session::load(long long id, SqlStatement *statement, int& column)
{
  DboKey key(tableName<C>(), id);
  Registry::iterator i = registry_.find(key);

  if (i == registry_.end()) {
    MetaDbo<C> *dbo = new MetaDbo<C>(id, -1, MetaDboBase::Persisted, *this, 0);

    if (statement) {
      C *obj = implLoad<C>(*dbo, statement, column);
      dbo->setObj(obj);
    }

    registry_[key] = dbo;

    return ptr<C>(dbo);
  } else {
    ClassRegistry::iterator cc = classRegistry_.find(&typeid(C));
    ClassMappingInfo *mapping = cc->second;
    column += mapping->columnCount - 1;

    return ptr<C>(dynamic_cast< MetaDbo<C> *>(i->second));
  }
}

template <class C>
ptr<C> Session::add(ptr<C>& obj)
{
  MetaDbo<C> *dbo = obj.obj();
  if (dbo && !dbo->session()) {
    dbo->setSession(this);
    dirtyObjects_.insert(dbo);
    dbo->incRef();

    SessionAddAction act(this);
    act.visit(*dbo->obj());
  }

  return obj;
}

template <class C>
ptr<C> Session::add(C *obj)
{
  ptr<C> result(obj);
  return add(result);
}

template <class C>
ptr<C> Session::load(long long id)
{
  int column = 0;
  return load<C>(id, 0, column);
}

template <class C>
Query< ptr<C> > Session::find(const std::string& where)
{
  if (!transaction_)
    throw std::logic_error("Dbo find(): no active transaction");

  std::string columns = sql_result_traits< ptr<C> >::getColumns(*this, 0);
  std::string from = std::string("from \"") + tableName<C>() + "\" " + where;

  return Query< ptr<C> >(*this, "select " + columns, from);
}

template <class Result>
Query<Result> Session::query(const std::string& sql)
{
  if (!transaction_)
    throw std::logic_error("Dbo find(): no active transaction");

  std::vector<std::string> aliases;
  std::string rest;
  parseSql(sql, aliases, rest);

  std::string columns
    = sql_result_traits<Result>::getColumns(*this, &aliases);

  if (!aliases.empty())
    throw std::logic_error("Session::query(): too many aliases for result");

  return Query<Result>(*this, "select " + columns, rest);
}

template<class C>
void Session::prune(MetaDbo<C> *obj)
{
  DboKey key(tableName<C>(), obj->id());
  registry_.erase(key);

  prune(static_cast<MetaDboBase *>(obj));
}

template<class C>
void Session::implSave(MetaDbo<C>& dbo)
{
  if (!transaction_)
    throw std::logic_error("Dbo save(): no active transaction");

  transaction_->objects_.push_back(new ptr<C>(&dbo));

  SaveDbAction action(dbo);
  action.visit(*dbo.obj());
  registry_[DboKey(tableName<C>(), dbo.id())] = &dbo;
}

template<class C>
void Session::implDelete(MetaDbo<C>& dbo)
{
  if (!transaction_)
    throw std::logic_error("Dbo save(): no active transaction");

  // when saved in transaction, we are already in this list
  if (!dbo.savedInTransaction())
    transaction_->objects_.push_back(new ptr<C>(&dbo));

  SqlStatement *statement
    = getStatement<C>(dbo.obj() != 0 ? SqlDeleteVersioned : SqlDelete);

  // when saved in the transaction, we will be at version() + 1
  doDelete(statement, dbo.id(), dbo.obj() != 0,
	   dbo.version() + (dbo.savedInTransaction() ? 1 : 0));
}

template<class C>
void Session::implTransactionDone(MetaDbo<C>& dbo, bool success)
{
  TransactionDoneAction action(dbo, success);
  action.visit(*dbo.obj());
}

template <class C>
C *Session::implLoad(MetaDboBase& dbo, SqlStatement *statement, int& column)
{
  if (!transaction_)
    throw std::logic_error("Dbo load(): no active transaction");

  LoadDbAction action(dbo, statement, column);

  C *result = new C();
  action.visit(*result);
  return result;
}

template <class C>
void Session::ClassMapping<C>
::createTable(Session& session, std::set<std::string>& tablesCreated)
{
  if (tablesCreated.count(tableName) == 0) {
    CreateSchema action(session, tableName, tablesCreated);
    C dummy;
    action.visit(dummy);
  }
}

template <class C>
void Session::ClassMapping<C>::prepareStatements(Session& session)
{
  // the prepare string was not yet created
  statements.push_back(std::string()); // no longer empty
  PrepareStatements prepareAction(session, tableName);

  C dummy;
  prepareAction.visitSelf(dummy);
  statements = prepareAction.getSelfSql();
  columnCount = prepareAction.columnCount();

  prepareAction.visitCollections(dummy);
  prepareAction.addCollectionsSql(statements);
}

  }
}

#endif // WT_DBO_SESSION_IMPL_H_
