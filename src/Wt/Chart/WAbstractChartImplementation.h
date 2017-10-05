// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CHART_WABSTRACTCHARTIMPLEMENTATION_H
#define CHART_WABSTRACTCHARTIMPLEMENTATION_H

#include <Wt/Chart/WChartGlobal.h>
#include <Wt/Chart/WAxis.h>
#include <Wt/WString.h>

namespace Wt {
  namespace Chart {
    
class WAbstractChartImplementation {
public:
  virtual ~WAbstractChartImplementation() {}
  
  struct RenderRange {
    double minimum;
    double maximum;
  };

  virtual ChartType chartType() const = 0;
  virtual Orientation orientation() const = 0;
  virtual int axisPadding() const = 0;
  
  virtual int numberOfCategories(Axis axis = Axis::X) const = 0;
  virtual WString categoryLabel(int u, Axis axis) const = 0;
  
  virtual RenderRange computeRenderRange(Axis axis, AxisScale scale) const = 0;
  
  virtual void update() = 0;
};
    
  }
}

#endif
