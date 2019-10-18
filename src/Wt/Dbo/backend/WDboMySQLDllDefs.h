// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDBOMYSQLDLLDEFS_H_
#define WDBOMYSQLDLLDEFS_H_

#include <Wt/WConfig.h>

// Source: http://www.nedprod.com/programs/gccvisibility.html

#ifdef WT_WIN32
  #define WTDBOMYSQL_IMPORT __declspec(dllimport)
  #define WTDBOMYSQL_EXPORT __declspec(dllexport)
  #define WTDBOMYSQL_DLLLOCAL
  #define WTDBOMYSQL_DLLPUBLIC
#else
  #define WTDBOMYSQL_IMPORT __attribute__ ((visibility("default")))
  #define WTDBOMYSQL_EXPORT __attribute__ ((visibility("default")))
  #define WTDBOMYSQL_DLLLOCAL __attribute__ ((visibility("hidden")))
  #define WTDBOMYSQL_DLLPUBLIC __attribute__ ((visibility("default")))
#endif

#ifdef wtdbomysql_EXPORTS
  #define WTDBOMYSQL_API WTDBOMYSQL_EXPORT
#else
  #ifdef WTDBOMYSQL_STATIC
    #define WTDBOMYSQL_API
  #else
    #define WTDBOMYSQL_API WTDBOMYSQL_IMPORT
  #endif
#endif

#endif // DLLDEFS_H_
