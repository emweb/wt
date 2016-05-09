/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAbstractItemModel"
#include "Wt/WBoostAny"
#include "Wt/Chart/WStandardChartProxyModel"

namespace Wt {
  namespace Chart {

WStandardChartProxyModel::WStandardChartProxyModel(WAbstractItemModel *sourceModel, WObject *parent)
  : WAbstractChartModel(parent),
    sourceModel_(sourceModel)
{
  sourceModel->columnsInserted().connect(
	this, &WStandardChartProxyModel::sourceModelModified);
  sourceModel->columnsRemoved().connect(
	this, &WStandardChartProxyModel::sourceModelModified);
  sourceModel->rowsInserted().connect(
	this, &WStandardChartProxyModel::sourceModelModified);
  sourceModel->rowsRemoved().connect(
	this, &WStandardChartProxyModel::sourceModelModified);
  sourceModel->dataChanged().connect(
	this, &WStandardChartProxyModel::sourceModelModified);
  sourceModel->headerDataChanged().connect(
	this, &WStandardChartProxyModel::sourceModelModified);
  sourceModel->layoutChanged().connect(
	this, &WStandardChartProxyModel::sourceModelModified);
  sourceModel->modelReset().connect(
	this, &WStandardChartProxyModel::sourceModelModified);
}

WStandardChartProxyModel::~WStandardChartProxyModel()
{ }

double WStandardChartProxyModel::data(int row, int column) const
{
  return asNumber(sourceModel_->data(row, column, DisplayRole));
}

WString WStandardChartProxyModel::displayData(int row, int column) const
{
  return asString(sourceModel_->data(row, column, DisplayRole));
}

WString WStandardChartProxyModel::headerData(int column) const
{
  return asString(sourceModel_->headerData(column, Horizontal, DisplayRole));
}

WString WStandardChartProxyModel::toolTip(int row, int column) const
{
  return asString(sourceModel_->data(row, column, ToolTipRole));
}

WFlags<ItemFlag> WStandardChartProxyModel::flags(int row, int column) const
{
  return sourceModel_->index(row, column).flags();
}

WLink *WStandardChartProxyModel::link(int row, int column) const
{
  boost::any result = sourceModel_->data(row, column, LinkRole);

  if (result.empty())
    return 0;
  else {
#ifndef WT_TARGET_JAVA
    link_ = boost::any_cast<WLink>(result);
    return &link_;
#else
    WLink c = boost::any_cast<WLink>(result);
    return &c;
#endif
  }
}


const WColor *WStandardChartProxyModel::color(int row, int column, int colorDataRole) const
{
  boost::any result = sourceModel_->data(row, column, colorDataRole);

  if (result.empty())
    return 0;
  else {
#ifndef WT_TARGET_JAVA
    color_ = boost::any_cast<WColor>(result);
    return &color_;
#else
    WColor c = boost::any_cast<WColor>(result);
    return &c;
#endif
  }
}

const WColor *WStandardChartProxyModel::markerPenColor(int row, int column) const
{
  return color(row, column, MarkerPenColorRole);
}

const WColor *WStandardChartProxyModel::markerBrushColor(int row, int column) const
{
  return color(row, column, MarkerBrushColorRole);
}

const WColor *WStandardChartProxyModel::barPenColor(int row, int column) const
{
  return color(row, column, BarPenColorRole);
}

const WColor *WStandardChartProxyModel::barBrushColor(int row, int column) const
{
  return color(row, column, BarBrushColorRole);
}

const double *WStandardChartProxyModel::markerScaleFactor(int row, int column) const
{
  boost::any result = sourceModel_->data(row, column, MarkerScaleFactorRole);

  if (result.empty()) {
    return WAbstractChartModel::markerScaleFactor(row, column);
  } else {
#ifndef WT_TARGET_JAVA
    return new double(asNumber(result));
#else
    double tmp = asNumber(result);
    return &tmp;
#endif
  }
}

int WStandardChartProxyModel::columnCount() const
{
  return sourceModel_->columnCount();
}

int WStandardChartProxyModel::rowCount() const
{
  return sourceModel_->rowCount();
}

void WStandardChartProxyModel::sourceModelModified()
{
  changed().emit();
}

  }
}
