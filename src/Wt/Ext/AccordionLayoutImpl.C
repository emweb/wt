/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Ext/AccordionLayoutImpl.h"
#include "Wt/WAccordionLayout"
#include <ostream>

namespace Wt {
  namespace Ext {

AccordionLayoutImpl::AccordionLayoutImpl(WAccordionLayout *layout)
  : DefaultLayoutImpl(layout)
{ }

void AccordionLayoutImpl::createConfig(std::ostream& config)
{
  config << ",layout:'accordion'";

  DefaultLayoutImpl::createConfig(config);
}

  }
}
