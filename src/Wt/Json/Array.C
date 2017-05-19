/*
 * Copyright (C) 2013 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Json/Array"

namespace Wt {
  namespace Json {

typedef std::vector<Value> Impl;

Array::Array()
  : Impl()
{ }

Array::Array(const Array& other)
  : Impl(other)
{ }

Array& Array::operator= (const Array& other)
{
  Impl::operator=(other);
  return *this;
}

#ifdef WT_CXX11
Array::Array(Array&& other)
  : Impl(std::move(other))
{ }

Array::Array(std::initializer_list<Value> list)
  : Impl(list)
{ }

Array& Array::operator= (Array&& other)
{
  Impl::operator=(std::move(other));
  return *this;
}
#endif

Array Array::Empty;

  }
}
