// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_DBACTION_IMPL_H_
#define WT_DBO_DBACTION_IMPL_H_

#include <Wt/Dbo/Exception>
#include <iostream>
#include <boost/lexical_cast.hpp>

namespace Wt {
  namespace Dbo {
    namespace Impl {
      extern std::string WTDBO_API createJoinName(RelationType type,
                                                  const char *c1,
                                                  const char *c2);
    }


template <class C, class Enable>
template <class A>
void persist<C, Enable>::apply(C& obj, A& action)
{
  obj.persist(action);
}

    /*
     * InitSchema
     */

template<class C>
void InitSchema::visit(C& obj)
{
  mapping_.surrogateIdFieldName = dbo_traits<C>::surrogateIdField();
  mapping_.versionFieldName = dbo_traits<C>::versionField();

  persist<C>::apply(obj, *this);
}

template<typename V>
void InitSchema::actId(V& value, const std::string& name, int size)
{
  mapping_.naturalIdFieldName = name;
  mapping_.naturalIdFieldSize = size;

  if (mapping_.surrogateIdFieldName)
    throw Exception("Error: Wt::Dbo::id() called for class C "
		    "with surrogate key: "
		    "Wt::Dbo::dbo_traits<C>::surrogateIdField() != 0");

  idField_ = true;
  field(*this, value, name, size);
  idField_ = false;
}

template<class C>
void InitSchema::actId(ptr<C>& value, const std::string& name, int size,
		       int fkConstraints)
{
  mapping_.naturalIdFieldName = name;
  mapping_.naturalIdFieldSize = size;

  if (mapping_.surrogateIdFieldName)
    throw Exception("Error: Wt::Dbo::id() called for class C "
		    "with surrogate key: "
		    "Wt::Dbo::dbo_traits<C>::surrogateIdField() != 0");

  idField_ = true;
  actPtr(PtrRef<C>(value, name, fkConstraints));
  idField_ = false;
}

template<typename V>
void InitSchema::act(const FieldRef<V>& field)
{
  int flags = FieldInfo::Mutable | FieldInfo::NeedsQuotes;

  if (idField_)
    flags |= FieldInfo::NaturalId; // Natural id

  if (!foreignKeyName_.empty())
    // Foreign key
    mapping_.fields.push_back
      (FieldInfo(field.name(), &typeid(V), field.sqlType(session_),
		 foreignKeyTable_, foreignKeyName_,
		 flags | FieldInfo::ForeignKey, fkConstraints_));
  else
    // Normal field
    mapping_.fields.push_back
      (FieldInfo(field.name(), &typeid(V), field.sqlType(session_), flags));
}

template<class C>
void InitSchema::actPtr(const PtrRef<C>& field)
{
  Session::Mapping<C> *mapping = session_.getMapping<C>();

  bool setName = foreignKeyName_.empty();

  if (setName) {
    foreignKeyName_ = field.name();
    foreignKeyTable_ = mapping->tableName;
    fkConstraints_ = field.fkConstraints();
  }

  field.visit(*this, &session_);

  if (setName) {
    foreignKeyName_.clear();
    foreignKeyTable_.clear();
    fkConstraints_ = 0;
  }
}

template<class C>
void InitSchema::actWeakPtr(const WeakPtrRef<C>& field)
{
  const char *joinTableName = session_.tableName<C>();
  std::string joinName = field.joinName();
  if (joinName.empty())
    joinName = mapping_.tableName;

  mapping_.sets.push_back
    (Impl::SetInfo(joinTableName, ManyToOne, joinName, std::string(), 0));
}

template<class C>
void InitSchema::actCollection(const CollectionRef<C>& field)
{
  const char *joinTableName = session_.tableName<C>();
  std::string joinName = field.joinName();
  if (joinName.empty())
    joinName = Impl::createJoinName(field.type(),
                                    mapping_.tableName, joinTableName);

  mapping_.sets.push_back
    (Impl::SetInfo(joinTableName, field.type(), joinName, field.joinId(),
		      field.fkConstraints()));
  if (field.literalJoinId())
    mapping_.sets.back().flags |= Impl::SetInfo::LiteralSelfId;
}

    /*
     * DropSchema
     */

template<class C>
void DropSchema::visit(C& obj)
{
  persist<C>::apply(obj, *this);

  drop(mapping_.tableName);
}

template<typename V>
void DropSchema::actId(V& value, const std::string& name, int size)
{ }

template<class C>
void DropSchema::actId(ptr<C>& value, const std::string& name, int size,
		       int fkConstraints)
{ }

template<typename V>
void DropSchema::act(const FieldRef<V>& field)
{ }

template<class C>
void DropSchema::actPtr(const PtrRef<C>& field)
{ }

template<class C>
void DropSchema::actWeakPtr(const WeakPtrRef<C>& field)
{ 
  const char *tableName = session_.tableName<C>();
  if (tablesDropped_.count(tableName) == 0) {
    DropSchema action(session_, 
		      *session_.getMapping(tableName), 
		      tablesDropped_);
    C dummy;
    action.visit(dummy);
  }
}

template<class C>
void DropSchema::actCollection(const CollectionRef<C>& field)
{
  if (field.type() == ManyToMany) {
    const char *joinTableName = session_.tableName<C>();
    std::string joinName = field.joinName();
    if (joinName.empty())
      joinName = Impl::createJoinName(field.type(),
                                      mapping_.tableName,
                                      joinTableName);

    if (tablesDropped_.count(joinName) == 0)
      drop(joinName);
  } else {
    const char *tableName = session_.tableName<C>();
    if (tablesDropped_.count(tableName) == 0) {
      DropSchema action(session_, 
			*session_.getMapping(tableName), 
			tablesDropped_);
      C dummy;
      action.visit(dummy);
    }
  }
}

    /*
     * DboAction
     */

template<class C>
void DboAction::actWeakPtr(const WeakPtrRef<C>& field)
{
  Impl::SetInfo *setInfo = &mapping_->sets[setIdx_++];

  if (dbo_->session()) {
    int statementIdx = Session::FirstSqlSelectSet + setStatementIdx_;

    const std::string& sql
      = dbo_->session()->getStatementSql(mapping_->tableName, statementIdx);

    field.value().setRelationData(dbo_, &sql, setInfo);
  } else
    field.value().setRelationData(dbo_, 0, setInfo);

  setStatementIdx_ += 1;
}

template<class C>
void DboAction::actCollection(const CollectionRef<C>& field)
{
  Impl::SetInfo *setInfo = &mapping_->sets[setIdx_++];

  if (dbo_->session()) {
    int statementIdx = Session::FirstSqlSelectSet + setStatementIdx_;

    const std::string& sql
      = dbo_->session()->getStatementSql(mapping_->tableName, statementIdx);

    field.value().setRelationData(dbo_, &sql, setInfo);
  } else
    field.value().setRelationData(dbo_, 0, setInfo);

  if (field.type() == ManyToOne)
    setStatementIdx_ += 1;
  else
    setStatementIdx_ += 3;
}

    /*
     * LoadDbAction
     */

template<typename V>
void LoadBaseAction::act(const FieldRef<V>& field)
{
  field.setValue(*session(), statement_, column_++);
}

template<class C>
void LoadBaseAction::actPtr(const PtrRef<C>& field)
{
  field.visit(*this, session());
}

template <class C>
LoadDbAction<C>::LoadDbAction(MetaDbo<C>& dbo, Session::Mapping<C>& mapping,
			      SqlStatement *statement, int& column)
  : LoadBaseAction(dbo, mapping, statement, column),
    dbo_(dbo)
{ }

template<class C>
void LoadDbAction<C>::visit(C& obj)
{
  ScopedStatementUse use(statement_);

  bool continueStatement = statement_ != 0;
  Session *session = dbo_.session();

  if (!continueStatement) {
    use(statement_ = session->template getStatement<C>(Session::SqlSelectById));
    statement_->reset();

    int column = 0;
    MetaDboBase *dbo = dynamic_cast<MetaDboBase *>(&dbo_);
    dbo->bindId(statement_, column);

    statement_->execute();

    if (!statement_->nextRow()) {
      throw ObjectNotFoundException(session->template tableName<C>(), 
				    boost::lexical_cast<std::string>(dbo_.id()));
    }
  }

  start();

  persist<C>::apply(obj, *this);

  if (!continueStatement && statement_->nextRow())
    throw Exception("Dbo load: multiple rows for id "
		    + boost::lexical_cast<std::string>(dbo_.id()) + " ??");

  if (continueStatement)
    use(0);
}

template<class C>
template<typename V>
void LoadDbAction<C>::actId(V& value, const std::string& name, int size)
{
  field(*this, value, name, size);

  dbo_.setId(value);
}

template<class C>
template<class D>
void LoadDbAction<C>::actId(ptr<D>& value, const std::string& name, int size,
			    int fkConstraints)
{ 
  actPtr(PtrRef<D>(value, name, fkConstraints));

  dbo_.setId(value);
}

    /*
     * SaveDbAction
     */

template<class C>
void SaveBaseAction::actPtr(const PtrRef<C>& field)
{
  switch (pass_) {
  case Dependencies:
    {
      MetaDboBase *dbob = field.value().obj();
      if (dbob)
	dbob->flush();
    }

    break;
  case Self:
    bindNull_ = !field.value();
    field.visit(*this, session());
    bindNull_ = false;

    break;
  case Sets:
    break;
  }
}

template<class C>
void SaveBaseAction::actWeakPtr(const WeakPtrRef<C>& field)
{
  switch (pass_) {
  case Dependencies:
    break;

  case Self:
    if (isInsert_)
      needSetsPass_ = true;

    break;
  case Sets:
    DboAction::actWeakPtr(field);
  }
}

template<class C>
void SaveBaseAction::actCollection(const CollectionRef<C>& field)
{
  switch (pass_) {
  case Dependencies:
    break;

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
	  = Session::FirstSqlSelectSet + setStatementIdx() + 1;

	SqlStatement *statement;

	statement = session()->getStatement(mapping().tableName, statementIdx);
	{
	  ScopedStatementUse use(statement);

	  for (typename std::set< ptr<C> >::iterator i = inserted.begin();
	       i != inserted.end(); ++i) {
	    MetaDboBase *dbo2 = dynamic_cast<MetaDboBase *>(i->obj());

	    // Make sure it is saved
	    dbo2->flush();

	    statement->reset();
	    int column = 0;

	    MetaDboBase *dbo1 = dynamic_cast<MetaDboBase *>(&dbo());
	    dbo1->bindId(statement, column);
	    dbo2->bindId(statement, column);

	    statement->execute();
	  }
	}

	std::set< ptr<C> >& erased = activity->erased;

	// Sql delete
	++statementIdx;

	statement = session()->getStatement(mapping().tableName, statementIdx);

	{
	  ScopedStatementUse use(statement);
	  for (typename std::set< ptr<C> >::iterator i = erased.begin();
	       i != erased.end(); ++i) {
	    MetaDboBase *dbo2 = dynamic_cast<MetaDboBase *>(i->obj());

	    // Make sure it is saved (?)
	    dbo2->flush();

	    statement->reset();
	    int column = 0;

	    MetaDboBase *dbo1 = dynamic_cast<MetaDboBase *>(&dbo());
	    dbo1->bindId(statement, column);
	    dbo2->bindId(statement, column);

	    statement->execute();
	  }
	}

	activity->transactionInserted.insert(activity->inserted.begin(),
					     activity->inserted.end());
	activity->transactionErased.insert(activity->erased.begin(),
					   activity->erased.end());

	activity->inserted.clear();
	activity->erased.clear();
      }
    }

    DboAction::actCollection(field);
  }
}

template <class C>
SaveDbAction<C>::SaveDbAction(MetaDbo<C>& dbo, Session::Mapping<C>& mapping)
  : SaveBaseAction(dbo, mapping),
    dbo_(dbo)
{ }

template<class C>
void SaveDbAction<C>::visit(C& obj)
{
  /*
   * (1) Dependencies
   */
  startDependencyPass();

  persist<C>::apply(obj, *this);

  /*
   * (2) Self
   */
  {
    ScopedStatementUse use(statement_);
    if (!statement_) {
      isInsert_ = dbo_.deletedInTransaction()
	|| (dbo_.isNew() && !dbo_.savedInTransaction());

      use(statement_ = isInsert_
	  ? dbo_.session()->template getStatement<C>(Session::SqlInsert)
	  : dbo_.session()->template getStatement<C>(Session::SqlUpdate));
    } else
      isInsert_ = false;

    startSelfPass();
    persist<C>::apply(obj, *this);

    if (!isInsert_) {
      MetaDboBase *dbo = dynamic_cast<MetaDboBase *>(&dbo_);
      dbo->bindId(statement_, column_);

      if (mapping().versionFieldName) {
	// when saved in the transaction, we will be at version() + 1
	statement_->bind(column_++, dbo_.version()
			 + (dbo_.savedInTransaction() ? 1 : 0));
      }
    }

    exec();

    if (!isInsert_) {
      int modifiedCount = statement_->affectedRowCount();
      if (modifiedCount != 1 && mapping().versionFieldName) {
	MetaDbo<C>& dbo = static_cast< MetaDbo<C>& >(dbo_);
	std::string idString = boost::lexical_cast<std::string>(dbo.id());

	throw StaleObjectException(idString, 
				   dbo_.session()->template tableName<C>(), 
				   dbo_.version());
      }
    }
  }

  /*
   * (3) collections:
   *  - references in select queries (for ManyToOne and ManyToMany)
   *  - inserts in ManyToMany collections
   *  - deletes from ManyToMany collections
   */
  if (needSetsPass_) {
    startSetsPass();
    persist<C>::apply(obj, *this);
  }
}

template<class C>
template<typename V>
void SaveDbAction<C>::actId(V& value, const std::string& name, int size)
{
  field(*this, value, name, size);

  /* Later, we may also want to support id changes ? */
  if (pass_ == Self && isInsert_)
    dbo_.setId(value);
}

template<class C>
template<class D>
void SaveDbAction<C>::actId(ptr<D>& value, const std::string& name, int size,
			   int fkConstraints)
{ 
  actPtr(PtrRef<D>(value, name, fkConstraints));

  /* Later, we may also want to support id changes ? */
  if (pass_ == Self && isInsert_)
    dbo_.setId(value);
}


    /*
     * TransactionDoneAction
     */

template<class C>
void TransactionDoneAction::visit(C& obj)
{
  persist<C>::apply(obj, *this);
}

template<typename V>
void TransactionDoneAction::actId(V& value, const std::string& name, int size)
{ 
  field(*this, value, name, size);
}

template<class C>
void TransactionDoneAction::actId(ptr<C>& value, const std::string& name,
				  int size, int fkConstraints)
{ 
  actPtr(PtrRef<C>(value, name, fkConstraints));
}

template<typename V>
void TransactionDoneAction::act(const FieldRef<V>& field)
{ }

template<class C>
void TransactionDoneAction::actPtr(const PtrRef<C>& field)
{ }

template<class C>
void TransactionDoneAction::actWeakPtr(const WeakPtrRef<C>& field)
{
  if (!success_)
    DboAction::actWeakPtr(field);
}

template<class C>
void TransactionDoneAction::actCollection(const CollectionRef<C>& field)
{
  if (!success_)
    DboAction::actCollection(field);

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

    /*
     * SessionAddAction
     */

template<class C>
void SessionAddAction::visit(C& obj)
{
  persist<C>::apply(obj, *this);
}

template<typename V>
void SessionAddAction::actId(V& value, const std::string& name, int size)
{ 
  field(*this, value, name, size);
}

template<class C>
void SessionAddAction::actId(ptr<C>& value, const std::string& name,
			     int size, int fkConstraints)
{ 
  actPtr(PtrRef<C>(value, name, fkConstraints));
}

template<typename V>
void SessionAddAction::act(const FieldRef<V>& field)
{ }

template<class C>
void SessionAddAction::actPtr(const PtrRef<C>& field)
{ }

template<class C>
void SessionAddAction::actCollection(const CollectionRef<C>& field)
{
  DboAction::actCollection(field);

  // FIXME: cascade add ?
}

template<class C>
void SessionAddAction::actWeakPtr(const WeakPtrRef<C>& field)
{
  DboAction::actWeakPtr(field);
}

    /*
     * SetReciproceAction
     */

template<class C>
void SetReciproceAction::visit(C& obj)
{
  persist<C>::apply(obj, *this);
}

template<typename V>
void SetReciproceAction::actId(V& value, const std::string& name, int size)
{ 
  field(*this, value, name, size);
}

template<class C>
void SetReciproceAction::actId(ptr<C>& value, const std::string& name,
			       int size, int fkConstraints)
{ 
  actPtr(PtrRef<C>(value, name, fkConstraints));
}

template<typename V>
void SetReciproceAction::act(const FieldRef<V>& field)
{ }

template<class C>
void SetReciproceAction::actPtr(const PtrRef<C>& field)
{ 
  if (field.name() == joinName_)
    field.value().resetObj(value_);
}

template<class C>
void SetReciproceAction::actWeakPtr(const WeakPtrRef<C>& field)
{
}

template<class C>
void SetReciproceAction::actCollection(const CollectionRef<C>& field)
{
}

    /*
     * ToAnysAction
     */

template<class C>
void ToAnysAction::visit(const ptr<C>& obj)
{
  if (!session_ && obj.session())
    session_ = obj.session();

  if (dbo_traits<C>::surrogateIdField())
    result_.push_back(obj.id());

  if (dbo_traits<C>::versionField())
    result_.push_back(obj.version());

  if (obj) {
    allEmpty_ = false;
    persist<C>::apply(const_cast<C&>(*obj), *this);
  } else {
    C dummy;
    allEmpty_ = true;
    persist<C>::apply(dummy, *this);
  }
}

template <typename V, class Enable = void>
struct ToAny
{
  static boost::any convert(const V& v) {
    return v;
  }  
};

template <typename Enum>
struct ToAny<Enum, typename boost::enable_if<boost::is_enum<Enum> >::type> 
{
  static boost::any convert(const Enum& v) {
    return static_cast<int>(v);
  }
};

template <typename V>
boost::any convertToAny(const V& v) {
  return ToAny<V>::convert(v);
}

template<typename V>
void ToAnysAction::actId(V& value, const std::string& name, int size)
{ 
  field(*this, value, name, size);
}

template<class C>
void ToAnysAction::actId(ptr<C>& value, const std::string& name,
			 int size, int fkConstraints)
{ 
  actPtr(PtrRef<C>(value, name, fkConstraints));
}

template<typename V>
void ToAnysAction::act(const FieldRef<V>& field)
{ 
  if (allEmpty_)
    result_.push_back(boost::any());
  else
    result_.push_back(convertToAny(field.value()));
}

template<class C>
void ToAnysAction::actPtr(const PtrRef<C>& field)
{
  field.visit(*this, session());
}

template<class C>
void ToAnysAction::actWeakPtr(const WeakPtrRef<C>& field)
{ }

template<class C>
void ToAnysAction::actCollection(const CollectionRef<C>& field)
{ }

    /*
     * FromAnyAction
     */

template<class C>
void FromAnyAction::visit(const ptr<C>& obj)
{
  if (!session_ && obj.session())
    session_ = obj.session();

  if (dbo_traits<C>::surrogateIdField()) {
    if (index_ == 0)
      throw Exception("dbo_result_traits::setValues(): cannot set surrogate "
		      "id.");
    --index_;
  }

  if (dbo_traits<C>::versionField()) {
    if (index_ == 0)
      throw Exception("dbo_result_traits::setValues(): "
		      "cannot set version field.");
    --index_;
  }

  persist<C>::apply(const_cast<C&>(*obj), *this);

  if (index_ == -1)
    obj.modify();
}

template <typename V, class Enable = void>
struct FromAny
{
  static V convert(const boost::any& v) {
    return boost::any_cast<V>(v);
  }  
};

template <typename Enum>
struct FromAny<Enum, typename boost::enable_if<boost::is_enum<Enum> >::type>
{
  static Enum convert(const boost::any& v) {
    return static_cast<Enum>(boost::any_cast<int>(v));
  }
};

template<typename V>
void FromAnyAction::actId(V& value, const std::string& name, int size)
{
  field(*this, value, name, size);
}

template<class C>
void FromAnyAction::actId(ptr<C>& value, const std::string& name, int size,
			  int fkConstraints)
{
  actPtr(PtrRef<C>(value, name, fkConstraints));
}

template<typename V>
void FromAnyAction::act(const FieldRef<V>& field)
{
  if (index_ == 0) {
    field.setValue(FromAny<V>::convert(value_));

    index_ = -1;
  } else if (index_ > 0)
    --index_;
}

template<class C>
void FromAnyAction::actPtr(const PtrRef<C>& field)
{
  field.visit(*this, session());
}

template<class C>
void FromAnyAction::actWeakPtr(const WeakPtrRef<C>& field)
{ }

template<class C>
void FromAnyAction::actCollection(const CollectionRef<C>& field)
{ }

  }
}

#endif // WT_DBO_DBACTION_H_
