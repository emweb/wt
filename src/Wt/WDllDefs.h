// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2006 Wim Dumon, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDLLDEFS_H_
#define WDLLDEFS_H_

// For backward compatibility wrt the WT_VERSION define
#include <Wt/WConfig.h>

// Source: http://www.nedprod.com/programs/gccvisibility.html

#ifdef WIN32
  #define WT_IMPORT __declspec(dllimport)
  #define WT_EXPORT __declspec(dllexport)
  #define WT_DLLLOCAL
  #define WT_DLLPUBLIC
#else
  #ifdef GCC_HASCLASSVISIBILITY
    #define WT_IMPORT __attribute__ ((visibility("default")))
    #define WT_EXPORT __attribute__ ((visibility("default")))
    #define WT_DLLLOCAL __attribute__ ((visibility("hidden")))
    #define WT_DLLPUBLIC __attribute__ ((visibility("default")))
  #else
    #define WT_IMPORT
    #define WT_EXPORT
    #define WT_DLLLOCAL
    #define WT_DLLPUBLIC
  #endif
#endif

// Define wt_EXPORTS for DLL builds
#ifdef WIN32
  #ifdef wt_EXPORTS
    #define WT_API WT_EXPORT
  #else
    #ifdef WT_STATIC
      #define WT_API
    #else
      #define WT_API WT_IMPORT
    #endif
  #endif
#else
  #define WT_API
#endif

#ifdef JAVA
#define WT_ARRAY volatile
#else
#define WT_ARRAY
#endif

#ifdef WIN32
typedef __int64 int64_t;            /* 64 bit signed */
typedef unsigned __int64 uint64_t;  /* 64 bit unsigned */
#else // WIN32
#include <stdint.h>
#endif // WIN32

#endif // DLLDEFS_H_
