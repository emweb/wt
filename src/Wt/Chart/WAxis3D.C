// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Chart/WAxis3D"

namespace Wt {
  namespace Chart {

void WAxis3D::getLabelTicks(std::vector<TickLabel>& ticks,
			    int segment) const
{
  WAxis::getLabelTicks(ticks, segment);
  
}

  }
}
