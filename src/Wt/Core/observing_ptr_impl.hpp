// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_CORE_OBSERVING_PTR_IMPL_H_
#define WT_CORE_OBSERVING_PTR_IMPL_H_

#include "observable.hpp"

#include <stdexcept>

namespace Wt { namespace Core {

template<typename T>
observing_ptr<T>::observing_ptr(T *t) noexcept
{
  reset(t);
}

template <typename T>
template <typename S, typename>
observing_ptr<T>::observing_ptr(const observing_ptr<S>& other) noexcept
{
  reset(other.get());
}

template <typename T>
template <typename S, typename>
observing_ptr<T>::observing_ptr(observing_ptr<S>&& other) noexcept
{
  doMove(other);
}

template <typename T>
template <typename S, typename>
observing_ptr<T>& observing_ptr<T>::operator=(const observing_ptr<S>& other) noexcept
{
  reset(other.get());

  return *this;
}

template <typename T>
template <typename S, typename>
observing_ptr<T>& observing_ptr<T>::operator=(observing_ptr<S>&& other) noexcept
{
  doMove(other);
  return *this;
}

template <typename T>
observing_ptr<T>::observing_ptr(const observing_ptr<T>& other) noexcept
{
  reset(other.get());
}

template <typename T>
observing_ptr<T>::observing_ptr(observing_ptr<T>&& other) noexcept
{
  doMove(other);
}

template <typename T>
observing_ptr<T>& observing_ptr<T>::operator=(const observing_ptr<T>& other) noexcept
{
  if (this == &other)
    return *this;

  reset(other.get());
  return *this;
}

template <typename T>
observing_ptr<T>& observing_ptr<T>::operator=(observing_ptr<T>&& other) noexcept
{
  doMove(other);
  return *this;
}

template <typename T>
template <typename S, typename>
void observing_ptr<T>::doMove(observing_ptr<S>& other) noexcept
{
  if (&impl_ == &other.impl_)
    return;

  if (impl_.observed_) {
    if (impl_.observed_ == other.impl_.observed_)
      return;
    impl_.observed_->removeObserver(&impl_);
  }

  impl_ = other.impl_;
  other.impl_ = Impl::observing_ptr_base{};
  if (impl_.observed_)
    impl_.observed_->replaceObserver(&other.impl_, &impl_);
}

template <typename T>
bool observing_ptr<T>::observedDeleted() const noexcept
{
  return impl_.cleared_;
}

template <typename T>
T *observing_ptr<T>::get() const noexcept
{
  // this cast shouldn't fail
  return static_cast<T *>(impl_.observed_);
}

template <typename T>
void observing_ptr<T>::reset(T *v) noexcept
{
  if (impl_.observed_) {
    if (impl_.observed_ == v)
      return;

    impl_.observed_->removeObserver(&impl_);
    impl_ = Impl::observing_ptr_base{};
  }

  observable *o = const_cast<typename std::remove_const<T>::type *>(v);

  if (o)
    impl_.set(o);
}

template <typename T>
observing_ptr<T>::operator bool() const noexcept
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
