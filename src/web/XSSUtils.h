// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef XSSUTILS_H_
#define XSSUTILS_H_

#include <string>

namespace Wt {
  namespace XSS {
    bool isBadTag(const std::string& name);
    bool isBadAttribute(const std::string& name);
    bool isBadAttributeValue(const std::string& name, const std::string& value);
  }
}

#endif // XSSUTILS_H_
