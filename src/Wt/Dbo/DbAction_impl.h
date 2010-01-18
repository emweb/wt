// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_DBACTION_IMPL_H_
#define WT_DBO_DBACTION_IMPL_H_

namespace Wt {
  namespace Dbo {

template <class C, class Enable>
template <class A>
void persist<C, Enable>::apply(C& obj, A& action)
{
  obj.persist(action);
}

    /*
     * PrepareStatements
     */

template<class C>
void PrepareStatements::visitSelf(C& obj)
{
  pass_ = Self;

  persist<C>::apply(obj, *this);
}

template<class C>
void PrepareStatements::visitCollections(C& obj)
{
  pass_ = Collections;

  persist<C>::apply(obj, *this);
}

template<typename V>
void PrepareStatements::act(const FieldRef<V>& field)
{
  if (pass_ == Self)
    if (!field.name().empty())
      fields_.push_back(field.name());
}

template<class C>
void PrepareStatements::actCollection(const CollectionRef<C>& field)
{
  if (pass_ == Self) {
    const char *joinTableName = session_.tableName<C>();
    std::string joinOtherId = field.type() == ManyToMany
      ? session_.manyToManyJoinId<C>(field.joinName(), field.joinId())
      : std::string();

    sets_.push_back(SetInfo(joinTableName, field.type(), field.joinName(),
			    field.joinId(), joinOtherId));
  }

  if (pass_ == Collections)
    session_.prepareStatements<C>();
}

template<class C>
void PrepareStatements::descend(ptr<C>& obj)
{
}

    /*
     * CreateSchema
     */

template<class C>
void CreateSchema::visit(C& obj)
{
  pass_ = Self;
  persist<C>::apply(obj, *this);
  exec();

  if (needSetsPass_) {
    pass_ = Sets;
    persist<C>::apply(obj, *this);
  }
}

template<typename V>
void CreateSchema::act(const FieldRef<V>& field)
{
  switch (pass_) {
  case Self:
    if (!field.name().empty()) {
      sql_ << ",\n  \"" << field.name() << "\" " << field.sqlType(session_);
      field.descend(*this);
    } else
      needSetsPass_ = true;

    break;
  case Sets:
    if (field.name().empty())
      field.descend(*this);
  }
}

template<class C>
void CreateSchema::actCollection(const CollectionRef<C>& field)
{
  switch (pass_) {
  case Self:
    needSetsPass_ = true;
    break;

  case Sets:
    if (field.type() == ManyToMany) {
      const char *tableName = session_.tableName<C>();
      
      if (tablesCreated_.count(tableName) != 0) {
	std::string joinOtherId
	  = session_.manyToManyJoinId<C>(field.joinName(), field.joinId());

	createJoinTable(field.joinName(), tableName, tableName_,
			field.joinId(), joinOtherId);
      }
    }
  }
}

template<class C>
void CreateSchema::descend(ptr<C>& obj)
{
  const char *tableName = session_.tableName<C>();

  if (tablesCreated_.count(tableName) == 0) {
    CreateSchema action(session_, tableName, tablesCreated_);
    C dummy;
    action.visit(dummy);
  }
}

    /*
     * SaveDbAction
     */

template<class C>
void SaveDbAction::visit(C& obj)
{
  tableName_ = dbo_.session()->template tableName<C>();
  /*
   * (1) Dependencies
   */
  startDependencyPass();
  persist<C>::apply(obj, *this);

  /*
   * (2) Self
   */
  if (!statement_) {
    isInsert_ = dbo_.deletedInTransaction()
      || (dbo_.isNew() && !dbo_.savedInTransaction());

    statement_ = isInsert_
      ? dbo_.session()->template getStatement<C>(Session::SqlInsert)
      : dbo_.session()->template getStatement<C>(Session::SqlUpdate);
  }

  startSelfPass();
  persist<C>::apply(obj, *this);
  exec();

  /*
   * (3) collections:
   *  - references in select queries (for ManyToOne and ManyToMany)
   *  - inserts in ManyToMany collections
   *  - deletes from ManyToMany collections
   */
  if (needSetsPass_) {
    loadSets_.setTableName(tableName_);

    startSetsPass();
    persist<C>::apply(obj, *this);
  }
}

template<typename V>
void SaveDbAction::act(const FieldRef<V>& field)
{
  switch (pass_) {
  case Dependencies:
    // First, descend into referenced dbo's (see descend())
    if (!field.name().empty())
      field.descend(*this);

    break;
  case Self:
    // Next, build the sql statement
    if (!field.name().empty())
      field.bindValue(statement_, column_++);

    break;
  case Sets:
    break;
  }
}

template<class C>
void SaveDbAction::actCollection(const CollectionRef<C>& field)
{
  switch (pass_) {
  case Self:
    if (isInsert_ || field.type() == ManyToMany)
      needSetsPass_ = true;

    break;
  case Sets:
    if (field.type() == ManyToMany) {
      typename collection< ptr<C> >::Activity *activity
	= field.value().activity();

      if (activity) {
	std::set< ptr<C> >& inserted = activity->inserted;

	// Sql insert
	int statementIdx
	  = Session::FirstSqlSelectSet + loadSets_.setStatementIdx() + 1;

	SqlStatement *statement = dbo_.session()->getStatement(tableName_,
							       statementIdx);

	for (typename std::set< ptr<C> >::iterator i = inserted.begin();
	     i != inserted.end(); ++i) {
	  // Make sure it is saved
	  i->flush();

	  statement->reset();
	  statement->bind(0, dbo_.id());
	  statement->bind(1, i->id());
	  statement->execute();
	}

	std::set< ptr<C> >& erased = activity->erased;

	// Sql delete
	++statementIdx;

	statement = dbo_.session()->getStatement(tableName_, statementIdx);

	for (typename std::set< ptr<C> >::iterator i = erased.begin();
	     i != erased.end(); ++i) {
	  // Make sure it is saved (?)
	  i->flush();

	  statement->reset();
	  statement->bind(0, dbo_.id());
	  statement->bind(1, i->id());
	  statement->execute();
	}

	activity->transactionInserted.insert(activity->inserted.begin(),
					     activity->inserted.end());
	activity->transactionErased.insert(activity->erased.begin(),
					   activity->erased.end());

	activity->inserted.clear();
	activity->erased.clear();
      }
    }

    loadSets_.actCollection(field);
  }
}

template<class C>
void SaveDbAction::descend(ptr<C>& obj)
{
  obj.flush();
}

    /*
     * LoadDbAction
     */

template<class C>
void LoadDbAction::visit(C& obj)
{
  bool continueStatement = statement_ != 0;
  Session *session = dbo_.session();

  if (!continueStatement) {
    statement_ = session->template getStatement<C>(Session::SqlSelectById);
    exec();
  }

  setTableName(session->template tableName<C>());

  start();
  persist<C>::apply(obj, *this);

  if (!continueStatement)
    done();
}

template<typename V>
void LoadDbAction::act(const FieldRef<V>& field)
{
  if (!field.name().empty())
    field.setValue(*dbo_.session(), statement_, column_++);
}

template<class C>
void LoadDbAction::actCollection(const CollectionRef<C>& field)
{
  if (dbo_.id() != -1) {
    if (field.value().arg() != dbo_.id()) {
      int statementIdx = Session::FirstSqlSelectSet + setStatementIdx_;

      SqlStatement *statement = dbo_.session()->getStatement(tableName_,
							     statementIdx);

      field.value().setStatement(statement, 0, dbo_.session());
      field.value().setArg(dbo_.id());
    } 
  } else
    field.value().clearStatement();

  if (field.type() == ManyToOne)
    ++setStatementIdx_;
  else
    setStatementIdx_ += 3;
}

template<class C>
void LoadDbAction::descend(ptr<C>& obj)
{ }

    /*
     * TransactionDoneAction
     */

template<class C>
void TransactionDoneAction::visit(C& obj)
{
  persist<C>::apply(obj, *this);
}

template<typename V>
void TransactionDoneAction::act(const FieldRef<V>& field)
{ }

template<class C>
void TransactionDoneAction::actCollection(const CollectionRef<C>& field)
{
  if (!success_)
    undoLoadSets_.actCollection(field);

  if (field.type() == ManyToMany) {
    if (success_)
      field.value().resetActivity();
    else {
      typename collection< ptr<C> >::Activity *activity
	= field.value().activity();

      if (activity) {
	activity->inserted = activity->transactionInserted;
	activity->transactionInserted.clear();
	activity->erased = activity->transactionErased;
	activity->transactionErased.clear();
      }
    }
  }
}

template<class C>
void TransactionDoneAction::descend(ptr<C>& obj)
{ }

    /*
     * TransactionDoneAction
     */

template<class C>
void SessionAddAction::visit(C& obj)
{
  persist<C>::apply(obj, *this);
}

template<typename V>
void SessionAddAction::act(const FieldRef<V>& field)
{ }

template<class C>
void SessionAddAction::actCollection(const CollectionRef<C>& field)
{
  if (field.value().session() != session_)
    field.value().setStatement(0, 0, session_);

  // FIXME: cascade add ?
}

template<class C>
void SessionAddAction::descend(ptr<C>& obj)
{ 
  // FIXME: cascade add ?
}

    /*
     * GetManyToManyJoinIdAction
     */

template<class C>
void GetManyToManyJoinIdAction::visit(C& obj)
{
  persist<C>::apply(obj, *this);
}

template<typename V>
void GetManyToManyJoinIdAction::act(const FieldRef<V>& field)
{ }

template<class C>
void GetManyToManyJoinIdAction::actCollection(const CollectionRef<C>& field)
{
  if (found_)
    return;

  if (field.type() == ManyToMany) {
    // second check make sure we find the other id if Many-To-Many between
    // same table
    if (field.joinName() == joinName_
	&& (result_.empty() || field.joinId() != result_)) {
      result_ = field.joinId();
      found_ = true;
    }
  }
}

template<class C>
void GetManyToManyJoinIdAction::descend(ptr<C>& obj)
{ }

  }
}

#endif // WT_DBO_DBACTION_H_
