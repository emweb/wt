// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2006 Wim Dumon, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDBOSQLITE3DLLDEFS_H_
#define WDBOSQLITE3DLLDEFS_H_

#include <Wt/WConfig.h>

// Source: http://www.nedprod.com/programs/gccvisibility.html

#ifdef WIN32
  #define WTDBOSQLITE3_IMPORT __declspec(dllimport)
  #define WTDBOSQLITE3_EXPORT __declspec(dllexport)
  #define WTDBOSQLITE3_DLLLOCAL
  #define WTDBOSQLITE3_DLLPUBLIC
#else
  #ifdef GCC_HASCLASSVISIBILITY
    #define WTDBOSQLITE3_IMPORT __attribute__ ((visibility("default")))
    #define WTDBOSQLITE3_EXPORT __attribute__ ((visibility("default")))
    #define WTDBOSQLITE3_DLLLOCAL __attribute__ ((visibility("hidden")))
    #define WTDBOSQLITE3_DLLPUBLIC __attribute__ ((visibility("default")))
  #else
    #define WTDBOSQLITE3_IMPORT
    #define WTDBOSQLITE3_EXPORT
    #define WTDBOSQLITE3_DLLLOCAL
    #define WTDBOSQLITE3_DLLPUBLIC
  #endif
#endif

// Define wt_EXPORTS for DLL builds
#ifdef WIN32
  #ifdef wtdbosqlite3_EXPORTS
    #define WTDBOSQLITE3_API WTDBOSQLITE3_EXPORT
  #else
    #ifdef WTDBOSQLITE3_STATIC
      #define WTDBOSQLITE3_API
    #else
      #define WTDBOSQLITE3_API WTDBOSQLITE3_IMPORT
    #endif
  #endif
#else
  #define WTDBOSQLITE3_API
#endif

#endif // DLLDEFS_H_
