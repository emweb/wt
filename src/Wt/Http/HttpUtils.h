// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WT_HTTP_UTILS_H_
#define WT_HTTP_UTILS_H_

#include <string>
#include "Wt/Http/Client"
#include "Wt/Http/Response"
#include "Wt/Http/Request"
#include <Wt/WDllDefs.h>

namespace Wt {
  namespace Http {
    namespace Utils {
      WT_API extern void parseFormUrlEncoded(const Http::Message& response,
					     Http::ParameterMap &params);
      WT_API extern const std::string *getParamValue(Http::ParameterMap &params,
						     const std::string &name);
    }
  }
}

#endif // WT_HTTP_UTILS_H_
