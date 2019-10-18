// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2006 Wim Dumon, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WHTTPDLLDEFS_H_
#define WHTTPDLLDEFS_H_

#include "Wt/WConfig.h"

// Source: http://www.nedprod.com/programs/gccvisibility.html

#ifdef WT_WIN32
  #define WTHTTP_IMPORT __declspec(dllimport)
  #define WTHTTP_EXPORT __declspec(dllexport)
  #define WTHTTP_DLLLOCAL
  #define WTHTTP_DLLPUBLIC
#else
  #define WTHTTP_IMPORT __attribute__ ((visibility("default")))
  #define WTHTTP_EXPORT __attribute__ ((visibility("default")))
  #define WTHTTP_DLLLOCAL __attribute__ ((visibility("hidden")))
  #define WTHTTP_DLLPUBLIC __attribute__ ((visibility("default")))
#endif

// Define wthttp_EXPORTS for DLL builds
#if defined(wthttp_EXPORTS)
  #define WTHTTP_API WTHTTP_EXPORT
#else
  #if defined(WTHTTP_STATIC)
    #define WTHTTP_API
  #else
    #define WTHTTP_API WTHTTP_IMPORT
  #endif
#endif

#endif
