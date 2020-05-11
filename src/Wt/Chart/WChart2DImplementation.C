/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Chart/WChart2DImplementation.h"
#include "Wt/Chart/WCartesianChart.h"
#include "Wt/Chart/WAbstractChartModel.h"
#include "Wt/WPainter.h"

#include "WebUtils.h"

namespace Wt {
  namespace Chart {

bool ExtremesIterator::startSeries(const WDataSeries& series, double groupWidth,
				   int numBarGroups, int currentBarGroup)
{
  if (axis_ == Axis::X) {
    return series.xAxis() == xAxis_;
  } else {
    return series.yAxis() == yAxis_;
  }
}

void ExtremesIterator::newValue(const WDataSeries& series, double x, double y,
				double stackY, int xRow, int xColumn,
				int yRow, int yColumn)
{
  double v = axis_ == Axis::X ? x : y;
  
  if (!Utils::isNaN(v) && (scale_ != AxisScale::Log || v > 0.0)) {
    maximum_ = std::max(v, maximum_);
    minimum_ = std::min(v, minimum_);
  }
}
    
WChart2DImplementation::WChart2DImplementation(WCartesianChart *chart)
  : chart_(chart)
{ }

ChartType WChart2DImplementation::chartType() const
{
  return chart_->type();
}

void WChart2DImplementation::update()
{
  chart_->update();
}

int WChart2DImplementation::axisPadding() const
{
  return chart_->axisPadding();
}

int WChart2DImplementation::numberOfCategories(Axis axis) const
{
  if (chart_->model())
    return chart_->model()->rowCount();
  else
    return 0;
}

Orientation WChart2DImplementation::orientation() const
{
  return chart_->orientation();
}

WString WChart2DImplementation::categoryLabel(int u, Axis axis) const
{
  if (chart_->XSeriesColumn() != -1) {
    if (u < chart_->model()->rowCount())
      return chart_->model()->displayData(u, chart_->XSeriesColumn());
    else
      return WString();
  } else {
    return WString();
  }
}

WChart2DImplementation::RenderRange WChart2DImplementation::computeRenderRange(Axis axis, int xAxis, int yAxis, AxisScale scale) const
{
  ExtremesIterator iterator(axis, xAxis, yAxis, scale);
  
  chart_->iterateSeries(&iterator, nullptr, false, axis == Axis::X);

  RenderRange range;
  range.minimum = iterator.minimum();
  range.maximum = iterator.maximum();

  return range;
}

bool WChart2DImplementation::onDemandLoadingEnabled() const
{
  return chart_->onDemandLoadingEnabled();
}

  }
}
