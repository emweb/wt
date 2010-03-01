/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"

#include "SizeHandle.h"
#include "JavaScriptLoader.h"

#ifndef WT_DEBUG_JS
#include "js/SizeHandle.min.js"
#endif

namespace Wt {

void SizeHandle::loadJavaScript(WApplication *app)
{
  const char *THIS_JS = "js/SizeHandle.js";

  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "SizeHandle", wtjs1);
    app->setJavaScriptLoaded(THIS_JS);
  }
}

}
