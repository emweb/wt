// This may look like C code, but it's really -*- C++ -*-
/*
* Copyright (C) 2017 Emweb bv, Herent, Belgium.
*
* See the LICENSE file for terms of use.
*/
#ifndef WDBOMSSQLSERVERDLLDEFS_H_
#define WDBOMSSQLSERVERDLLDEFS_H_

#include <Wt/WConfig.h>

// Source: http://www.nedprod.com/programs/gccvisibility.html

#ifdef WT_WIN32
  #define WTDBOMSSQLSERVER_IMPORT __declspec(dllimport)
  #define WTDBOMSSQLSERVER_EXPORT __declspec(dllexport)
  #define WTDBOMSSQLSERVER_DLLLOCAL
  #define WTDBOMSSQLSERVER_DLLPUBLIC
#else
  #define WTDBOMSSQLSERVER_IMPORT __attribute__ ((visibility("default")))
  #define WTDBOMSSQLSERVER_EXPORT __attribute__ ((visibility("default")))
  #define WTDBOMSSQLSERVER_DLLLOCAL __attribute__ ((visibility("hidden")))
  #define WTDBOMSSQLSERVER_DLLPUBLIC __attribute__ ((visibility("default")))
#endif

#ifdef wtdbomssqlserver_EXPORTS
  #define WTDBOMSSQLSERVER_API WTDBOMSSQLSERVER_EXPORT
#else
  #ifdef WTDBOMSSQLSERVER_STATIC
    #define WTDBOMSSQLSERVER_API
  #else
    #define WTDBOMSSQLSERVER_API WTDBOMSSQLSERVER_IMPORT
  #endif
#endif

#endif // WDBOMSSQLSERVERDLLDEFS_H_
