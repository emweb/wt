/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WAbstractItemModel.h"
#include "Wt/Chart/WStandardChartProxyModel.h"

namespace Wt {
  namespace Chart {

WStandardChartProxyModel
::WStandardChartProxyModel(const std::shared_ptr<WAbstractItemModel>&
			   sourceModel)
  : sourceModel_(sourceModel)
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
  return asNumber(sourceModel_->data(row, column, ItemDataRole::Display));
}

WString WStandardChartProxyModel::displayData(int row, int column) const
{
  return asString(sourceModel_->data(row, column, ItemDataRole::Display));
}

WString WStandardChartProxyModel::headerData(int column) const
{
  return asString(sourceModel_->headerData(column, 
					   Orientation::Horizontal, 
					   ItemDataRole::Display));
}

WString WStandardChartProxyModel::toolTip(int row, int column) const
{
  return asString(sourceModel_->data(row, column, ItemDataRole::ToolTip));
}

WFlags<ItemFlag> WStandardChartProxyModel::flags(int row, int column) const
{
  return sourceModel_->index(row, column).flags();
}

WLink *WStandardChartProxyModel::link(int row, int column) const
{
  cpp17::any result = sourceModel_->data(row, column, ItemDataRole::Link);

  if (result.empty())
    return 0;
  else {
#ifndef WT_TARGET_JAVA
    link_ = cpp17::any_cast<WLink>(result);
    return &link_;
#else
    WLink c = boost::any_cast<WLink>(result);
    return &c;
#endif
  }
}


const WColor *WStandardChartProxyModel::color(int row, int column, int colorDataRole) const
{
  cpp17::any result = sourceModel_->data(row, column, colorDataRole);

  if (result.empty())
    return nullptr;
  else {
#ifndef WT_TARGET_JAVA
    color_ = cpp17::any_cast<WColor>(result);
    return &color_;
#else
    WColor c = cpp17::any_cast<WColor>(result);
    return &c;
#endif
  }
}

const WColor *WStandardChartProxyModel::markerPenColor(int row, int column) const
{
  return color(row, column, ItemDataRole::MarkerPenColor);
}

const WColor *WStandardChartProxyModel::markerBrushColor(int row, int column) const
{
  return color(row, column, ItemDataRole::MarkerBrushColor);
}

const WColor *WStandardChartProxyModel::barPenColor(int row, int column) const
{
  return color(row, column, ItemDataRole::BarPenColor);
}

const WColor *WStandardChartProxyModel::barBrushColor(int row, int column) const
{
  return color(row, column, ItemDataRole::BarBrushColor);
}

const double *WStandardChartProxyModel::markerScaleFactor(int row, int column) const
{
  cpp17::any result = sourceModel_->data(row, column, 
				      ItemDataRole::MarkerScaleFactor);

  if (result.empty()) {
    return WAbstractChartModel::markerScaleFactor(row, column);
  } else {
#ifndef WT_TARGET_JAVA
    scale_ = asNumber(result);
    return &scale_;
#else
    double scale = asNumber(result);
    return &scale;
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
