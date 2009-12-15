// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_DBO_PTR_IMPL_H_
#define WT_DBO_DBO_PTR_IMPL_H_

#include <stdexcept>

namespace Wt {
  namespace Dbo {

template <class C>
Dbo<C>::~Dbo()
{
  if (refCount_)
    throw std::logic_error("Dbo: refCount > 0");

  if (session())
    session()->prune(this);

  delete obj_;
}

template <class C>
void Dbo<C>::flush()
{
  if (state_ & NeedsDelete) {
    state_ &= ~NeedsDelete;

    /*
    if (!obj_)
      doLoad();
    */

    session()->implDelete(*this);

    setTransactionState(DeletedInTransaction);
  } else if (state_ & NeedsSave) {
    state_ &= ~NeedsSave;

    session()->implSave(*this);

    setTransactionState(SavedInTransaction);
  }
}

template <class C>
void Dbo<C>::prune()
{
  session()->prune(this);
  setId(-1);
  setVersion(-1);
  setState(New);
}

template <class C>
void Dbo<C>::transactionDone(bool success)
{
  Session *s = session();

  if (success) {
    if (deletedInTransaction()) {
      prune();
      setSession(0);
    } else if (savedInTransaction()) {
      setVersion(version() + 1);
      setState(Persisted);
    } 
  } else {
    if (deletedInTransaction()) {
      state_ |= NeedsDelete;
      session()->needsFlush(this);
    } else if (savedInTransaction()) {
      if (isNew())
	prune();
      else {
	state_ |= NeedsSave;
	session()->needsFlush(this);
      }
    }
  }

  if (obj_)
    s->implTransactionDone(*this, success);

  resetTransactionState();
}

template <class C>
void Dbo<C>::purge()
{
  if (isPersisted() && !isDirty() && !inTransaction()) {
    delete obj_;
    obj_ = 0;
    setVersion(-1);
  }
}

template <class C>
void Dbo<C>::reread()
{
  if (isPersisted()) {
    session()->prune(this);

    delete obj_;
    obj_ = 0;
    setVersion(-1);

    state_ = Persisted;
  }
}

template <class C>
void Dbo<C>::setObj(C *obj)
{
  obj_ = obj;
}

template <class C>
C *Dbo<C>::obj()
{
  if (!obj_ && !isDeleted())
    doLoad();

  return obj_;
}

template <class C>
Dbo<C>::Dbo(C *obj)
  : DboBase(-1, -1, New | NeedsSave, 0),
    obj_(obj)
{ }

template <class C>
Dbo<C>::Dbo(long long id, int version, int state, Session& session, C *obj)
  : DboBase(id, version, state, &session),
    obj_(obj)
{ }

template <class C>
void Dbo<C>::doLoad()
{
  int column = 0;
  obj_ = session()->template implLoad<C>(*this, 0, column);
}

template <class C>
ptr<C>::ptr(C *obj)
  : obj_(0)
{
  if (obj) {
    obj_ = new Dbo<C>(obj);
    takeObj();
  }
}

template <class C>
ptr<C>::ptr(const ptr<C>& other)
  : obj_(other.obj_)
{
  takeObj();
}

template <class C>
ptr<C>::~ptr()
{
  freeObj();
}

template <class C>
void ptr<C>::reset(C *obj)
{
  freeObj();
  obj_ = new Dbo<C>(obj);
  takeObj();
}

template <class C>
ptr<C>& ptr<C>::operator= (const ptr<C>& other)
{
  if (obj_ != other.obj_) {
    freeObj();
    obj_ = other.obj_;
    takeObj();
  }

  return *this;
}

template <class C>
const C *ptr<C>::operator->() const
{
  if (obj_)
    return obj_->obj();
  else
    return 0;
}

template <class C>
const C& ptr<C>::operator*() const
{
  if (obj_)
    return *obj_->obj();
  else
    throw std::runtime_error("ptr: null dereference");
}

template <class C>
C *ptr<C>::modify() const
{
  if (obj_) {
    obj_->setDirty();
    return obj_->obj();
  } else
    throw std::runtime_error("ptr: null dereference");
}

template <class C>
bool ptr<C>::operator== (const ptr<C>& other) const
{
  return obj_ == other.obj_;
}

template <class C>
bool ptr<C>::operator< (const ptr<C>& other) const
{
  return obj_ < other.obj_;
}

template <class C>
ptr<C>::operator bool() const
{
  return obj_ != 0;
}

template <class C>
void ptr<C>::flush() const
{
  if (obj_)
    obj_->flush();
}

template <class C>
void ptr<C>::purge()
{
  if (obj_)
    obj_->purge();
}

template <class C>
void ptr<C>::reread()
{
  if (obj_)
    obj_->reread();
}

template <class C>
void ptr<C>::remove()
{
  if (obj_)
    obj_->remove();
}

template <class C>
long long ptr<C>::id() const
{
  if (obj_)
    return obj_->id();
  else
    return -1;
}

template <class C>
ptr<C>::ptr(Dbo<C> *obj)
  : obj_(obj)
{
  takeObj();
}

template <class C>
void ptr<C>::transactionDone(bool success)
{
  obj_->transactionDone(success);
}

template <class C>
void ptr<C>::takeObj()
{
  if (obj_)
    obj_->incRef();
}

template <class C>
void ptr<C>::freeObj()
{
  if (obj_) {
    obj_->decRef();
    obj_ = 0;
  }
}

template <class C>
std::string sql_result_traits< ptr<C> >
::getColumns(Session& session, std::vector<std::string> *aliases)
{
  return session.getColumns(session.tableName<C>(), aliases);
}

template <class C>
ptr<C> sql_result_traits< ptr<C> >
::loadValues(Session& session, SqlStatement& statement, int& column)
{
  long long id;
  statement.getResult(column++, &id);

  return session.template load<C>(id, &statement, column);
}

  }
}

#endif // WT_DBO_PTR_H_
