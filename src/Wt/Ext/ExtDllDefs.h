// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2007 Wim Dumon, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef EXTDLLDEFS_H_
#define EXTDLLDEFS_H_

#include "Wt/WConfig.h"

// Source: http://www.nedprod.com/programs/gccvisibility.html

#ifdef WT_WIN32
  #define WT_EXT_IMPORT __declspec(dllimport)
  #define WT_EXT_EXPORT __declspec(dllexport)
  #define WT_EXT_DLLLOCAL
  #define WT_EXT_DLLPUBLIC
#else
  #define WT_EXT_IMPORT
  #ifdef GCC_HASCLASSVISIBILITY
    #define WT_EXT_IMPORT __attribute__ ((visibility("default")))
    #define WT_EXT_EXPORT __attribute__ ((visibility("default")))
    #define WT_EXT_DLLLOCAL __attribute__ ((visibility("hidden")))
    #define WT_EXT_DLLPUBLIC __attribute__ ((visibility("default")))
  #else
    #define WT_EXT_IMPORT
    #define WT_EXT_EXPORT
    #define WT_EXT_DLLLOCAL
    #define WT_EXT_DLLPUBLIC
  #endif
#endif

// Define wtext_EXPORTS for DLL builds
#ifdef WT_WIN32
  #ifdef wtext_EXPORTS
    #define WT_EXT_API WT_EXT_EXPORT
  #else
    #ifdef WT_EXT_STATIC
      #define WT_EXT_API
    #else
      #define WT_EXT_API WT_EXT_IMPORT
    #endif
  #endif
#else
  #define WT_EXT_API
#endif

#endif // EXTDLLDEFS_H_

