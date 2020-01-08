// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDBOFIREBIRDDLLDEFS_H_
#define WDBOFIREBIRDDLLDEFS_H_

#include <Wt/WConfig.h>

// Source: http://www.nedprod.com/programs/gccvisibility.html

#ifdef WT_WIN32
  #define WTDBOFIREBIRD_IMPORT __declspec(dllimport)
  #define WTDBOFIREBIRD_EXPORT __declspec(dllexport)
  #define WTDBOFIREBIRD_DLLLOCAL
  #define WTDBOFIREBIRD_DLLPUBLIC
#else
  #define WTDBOFIREBIRD_IMPORT __attribute__ ((visibility("default")))
  #define WTDBOFIREBIRD_EXPORT __attribute__ ((visibility("default")))
  #define WTDBOFIREBIRD_DLLLOCAL __attribute__ ((visibility("hidden")))
  #define WTDBOFIREBIRD_DLLPUBLIC __attribute__ ((visibility("default")))
#endif

#ifdef wtdbofirebird_EXPORTS
  #define WTDBOFIREBIRD_API WTDBOFIREBIRD_EXPORT
#else
  #ifdef WTDBOFIREBIRD_STATIC
    #define WTDBOFIREBIRD_API
  #else
    #define WTDBOFIREBIRD_API WTDBOFIREBIRD_IMPORT
  #endif
#endif

#endif // DLLDEFS_H_
