// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_JAVASCRIPT_LOADER_H_
#define WT_JAVASCRIPT_LOADER_H_

#define WT_JS(...) #__VA_ARGS__

#ifndef WT_DEBUG_JS

#define WT_DECLARE_WT_MEMBER(i, name, ...)	       \
  namespace {					       \
    const char *wtjs##i(Wt::WApplication *app) {       \
      return WT_CLASS "." name " = " #__VA_ARGS__ ";"; \
    }						       \
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
