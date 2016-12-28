/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"

#include "Resizable.h"

#ifndef WT_DEBUG_JS
#include "js/Resizable.min.js"
#endif

namespace Wt {

void Resizable::loadJavaScript(WApplication *app)
{
  LOAD_JAVASCRIPT(app, "js/Resizable.js", "Resizable", wtjs1);
}

}
