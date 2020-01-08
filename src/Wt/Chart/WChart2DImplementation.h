// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CHART_WCHART2DIMPLEMENTATION_H
#define CHART_WCHART2DIMPLEMENTATION_H

#include <Wt/Chart/WCartesianChart.h>
#include <Wt/Chart/WAbstractChartImplementation.h>

namespace Wt {
  namespace Chart {

class ExtremesIterator final : public SeriesIterator
{
public:
  ExtremesIterator(Axis axis, int yAxis, AxisScale scale)
    : axis_(axis), yAxis_(yAxis), scale_(scale),
      minimum_(DBL_MAX),
      maximum_(-DBL_MAX)
  { }
  
  virtual bool startSeries(const WDataSeries& series, double groupWidth,
			   int numBarGroups, int currentBarGroup) override;
  
  virtual void newValue(const WDataSeries& series, double x, double y,
			double stackY, int xRow, int xColumn,
			int yRow, int yColumn) override;
  
  double minimum() { return minimum_; }
  double maximum() { return maximum_; }

private:
  Axis axis_;
  int yAxis_;
  AxisScale scale_;
  double minimum_, maximum_;
};


class WChart2DImplementation : public WAbstractChartImplementation
{
public:
  WChart2DImplementation(WCartesianChart *chart);

  virtual ChartType chartType() const override;
  virtual Orientation orientation() const override;
  virtual int axisPadding() const override;

  virtual int numberOfCategories(Axis axis = Axis::X) const override;
  virtual WString categoryLabel(int u, Axis axis = Axis::X) const override;
  virtual RenderRange computeRenderRange(Axis axis, int yAxis, AxisScale scale) const override;

  virtual bool onDemandLoadingEnabled() const override;

  virtual void update() override;
  
private:
  WCartesianChart *chart_;
};

  }
}

#endif
