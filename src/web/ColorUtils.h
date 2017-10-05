// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef COLOR_UTILS_H_
#define COLOR_UTILS_H_

#include <string>
#include <vector>

#include <Wt/WString.h>
#include <Wt/WColor.h>

namespace Wt {
  namespace Color {
#ifdef WT_TARGET_JAVA	
    class ColorUtils {
    };
#endif //WT_TARGET_JAVA

    extern WColor parseCssColor(const std::string &name);
  }
}

#endif // COLOR_UTILS_H_
