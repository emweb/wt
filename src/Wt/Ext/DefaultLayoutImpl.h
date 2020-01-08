// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef EXT_DEFAULT_LAYOUT_IMPL_H_
#define EXT_DEFAULT_LAYOUT_IMPL_H_

#include "Wt/Ext/LayoutImpl.h"

namespace Wt {

  class WDefaultLayout;

  namespace Ext {

class DefaultLayoutImpl : public LayoutImpl
{
public:
  DefaultLayoutImpl(WDefaultLayout *layout);
  ~DefaultLayoutImpl();
};

  }
}
#endif // EXT_DEFAULT_LAYOUT_IMPL_H_
