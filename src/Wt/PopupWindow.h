// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_POPUP_WINDOW_H_
#define WT_POPUP_WINDOW_H_

namespace Wt {

  class WApplication;

  class PopupWindow {
  public:
    static void loadJavaScript(WApplication *app);
  };
}

#endif // WT_POPUP_WINDOW_H_
