// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef DATE_UTILS_H_
#define DATE_UTILS_H_

#include <Wt/WDllDefs.h>
#include <Wt/WStringStream.h>

#include <chrono>
#include <string>

namespace Wt {
  namespace DateUtils {

    extern WT_API void httpDateBuf(const std::chrono::system_clock::time_point& tp, Wt::WStringStream& buf);
    extern WT_API std::string httpDate(const std::chrono::system_clock::time_point &tp);

  }
}

#endif // DATE_UTILS_H_
