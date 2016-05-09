/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "WAbstractChartModel"

namespace Wt {
  namespace Chart {

WAbstractChartModel::WAbstractChartModel(WObject *parent)
  : WObject(parent)
{ }

WAbstractChartModel::~WAbstractChartModel()
{ }

WString WAbstractChartModel::displayData(int row, int column) const
{
  return boost::lexical_cast<std::string>(data(row, column));
}

WString WAbstractChartModel::headerData(int column) const
{
  return Wt::WString();
}

WString WAbstractChartModel::toolTip(int row, int column) const
{
  return Wt::WString();
}

WFlags<ItemFlag> WAbstractChartModel::flags(int row, int column) const
{
  return 0;
}

WLink *WAbstractChartModel::link(int row, int column) const
{
  return 0;
}

const WColor *WAbstractChartModel::markerPenColor(int row, int column) const
{
  return 0;
}

const WColor *WAbstractChartModel::markerBrushColor(int row, int column) const
{
  return 0;
}

const WColor *WAbstractChartModel::barPenColor(int row, int column) const
{
  return 0;
}

const WColor *WAbstractChartModel::barBrushColor(int row, int column) const
{
  return 0;
}

const double *WAbstractChartModel::markerScaleFactor(int row, int column) const
{
  return 0;
}

  }
}
