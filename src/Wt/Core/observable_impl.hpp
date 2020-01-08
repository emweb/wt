// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_CORE_OBSERVABLE_IMPL_H_
#define WT_CORE_OBSERVABLE_IMPL_H_

#include <vector>
#include "observing_ptr.hpp"

namespace Wt { namespace Core {

template<typename... Args, typename ClassType>
auto observable::bindSafe(void(ClassType::*f)(Args...)) noexcept
    -> std::function<void(Args...)>
{
  observing_ptr<ClassType> p(dynamic_cast<ClassType *>(this));

  return [p, f](Args... a) {
    if (p.get())
      (p.get()->*f)(a...);
  };
}

template<typename... Args, typename ClassType>
auto observable::bindSafe(void(ClassType::*f)(Args...) const) const noexcept
    -> std::function<void(Args...)>
{
  observing_ptr<const ClassType> p(dynamic_cast<const ClassType *>(this));

  return [p, f](Args... a) {
    if (p.get())
      (p.get()->*f)(a...);
  };
}

template<typename FirstArg, typename... Args, class F>
auto observable::bindSafe(F&& f) const noexcept
    -> std::function<decltype(f(std::declval<FirstArg>(), 
				std::declval<Args>()...))(FirstArg, Args...)>
{
  observing_ptr<const observable> p(this);

  return [p, f](FirstArg a1, Args... a) {
    if (p.get())
      f(a1, a...);
  };
}

/*
template<class F>
auto observable::bindSafe(F&& f) const
  -> decltype(bindSafe(&std::remove_reference<F>::type::operator()))
{

}
*/

template<typename... Args>
std::function<void(Args...)> 
observable::bindSafe(const std::function<void(Args...)>& f) const noexcept
{
  return bindSafeFunction(f);
}

template<typename... Args>
std::function<void(Args...)> 
observable::bindSafeFunction(std::function<void(Args...)> f) const noexcept
{
  observing_ptr<const observable> p(this);

  return [p, f](Args... a) {
    if (p.get())
      f(a...);
  };
}

}}

#endif // WT_CORE_OBSERVABLE_IMPL_H_
