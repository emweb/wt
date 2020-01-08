// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef PDF_UTILS_H_
#define PDF_UTILS_H_

#include <string>

#include "Wt/WFont.h"

namespace Wt {
  namespace Pdf {
    std::string toBase14Font(const WFont& font);
  }
}

#endif // PDF_UTILS_H_
