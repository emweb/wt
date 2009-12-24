/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/WtSqlTraits"

#ifndef DOXYGEN_ONLY

namespace Wt {
  namespace Dbo {

const char *sql_value_traits<WDate>::format = "yyyy-MM-dd";
const char *sql_value_traits<WDateTime>::format = "yyyy-MM-dd HH:mm:ss.zzz";

  }
}

#endif // DOXYGEN_ONLY
