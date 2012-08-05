// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WT_AUTH_UTILS_H_
#define WT_AUTH_UTILS_H_

#include <string>
#include "Wt/Http/Client"
#include "Wt/Http/Response"
#include "Wt/Http/Request"
#include <Wt/WDllDefs.h>

namespace Wt {
  namespace Auth {
    namespace Utils {
      WT_API extern void parseFormUrlEncoded(const Http::Message& response,
					     Http::ParameterMap &params);
      WT_API extern const std::string *getParamValue(Http::ParameterMap &params,
						     const std::string &name);

      WT_API extern std::string createSalt(unsigned int length);

      // decodeAscii(encodeAscii(a)) == a only if
      // its length multiple of 3 bytes
      WT_API extern std::string encodeAscii(const std::string& a);
      WT_API extern std::string decodeAscii(const std::string& a);
    }
  }
}

#endif // WT_AUTH_UTILS_H_
