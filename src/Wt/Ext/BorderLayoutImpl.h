// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef EXT_BORDER_LAYOUT_IMPL_H_
#define EXT_BORDER_LAYOUT_IMPL_H_

#include "Wt/Ext/LayoutImpl.h"

namespace Wt {
  class WBorderLayout;

  namespace Ext {

class BorderLayoutImpl : public LayoutImpl
{
public:
  BorderLayoutImpl(WBorderLayout *layout);

protected:
  virtual void createConfig(std::ostream& config);

private:
  virtual void addLayoutConfig(LayoutItemImpl *item, std::ostream& config);
};

  }
}

#endif // EXT_BORDER_LAYOUT_IMPL_H_
