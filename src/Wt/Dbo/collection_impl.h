// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_COLLECTION_IMPL_H_
#define WT_DBO_COLLECTION_IMPL_H_

#include <Wt/Dbo/collection>
#include <Wt/Dbo/Exception>
#include <Wt/Dbo/SqlStatement>

namespace Wt {
  namespace Dbo {

template <class C>
collection<C>::iterator::iterator(const iterator& other)
  : impl_(other.impl_)
{ 
  if (impl_)
    ++impl_->useCount_;
}

template <class C>
collection<C>::iterator::~iterator()
{
  releaseImpl();
}

template <class C>
void collection<C>::iterator::releaseImpl()
{
  if (impl_) {
    --impl_->useCount_;

    if (impl_->useCount_ == 0)
      delete impl_;
  }
}

template <class C>
void collection<C>::iterator::takeImpl()
{
  if (impl_)
    ++impl_->useCount_;
}

template <class C>
typename collection<C>::iterator&
collection<C>::iterator::operator= (const iterator& other)
{
  if (impl_ != other.impl_) {
    releaseImpl();
    impl_ = other.impl_;
    takeImpl();
  }

  return *this;
}

template <class C>
C&
collection<C>::iterator::operator* ()
{
  if (impl_ && !impl_->ended_)
    return impl_->current_;
  else
    throw Exception("set< ptr<C> >::operator* : read beyond end.");
}

template <class C>
C *
collection<C>::iterator::operator-> ()
{
  if (impl_ && !impl_->ended_)
    return &impl_->current_;
  else
    return 0;
}


template <class C>
bool collection<C>::iterator::operator== (const iterator& other) const
{
  return impl_ == other.impl_
    || (!impl_ && other.impl_->ended_)
    || (impl_->ended_ && !other.impl_);
}

template <class C>
bool collection<C>::iterator::operator!= (const iterator& other) const
{
  return !(*this == other);
}

template <class C>
typename collection<C>::iterator& collection<C>::iterator::operator++ ()
{
  if (impl_)
    impl_->fetchNextRow();

  return *this;
}

template <class C>
typename collection<C>::iterator collection<C>::iterator::operator++ (int)
{
  if (impl_)
    impl_->fetchNextRow();

  return *this;
}

template <class C>
collection<C>::iterator::
shared_impl::shared_impl(const collection<C>& collection,
			 SqlStatement *statement)
  : collection_(collection),
    statement_(statement),
    useCount_(0),
    ended_(false)
{
  fetchNextRow();
}

template <class C>
collection<C>::iterator::shared_impl::~shared_impl()
{
  if (!ended_ && statement_) {
    statement_->done();
    collection_.iterateDone();
  }
}

template <class C>
void collection<C>::iterator::shared_impl::fetchNextRow()
{
  if (ended_)
    throw Exception("set< ptr<C> >::operator++ : beyond end.");

  if (!statement_ || !statement_->nextRow()) {
    ended_ = true;
    if (statement_) {
      statement_->done();
      collection_.iterateDone();
    }
  } else {
    int column = 0;
    current_
      = query_result_traits<C>::load(*collection_.session(), *statement_, column);
  }
}

template <class C>
collection<C>::iterator::iterator()
  : impl_(0)
{ }

template <class C>
collection<C>::iterator::iterator(const collection<C>& collection,
				  SqlStatement *statement)
{
  impl_ = new shared_impl(collection, statement);
  takeImpl();
}

template <class C>
collection<C>::const_iterator::const_iterator(const const_iterator& other)
  : impl_(other.impl_)
{ }

template <class C>
collection<C>::const_iterator::const_iterator(const typename collection<C>::iterator& other)
  : impl_(other)
{ }

template <class C>
typename collection<C>::const_iterator&
collection<C>::const_iterator::operator= (const const_iterator& other)
{
  impl_ = other.impl_;

  return *this;
}

template <class C>
C
collection<C>::const_iterator::operator* ()
{
  return impl_.operator*();
}

template <class C>
const C *
collection<C>::const_iterator::operator-> ()
{
  return impl_.operator->();
}

template <class C>
bool collection<C>::const_iterator::operator== (const const_iterator& other)
  const
{
  return impl_ == other.impl_;
}

template <class C>
bool collection<C>::const_iterator::operator!= (const const_iterator& other)
  const
{
  return impl_ != other.impl_;
}

template <class C>
typename collection<C>::const_iterator&
collection<C>::const_iterator::operator++ ()
{
  ++impl_;

  return *this;
}

template <class C>
typename collection<C>::const_iterator
collection<C>::const_iterator::operator++ (int)
{
  impl_++;

  return *this;
}

template <class C>
collection<C>::const_iterator::const_iterator()
  : impl_()
{ }

template <class C>
collection<C>::const_iterator::const_iterator(const collection<C>& collection,
					      SqlStatement *statement)
  : impl_(collection, statement)
{ }

template <class C>
collection<C>::collection()
  : session_(0),
    type_(RelationCollection)
{
  data_.relation.sql = 0;
  data_.relation.dbo = 0;
  data_.relation.setInfo = 0;
  data_.relation.activity = 0;
}

template <class C>
collection<C>::collection(Session *session, SqlStatement *statement,
			  SqlStatement *countStatement)
  : session_(session),
    type_(QueryCollection)
{
  data_.query.statement = statement;
  data_.query.countStatement = countStatement;
  data_.query.size = -1;
}

template <class C>
collection<C>::collection(const collection<C>& other)
  : session_(other.session_),
    type_(other.type_),
    data_(other.data_)
{
  if (type_ == RelationCollection)
    data_.relation.activity = 0;
}

template <class C>
collection<C>::~collection()
{
  if (type_ == RelationCollection)
    delete data_.relation.activity;
  else {
    if (data_.query.statement)
      data_.query.statement->done();      
    if (data_.query.countStatement)
      data_.query.countStatement->done();
  }
}

template <class C>
void collection<C>::iterateDone() const
{
  if (type_ == QueryCollection)
    data_.query.statement = 0;
}

template <class C>
SqlStatement *collection<C>::executeStatement() const
{
  SqlStatement *statement = 0;

  if (session_)
    session_->flush();

  if (type_ == QueryCollection)
    statement = data_.query.statement;
  else {
    if (data_.relation.sql) {
      statement = session_->getOrPrepareStatement(*data_.relation.sql);
      int column = 0;
      data_.relation.dbo->bindId(statement, column);
    }
  }

  if (statement)
    statement->execute();

  return statement;
}

template <class C>
typename collection<C>::iterator collection<C>::begin()
{
  return iterator(*this, executeStatement());
}

template <class C>
typename collection<C>::iterator collection<C>::end()
{
  return iterator();
}

template <class C>
typename collection<C>::const_iterator collection<C>::begin() const
{
  return const_iterator(*this, executeStatement());
}

template <class C>
typename collection<C>::const_iterator collection<C>::end() const
{
  return const_iterator();
}

template <class C>
C collection<C>::front() const
{
  return *(const_iterator(*this, executeStatement()));
}

template <class C>
typename collection<C>::size_type collection<C>::size() const
{
  if (type_ == QueryCollection && data_.query.size != -1)
    return data_.query.size;

  SqlStatement *countStatement = 0;

  if (session_)
    session_->flush();

  if (type_ == QueryCollection)
    countStatement = data_.query.countStatement;
  else {
    if (data_.relation.sql) {
      const std::string *sql = data_.relation.sql;
      std::size_t f = Impl::ifind(*sql, " from ");
      std::string countSql = "select count(1)" + sql->substr(f);

      countStatement = session_->getOrPrepareStatement(countSql);
      int column = 0;
      data_.relation.dbo->bindId(countStatement, column);
    }
  }

  if (countStatement) {
    ScopedStatementUse use(countStatement);

    countStatement->execute();

    if (!countStatement->nextRow())
      throw Exception("collection<C>::size(): no result?");

    int result;
    if (!countStatement->getResult(0, &result))
      throw Exception("collection<C>::size(): null?");
    
    if (countStatement->nextRow())
      throw Exception("collection<C>::size(): multiple results?");

    if (type_ == QueryCollection) {
      data_.query.size = result;
      data_.query.countStatement = 0;
    }

    return result;
  } else
    return 0;
}

template <class C>
Query<C, DynamicBinding> collection<C>::find() const
{
  if (type_ != RelationCollection)
    throw Exception("collection<C>::find() "
		    "only for a many-side relation collection.");

  if (session_ && data_.relation.sql) {
    const std::string *sql = data_.relation.sql;
    std::size_t f = Impl::ifind(*sql, " from ");
    std::size_t w = Impl::ifind(*sql, " where ");
    std::string tableName = sql->substr(f + 6, w - f - 6);

    Query<C, DynamicBinding> result = Query<C, DynamicBinding>
      (*session_, tableName, "").where(sql->substr(w + 7));

    if (!data_.relation.dbo->isPersisted())
      data_.relation.dbo->flush();
    data_.relation.dbo->bindId(result.parameters_);

    return result;
  } else
    return Query<C, DynamicBinding>();
}

template <class C>
void collection<C>::insert(C c)
{
  RelationData& relation = data_.relation;

  if (type_ != RelationCollection || relation.setInfo == 0)
    throw Exception("collection<C>::insert() only for a relational "
		    "collection.");

  if (relation.dbo) {
    relation.dbo->setDirty();
    if (relation.dbo->session())
      relation.dbo->session()->add(c);
  }

  if (relation.setInfo->type == ManyToMany) {
    if (!relation.activity)
      relation.activity = new Activity();

    bool wasJustErased = relation.activity->erased.erase(c) > 0;
    relation.activity->transactionErased.erase(c);

    if (!wasJustErased && !relation.activity->transactionInserted.count(c))
      relation.activity->inserted.insert(c);
  } else {
    SetReciproceAction setPtr(session_, relation.setInfo->joinName,
			      relation.dbo);
    setPtr.visit(*c.modify());
  }
}

template <class C>
void collection<C>::erase(C c)
{
  RelationData& relation = data_.relation;

  if (type_ != RelationCollection || relation.setInfo == 0)
    throw Exception("collection<C>::erase() only for a relational relation.");

  if (relation.dbo)
    relation.dbo->setDirty();

  if (relation.setInfo->type == ManyToMany) {
    if (!relation.activity)
      relation.activity = new Activity();

    bool wasJustInserted = relation.activity->inserted.erase(c) > 0;
    relation.activity->transactionInserted.erase(c);

    if (!wasJustInserted && !relation.activity->transactionErased.count(c))
      relation.activity->erased.insert(c);
  } else {
    SetReciproceAction setPtr(session_, relation.setInfo->joinName, 0);
    setPtr.visit(*c.modify());
  }
}

template <class C>
void collection<C>::clear()
{
  RelationData& relation = data_.relation;

  if (type_ != RelationCollection || relation.setInfo == 0)
    throw Exception("collection<C>::clear() only for a relational relation.");

  if (relation.setInfo->type == ManyToMany) {
    if (relation.activity) {
      relation.activity->transactionInserted.clear();
      relation.activity->transactionErased.clear();      
    }
  }

  if (relation.dbo) {
    const std::string *sql = relation.sql;
    std::size_t f = Impl::ifind(*sql, " from ");
    std::string deleteSql = "delete" + sql->substr(f);

    Call call = session_->execute(deleteSql);
    int column = 0;
    relation.dbo->bindId(call.statement_, column);
    call.run();
  }
}

template <class C>
int collection<C>::count(C c) const
{
  if (!session_)
    throw Exception("collection<C>::count() only for a collection "
		    "that is bound to a session.");

  session_->flush();

  if (type_ != RelationCollection)
    throw Exception("collection<C>::count() only for a relational "
			     "relation.");

  const RelationData& relation = data_.relation;
  Session::MappingInfo *mapping
    = session_->getMapping(relation.setInfo->tableName); 

  Query<C, DynamicBinding> q = find().where(mapping->idCondition);
  c.obj()->bindId(q.parameters_);
  return q.resultList().size();
}

template <class C>
void collection<C>::resetActivity()
{
  RelationData& relation = data_.relation;
  delete relation.activity;
  relation.activity = 0;
}

template <class C>
void collection<C>::setRelationData(MetaDboBase *dbo,
				    const std::string *sql,
				    Session::SetInfo *setInfo)
{
  session_ = dbo->session();

  data_.relation.sql = sql;
  data_.relation.dbo = dbo;
  data_.relation.setInfo = setInfo;
}

  }
}

#endif // WT_DBO_COLLECTION_IMPL_H_
