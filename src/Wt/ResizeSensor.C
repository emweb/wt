/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"

#include "ResizeSensor.h"

#ifndef WT_DEBUG_JS
#include "js/ResizeSensor.min.js"
#endif

namespace Wt {

void ResizeSensor::loadJavaScript(WApplication *app)
{
  LOAD_JAVASCRIPT(app, "js/ResizeSensor.js", "ResizeSensor", wtjs1);
}

void ResizeSensor::applyIfNeeded(WWidget *w)
{
  if (!w->javaScriptMember(WWidget::WT_RESIZE_JS).empty()) {
    WApplication *app = WApplication::instance();
    loadJavaScript(app);
    w->setJavaScriptMember
	(" ResizeSensor",
	 "new " WT_CLASS ".ResizeSensor(" WT_CLASS "," + w->jsRef() + ")");
  }
}

}
