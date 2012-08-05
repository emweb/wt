/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
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
    modelColumn_(modelColumn),
    XSeriesColumn_(-1),
    stacked_(false),
    type_(type),
    axis_(axis),
    customFlags_(0),
    fillRange_(NoFill),
    marker_(type == PointSeries ? CircleMarker : NoMarker),
    markerSize_(6),
    legend_(true),
    xLabel_(false),
    yLabel_(false),
    barWidth_(0.8),
    hidden_(false)
{ }

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
  set(axis_, axis);
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
	  ->borderPen(chart_->seriesIndexOf(modelColumn_));
      else
	return chart_->palette()
	  ->strokePen(chart_->seriesIndexOf(modelColumn_));
    else {
      WPen defaultPen;
      defaultPen.setCapStyle(SquareCap);
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
      return chart_->palette()->brush(chart_->seriesIndexOf(modelColumn_));
    else
      return WBrush();
}

WColor WDataSeries::labelColor() const
{
  if (customFlags_ & CustomLabelColor)
    return labelColor_;
  else
    if (chart_)
      return chart_->palette()->fontColor(chart_->seriesIndexOf(modelColumn_));
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
    return chart_->mapFromDevice(deviceCoordinates, axis_);
  else
    return WPointF();
}

WPointF WDataSeries::mapToDevice(const boost::any& xValue,
				 const boost::any& yValue,
				 int segment) const
{
  if (chart_)
    return chart_->mapToDevice(xValue, yValue, axis_, segment);
  else
    return WPointF();
}

void WDataSeries::setXSeriesColumn(int modelColumn) 
{
  XSeriesColumn_ = modelColumn;
}

  }
}
