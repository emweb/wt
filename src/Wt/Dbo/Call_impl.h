// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DBO_CALL_IMPL_H_
#define WT_DBO_CALL_IMPL_H_

namespace Wt {
  namespace Dbo {

template<typename T> Call& Call::bind(const T& value)
{
  sql_value_traits<T>::bind(value, statement_, column_++, -1);

  return *this;
}

  }
}


#endif // WT_DBO_CALL_IMPL_H_
