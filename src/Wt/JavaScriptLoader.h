// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_JAVASCRIPT_LOADER_H_
#define WT_JAVASCRIPT_LOADER_H_

#include <cstring>

#define WT_JS(...) #__VA_ARGS__

#ifndef WT_DEBUG_JS

/*
 * If the function is not a constructor, then we make sure that
 * it is always called with 'this' == WT.
 *
 * A function that is a constructor is indicated by starting with
 * 'ctor.' (this is removed in the actual member name).
 *
 * This is in sync with the JavaScript WT_DECLARE_WT_MEMBER routine
 * used using WT_DEBUG_JS
 */

#define WT_DECLARE_WT_MEMBER(i, name, ...)				\
  namespace {								\
    std::string wtjs##i(Wt::WApplication *app) {			\
      const char *s = #__VA_ARGS__;					\
      if (std::string(name).find(".prototype") != std::string::npos)	\
	return std::string(WT_CLASS "." name " = ")			\
	  + s + ";";							\
      else if (std::strncmp(name, "ctor.", 5) == 0)			\
	return WT_CLASS "." + std::string(name).substr(5) + " = "	\
	  + s + ";";							\
      else								\
        return std::string(WT_CLASS "." name " = function() { (")	\
          + s + ").apply(" WT_CLASS ", arguments) };";			\
    }									\
  }

#define WT_DECLARE_APP_MEMBER(i, name, ...)				\
  namespace {								\
    std::string appjs##i(Wt::WApplication *app) {			\
      return app->javaScriptClass() + "." name " = " #__VA_ARGS__ ";";	\
    }									\
  }

#define LOAD_JAVASCRIPT(app, jsFile, name, jsi)	\
  app->doJavaScript(jsi(app), false)

#else // !WT_DEBUG_JS

#define LOAD_JAVASCRIPT(app, jsFile, name, jsi)	\
  app->loadJavaScript(jsFile)

#endif // WT_DEBUG_JS

#endif // WT_JAVASCRIPT_LOADER_H_
