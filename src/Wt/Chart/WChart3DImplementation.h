// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CHART_WCHART3DIMPLEMENTATION_H
#define CHART_WCHART3DIMPLEMENTATION_H

#include <Wt/Chart/WAbstractChartImplementation.h>

namespace Wt {

class WTransform;

  namespace Chart {

class WCartesian3DChart;

class WChart3DImplementation : public WAbstractChartImplementation
{
public:
  WChart3DImplementation(WCartesian3DChart *chart);
  // ~WChart3DImplementation() {}

  virtual ChartType chartType() const override;
  virtual Orientation orientation() const override;
  virtual int axisPadding() const override;
  
  virtual int numberOfCategories(Axis axis = Axis::X) const override;
  virtual WString categoryLabel(int u, Axis axis) const override;
  
  virtual RenderRange computeRenderRange(
      Axis axis, int yAxis, AxisScale scale = AxisScale::Linear)
    const override;

  virtual bool onDemandLoadingEnabled() const override;
  
  virtual void update() override;

private:
  WCartesian3DChart *chart_;
};

  }
}

#endif
