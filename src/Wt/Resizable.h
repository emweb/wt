// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_RESIZABLE_H_
#define WT_RESIZABLE_H_

namespace Wt {

  class WApplication;

  class Resizable {
  public:
    static void loadJavaScript(WApplication *app);
  };
}

#endif // WT_RESIZABLE_H_
