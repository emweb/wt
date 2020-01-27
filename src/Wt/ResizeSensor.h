// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_RESIZE_SENSOR_H_
#define WT_RESIZE_SENSOR_H_

namespace Wt {

  class WApplication;
  class WWidget;

  class ResizeSensor {
  public:
    static void applyIfNeeded(WWidget *w);
    static void loadJavaScript(WApplication *app);
  };
}

#endif // WT_RESIZE_SENSOR_H_
