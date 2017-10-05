// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2006 Wim Dumon, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDBODLLDEFS_H_
#define WDBODLLDEFS_H_

#include <Wt/WConfig.h>

// Source: http://www.nedprod.com/programs/gccvisibility.html

#ifdef WT_WIN32
  #define WTDBO_IMPORT __declspec(dllimport)
  #define WTDBO_EXPORT __declspec(dllexport)
  #define WTDBO_DLLLOCAL
  #define WTDBO_DLLPUBLIC
#else
  #if __GNUC__ >= 4
    #define WTDBO_IMPORT __attribute__ ((visibility("default")))
    #define WTDBO_EXPORT __attribute__ ((visibility("default")))
    #define WTDBO_DLLLOCAL __attribute__ ((visibility("hidden")))
    #define WTDBO_DLLPUBLIC __attribute__ ((visibility("default")))
  #else
    #define WTDBO_IMPORT
    #define WTDBO_EXPORT
    #define WTDBO_DLLLOCAL
    #define WTDBO_DLLPUBLIC
  #endif
#endif

// Define wt_EXPORTS for DLL builds
#ifdef wtdbo_EXPORTS
  #define WTDBO_API WTDBO_EXPORT
#else
  #ifdef WTDBO_STATIC
    #define WTDBO_API
  #else
    #define WTDBO_API WTDBO_IMPORT
  #endif
#endif

#endif // DLLDEFS_H_
