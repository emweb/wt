// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_DBO_WEAK_PTR_IMPL_H_
#define WT_DBO_DBO_WEAK_PTR_IMPL_H_

namespace Wt {
  namespace Dbo {

template <class C>
weak_ptr<C>::accessor::accessor(const ptr<C>& p)
  : p_(p)
{ }

template <class C>
const C *weak_ptr<C>::accessor::operator->() const
{
  return p_.operator->();
}
 
template <class C>
const C& weak_ptr<C>::accessor::operator*() const
{
  return *p_;
}

template <class C>
weak_ptr<C>::accessor::operator const C*() const
{
  return p_.operator->();
}

template <class C>
weak_ptr<C>::mutator::mutator(const ptr<C>& p)
  : p_(p)
{ }

template <class C>
weak_ptr<C>::mutator::~mutator()
{
  p_.modify();
}

template <class C>
C *weak_ptr<C>::mutator::operator->() const
{
  return p_.modify();
}
 
template <class C>
C& weak_ptr<C>::mutator::operator*() const
{
  return *p_.modify();
}

template <class C>
weak_ptr<C>::mutator::operator C*() const
{
  return p_.modify();
}

template <class C>
weak_ptr<C>::weak_ptr()
{ }

template <class C>
weak_ptr<C>::weak_ptr(const weak_ptr<C>& other)
  : collection_(other.collection_)
{ }

template <class C>
template <class D, typename>
weak_ptr<C>::weak_ptr(const weak_ptr<D>& other)
  : collection_(other.collection_)
{ }

template <class C>
weak_ptr<C>::weak_ptr(weak_ptr<C>&& other) noexcept
  : collection_(std::move(other.collection_))
{ }

template <class C>
template <class D, typename>
weak_ptr<C>::weak_ptr(weak_ptr<D>&& other) noexcept
  : collection_(std::move(other.collecton_))
{ }

template <class C>
void weak_ptr<C>::reset(C *obj)
{
  *this = ptr<MutC>(const_cast<MutC*>(obj));
}

template <class C>
weak_ptr<C>& weak_ptr<C>::operator= (const weak_ptr<C>& other)
{
  if (this == &other)
    return *this;

  return *this = other.query();
}

template <class C>
template <class D, typename>
weak_ptr<C>& weak_ptr<C>::operator= (const weak_ptr<D>& other)
{
  return *this = other.query();
}

template <class C>
weak_ptr<C>& weak_ptr<C>::operator= (weak_ptr<C>&& other) noexcept
{
  if (this == &other)
    return *this;

  collection_ = std::move(other.collection_);

  return *this;
}

template <class C>
template <class D, typename>
weak_ptr<C>& weak_ptr<C>::operator= (weak_ptr<D>&& other) noexcept
{
  collection_ = std::move(other.collection_);

  return *this;
}

template <class C>
weak_ptr<C>& weak_ptr<C>::operator= (const ptr<C>& other)
{
  collection_.clear();

  if (other)
    collection_.insert(other);

  return *this;
}

template <class C>
template <class D, typename>
weak_ptr<C>& weak_ptr<C>::operator= (const ptr<D>& other)
{
  collection_.clear();

  if (other)
    collection_.insert(other);

  return *this;
}

template <class C>
typename weak_ptr<C>::accessor weak_ptr<C>::operator->() const
{
  ptr<C> current = query();

  if (!current)
    throw Exception("Wt::Dbo::weak_ptr: null dereference");

  return accessor(current);
}

template <class C>
typename weak_ptr<C>::mutator weak_ptr<C>::modify() const
{
  ptr<C> current = query();

  if (!current)
    throw Exception("Wt::Dbo::weak_ptr: null dereference");

  return mutator(current);
}

template <class C>
bool weak_ptr<C>::operator== (const weak_ptr<MutC>& other) const
{
  return query() == other.query();
}

template <class C>
bool weak_ptr<C>::operator== (const weak_ptr<const C>& other) const
{
  return query() == other.query();
}

template <class C>
bool weak_ptr<C>::operator== (const ptr<MutC>& other) const
{
  return query() == other;
}

template <class C>
bool weak_ptr<C>::operator== (const ptr<const C>& other) const
{
  return query() == other;
}

template <class C>
bool weak_ptr<C>::operator!= (const weak_ptr<MutC>& other) const
{
  return !(*this == other);
}

template <class C>
bool weak_ptr<C>::operator!= (const weak_ptr<const C>& other) const
{
  return !(*this == other);
}

template <class C>
bool weak_ptr<C>::operator!= (const ptr<MutC>& other) const
{
  return !(*this == other);
}

template <class C>
bool weak_ptr<C>::operator!= (const ptr<const C>& other) const
{
  return !(*this == other);
}

template <class C>
weak_ptr<C>::operator bool() const
{
  return bool(query());
}

template <class C>
weak_ptr<C>::operator ptr<C>() const
{
  return query();
}

template <class C>
template <class D, typename>
weak_ptr<C>::operator ptr<D>() const
{
  return query();
}

template <class C>
ptr<C> weak_ptr<C>::query() const
{
  typename collection< ptr<MutC> >::const_iterator i = collection_.begin();
  if (i == collection_.end())
    return ptr<C>();
  else
    return *i;
}

template <class C>
ptr<C> weak_ptr<C>::lock() const
{
  typename collection< ptr<MutC> >::const_iterator i = collection_.begin();
  if (i == collection_.end())
    return ptr<C>();
  else
    return *i;
}

template <class C>
typename dbo_traits<C>::IdType weak_ptr<C>::id() const
{
  return query().id();
}

template <class C>
void weak_ptr<C>::setRelationData(MetaDboBase *dbo, const std::string *sql,
				  Impl::SetInfo *info)
{
  collection_.setRelationData(dbo, sql, info);
}

  }
}

#endif // WT_DBO_WEAK_WEAK_PTR_IMPL_H_
