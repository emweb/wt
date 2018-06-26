/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"

#include "SizeHandle.h"

#ifndef WT_DEBUG_JS
#include "js/SizeHandle.min.js"
#endif

namespace Wt {

void SizeHandle::loadJavaScript(WApplication *app)
{
  LOAD_JAVASCRIPT(app, "js/SizeHandle.js", "SizeHandle", wtjs1);
}

}
