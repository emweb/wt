// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_SESSION_IMPL_H_
#define WT_DBO_SESSION_IMPL_H_

#include <iostream>

#include <Wt/Dbo/SqlConnection>
#include <Wt/Dbo/Query>

namespace Wt {
  namespace Dbo {
    namespace Impl {
      template <class C, typename T>
      struct LoadHelper
      {
	static ptr<C> load(Session *session, SqlStatement *statement,
			   int& column)
	{
	  return session->loadWithNaturalId<C>(statement, column);
	};
      };

      template <class C>
      struct LoadHelper<C, long long>
      {
	static ptr<C> load(Session *session, SqlStatement *statement,
			   int& column)
	{
	  return session->loadWithLongLongId<C>(statement, column);
	}
      };
    }

template <class C>
void Session::mapClass(const char *tableName)
{
  if (schemaInitialized_)
    throw Exception("Cannot map tables after schema was initialized.");

  if (classRegistry_.find(&typeid(C)) != classRegistry_.end())
    return;

  Mapping<C> *mapping = new Mapping<C>();
  mapping->tableName = tableName;

  classRegistry_[&typeid(C)] = mapping;
  tableRegistry_[tableName] = mapping;
}

template <class C>
SqlStatement *Session::getStatement(int statementIdx)
{
  initSchema();

  ClassRegistry::iterator i = classRegistry_.find(&typeid(C));
  MappingInfo *mapping = i->second;

  std::string id = statementId(mapping->tableName, statementIdx);

  SqlStatement *result = getStatement(id);

  if (!result)
    result = prepareStatement(id, mapping->statements[statementIdx]);

  return result;
}

template <class C>
const char *Session::tableName() const
{
  ClassRegistry::const_iterator i = classRegistry_.find(&typeid(C));
  if (i != classRegistry_.end())
    return dynamic_cast< Mapping<C> *>(i->second)->tableName;
  else
    throw Exception(std::string("Class ") + typeid(C).name()
		    + " was not mapped.");
}

template <class C>
Session::Mapping<C> *Session::getMapping() const
{
  if (!schemaInitialized_)
    initSchema();

  ClassRegistry::const_iterator i = classRegistry_.find(&typeid(C));
  if (i != classRegistry_.end()) {
    Session::Mapping<C> *mapping = dynamic_cast< Mapping<C> *>(i->second);
    if (!mapping->initialized_)
      mapping->init(*const_cast<Session *>(this));
    return mapping;
  } else
    throw Exception(std::string("Class ") + typeid(C).name()
		    + " was not mapped.");
}

template <class C>
ptr<C> Session::load(SqlStatement *statement, int& column)
{
  return Impl::LoadHelper<C, typename dbo_traits<C>::IdType>
    ::load(this, statement, column);
}

template <class C>
ptr<C> Session::loadWithNaturalId(SqlStatement *statement, int& column)
{
  Mapping<C> *mapping = getMapping<C>();

  /* Natural id is possibly multiple fields anywhere */
  MetaDbo<C> *dbo = new MetaDbo<C>(dbo_traits<C>::invalidId(), -1,
				   MetaDboBase::Persisted, *this, 0);
  implLoad<C>(*dbo, statement, column);

  typename Mapping<C>::Registry::iterator i
    = mapping->registry_.find(dbo->id());

  if (i == mapping->registry_.end()) {
    mapping->registry_[dbo->id()] = dbo;
    return ptr<C>(dbo);
  } else {
    dbo->setSession(0);
    delete dbo;
    return ptr<C>(i->second);
  }
}

template <class C>
ptr<C> Session::loadWithLongLongId(SqlStatement *statement, int& column)
{
  Mapping<C> *mapping = getMapping<C>();

  if (mapping->surrogateIdFieldName) {
    /*
     * If mapping uses surrogate keys, then we can first read the id and
     * decide if we already have it.
     *
     * If not, then we need to first read the object, get the id, and if
     * we already had it, delete the redundant copy.
     */
    long long id;

    /* Auto-generated surrogate key is first field */
    statement->getResult(column++, &id);

    typename Mapping<C>::Registry::iterator i = mapping->registry_.find(id);

    if (i == mapping->registry_.end()) {
      MetaDbo<C> *dbo
	= new MetaDbo<C>(id, -1, MetaDboBase::Persisted, *this, 0);
      implLoad<C>(*dbo, statement, column);

      mapping->registry_[id] = dbo;

      return ptr<C>(dbo);
    } else {
      column += (int)mapping->fields.size() + 1; // + version

      return ptr<C>(i->second);
    }
  } else
    return loadWithNaturalId<C>(statement, column);
}

template <class C>
ptr<C> Session::add(ptr<C>& obj)
{
  initSchema();

  MetaDbo<C> *dbo = obj.obj();
  if (dbo && !dbo->session()) {
    dbo->setSession(this);
    needsFlush(dbo);

    SessionAddAction act(*dbo, *getMapping<C>());
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
ptr<C> Session::load(const typename dbo_traits<C>::IdType& id,
		     bool forceReread)
{
  ptr<C> result = loadLazy<C>(id);
  if (forceReread)
    result.reread();
  *result; // Dereference to do actual load or throw exception
  return result;
}

template <class C>
ptr<C> Session::loadLazy(const typename dbo_traits<C>::IdType& id)
{
  initSchema();

  Mapping<C> *mapping = getMapping<C>();
  typename Mapping<C>::Registry::iterator i = mapping->registry_.find(id);

  if (i == mapping->registry_.end()) {
    MetaDbo<C> *dbo = new MetaDbo<C>(id, -1, MetaDboBase::Persisted, *this, 0);
    mapping->registry_[id] = dbo;
    return ptr<C>(dbo);
  } else
    return ptr<C>(i->second);
}

template <class C, typename BindStrategy>
Query< ptr<C>, BindStrategy > Session::find(const std::string& where)
{
  initSchema();

  return Query< ptr<C>, BindStrategy >
    (*this, '"' + Impl::quoteSchemaDot(tableName<C>()) + '"', where);
}

template <class Result>
Query<Result> Session::query(const std::string& sql)
{
  return query<Result, DynamicBinding>(sql);
}

template <class Result, typename BindStrategy>
Query<Result, BindStrategy> Session::query(const std::string& sql)
{
  initSchema();

  return Query<Result, BindStrategy>(*this, sql);
}

template<class C>
void Session::prune(MetaDbo<C> *obj)
{
  getMapping<C>()->registry_.erase(obj->id());

  discardChanges(obj);
}

template<class C>
void Session::implSave(MetaDbo<C>& dbo)
{
  if (!transaction_)
    throw Exception("Dbo save(): no active transaction");

  if (!dbo.savedInTransaction())
    transaction_->objects_.push_back(new ptr<C>(&dbo));

  Session::Mapping<C> *mapping = getMapping<C>();

  SaveDbAction<C> action(dbo, *mapping);
  action.visit(*dbo.obj());

  mapping->registry_[dbo.id()] = &dbo;
}

template<class C>
void Session::implDelete(MetaDbo<C>& dbo)
{
  if (!transaction_)
    throw Exception("Dbo save(): no active transaction");

  // when saved in transaction, we are already in this list
  if (!dbo.savedInTransaction())
    transaction_->objects_.push_back(new ptr<C>(&dbo));

  bool versioned = getMapping<C>()->versionFieldName && dbo.obj() != 0;
  SqlStatement *statement
    = getStatement<C>(versioned ? SqlDeleteVersioned : SqlDelete);

  // when saved in the transaction, we will be at version() + 1

  statement->reset();
  ScopedStatementUse use(statement);

  int column = 0;
  dbo.bindId(statement, column);

  int version = -1;
  if (versioned) {
    version = dbo.version() + (dbo.savedInTransaction() ? 1 : 0);
    statement->bind(column++, version);
  }

  statement->execute();

  if (versioned) {
    int modifiedCount = statement->affectedRowCount();
    if (modifiedCount != 1)
      throw StaleObjectException(boost::lexical_cast<std::string>(dbo.id()),
				 version);
  }
}

template<class C>
void Session::implTransactionDone(MetaDbo<C>& dbo, bool success)
{
  TransactionDoneAction action(dbo, *this, *getMapping<C>(), success);
  action.visit(*dbo.obj());
}

template <class C>
void Session::implLoad(MetaDbo<C>& dbo, SqlStatement *statement, int& column)
{
  if (!transaction_)
    throw Exception("Dbo load(): no active transaction");

  LoadDbAction<C> action(dbo, *getMapping<C>(), statement, column);

  C *obj = new C();
  try {
    action.visit(*obj);
    dbo.setObj(obj);
  } catch (...) {
    delete obj;
    throw;
  }
}

template <class C>
Session::Mapping<C>::~Mapping()
{
  for (typename Registry::iterator i = registry_.begin();
       i != registry_.end(); ++i) {
    i->second->setState(MetaDboBase::Orphaned);
  }
}

template <class C>
void Session::Mapping<C>
::dropTable(Session& session, std::set<std::string>& tablesDropped)
{
  if (tablesDropped.count(tableName) == 0) {
    DropSchema action(session, *this, tablesDropped);
    C dummy;
    action.visit(dummy);
  }
}

template <class C>
void Session::Mapping<C>::rereadAll()
{
  for (typename Registry::iterator i = registry_.begin();
       i != registry_.end(); ++i)
    i->second->reread();
}

template <class C>
void Session::Mapping<C>::init(Session& session)
{
  if (!initialized_) {
    initialized_ = true;

    InitSchema action(session, *this);
    C dummy;
    action.visit(dummy);
  }
}

  }
}

#endif // WT_DBO_SESSION_IMPL_H_
