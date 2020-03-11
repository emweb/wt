/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "EscapeOStream.h"
#include <cstring>

namespace Wt {
  namespace Dbo {
    namespace Utils {

std::string& replace(std::string& s, char c, const std::string& r)
{
  std::string::size_type p = 0;

  while ((p = s.find(c, p)) != std::string::npos) {
    s.replace(p, 1, r);
    p += r.length();
  }

  return s;
}

    }

  }
}

#define WT_DBO_ESCAPEOSTREAM
#include "../../web/EscapeOStream.C"
#undef WT_DBO_ESCAPEOSTREAM
