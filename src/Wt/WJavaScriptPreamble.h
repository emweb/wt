// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_JAVASCRIPT_PREAMBLE_H_
#define WT_JAVASCRIPT_PREAMBLE_H_

#include <Wt/WObject.h>

#define WT_JS(...) #__VA_ARGS__

namespace Wt {
#ifdef WT_TARGET_JAVA
/*! \brief Enumeration for a JavaScript object type.
 *
 * This is an internal %Wt type.
 */
#endif
enum JavaScriptObjectType {
  JavaScriptFunction,     // called with this == WT
  JavaScriptConstructor,
  JavaScriptObject,
  JavaScriptPrototype
};

#ifdef WT_TARGET_JAVA
/*! \brief Enumeration for a JavaScript object scope.
 *
 * This is an internal %Wt type.
 */
#endif
enum JavaScriptScope {
  ApplicationScope,
  WtClassScope
};

#ifdef WT_TARGET_JAVA
/*! \brief Javascript preamble.
 *
 * This is an internal %Wt type.
 */
#endif
class WT_API WJavaScriptPreamble 
{
public:
  WJavaScriptPreamble(JavaScriptScope scope, JavaScriptObjectType type,
		      const char *name, const char *src);

  JavaScriptScope scope;
  JavaScriptObjectType type;
  const char *name, *src;
};

}

#ifndef WT_DEBUG_JS

#define WT_DECLARE_WT_MEMBER(i, type, name, ...)			\
  namespace {								\
  using namespace Wt;							\
  WJavaScriptPreamble wtjs##i() {					\
    return WJavaScriptPreamble(WtClassScope, type, name, #__VA_ARGS__); \
  }									\
  }

#define WT_DECLARE_WT_MEMBER_BIG(i, type, name, ...)			\
  namespace {								\
  using namespace Wt;							\
  WJavaScriptPreamble wtjs##i() {					\
    return WJavaScriptPreamble(WtClassScope, type, name, #__VA_ARGS__); \
  }									\
  }

#define WT_DECLARE_APP_MEMBER(i, type, name, ...)			\
  namespace {								\
  using namespace Wt;							\
  WJavaScriptPreamble appjs##i() {					\
    return WJavaScriptPreamble(ApplicationScope, type, name, #__VA_ARGS__); \
  }									\
  }

#define LOAD_JAVASCRIPT(app, jsFile, name, jsi)	\
  app->loadJavaScript(jsFile, jsi())

#else // !WT_DEBUG_JS

#define LOAD_JAVASCRIPT(app, jsFile, name, jsi)	\
  app->loadJavaScript(jsFile)

#endif // WT_DEBUG_JS

#endif // WT_JAVASCRIPT_PREAMBLE_H_
