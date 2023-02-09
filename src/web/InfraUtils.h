// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WHATWG_INFRA_UTILS_H_
#define WT_WHATWG_INFRA_UTILS_H_

#include "Wt/WDllDefs.h"

#include <string>

namespace Wt {

  namespace WHATWG {

    // Infra standards-compliant utility functions
    namespace Infra {

      // https://infra.spec.whatwg.org/#strip-newlines
      extern void WT_API stripNewlines(std::string &s);

      // https://infra.spec.whatwg.org/#strip-leading-and-trailing-ascii-whitespace
      extern void WT_API trim(std::string &s);

    }
  }
}

#endif // WT_WHATWG_INFRA_UTILS_H_
