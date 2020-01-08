// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CHART_WLEGEND3D
#define CHART_WLEGEND3D

#include "WLegend.h"

#include <Wt/Chart/WAbstractDataSeries3D.h>

namespace Wt {
  namespace Chart {
   
class WLegend3D : public WLegend 
{
public:
  void renderLegend
    (WPainter* painter,
     const std::vector<std::unique_ptr<WAbstractDataSeries3D> >& dataseries);

  int width();
  int height
    (const std::vector<std::unique_ptr<WAbstractDataSeries3D> >& dataseries);
};
 
  }
}

#endif
