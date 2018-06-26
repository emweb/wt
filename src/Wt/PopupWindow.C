/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/PopupWindow.h"

#ifndef WT_DEBUG_JS
#include "js/PopupWindow.min.js"
#endif

namespace Wt {

void PopupWindow::loadJavaScript(WApplication *app)
{
  LOAD_JAVASCRIPT(app, "js/PopupWindow.js", "PopupWindow", wtjs1);
}

}
