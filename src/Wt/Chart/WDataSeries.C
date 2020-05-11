/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WPointF.h>
#include <Wt/Chart/WChartPalette.h>
#include <Wt/Chart/WDataSeries.h>
#include <Wt/Chart/WCartesianChart.h>

namespace Wt {
  namespace Chart {

WDataSeries::WDataSeries(int modelColumn, SeriesType type, Axis axis)
  : chart_(nullptr),
    model_(nullptr),
    modelColumn_(modelColumn),
    XSeriesColumn_(-1),
    stacked_(false),
    type_(type),
    xAxis_(0),
    yAxis_(axis == Axis::Y1 ? 0 : 1),
    customFlags_(None),
    fillRange_(FillRangeType::None),
    marker_(type == SeriesType::Point ?
            MarkerType::Circle : MarkerType::None),
    markerSize_(6),
    legend_(true),
    xLabel_(false),
    yLabel_(false),
    barWidth_(0.8),
    hidden_(false),
    offset_(0.0),
    scale_(1.0),
    offsetDirty_(true),
    scaleDirty_(true)
{ }

WDataSeries::WDataSeries(int modelColumn, SeriesType type, int axis)
  : chart_(nullptr),
    model_(nullptr),
    modelColumn_(modelColumn),
    XSeriesColumn_(-1),
    stacked_(false),
    type_(type),
    xAxis_(0),
    yAxis_(axis),
    customFlags_(None),
    fillRange_(FillRangeType::None),
    marker_(type == SeriesType::Point ? 
	    MarkerType::Circle : MarkerType::None),
    markerSize_(6),
    legend_(true),
    xLabel_(false),
    yLabel_(false),
    barWidth_(0.8),
    hidden_(false),
    offset_(0.0),
    scale_(1.0),
    offsetDirty_(true),
    scaleDirty_(true)
{ }

WDataSeries::~WDataSeries()
{
  if (model_) {
    for (unsigned i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();
  }
}

void WDataSeries::setBarWidth(const double width) 
{
  barWidth_ = width;
}

double WDataSeries::barWidth() const 
{
  return barWidth_;
}

void WDataSeries::setType(SeriesType type)
{
  set(type_, type);
}

void WDataSeries::setStacked(bool stacked)
{
  set(stacked_, stacked);
}

void WDataSeries::setModelColumn(int modelColumn)
{
  set(modelColumn_, modelColumn);
}

void WDataSeries::bindToAxis(Axis axis)
{
  set(yAxis_, axis == Axis::Y1 ? 0 : 1);
}

void WDataSeries::bindToXAxis(int xAxis)
{
  set(xAxis_, xAxis);
}

void WDataSeries::bindToYAxis(int yAxis)
{
  set(yAxis_, yAxis);
}

void WDataSeries::setCustomFlags(WFlags<CustomFlag> flags)
{
  set(customFlags_, flags);
}

void WDataSeries::setPen(const WPen& pen)
{
  set(pen_, pen);

  customFlags_ |= CustomFlag::Pen;
}

void WDataSeries::setShadow(const WShadow& shadow)
{
  set(shadow_, shadow);
}

const WShadow& WDataSeries::shadow() const
{
  return shadow_;
}

WPen WDataSeries::pen() const
{
  if (customFlags_.test(CustomFlag::Pen))
    return pen_;
  else
    if (chart_)
      if (type_ == SeriesType::Bar)
	return chart_->palette()
	  ->borderPen(chart_->seriesIndexOf(*this));
      else
	return chart_->palette()
	  ->strokePen(chart_->seriesIndexOf(*this));
    else {
      WPen defaultPen;
      defaultPen.setCapStyle(PenCapStyle::Round);
      defaultPen.setJoinStyle(PenJoinStyle::Round);
      return defaultPen;
    }
}

void WDataSeries::setBrush(const WBrush& brush)
{
  set(brush_, brush);

  customFlags_ |= CustomFlag::Brush;
}

WBrush WDataSeries::brush() const
{
  if (customFlags_.test(CustomFlag::Brush))
    return brush_;
  else
    if (chart_)
      return chart_->palette()->brush(chart_->seriesIndexOf(*this));
    else
      return WBrush();
}

WColor WDataSeries::labelColor() const
{
  if (customFlags_.test(CustomFlag::LabelColor))
    return labelColor_;
  else
    if (chart_)
      return chart_->palette()->fontColor(chart_->seriesIndexOf(*this));
    else
      return WColor(StandardColor::Black);
}

void WDataSeries::setLabelColor(const WColor& color)
{
  set(labelColor_, color);

  customFlags_ |= CustomFlag::LabelColor;
}

void WDataSeries::setFillRange(FillRangeType fillRange)
{
  set(fillRange_, fillRange);
}

FillRangeType WDataSeries::fillRange() const
{
  if (type_ == SeriesType::Bar && fillRange_ == FillRangeType::None)
    return FillRangeType::ZeroValue;
  else
    return fillRange_;
}

void WDataSeries::setMarker(MarkerType marker)
{
  set(marker_, marker);
}

void WDataSeries::setCustomMarker(const WPainterPath& path)
{
  set(marker_, MarkerType::Custom);

  customMarker_ = path;
}

void WDataSeries::setMarkerSize(double size)
{
  set(markerSize_, size);
}

void WDataSeries::setMarkerPen(const WPen& pen)
{
  set(markerPen_, pen);

  customFlags_ |= CustomFlag::MarkerPen;
}

WPen WDataSeries::markerPen() const
{
  if (customFlags_.test(CustomFlag::MarkerPen))
    return markerPen_;
  else
    return pen();
}

void WDataSeries::setMarkerBrush(const WBrush& brush)
{
  set(markerBrush_, brush);

  customFlags_ |= CustomFlag::MarkerBrush;
}

WBrush WDataSeries::markerBrush() const
{
  if (customFlags_.test(CustomFlag::MarkerBrush))
    return markerBrush_;
  else
    return brush();
}

void WDataSeries::setLegendEnabled(bool enabled)
{
  set(legend_, enabled);
}

bool WDataSeries::isLegendEnabled() const
{
  if (!isHidden())
    return legend_;
  else
    return false;
}

void WDataSeries::setLabelsEnabled(Axis axis, bool enabled)
{
  if (axis == Axis::X)
    xLabel_ = enabled;
  else
    yLabel_ = enabled;

  update();
}

bool WDataSeries::isLabelsEnabled(Axis axis) const
{
  return axis == Axis::X ? xLabel_ : yLabel_;
}

void WDataSeries::setHidden(bool hidden)
{
  hidden_ = hidden;
}

bool WDataSeries::isHidden() const 
{
  return hidden_;
}

void WDataSeries::setChart(WCartesianChart *chart)
{
  chart_ = chart;
}

void WDataSeries::update()
{
  if (chart_)
    chart_->update();
}

WPointF WDataSeries::mapFromDevice(const WPointF& deviceCoordinates) const
{
  if (chart_)
    return chart_->mapFromDevice(deviceCoordinates,
                                 chart_->xAxis(xAxis_),
                                 chart_->yAxis(yAxis_));
  else
    return WPointF();
}

WPointF WDataSeries::mapToDevice(const cpp17::any& xValue, const cpp17::any& yValue,
				 int segment) const
{
  if (chart_)
    return chart_->mapToDevice(xValue, yValue,
                               chart_->xAxis(xAxis_),
                               chart_->yAxis(yAxis_),
                               segment);
  else
    return WPointF();
}

void WDataSeries::setXSeriesColumn(int modelColumn) 
{
  XSeriesColumn_ = modelColumn;
}

void WDataSeries::setOffset(double offset)
{
  if (offset_ != offset) {
    offset_ = offset;
    update();
  }
  offsetDirty_ =  true;
}

void WDataSeries::setScale(double scale)
{
  if (scale_ != scale) {
    scale_ = scale;
    update();
  }
  scaleDirty_ = true;
}

void WDataSeries::setModel(const std::shared_ptr<WAbstractChartModel>& model)
{
  if (model_) {
    /* disconnect slots from previous model */
    for (unsigned i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();

    modelConnections_.clear();
  }

  model_ = model;

  if (model_) {
#ifdef WT_TARGET_JAVA
    modelConnections_.push_back(model_->changed().connect(this, std::bind(&WDataSeries::modelReset, this)));
#else // !WT_TARGET_JAVA
    modelConnections_.push_back(model_->changed().connect(std::bind(&WDataSeries::modelReset, this)));
#endif // WT_TARGET_JAVA
  }

  if (chart_)
    chart_->update();
}

void WDataSeries::modelReset()
{
  if (chart_)
    chart_->modelReset();
}

std::shared_ptr<WAbstractChartModel> WDataSeries::model() const
{
  if (model_)
    return model_;

  if (chart_)
    return chart_->model();

  return nullptr;
}

  }
}
