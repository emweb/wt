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

#ifdef WT_WIN32
  #define WT_IMPORT __declspec(dllimport)
  #define WT_EXPORT __declspec(dllexport)
  #define WT_DLLLOCAL
  #define WT_DLLPUBLIC
#else
  #if __GNUC__ >= 4
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
#ifdef wt_EXPORTS
  #define WT_API WT_EXPORT
#else
  #ifdef WT_STATIC
    #define WT_API
  #else
    #define WT_API WT_IMPORT
  #endif
#endif

#if defined(wthttp_EXPORTS) || defined(wttest_EXPORTS)
  #define WTCONNECTOR_API WT_EXPORT
#else
  #if defined(WTHTTP_STATIC) || defined(WTISAPI_STATIC)
    // WTISAPI_STATIC is defined when building WTISAPI,
    // when using ISAPI, the user should also define WTISAPI_STATIC
    #define WTCONNECTOR_API 
  #else
    #define WTCONNECTOR_API WT_IMPORT
  #endif
#endif

#ifndef WT_TARGET_JAVA
#define WT_ARRAY
#define W_JAVA_COMPARATOR(type)
#define WT_USTRING Wt::WString
#define WT_UCHAR std::string
#define WT_BOSTREAM std::ostream
#else
#define WT_ARRAY volatile
#define W_JAVA_COMPARATOR(type) : public Comparator<type>
#define WT_USTRING std::string
#define WT_UCHAR char
#define WT_BOSTREAM std::bostream
#endif

#ifdef _MSC_VER
typedef __int64 int64_t;            /* 64 bit signed */
typedef unsigned __int64 uint64_t;  /* 64 bit unsigned */
typedef __int32 int32_t;            /* 64 bit signed */
typedef unsigned __int32 uint32_t;  /* 32 bit unsigned */
#else // _MSC_VER
#include <stdint.h>
#endif // _MSC_VER

#ifdef wt_EXPORTS
  #define DATE_BUILD_DLL
#else
  #ifdef WT_STATIC
    #define DATE_BUILD_LIB
  #endif
#endif

#ifdef WT_WIN32
#define USE_OS_TZDB 0
#else
#define USE_OS_TZDB 1
#endif

#endif // DLLDEFS_H_
