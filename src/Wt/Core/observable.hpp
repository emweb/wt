// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_CORE_OBSERVABLE_H_
#define WT_CORE_OBSERVABLE_H_

#include <Wt/WDllDefs.h>

#include <memory>
#include <functional>

namespace Wt { 

/*! \brief Namespace for core utility classes
 */
namespace Core {

namespace Impl {
  struct observer_info;
  struct observing_ptr_base;
}

/*! \class observable Wt/Core/observable Wt/Core/observable
 *  \brief A base class for objects whose life-time can be tracked
 * 
 * This class provides the ability to be observed by a observing_ptr,
 * which is a smart pointer that is aware of the deletion of this
 * object. It is used by the signal/slot system to automatically
 * disconnect a slot when the receiving object has been deleted, and
 * can wrap itself inside a function that guards against deletion of
 * the object using bindSafe().
 *
 * \sa Signal
 */
class WT_API observable
{
public:
  /*! \brief Default constructor.
   */
  observable() noexcept;

  /*! \brief Destructor.
   *
   * Destruction may result in automatic disconnects from signal
   * connections.
   */
  virtual ~observable();
  
  observable(const observable&) = delete;
  observable &operator =(const observable &) = delete;

  observable(observable &&) = delete;
  observable &operator =(observable &&) = delete;

#ifdef DOXYGEN_ONLY
  /*! \brief Protects a method call against object destruction.
   *
   * This returns a function that binds the current object to the
   * method (as with std::bind()), but guarantees that the resulting
   * function can safely be called regardless of whether the object
   * was deleted in the mean-time. If the object was deleted, a call
   * to the resulting function will result in a no-op. This is useful
   * when the method will be called sometime in the future by for
   * example an async method such as WServer::post().
   *
   * The resulting function object has the same signature as the
   * passed method.
   */
  template <typename... Args, typename C>
  auto bindSafe(void(C::*method)(Args...)) noexcept;

  /*! \brief Protects a const method call against object destruction.
   *
   * \sa bindSafe(void(C::*method)(Args...))
   */
  template <typename... Args, typename C>
  auto bindSafe(void(C::*method)(Args...) const) const noexcept;

  /*! \brief Protects a function against object destruction.
   *
   * This is useful only if somehow the function depends on the object
   * to be valid.
   *
   * This guarantees that the result function can safely be called
   * regardless of whether the object was deleted in the mean-time. If
   * the object was deleted, a call to the resulting function will
   * result in a no-op. This is useful when the method will be called
   * sometime in the future by for example an async method such as
   * WServer::post().
   *
   * The resulting function object has the same signature as the
   * passed function.
   *
   * The past function object can be a std::function or a lambda.
   */
  template <typename Function>
  auto bindSafe(const Function& function) noexcept;
#else
  // non-const member function pointers
  template<typename... Args, typename ClassType>
  auto bindSafe(void(ClassType::*f)(Args...)) noexcept
    -> std::function<void(Args...)>;

  // const member function pointers
  template<typename... Args, typename ClassType>
  auto bindSafe(void(ClassType::*f)(Args...) const) const noexcept
    -> std::function<void(Args...)>;

  // qualified functionoids
  template<typename FirstArg, typename... Args, class F>
  auto bindSafe(F&& f) const noexcept
    -> std::function<decltype(f(std::declval<FirstArg>(), 
				std::declval<Args>()...))(FirstArg, Args...)>;

  // unqualified functionoids, should have an unambiguous
  // `T::operator()` and use that.
  template<class F>
  auto bindSafe(F&& f) const noexcept
    -> decltype(bindSafe(&std::remove_reference<F>::type::operator())) {
    decltype(bindSafe(&std::remove_reference<F>::type::operator())) f1 = f;

    return bindSafeFunction(f1);
  }

  // explicit variant for std::function
  template<typename... Args>
  std::function<void(Args...)> bindSafe(const std::function<void(Args...)>& f)
    const noexcept;
#endif

private:
  std::unique_ptr<Impl::observer_info> observerInfo_;

  void addObserver(Impl::observing_ptr_base *observer) noexcept;
  void removeObserver(Impl::observing_ptr_base *observer) noexcept;
  void replaceObserver(Impl::observing_ptr_base *original,
                       Impl::observing_ptr_base *observer) noexcept;

  //explicit variant for std::function
  template<typename... Args>
  std::function<void(Args...)> bindSafeFunction
    (std::function<void(Args...)> f) const noexcept;

  template <typename T> friend class observing_ptr;
  friend struct Impl::observing_ptr_base;
};

}}

#include "observable_impl.hpp"

#endif // WT_CORE_OBSERVABLE_H_
