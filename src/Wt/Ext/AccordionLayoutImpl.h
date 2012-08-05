// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef EXT_ACCORDION_LAYOUT_IMPL_H_
#define EXT_ACCORDION_LAYOUT_IMPL_H_

#include "Wt/Ext/DefaultLayoutImpl.h"

namespace Wt {

  class WAccordionLayout;

  namespace Ext {

class AccordionLayoutImpl : public DefaultLayoutImpl
{
public:
  AccordionLayoutImpl(WAccordionLayout *layout);

protected:
  virtual void createConfig(std::ostream& config);
};

  }
}
#endif // EXT_ACCORDION_LAYOUT_IMPL_H_
