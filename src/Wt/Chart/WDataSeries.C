/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WPointF>
#include <Wt/Chart/WChartPalette>
#include <Wt/Chart/WDataSeries>
#include <Wt/Chart/WCartesianChart>

namespace Wt {
  namespace Chart {

WDataSeries::WDataSeries(int modelColumn, SeriesType type, Axis axis)
  : chart_(0),
    model_(0),
    modelColumn_(modelColumn),
    XSeriesColumn_(-1),
    stacked_(false),
    type_(type),
    xAxis_(0),
    yAxis_(axis == Y1Axis ? 0 : 1),
    customFlags_(0),
    fillRange_(NoFill),
    marker_(type == PointSeries ? CircleMarker : NoMarker),
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

WDataSeries::WDataSeries(int modelColumn, SeriesType type, int yAxis)
  : chart_(0),
    model_(0),
    modelColumn_(modelColumn),
    XSeriesColumn_(-1),
    stacked_(false),
    type_(type),
    xAxis_(0),
    yAxis_(yAxis),
    customFlags_(0),
    fillRange_(NoFill),
    marker_(type == PointSeries ? CircleMarker : NoMarker),
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

WDataSeries::WDataSeries(const WDataSeries &other)
  : chart_(0),
    model_(other.model_),
    modelColumn_(other.modelColumn_),
    XSeriesColumn_(other.XSeriesColumn_),
    stacked_(other.stacked_),
    type_(other.type_),
    xAxis_(other.xAxis_),
    yAxis_(other.yAxis_),
    customFlags_(other.customFlags_),
    pen_(other.pen_),
    markerPen_(other.markerPen_),
    brush_(other.brush_),
    markerBrush_(other.markerBrush_),
    labelColor_(other.labelColor_),
    shadow_(other.shadow_),
    fillRange_(other.fillRange_),
    marker_(other.marker_),
    markerSize_(other.markerSize_),
    legend_(other.legend_),
    xLabel_(other.xLabel_),
    yLabel_(other.yLabel_),
    barWidth_(other.barWidth_),
    hidden_(other.hidden_),
    customMarker_(other.customMarker_),
    offset_(other.offset_),
    scale_(other.scale_),
    offsetDirty_(true),
    scaleDirty_(true)
{
  if (model_) {
    modelConnections_.push_back(model_->changed().connect
                                (this, &WDataSeries::modelReset));
  }
}

WDataSeries &WDataSeries::operator=(const WDataSeries &rhs)
{
  if (&rhs != this && model_) {
    for (unsigned i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();

    modelConnections_.clear();
  }
  model_ = rhs.model_;
  if (model_) {
    modelConnections_.push_back(model_->changed().connect
                                (this, &WDataSeries::modelReset));
  }
  modelColumn_ = rhs.modelColumn_;
  XSeriesColumn_ = rhs.XSeriesColumn_;
  stacked_ = rhs.stacked_;
  type_ = rhs.type_;
  xAxis_ = rhs.xAxis_;
  yAxis_ = rhs.yAxis_;
  customFlags_ = rhs.customFlags_;
  pen_ = rhs.pen_;
  markerPen_ = rhs.markerPen_;
  brush_ = rhs.brush_;
  markerBrush_ = rhs.markerBrush_;
  labelColor_ = rhs.labelColor_;
  shadow_ = rhs.shadow_;
  fillRange_ = rhs.fillRange_;
  marker_ = rhs.marker_;
  markerSize_ = rhs.markerSize_;
  legend_ = rhs.legend_;
  xLabel_ = rhs.xLabel_;
  yLabel_ = rhs.yLabel_;
  barWidth_ = rhs.barWidth_;
  hidden_ = rhs.hidden_;
  customMarker_ = rhs.customMarker_;
  offset_ = rhs.offset_;
  scale_ = rhs.scale_;
  offsetDirty_ = true;
  scaleDirty_ = true;

  return *this;
}

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
  set(yAxis_, axis == Y1Axis ? 0 : 1);
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

  customFlags_ |= CustomPen;
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
  if (customFlags_ & CustomPen)
    return pen_;
  else
    if (chart_)
      if (type_ == BarSeries)
	return chart_->palette()
	  ->borderPen(chart_->seriesIndexOf(*this));
      else
	return chart_->palette()
	  ->strokePen(chart_->seriesIndexOf(*this));
    else {
      WPen defaultPen;
      defaultPen.setCapStyle(RoundCap);
      defaultPen.setJoinStyle(RoundJoin);
      return defaultPen;
    }
}

void WDataSeries::setBrush(const WBrush& brush)
{
  set(brush_, brush);

  customFlags_ |= CustomBrush;
}

WBrush WDataSeries::brush() const
{
  if (customFlags_ & CustomBrush)
    return brush_;
  else
    if (chart_)
      return chart_->palette()->brush(chart_->seriesIndexOf(*this));
    else
      return WBrush();
}

WColor WDataSeries::labelColor() const
{
  if (customFlags_ & CustomLabelColor)
    return labelColor_;
  else
    if (chart_)
      return chart_->palette()->fontColor(chart_->seriesIndexOf(*this));
    else
      return black;
}

void WDataSeries::setLabelColor(const WColor& color)
{
  set(labelColor_, color);

  customFlags_ |= CustomLabelColor;
}

void WDataSeries::setFillRange(FillRangeType fillRange)
{
  set(fillRange_, fillRange);
}

FillRangeType WDataSeries::fillRange() const
{
  if (type_ == BarSeries && fillRange_ == NoFill)
    return ZeroValueFill;
  else
    return fillRange_;
}

void WDataSeries::setMarker(MarkerType marker)
{
  set(marker_, marker);
}

void WDataSeries::setCustomMarker(const WPainterPath& path)
{
  set(marker_, CustomMarker);

  customMarker_ = path;
}

void WDataSeries::setMarkerSize(double size)
{
  set(markerSize_, size);
}

void WDataSeries::setMarkerPen(const WPen& pen)
{
  set(markerPen_, pen);

  customFlags_ |= CustomMarkerPen;
}

WPen WDataSeries::markerPen() const
{
  if (customFlags_ & CustomMarkerPen)
    return markerPen_;
  else
    return pen();
}

void WDataSeries::setMarkerBrush(const WBrush& brush)
{
  set(markerBrush_, brush);

  customFlags_ |= CustomMarkerBrush;
}

WBrush WDataSeries::markerBrush() const
{
  if (customFlags_ & CustomMarkerBrush)
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
  if (axis == XAxis)
    xLabel_ = enabled;
  else
    yLabel_ = enabled;

  update();
}

bool WDataSeries::isLabelsEnabled(Axis axis) const
{
  return axis == XAxis ? xLabel_ : yLabel_;
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

WPointF WDataSeries::mapToDevice(const boost::any& xValue,
				 const boost::any& yValue,
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

void WDataSeries::setModel(WAbstractChartModel *model)
{
  if (model_) {
    /* disconnect slots from previous model */
    for (unsigned i = 0; i < modelConnections_.size(); ++i)
      modelConnections_[i].disconnect();

    modelConnections_.clear();
  }

  model_ = model;

  if (model_) {
    modelConnections_.push_back(model_->changed().connect
                                (this, &WDataSeries::modelReset));
  }

  if (chart_)
    chart_->update();
}

void WDataSeries::modelReset()
{
  if (chart_)
    chart_->modelReset();
}

const WAbstractChartModel *WDataSeries::model() const
{
  if (model_)
    return model_;
  if (chart_)
    return chart_->model();
  return 0;
}

  }
}
