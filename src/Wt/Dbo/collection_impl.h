// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_COLLECTION_IMPL_H_
#define WT_DBO_COLLECTION_IMPL_H_

#include <Wt/Dbo/collection.h>
#include <Wt/Dbo/Exception.h>
#include <Wt/Dbo/SqlStatement.h>

#include <algorithm>

namespace Wt {
  namespace Dbo {
    namespace Impl {

      template <class C>
      struct Helper {
	static void skipIfRemoved
	(typename collection<C>::iterator::shared_impl& it)
	{ }
      };


      template <class T>
      struct Helper< ptr<T> >
      {
	static void skipIfRemoved
	(typename collection< ptr<T> >::iterator::shared_impl& it)
	{
	  if (std::find(it.collection_.manualModeRemovals().begin(),
			it.collection_.manualModeRemovals().end(),
			it.current_) != it.collection_.manualModeRemovals().end())
	    it.fetchNextRow();
	}
      };
    }

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
    return impl_->current();
  else
    throw Exception("collection< ptr<C> >::iterator::operator* : read beyond end.");
}

template <class C>
C *
collection<C>::iterator::operator-> ()
{
  if (impl_ && !impl_->ended_)
    return &impl_->current();
  else
    return nullptr;
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
    queryEnded_(false),
    posPastQuery_(0),
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
  else if (queryEnded_) {
    posPastQuery_++;
    if (posPastQuery_ == collection_.manualModeInsertions().size())
      ended_ = true;
    else
      current_ = collection_.manualModeInsertions()[posPastQuery_];
    return;
  }

  if (!statement_ || !statement_->nextRow()) {
    queryEnded_ = true;
    if (collection_.manualModeInsertions().size() == 0)
      ended_ = true;
    if (statement_) {
      statement_->done();
      collection_.iterateDone();
    }
  } else {
    int column = 0;
    current_
      = query_result_traits<C>::load(*collection_.session(), *statement_, column);

    Impl::Helper<C>::skipIfRemoved(*this);
  }
}

template <class C>
typename collection<C>::value_type& collection<C>::iterator::shared_impl::current()
{
  return current_;
}

template <class C>
collection<C>::iterator::iterator()
  : impl_(nullptr)
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
  : session_(nullptr),
    type_(RelationCollection)
{
  data_.relation.sql = nullptr;
  data_.relation.dbo = nullptr;
  data_.relation.setInfo = nullptr;
  data_.relation.activity = nullptr;
}

template <class C>
collection<C>::collection(Session *session, SqlStatement *statement,
			  SqlStatement *countStatement)
  : session_(session),
    type_(QueryCollection)
{
  data_.query = new QueryData();
  data_.query->useCount = 1;
  data_.query->statement = statement;
  data_.query->countStatement = countStatement;
  data_.query->size = -1;
}

template <class C>
collection<C>::collection(const collection<C>& other)
  : session_(other.session_),
    type_(other.type_),
    data_(other.data_)
{
  if (type_ == RelationCollection)
    data_.relation.activity = nullptr;
  else
    ++data_.query->useCount;
}

template <class C>
collection<C>::collection(collection<C>&& other) noexcept
  : session_(other.session_),
    type_(other.type_),
    data_(other.data_),
    manualModeInsertions_(std::move(other.manualModeInsertions_)),
    manualModeRemovals_(std::move(other.manualModeRemovals_))
{
  other.session_ = nullptr;
  other.type_ = RelationCollection;
  other.data_.relation.sql = nullptr;
  other.data_.relation.dbo = nullptr;
  other.data_.relation.setInfo = nullptr;
  other.data_.relation.activity = nullptr;
}

template <class C>
void collection<C>::releaseQuery()
{
  if (type_ == QueryCollection) {
    if (--data_.query->useCount == 0) {
      if (data_.query->statement)
	data_.query->statement->done();      
      if (data_.query->countStatement)
	data_.query->countStatement->done();
      delete data_.query;
    }
  }
}

template <class C>
collection<C>& collection<C>::operator=(const collection<C>& other)
{
  if (this != &other) {
    if (type_ == RelationCollection)
      delete data_.relation.activity;
    else
      releaseQuery();

    session_ = other.session_;
    type_ = other.type_;
    data_ = other.data_;

    if (type_ == RelationCollection)
      data_.relation.activity = nullptr;
    else
      ++data_.query->useCount;
  }

  return *this;
}

template <class C>
collection<C>& collection<C>::operator=(collection<C>&& other) noexcept
{
  if (this != &other) {
    if (type_ == RelationCollection)
      delete data_.relation.activity;
    else
      releaseQuery();

    session_ = other.session_;
    type_ = other.type_;
    data_ = other.data_;
    manualModeInsertions_ = std::move(other.manualModeInsertions_);
    manualModeRemovals_ = std::move(other.manualModeRemovals_);

    other.session_ = nullptr;
    other.type_ = RelationCollection;
    other.data_.relation.sql = nullptr;
    other.data_.relation.dbo = nullptr;
    other.data_.relation.setInfo = nullptr;
    other.data_.relation.activity = nullptr;
  }

  return *this;
}

template <class C>
collection<C>::~collection()
{
  if (type_ == RelationCollection)
    delete data_.relation.activity;
  else
    releaseQuery();
}

template <class C>
void collection<C>::iterateDone() const
{
  if (type_ == QueryCollection)
    data_.query->statement = nullptr;
}

template <class C>
SqlStatement *collection<C>::executeStatement() const
{
  SqlStatement *statement = nullptr;

  if (session_ && session_->flushMode() == FlushMode::Auto)
    session_->flush();

  if (type_ == QueryCollection)
    statement = data_.query->statement;
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
bool collection<C>::empty() const
{
  return size() == 0;
}

template <class C>
typename collection<C>::size_type collection<C>::size() const
{
  if (type_ == QueryCollection && data_.query->size != -1)
    return data_.query->size;

  SqlStatement *countStatement = nullptr;

  if (session_ && session_->flushMode() == FlushMode::Auto)
    session_->flush();

  if (type_ == QueryCollection)
    countStatement = data_.query->countStatement;
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
      data_.query->size = result;
      data_.query->countStatement = nullptr;
    }

    if (type_ != QueryCollection) {
      result += manualModeInsertions_.size();
      result -= manualModeRemovals_.size();
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

  if (type_ != RelationCollection || relation.setInfo == nullptr)
    throw Exception("collection<C>::insert() only for a relational "
		    "collection.");

  if (session_->flushMode() == FlushMode::Auto) {
    if (relation.dbo) {
      relation.dbo->setDirty();
      if (relation.dbo->session())
	relation.dbo->session()->add(c);
    }
  } else if (session_->flushMode() == FlushMode::Manual) {
    manualModeInsertions_.push_back(c);
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

  if (type_ != RelationCollection || relation.setInfo == nullptr)
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
    SetReciproceAction setPtr(session_, relation.setInfo->joinName, nullptr);
    setPtr.visit(*c.modify());
  }

  typename std::vector<C>::iterator pos =
    std::find(manualModeInsertions_.begin(), manualModeInsertions_.end(), c);
  if (pos != manualModeInsertions_.end())
    manualModeInsertions_.erase(pos);

  if (session_->flushMode() == FlushMode::Manual) {
    manualModeRemovals_.push_back(c);
  }
}

template <class C>
void collection<C>::clear()
{
  RelationData& relation = data_.relation;

  if (type_ != RelationCollection || relation.setInfo == nullptr)
    throw Exception("collection<C>::clear() only for a relational relation.");

  if (relation.setInfo->type == ManyToMany) {
    if (relation.activity) {
      relation.activity->transactionInserted.clear();
      relation.activity->transactionErased.clear();      
    }
  }

  if (relation.dbo) {
    const std::string *sql = relation.sql;
    std::string deleteSql;

    if (relation.setInfo->type == ManyToMany) {
      std::size_t o = Impl::ifind(*sql, " on ");
      std::size_t j = Impl::ifind(*sql, " join ");
      std::size_t w = Impl::ifind(*sql, " where ");
      deleteSql = "delete from " + sql->substr(j + 5, o - j - 5)
	+ sql->substr(w);
    } else {
      std::size_t f = Impl::ifind(*sql, " from ");
      deleteSql = "delete" + sql->substr(f);
    }

    Call call = session_->execute(deleteSql);
    int column = 0;
    relation.dbo->bindId(call.statement_, column);
    call.run();
  }
  manualModeInsertions_.clear();
  manualModeRemovals_.clear();
}

template <class C>
int collection<C>::count(C c) const
{
  if (!session_)
    throw Exception("collection<C>::count() only for a collection "
		    "that is bound to a session.");

  if (session_->flushMode() == FlushMode::Auto)
    session_->flush();

  if (type_ != RelationCollection)
    throw Exception("collection<C>::count() only for a relational "
		    "relation.");

  if (!c)
    return 0;

  const RelationData& relation = data_.relation;
  Impl::MappingInfo *mapping
    = session_->getMapping(relation.setInfo->tableName); 

  Query<C, DynamicBinding> q = find().where(mapping->idCondition);
  c.obj()->bindId(q.parameters_);
  int result = q.resultList().size();

  result += 
    std::count(manualModeInsertions_.begin(), manualModeInsertions_.end(), c);
  result -=
    std::count(manualModeRemovals_.begin(), manualModeRemovals_.end(), c);
  return result;
}

template <class C>
void collection<C>::resetActivity()
{
  RelationData& relation = data_.relation;
  delete relation.activity;
  relation.activity = nullptr;
}

template <class C>
void collection<C>::setRelationData(MetaDboBase *dbo,
				    const std::string *sql,
				    Impl::SetInfo *setInfo)
{
  session_ = dbo->session();

  data_.relation.sql = sql;
  data_.relation.dbo = dbo;
  data_.relation.setInfo = setInfo;
}

  }
}

#endif // WT_DBO_COLLECTION_IMPL_H_
