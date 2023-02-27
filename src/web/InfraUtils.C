/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "InfraUtils.h"

#include <boost/algorithm/string/trim.hpp>

#include <algorithm>

namespace Wt {
  namespace WHATWG {
    namespace Infra {

      void stripNewlines(std::string &s)
      {
        s.erase(std::remove_if(begin(s), end(s), [](char c) {
          return c == '\r' || c == '\n';
        }), s.end());
      }

      void trim(std::string &s)
      {
        boost::algorithm::trim_if(s, [](char c) {
          return c == '\t' || c == '\n' || c == '\f' || c == '\r' || c == ' ';
        });
      }

    }
  }
}
