// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_SIZE_HANDLE_H_
#define WT_SIZE_HANDLE_H_

namespace Wt {

  class WApplication;

  class SizeHandle {
  public:
    static void loadJavaScript(WApplication *app);
  };
}

#endif // WT_SIZE_HANDLE_H_
