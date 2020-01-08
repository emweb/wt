/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Json/Array.h"

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

Array Array::Empty;

  }
}
