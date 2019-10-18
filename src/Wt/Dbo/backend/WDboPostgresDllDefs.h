// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDBOPOSTGRESDLLDEFS_H_
#define WDBOPOSTGRESDLLDEFS_H_

#include <Wt/WConfig.h>

// Source: http://www.nedprod.com/programs/gccvisibility.html

#ifdef WT_WIN32
  #define WTDBOPOSTGRES_IMPORT __declspec(dllimport)
  #define WTDBOPOSTGRES_EXPORT __declspec(dllexport)
  #define WTDBOPOSTGRES_DLLLOCAL
  #define WTDBOPOSTGRES_DLLPUBLIC
#else
  #define WTDBOPOSTGRES_IMPORT __attribute__ ((visibility("default")))
  #define WTDBOPOSTGRES_EXPORT __attribute__ ((visibility("default")))
  #define WTDBOPOSTGRES_DLLLOCAL __attribute__ ((visibility("hidden")))
  #define WTDBOPOSTGRES_DLLPUBLIC __attribute__ ((visibility("default")))
#endif

#ifdef wtdbopostgres_EXPORTS
  #define WTDBOPOSTGRES_API WTDBOPOSTGRES_EXPORT
#else
  #ifdef WTDBOPOSTGRES_STATIC
    #define WTDBOPOSTGRES_API
  #else
    #define WTDBOPOSTGRES_API WTDBOPOSTGRES_IMPORT
  #endif
#endif

#endif // DLLDEFS_H_
