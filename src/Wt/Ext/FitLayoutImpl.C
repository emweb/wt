/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Ext/FitLayoutImpl.h"
#include "Wt/WFitLayout"
#include <ostream>

namespace Wt {
  namespace Ext {

FitLayoutImpl::FitLayoutImpl(WFitLayout *layout)
  : LayoutImpl(layout)
{ }

void FitLayoutImpl::createConfig(std::ostream& config)
{
  config << ",layout:'fit'";

  LayoutImpl::createConfig(config);
}

  }
}
