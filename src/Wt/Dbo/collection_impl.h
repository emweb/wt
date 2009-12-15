// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_COLLECTION_IMPL_H_
#define WT_DBO_COLLECTION_IMPL_H_

#include <stdexcept>

#include <Wt/Dbo/collection>
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
  releaseImpl();
  impl_ = other.impl_;
  takeImpl();

  return *this;
}

template <class C>
C&
collection<C>::iterator::operator* ()
{
  if (impl_ && !impl_->ended_)
    return impl_->current_;
  else
    throw std::runtime_error("set< ptr<C> >::operator* : read beyond end.");
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
collection<C>::iterator::shared_impl::shared_impl(const collection<C>& self)
  : self_(self),
    useCount_(0),
    ended_(false)
{ 
  fetchNextRow();
}

template <class C>
collection<C>::iterator::shared_impl::~shared_impl()
{
  if (!ended_)
    self_.statement_->reset();
}

template <class C>
void collection<C>::iterator::shared_impl::fetchNextRow()
{
  if (ended_)
    throw std::runtime_error("set< ptr<C> >::operator++ : beyond end.");

  if (!self_.statement_ || !self_.statement_->nextRow())
    ended_ = true;
  else
    current_ = self_.loadNext();
}

template <class C>
collection<C>::iterator::iterator()
  : impl_(0)
{ }

template <class C>
collection<C>::iterator::iterator(const collection<C>& self)
{
  impl_ = new shared_impl(self);
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
const C&
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
collection<C>::const_iterator::const_iterator(const collection<C>& self)
  : impl_(self)
{ }

template <class C>
collection<C>::collection()
  : statement_(0),
    arg_(-1),
    session_(0),
    activity_(0)
{ }

template <class C>
collection<C>::~collection()
{ 
  delete activity_;
}

template <class C>
typename collection<C>::iterator collection<C>::begin()
{
  if (statement_) {
    if (arg_ != -1) {
      statement_->reset();
      statement_->bind(0, arg_);
    }

    statement_->execute();
  }

  return iterator(*this);
}

template <class C>
typename collection<C>::iterator collection<C>::end()
{
  return iterator();
}

template <class C>
typename collection<C>::const_iterator collection<C>::begin() const
{
  if (statement_) {
    session_->flush();
    if (arg_ != -1) {
      statement_->reset();
      statement_->bind(0, arg_);
    }
    statement_->execute();
  }

  return const_iterator(*this);
}

template <class C>
typename collection<C>::const_iterator collection<C>::end() const
{
  return const_iterator();
}

template <class C>
typename collection<C>::size_type collection<C>::size() const
{
  if (session_)
    session_->flush();

  if (!statement_)
    return 0;

  if (!countStatement_) {
    if (arg_ != -1) {
      std::string sql = statement_->sql();
      std::size_t f = sql.find(" from ");
      sql = "select count(*)" + sql.substr(f);
      countStatement_ = session_->getOrPrepareStatement(sql);
    } else
      throw std::logic_error("collection<C>::size(): no statement?");
  }

  if (arg_ != -1) {
    countStatement_->reset();
    countStatement_->bind(0, arg_);
  }

  countStatement_->execute();

  if (!countStatement_->nextRow())
    throw std::runtime_error("collection<C>::size(): no result?");

  int result;
  if (!countStatement_->getResult(0, &result))
    throw std::runtime_error("collection<C>::size(): null?");
    
  if (countStatement_->nextRow())
    throw std::runtime_error("collection<C>::size(): multiple results?");

  countStatement_->reset();

  return result;
}

template <class C>
void collection<C>::insert(C c)
{
  if (!activity_)
    activity_ = new Activity();

  bool wasJustErased = activity_->erased.erase(c) > 0;
  activity_->transactionErased.erase(c);

  if (!wasJustErased && !activity_->transactionInserted.count(c))
    activity_->inserted.insert(c);
}

template <class C>
void collection<C>::erase(C c)
{
  if (!activity_)
    activity_ = new Activity();

  bool wasJustInserted = activity_->inserted.erase(c) > 0;
  activity_->transactionInserted.erase(c);

  if (!wasJustInserted && !activity_->transactionErased.count(c))
    activity_->erased.insert(c);
}

template <class C>
void collection<C>::resetActivity()
{
  delete activity_;
  activity_ = 0;
}

template <class C>
void collection<C>::setStatement(SqlStatement *statement,
				 SqlStatement *countStatement,
				 Session *session)
{
  statement_ = statement;
  countStatement_ = countStatement;
  session_ = session;
}

template <class C>
void collection<C>::clearStatement()
{
  statement_ = 0;
  countStatement_ = 0;
  session_ = 0;
}

template <class C>
void collection<C>::setArg(long long arg)
{
  arg_ = arg;
}

template <class C> C collection<C>::loadNext() const
{
  int column = 0;
  return sql_result_traits<C>::loadValues(*session_, *statement_, column);
}

  }
}

#endif // WT_DBO_COLLECTION_IMPL_H_
