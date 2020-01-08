// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_POPUP_WINDOW_H_
#define WT_POPUP_WINDOW_H_

namespace Wt {

  class WApplication;

  /*! \brief Internal class that provides a JavaScript popup window managing function.
   */
  class PopupWindow {
  public:
    /*! \brief Loads the PopupWindow JavaScript support function.
     */
    static void loadJavaScript(WApplication *app);
  };
}

#endif // WT_POPUP_WINDOW_H_
