/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cmath>
#include <limits>
#include <stdio.h>
#include <math.h>

#include <Wt/WAbstractItemModel>
#include <Wt/WDate>

#include <Wt/Chart/WAxis>
#include <Wt/Chart/WCartesianChart>
#include <Wt/Chart/WChart2DRenderer>

namespace {
  const int AXIS_MARGIN = 4;
  const int AUTO_V_LABEL_PIXELS = 25;
  const int AUTO_H_LABEL_PIXELS = 60;

#ifndef __linux__
  double round(double d)
  {
    return (int)(d + 0.5);
  }
#endif

  inline bool myisnan(double d)
  {
    return !(d == d);
  }

  double round125(double v) {
    double n = pow(10, floor(log10(v)));
    double msd = v / n;

    if (msd < 1.5)
      return n;
    else if (msd < 3.3)
      return 2*n;
    else if (msd < 7)
      return 5*n;
    else
      return 10*n;
  }

  double roundUp125(double v, double t) {
    return t * ceil((v - std::numeric_limits<double>::epsilon()) / t);
  }

  double roundDown125(double v, double t) {
    return t * floor((v + std::numeric_limits<double>::epsilon()) / t);
  }
}

namespace Wt {
  namespace Chart {

class ExtremesIterator : public SeriesIterator
{
public:
  ExtremesIterator(Axis axis)
    : axis_(axis),
      minimum_(WAxis::AUTO_MINIMUM),
      maximum_(WAxis::AUTO_MAXIMUM)
  { }

  virtual bool startSeries(const WDataSeries& series, double groupWidth,
			   int numBarGroups, int currentBarGroup)
  {
    return series.axis() == axis_;
  }

  virtual void newValue(const WDataSeries& series, double x, double y,
			double stackY)
  {
    if (!myisnan(y)) {
      maximum_ = std::max(y, maximum_);
      minimum_ = std::min(y, minimum_);
    }
  }

  double minimum() { return minimum_; }
  double maximum() { return maximum_; }

private:
  Axis axis_;
  double minimum_, maximum_;
};

WAxis::TickLabel::TickLabel(double v, TickLength length, const WString& l)
  : u(v),
    tickLength(length),
    label(l)
{ }

const double WAxis::AUTO_MINIMUM = DBL_MAX;
const double WAxis::AUTO_MAXIMUM = -DBL_MAX;

WAxis::Segment::Segment()
  : minimum(AUTO_MINIMUM),
    maximum(AUTO_MAXIMUM)
{ }

WAxis::WAxis()
  : chart_(0),
    axis_(XAxis),
    visible_(true),
    location_(MinimumValue),
    scale_(LinearScale),
    labelInterval_(0),
    gridLines_(false),
    gridLinesPen_(gray),
    margin_(0),
    labelAngle_(0)
{
  titleFont_.setFamily(WFont::SansSerif);
  titleFont_.setSize(WFont::FixedSize, WLength(12, WLength::Point));

  segments_.push_back(Segment());
}

void WAxis::init(WCartesianChart *chart, Axis axis)
{
  chart_ = chart;
  axis_ = axis;

  if (axis == XAxis)
    if (chart->type() == CategoryChart)
      scale_ = CategoryScale;
    else if (scale_ != DateScale)
      scale_ = LinearScale;

  if (axis == Y2Axis)
    visible_ = false;
}

void WAxis::setVisible(bool visible)
{
  set(visible_, visible);
}

void WAxis::setScale(AxisScale scale)
{
  set(scale_, scale);
}

void WAxis::setLocation(AxisLocation location)
{
  set(location_, location);
}

void WAxis::setMinimum(double minimum)
{
  Segment& s = segments_[0];

  if (set(s.minimum, minimum))
    s.maximum = std::max(s.minimum, s.maximum);
}

double WAxis::minimum() const
{
  return segments_[0].minimum;
}

void WAxis::setMaximum(double maximum)
{
  Segment& s = segments_[segments_.size() - 1];

  if (set(s.maximum, maximum))
    s.minimum = std::min(s.minimum, s.maximum);
}

    double WAxis::maximum() const
{
  return segments_[segments_.size() - 1].maximum;
}

void WAxis::setRange(double minimum, double maximum)
{
  segments_[0].minimum = minimum;
  segments_[segments_.size() - 1].maximum = maximum;

  update();
}

void WAxis::setBreak(double minimum, double maximum)
{
  if (segments_.size() != 2) {
    segments_.push_back(Segment());
    segments_[1].maximum = segments_[0].maximum;
  }

  segments_[0].maximum = minimum;
  segments_[1].minimum = maximum;

  update();
}

void WAxis::setLabelInterval(double labelInterval)
{
  set(labelInterval_, labelInterval);
}

void WAxis::setLabelFormat(const WString& format)
{
  set(labelFormat_, format);
}

void WAxis::setLabelAngle(double angle)
{
  set(labelAngle_, angle);
}

void WAxis::setGridLinesEnabled(bool enabled)
{
  set(gridLines_, enabled);
}

void WAxis::setPen(const WPen& pen)
{
  set(pen_, pen);
}

void WAxis::setGridLinesPen(const WPen& pen)
{
  set(gridLinesPen_, pen);
}

void WAxis::setMargin(int pixels)
{
  set(margin_, pixels);
}

void WAxis::setTitle(const WString& title)
{
  set(title_, title);
}

void WAxis::setTitleFont(const WFont& titleFont)
{
  set(titleFont_, titleFont);
}

void WAxis::update()
{
  if (chart_)
    chart_->update();
}

void WAxis::prepareRender(WChart2DRenderer& renderer) const
{
  double totalRenderRange = 0;

  for (unsigned i = 0; i < segments_.size(); ++i) {
    const Segment& s = segments_[i];

    computeRange(renderer, s);
    totalRenderRange += s.renderMaximum - s.renderMinimum;
  }

  bool vertical = axis_ != XAxis;

  double totalRenderLength
    = vertical ? renderer.chartArea().height() : renderer.chartArea().width();
  double totalRenderStart
    = vertical ? renderer.chartArea().bottom() : renderer.chartArea().left();

  const double SEGMENT_MARGIN = 40;

  totalRenderLength -= SEGMENT_MARGIN * (segments_.size() - 1);

  /*
   * Iterate twice, since we adjust the render extrema based on the size
   * and vice-versa
   */
  for (unsigned it = 0; it < 2; ++it) {
    double rs = totalRenderStart;
    double TRR = totalRenderRange;
    totalRenderRange = 0;

    for (unsigned i = 0; i < segments_.size(); ++i) {
      const Segment& s = segments_[i];

      double diff = s.renderMaximum - s.renderMinimum;

      s.renderStart = rs;
      s.renderLength = diff / TRR * totalRenderLength;

      if (i == 0) {
	renderInterval_ = labelInterval_;
	if (renderInterval_ == 0)
	  if (scale_ == CategoryScale) {
	    double numLabels = calcAutoNumLabels(s) / 1.5;

	    renderInterval_
	      = std::max(1.0, floor(chart_->model()->rowCount() / numLabels));
	  } else if (scale_ == LinearScale) {
	    double numLabels = calcAutoNumLabels(s);

	    renderInterval_ = round125(diff / numLabels);
	  }
      }

      if (scale_ == LinearScale) {
	if (it == 0) {
	  if (s.minimum == AUTO_MINIMUM)
	    s.renderMinimum
	      = roundDown125(s.renderMinimum, renderInterval_);
      
	  if (s.maximum == AUTO_MAXIMUM)
	    s.renderMaximum
	      = roundUp125(s.renderMaximum, renderInterval_);
	}
      }

      totalRenderRange += s.renderMaximum - s.renderMinimum;

      if (axis_ == XAxis)
	rs += s.renderLength + SEGMENT_MARGIN;
      else
	rs -= s.renderLength + SEGMENT_MARGIN;
    }
  }
}

void WAxis::computeRange(WChart2DRenderer& renderer, const Segment& segment)
  const
{
  if (scale_ == CategoryScale) {
    segment.renderMinimum = -0.5;
    segment.renderMaximum = chart_->model()->rowCount() - 0.5;    
  } else {
    segment.renderMinimum = segment.minimum;
    segment.renderMaximum = segment.maximum;

    bool findMinimum = segment.renderMinimum == AUTO_MINIMUM;
    bool findMaximum = segment.renderMaximum == AUTO_MAXIMUM;

    if (findMinimum || findMaximum) {
      double minimum = std::numeric_limits<double>::max();
      double maximum = std::numeric_limits<double>::min();

      if (axis_ == XAxis) {
	int dataColumn = chart_->XSeriesColumn();

	if (dataColumn != -1) {
	  WAbstractItemModel *model = chart_->model();

	  for (int i = 0; i < model->rowCount(); ++i) {
	    double v;

	    if (scale_ != DateScale)
	      v = asNumber(model->data(i, dataColumn));
	    else
	      v = getDateValue(model->data(i, dataColumn));

	    if (myisnan(v))
	      continue;

	    if (findMaximum)
	      maximum = std::max(v, maximum);
	    if (findMaximum)
	      minimum = std::min(v, minimum);
	  }
	}
      } else {
	ExtremesIterator iterator(axis_);
	renderer.iterateSeries(&iterator);

	minimum = iterator.minimum();
	maximum = iterator.maximum();
      }

      if (minimum == std::numeric_limits<double>::max())
	if (scale_ == LogScale)
	  minimum = 1;
	else
	  minimum = 0;

      if (maximum == -std::numeric_limits<double>::max())
	if (scale_ == LogScale)
	  maximum = 10;
	else
	  maximum = 100;

      if (findMinimum)
	segment.renderMinimum
	  = std::min(minimum, findMaximum ? maximum : segment.maximum);

      if (findMaximum)
	segment.renderMaximum
	  = std::max(maximum, findMinimum ? minimum : segment.minimum);
    }
    
    double diff = segment.renderMaximum - segment.renderMinimum;

    if (fabs(diff) < std::numeric_limits<double>::epsilon()) {
      /*
       * When the two values or equal, there is no way of knowing what
       * is a plausible range. Take the surrounding integer values
       */
      if (scale_ == LogScale) {
	if (findMinimum)
	  segment.renderMinimum
	    = pow(10, (floor(log10(segment.renderMinimum - 1E-4))));
	if (findMaximum)
	  segment.renderMaximum
	    = pow(10, (ceil(log10(segment.renderMaximum + 1E-4))));
      } else {
	if (findMinimum)
	  segment.renderMinimum = floor(segment.renderMinimum - 1E-4);
	if (findMaximum)
	  segment.renderMaximum = ceil(segment.renderMaximum + 1E-4);
      }

      diff = segment.renderMaximum - segment.renderMinimum;
    }

    if (scale_ == LinearScale) {
      if (findMinimum && segment.renderMinimum >= 0
	  && (segment.renderMinimum - 0.50 * diff <= 0))
	segment.renderMinimum = 0;

      if (findMaximum && segment.renderMaximum <= 0
	  && (segment.renderMaximum + 0.50 * diff >= 0))
	segment.renderMaximum = 0;
    } else if (scale_ == LogScale) {
      double minLog10 = floor(log10(segment.renderMinimum));
      double maxLog10 = ceil(log10(segment.renderMaximum));

      if (findMinimum)
	segment.renderMinimum = pow(10, (minLog10));

      if (findMinimum)
	segment.renderMaximum = pow(10, (maxLog10));
    }
  }
}

double WAxis::map(int rowIndex, int columnIndex, AxisLocation otherLocation,
		  int segment) const
{
  if (scale_ == CategoryScale)
    return map(static_cast<double>(rowIndex), otherLocation, segment);
  else
    return map(chart_->model()->data(rowIndex, columnIndex), otherLocation,
	       segment);
}

double WAxis::map(const boost::any& value, AxisLocation otherLocation,
		  int segment) const
{
  assert (scale_ != CategoryScale);

  if ((scale_ == LinearScale) || (scale_ == LogScale)) {
    return map(asNumber(value), otherLocation, segment);
  } else
    return map(getDateValue(value), otherLocation, segment);
}

double WAxis::getDateValue(const boost::any& v)
{
  if (v.type() != typeid(WDate))
    return std::numeric_limits<double>::signaling_NaN();
  else {
    WDate d = boost::any_cast<WDate>(v);
    return static_cast<double>(d.modifiedJulianDay());
  }
}

double WAxis::map(double u, AxisLocation otherLocation, int segment) const
{
  if (myisnan(u))
      return u;

  const Segment& s = segments_[segment];

  double d;
  if (scale_ != LogScale) {
    int borderMin, borderMax;

    if (scale_ == CategoryScale)
      borderMin = borderMax = 5;
    else {
      borderMin = (s.renderMinimum == 0 && otherLocation == ZeroValue) ? 0 : 5;
      borderMax = (s.renderMinimum == 0 && otherLocation == ZeroValue) ? 0 : 5;
    }

    int remainLength = static_cast<int>(s.renderLength) - borderMin - borderMax;

    d = borderMin
      + (u - s.renderMinimum)
      / (s.renderMaximum - s.renderMinimum)
      * remainLength;
  } else {
    u = std::max(s.renderMinimum, u);
    d = (log(u) - log(s.renderMinimum))
      / (log(s.renderMaximum) - log(s.renderMinimum))
      * s.renderLength;
  }

  if (axis_ == XAxis)
    return s.renderStart + d;
  else
    return s.renderStart - d;
}

WString WAxis::label(double u) const
{
  WString text;

  if (scale_ == CategoryScale) {
    if (chart_->XSeriesColumn() != -1) {
      text = asString(chart_->model()->data((int)u, chart_->XSeriesColumn()));
    } else {
      char buf[10];
      sprintf(buf, "%.4g", u+1);
      text = WString::fromUTF8(buf);
    }
  } else if (scale_ == DateScale) {
    WDate d(static_cast<long>(u));
    WString format = labelFormat_;

    if (format.empty()) {
      return d.toString("dd/MM/yyyy");
    } else
      return d.toString(format);
  } else {
    std::string format = labelFormat_.toUTF8();

    if (format.empty())
      format = "%.4g";

    char buf[10];
    sprintf(buf, format.c_str(), u);
    text = WString::fromUTF8(buf);
  }

  return text;
}

void WAxis::getLabelTicks(WChart2DRenderer& renderer,
			  std::vector<TickLabel>& ticks, int segment) const
{
  static double EPSILON = 1E-3;

  const Segment& s = segments_[segment];

  switch (scale_) {
  case CategoryScale: {
    int renderInterval = std::max(1, static_cast<int>(renderInterval_));
    if (renderInterval == 1) {
      ticks.push_back(TickLabel(-0.5, TickLabel::Long));
      for (int i = 0; i < chart_->model()->rowCount(); ++i) {
	ticks.push_back(TickLabel(i + 0.5, TickLabel::Long));
	ticks.push_back(TickLabel(i, TickLabel::Zero,
				  label(static_cast<double>(i))));
      }
    } else {
      /*
       * We could do a special effort for date X series here...
       */
      for (int i = 0; i < chart_->model()->rowCount(); i += renderInterval) {
	ticks.push_back(TickLabel(i, TickLabel::Long,
				  label(static_cast<double>(i))));
      }
    }
    break;
  }
  case LinearScale: {
    for (int i = 0;; ++i) {
      double v = s.renderMinimum + renderInterval_ * i;

      if (v - s.renderMaximum > EPSILON * renderInterval_)
	break;

      WString t;

      if (i % 2 == 0)
	t = label(v);
 
      ticks.push_back
	(TickLabel(v, i % 2 == 0 ? TickLabel::Long : TickLabel::Short, t));
    }

    break;
  }
  case LogScale: {
    double v = s.renderMinimum;
    double p = v;
    int i = 0;
    for (;; ++i) {
      if (v - s.renderMaximum > EPSILON * s.renderMaximum)
	break;

      if (i == 9) {
	v = p = 10 * p;
	i = 0;
      }

      if (i == 0)
	ticks.push_back(TickLabel(v, TickLabel::Long, label(v)));
      else
	ticks.push_back(TickLabel(v, TickLabel::Short));

      v += p;
    }

    break;
  }
  case DateScale: {
    long daysRange = static_cast<long>(s.renderMaximum - s.renderMinimum);

    double numLabels = calcAutoNumLabels(s);

    double days = daysRange / numLabels;

    enum { Days, Months, Years } unit;
    int interval;

    WDate d(static_cast<long>(s.renderMinimum));

    if (days > 200) {
      unit = Years;
      interval = std::max(1, static_cast<int>(round125(days / 365)));

      if (d.day() != 1 && d.month() != 1)
	d.setDate(d.year(), 1, 1);
    } else if (days > 20) {
      unit = Months;
      double i = days / 30;
      if (i < 1.3)
	interval = 1;
      else if (i < 2.3)
	interval = 2;
      else if (i < 3.3)
	interval = 3;
      else if (i < 4.3)
	interval = 4;
      else
	interval = 6;

      if (d.day() != 1) {
	d.setDate(d.year(), d.month(), 1);
      }

      if ((d.month() - 1) % interval != 0) {
	int m = (((d.month() - 1) / interval) * interval) + 1;
	d.setDate(d.year(), m, 1);
      }
    } else {
      unit = Days;
      if (days < 1.3)
	interval = 1;
      else
	interval = 7 * std::max(1, static_cast<int>((days + 5) / 7));
    }

    bool atTick = (interval > 1) || (unit == Days);

    for (;;) {
      long dl = d.modifiedJulianDay();
      if (dl > s.renderMaximum)
	break;

      WDate next;
      switch (unit) {
      case Years:
	next = d.addYears(interval); break;
      case Months:
	next = d.addMonths(interval); break;
      case Days:
	next = d.addDays(interval);
      }

      WString text;

      if (!labelFormat_.empty())
	text = d.toString(labelFormat_);
      else {
	if (atTick)
	  text = d.toString("dd/MM/yy");
	else
	  switch (unit) {
	  case Months:
	    text = d.toString("MMM yy"); break;
	  case Years:
	    text = d.toString("yyyy"); break;
	  default:
	    break;
	  }
      }

      if (dl >= s.renderMinimum)
	ticks.push_back(TickLabel(static_cast<double>(dl),
				  TickLabel::Long,
				  atTick ? text : WString()));

      if (!atTick) {
	double tl = (next.modifiedJulianDay() + dl)/2;

	if (tl >= s.renderMinimum && tl <= s.renderMaximum) {
	  ticks.push_back(TickLabel(static_cast<double>(tl), TickLabel::Zero,
				    text));
	}
      }

      d = next;
    }

    break;
  }
  }
}

double WAxis::calcAutoNumLabels(const Segment& s) const
{
  bool vertical = (axis_ != XAxis) == (chart_->orientation() == Vertical);

  return s.renderLength
    / (vertical ? AUTO_V_LABEL_PIXELS : AUTO_H_LABEL_PIXELS);
}

  }
}
