// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2018 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_DATE_TZ_HPP
#define WT_DATE_TZ_HPP

#include <Wt/WDllDefs.h>

#ifdef WT_WIN32

#define USE_OS_TZDB 0
#define HAS_REMOTE_API 0

#ifdef wt_EXPORTS
  #define DATE_BUILD_DLL
#else
  #ifdef WT_STATIC
    #define DATE_BUILD_LIB
  #else
    #define DATE_USE_DLL
  #endif
#endif

#else // !WT_WIN32

#define USE_OS_TZDB 1

#endif // WT_WIN32

#include "include/date/tz.h"

#endif // WT_DATE_TZ_HPP
