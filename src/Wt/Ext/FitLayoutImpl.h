// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef EXT_FIT_LAYOUT_IMPL_H_
#define EXT_FIT_LAYOUT_IMPL_H_

#include "Wt/Ext/LayoutImpl.h"

namespace Wt {
  class WFitLayout;

  namespace Ext {

class FitLayoutImpl : public LayoutImpl
{
public:
  FitLayoutImpl(WFitLayout *layout);

protected:
  virtual void createConfig(std::ostream& config);
};

  }
}

#endif // EXT_BORDER_LAYOUT_H_
