/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "WAbstractChartModel.h"

namespace Wt {
  namespace Chart {

WAbstractChartModel::WAbstractChartModel()
{ }

WAbstractChartModel::~WAbstractChartModel()
{ }

WString WAbstractChartModel::displayData(int row, int column) const
{
  return boost::lexical_cast<std::string>(data(row, column));
}

WString WAbstractChartModel::headerData(WT_MAYBE_UNUSED int column) const
{
  return Wt::WString();
}

WString WAbstractChartModel::toolTip(WT_MAYBE_UNUSED int row, WT_MAYBE_UNUSED int column) const
{
  return Wt::WString();
}

WFlags<ItemFlag> WAbstractChartModel::flags(WT_MAYBE_UNUSED int row, WT_MAYBE_UNUSED int column) const
{
  return None;
}

WLink *WAbstractChartModel::link(WT_MAYBE_UNUSED int row, WT_MAYBE_UNUSED int column) const
{
  return nullptr;
}

const WColor *WAbstractChartModel::markerPenColor(WT_MAYBE_UNUSED int row, WT_MAYBE_UNUSED int column) const
{
  return nullptr;
}

const WColor *WAbstractChartModel::markerBrushColor(WT_MAYBE_UNUSED int row, WT_MAYBE_UNUSED int column) const
{
  return nullptr;
}

const MarkerType *WAbstractChartModel::markerType(WT_MAYBE_UNUSED int row, WT_MAYBE_UNUSED int column) const
{
  return nullptr;
}

const WColor *WAbstractChartModel::barPenColor(WT_MAYBE_UNUSED int row, WT_MAYBE_UNUSED int column) const
{
  return nullptr;
}

const WColor *WAbstractChartModel::barBrushColor(WT_MAYBE_UNUSED int row, WT_MAYBE_UNUSED int column) const
{
  return nullptr;
}

const double *WAbstractChartModel::markerScaleFactor(WT_MAYBE_UNUSED int row, WT_MAYBE_UNUSED int column) const
{
  return nullptr;
}

  }
}
