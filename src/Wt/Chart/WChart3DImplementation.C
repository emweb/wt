// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Chart/WChart3DImplementation.h"

#include "Wt/Chart/WCartesian3DChart.h"
#include "Wt/Chart/WGridData.h"
#include "Wt/WAbstractItemModel.h"
#include "Wt/WAny.h"
#include "Wt/WTransform.h"

namespace Wt {
  namespace Chart {

WChart3DImplementation::WChart3DImplementation(WCartesian3DChart *chart)
  : chart_(chart)
{}

ChartType WChart3DImplementation::chartType() const
{
  return chart_->type();
}

Orientation WChart3DImplementation::orientation() const
{
  return Orientation::Vertical;
}

int WChart3DImplementation::axisPadding() const
{
  return 0;
}

int WChart3DImplementation::numberOfCategories(Axis axis) const
{
  if (chart_->dataSeries().size() == 0)
    return 10;

  WAbstractGridData *first;
  if (axis == Axis::X3D) {
    first = dynamic_cast<WAbstractGridData*>(chart_->dataSeries()[0]);
    if (first == nullptr) {
      throw WException("WChart3DImplementation: can only count the categories in WAbstractGridData");
    } else {
      return first->nbXPoints();
    }
  } else if (axis == Axis::Y3D) {
    first = dynamic_cast<WAbstractGridData*>(chart_->dataSeries()[0]);
    if (first == nullptr) {
      throw WException("WChart3DImplementation: can only count the categories in WAbstractGridData");
    } else {
      return first->nbYPoints();
    }
  } else {
    throw WException("WChart3DImplementation: don't know this type of axis");
  }
}

WString WChart3DImplementation::categoryLabel(int u, Axis axis) const
{
  if (chart_->dataSeries().size() == 0)
    return WString(std::to_string(u));

  WAbstractGridData *first = dynamic_cast<WAbstractGridData*>(chart_->dataSeries()[0]);
  if (!first) {
    throw WException("WChart3DImplementation: "
		     "can only count the categories in WAbstractGridData");
  }

  return first->axisLabel(u, axis);
}

WAbstractChartImplementation::RenderRange WChart3DImplementation::computeRenderRange(Axis axis, int xAxis, int yAxis, AxisScale scale) const
{
  WAbstractChartImplementation::RenderRange range;
  
  const std::vector<WAbstractDataSeries3D*> series = chart_->dataSeries();
  if (series.size() == 0) {
    range.minimum = 0;
    range.maximum = 100;
    return range;
  }

  double min=std::numeric_limits<double>::max();
  double max=std::numeric_limits<double>::min();
  int xDim = 0, yDim = 0;
  double stackedBarsHeight = 0.0;
  WAbstractGridData* griddata;
  switch (chart_->type()) {
  case ChartType::Scatter:
    for (unsigned i = 0; i < series.size(); i++) {
      double seriesMin = series[i]->minimum(axis);
      double seriesMax = series[i]->maximum(axis);
      if (seriesMin < min) {
	min = seriesMin;
      }
      if (seriesMax > max) {
	max = seriesMax;
      }
    }
    range.minimum = min;
    range.maximum = max;
    return range;
  case ChartType::Category:
    for (unsigned k = 0; k < series.size(); k++) {
      griddata = dynamic_cast<WAbstractGridData*>(series[k]);
      if ( griddata == 0 
	   || griddata->type() != Series3DType::Bar) {
	throw WException("WChart3DImplementation: not all data is categorical");
      }
    }
    xDim = dynamic_cast<WAbstractGridData*>(series[0])->nbXPoints();
    yDim = dynamic_cast<WAbstractGridData*>(series[0])->nbYPoints();
    min = 0.0;

    for (int i=0; i < xDim; i++) {
      for (int j=0; j < yDim; j++) {
	for (unsigned k = 0; k < series.size(); k++) {
	  if (series[k]->isHidden())
	    continue;
	  griddata = dynamic_cast<WAbstractGridData*>(series[k]);
	  stackedBarsHeight += Wt::asNumber(griddata->data(i,j));
	}
	if (stackedBarsHeight > max) {
	  max = stackedBarsHeight;
	}
	stackedBarsHeight = 0;
      }
    }
    if (max == std::numeric_limits<double>::min()) {
      max = 100.0;
    }
    range.minimum = min;
    range.maximum = max;
    return range;
  default:
    throw WException("WChart3DImplementation: don't know this axis-type");
  }
}

void WChart3DImplementation::update()
{
  chart_->updateChart(ChartUpdates::GLContext | ChartUpdates::GLTextures);
}

bool WChart3DImplementation::onDemandLoadingEnabled() const
{
  return false;
}


  }
}
