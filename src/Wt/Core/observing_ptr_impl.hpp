// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_CORE_OBSERVING_PTR_IMPL_H_
#define WT_CORE_OBSERVING_PTR_IMPL_H_

#include "observable.hpp"

namespace Wt { namespace Core {

template<typename T>
observing_ptr<T>::observing_ptr(T *t)
{
  reset(t);
}

template <typename T>
template <typename S>
observing_ptr<T>::observing_ptr(const observing_ptr<S>& other)
{
  reset(dynamic_cast<T *>(other.get()));
}

template <typename T>
template <typename S>
observing_ptr<T>& observing_ptr<T>::operator=(const observing_ptr<S>& other)
{
  reset(dynamic_cast<T *>(other.get()));

  return *this;
}

template <typename T>
observing_ptr<T>::observing_ptr(const observing_ptr<T>& other)
{
  reset(other.get());
}

template <typename T>
observing_ptr<T>& observing_ptr<T>::operator=(const observing_ptr<T>& other)
{
  reset(other.get());
  return *this;
}

template <typename T>
bool observing_ptr<T>::observedDeleted() const
{
  return impl_.cleared_;
}

template <typename T>
T *observing_ptr<T>::get() const
{
  return dynamic_cast<T *>(impl_.observed_);
}

template <typename T>
void observing_ptr<T>::reset(T *v) 
{
  if (impl_.observed_) {
    impl_.observed_->removeObserver(&impl_);
    impl_.clear();
  }

  observable *o = dynamic_cast<observable *>
    (const_cast<typename std::remove_const<T>::type *>(v));

  if (o)
    impl_.set(o);
}

template <typename T>
observing_ptr<T>::operator bool() const
{
  return impl_.observed_ != nullptr;
}

template <typename T>
T *observing_ptr<T>::operator->() const
{
  if (impl_.observed_ == nullptr)
    throw std::runtime_error("observing_ptr<T> null pointer dereference");

  return get();
}

template <typename T>
T& observing_ptr<T>::operator*() const
{
  if (impl_.observed_ == nullptr)
    throw std::runtime_error("observing_ptr<T> null pointer dereference");

  return *get();
}

}}

#endif // WT_CORE_OBSERVING_PTR_IMPL_H_
