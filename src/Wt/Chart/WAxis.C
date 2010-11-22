/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cmath>
#include <limits>
#include <stdio.h>
#include <math.h>

#include <boost/lexical_cast.hpp>

#include "WtException.h"

#include "Wt/WAbstractItemModel"
#include "Wt/WDate"

#include "Wt/Chart/WAxis"
#include "Wt/Chart/WCartesianChart"
#include "Wt/Chart/WChart2DRenderer"

#include "Utils.h"

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

  double round125(double v) {
    double n = std::pow(10, std::floor(std::log10(v)));
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
    return t * std::ceil((v - std::numeric_limits<double>::epsilon()) / t);
  }

  double roundDown125(double v, double t) {
    return t * std::floor((v + std::numeric_limits<double>::epsilon()) / t);
  }
}

namespace Wt {
  namespace Chart {

class ExtremesIterator : public SeriesIterator
{
public:
  ExtremesIterator(Axis axis, AxisScale scale)
    : axis_(axis),scale_(scale),
      minimum_(WAxis::AUTO_MINIMUM),
      maximum_(WAxis::AUTO_MAXIMUM)
  { }

  virtual bool startSeries(const WDataSeries& series, double groupWidth,
			   int numBarGroups, int currentBarGroup)
  {
    return series.axis() == axis_;
  }

  virtual void newValue(const WDataSeries& series, double x, double y,
			double stackY, const WModelIndex& xIndex,
			const WModelIndex& yIndex)
  {
    if (!Utils::isNaN(y) && (scale_!=LogScale || y>0.0)) {
      maximum_ = std::max(y, maximum_);
      minimum_ = std::min(y, minimum_);
    }
  }

  double minimum() { return minimum_; }
  double maximum() { return maximum_; }

private:
  Axis axis_;
  AxisScale scale_;
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
    maximum(AUTO_MAXIMUM),
    renderLength(AUTO_MAXIMUM),
    renderStart(AUTO_MAXIMUM)
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
  labelFont_.setFamily(WFont::SansSerif);
  labelFont_.setSize(WFont::FixedSize, WLength(10, WLength::Point));

  segments_.push_back(Segment());
}

void WAxis::init(WCartesianChart *chart, Axis axis)
{
  chart_ = chart;
  axis_ = axis;

  if (axis == XAxis) {
    if (chart->type() == CategoryChart)
      scale_ = CategoryScale;
    else if (scale_ != DateScale)
      scale_ = LinearScale;
  }

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

void WAxis::setLocation(AxisValue location)
{
  set(location_, location);
}

void WAxis::setMinimum(double minimum)
{
  Segment& s = segments_.front();

#ifndef WT_TARGET_JAVA
  if (set(s.minimum, minimum))
    s.maximum = std::max(s.minimum, s.maximum);
#else
  set(s.minimum, minimum);
  set(s.maximum, std::max(s.minimum, s.maximum));
#endif // WT_TARGET_JAVA
}

double WAxis::minimum() const
{
  return autoLimits() & MinimumValue ? segments_.front().renderMinimum
    : segments_.front().minimum;
}

void WAxis::setMaximum(double maximum)
{
  Segment& s = segments_.back();

#ifndef WT_TARGET_JAVA
  if (set(s.maximum, maximum))
    s.minimum = std::min(s.minimum, s.maximum);
#else
  set(s.maximum, maximum);
  set(s.minimum, std::min(s.minimum, s.maximum));
#endif // WT_TARGET_JAVA
}

double WAxis::maximum() const
{
  const Segment& s = segments_.back();

  return autoLimits() & MaximumValue ? s.renderMaximum
    : s.maximum;
}

void WAxis::setRange(double minimum, double maximum)
{
  if (maximum > minimum) {
    segments_.front().minimum = minimum;
    segments_.back().maximum = maximum;

    update();
  }
}

void WAxis::setAutoLimits(WFlags<AxisValue> locations)
{
  if (locations & MinimumValue)
    set(segments_.front().minimum, AUTO_MINIMUM);
  if (locations & MaximumValue)
    set(segments_.back().maximum, AUTO_MAXIMUM);
}

WFlags<AxisValue> WAxis::autoLimits() const
{
  WFlags<AxisValue> result = 0;

  if (segments_.front().minimum == AUTO_MINIMUM)
    result |= MinimumValue;

  if (segments_.back().maximum == AUTO_MAXIMUM)
    result |= MaximumValue;

  return result;
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

void WAxis::setLabelFont(const WFont& labelFont)
{
  set(labelFont_, labelFont);
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

  static const int CLIP_MARGIN = 5;

  double clipMin = segments_.front().renderMinimum == 0 ? 0 : CLIP_MARGIN;
  double clipMax = segments_.back().renderMaximum == 0 ? 0 : CLIP_MARGIN;

  double totalRenderLength
    = vertical ? renderer.chartArea().height() : renderer.chartArea().width();
  double totalRenderStart
    = vertical ? renderer.chartArea().bottom() - clipMin
    : renderer.chartArea().left() + clipMin;

  const double SEGMENT_MARGIN = 40;

  // 6 pixels additional margin to avoid clipping lines that render
  // the extreme values
  totalRenderLength
    -= SEGMENT_MARGIN * (segments_.size() - 1) + clipMin + clipMax;

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
	if (renderInterval_ == 0) {
	  if (scale_ == CategoryScale) {
	    double numLabels = calcAutoNumLabels(s) / 1.5;

	    renderInterval_
	      = std::max(1.0, std::floor(chart_->model()->rowCount() / numLabels));
	  } else if (scale_ == LinearScale) {
	    double numLabels = calcAutoNumLabels(s);

	    renderInterval_ = round125(diff / numLabels);
	  }
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

void WAxis::setOtherAxisLocation(AxisValue otherLocation) const
{
  if (scale_ != LogScale) {
    for (unsigned i = 0; i < segments_.size(); ++i) {
      const Segment& s = segments_[i];

      int borderMin, borderMax;

      if (scale_ == CategoryScale)
	borderMin = borderMax = 5;
      else {
	borderMin = (s.renderMinimum == 0 && otherLocation == ZeroValue)
	  ? 0 : 5;
	borderMax = (s.renderMinimum == 0 && otherLocation == ZeroValue)
	  ? 0 : 5;
      }

      s.renderLength -= (borderMin + borderMax);

      if (axis_ == XAxis)
	s.renderStart += borderMin;
      else
	s.renderStart -= borderMin;
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
      double maximum = -std::numeric_limits<double>::max();

      if (axis_ == XAxis) {
	int dataColumn = chart_->XSeriesColumn();

	if (dataColumn != -1) {
	  WAbstractItemModel *model = chart_->model();

	  for (int i = 0; i < model->rowCount(); ++i) {
	    double v = getValue(model->data(i, dataColumn));

	    if (Utils::isNaN(v))
	      continue;

	    if (findMaximum)
	      maximum = std::max(v, maximum);
	    if (findMinimum)
	      minimum = std::min(v, minimum);
	  }
	}
      } else {
	ExtremesIterator iterator(axis_, scale_);
	renderer.iterateSeries(&iterator);

	minimum = iterator.minimum();
	maximum = iterator.maximum();
      }

      if (minimum == std::numeric_limits<double>::max()) {
	if (scale_ == LogScale)
	  minimum = 1;
	else if (scale_ == DateScale)
	  minimum = WDate::currentDate().toJulianDay() - 10;
	else
	  minimum = 0;
      }

      if (maximum == -std::numeric_limits<double>::max()) {
	if (scale_ == LogScale)
	  maximum = 10;
	else if (scale_ == DateScale)
	  maximum = WDate::currentDate().toJulianDay();
	else
	  maximum = 100;
      }

      if (findMinimum)
	segment.renderMinimum
	  = std::min(minimum, findMaximum ? maximum : segment.maximum);

      if (findMaximum)
	segment.renderMaximum
	  = std::max(maximum, findMinimum ? minimum : segment.minimum);
    }
    
    double diff = segment.renderMaximum - segment.renderMinimum;

    if (std::fabs(diff) < std::numeric_limits<double>::epsilon()) {
      /*
       * When the two values or equal, there is no way of knowing what
       * is a plausible range. Take the surrounding integer values
       */
      if (scale_ == LogScale) {
	if (findMinimum)
	  segment.renderMinimum
	    = std::pow(10,
		       (std::floor(std::log10(segment.renderMinimum - 0.1))));
	if (findMaximum)
	  segment.renderMaximum
	    = std::pow(10,
		       (std::ceil(std::log10(segment.renderMaximum + 0.1))));
      } else {
	if (findMinimum)
	  segment.renderMinimum = std::floor(segment.renderMinimum - 1E-4);
	if (findMaximum)
	  segment.renderMaximum = std::ceil(segment.renderMaximum + 1E-4);
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
      double minLog10 = std::floor(std::log10(segment.renderMinimum));
      double maxLog10 = std::ceil(std::log10(segment.renderMaximum));

      if (findMinimum)
	segment.renderMinimum = std::pow(10, (minLog10));

      if (findMinimum)
	segment.renderMaximum = std::pow(10, (maxLog10));
    }
  }
}

double WAxis::mapToDevice(const boost::any& value, int segment) const
{
  assert (scale_ != CategoryScale);

  return mapToDevice(getValue(value), segment);
}

double WAxis::getValue(const boost::any& v) const
{
  switch (scale_) {
  case LinearScale:
  case LogScale:
    return asNumber(v);
  case DateScale:
    if (v.type() == typeid(WDate)) {
      WDate d = boost::any_cast<WDate>(v);
      return static_cast<double>(d.toJulianDay());
    } 

#ifndef WT_TARGET_JAVA
    else if (v.type() == typeid(WDateTime)) {
      WDateTime dt = boost::any_cast<WDateTime>(v);
      return static_cast<double>(dt.date().toJulianDay());
    } 
#endif

    else {
      return std::numeric_limits<double>::signaling_NaN();
    }
  case DateTimeScale:
    if (v.type() == typeid(WDate)) {
      WDate d = boost::any_cast<WDate>(v);
      WDateTime dt;
      dt.setDate(d);
      return (double)dt.toTime_t();
    }

#ifndef WT_TARGET_JAVA
    else if (v.type() == typeid(WDateTime)) {
      WDateTime dt = boost::any_cast<WDateTime>(v);
      return static_cast<double>(dt.toTime_t());
    }
#endif

    else {
      return std::numeric_limits<double>::signaling_NaN();
    }
  default:
    return -1.0;
  }
}

double WAxis::mapToDevice(double u, int segment) const
{
  if (Utils::isNaN(u))
      return u;

  const Segment& s = segments_[segment];

  double d;
  if (scale_ != LogScale) {
    d = (u - s.renderMinimum)
      / (s.renderMaximum - s.renderMinimum)
      * s.renderLength;
  } else {
    u = std::max(s.renderMinimum, u);
    d = (std::log(u) - std::log(s.renderMinimum))
      / (std::log(s.renderMaximum) - std::log(s.renderMinimum))
      * s.renderLength;
  }

  if (axis_ == XAxis)
    return s.renderStart + d;
  else
    return s.renderStart - d;
}

double WAxis::mapFromDevice(double d) const
{
  for (unsigned i = 0; i < segments_.size(); ++i) {
    const Segment& s = segments_[i];

    bool lastSegment = (i == segments_.size() - 1);

    if (lastSegment || d < mapToDevice(s.renderMaximum, i)) {
      if (axis_ == XAxis)
	d = d - s.renderStart;
      else
	d = s.renderStart - d;

      if (scale_ != LogScale) {
	return s.renderMinimum + d * (s.renderMaximum - s.renderMinimum)
	  / s.renderLength;
      } else {
	return std::exp(std::log(s.renderMinimum)
			+ d * (std::log(s.renderMaximum)
			       - std::log(s.renderMinimum)) / s.renderLength);
      }
    }
  }

  return 0;
}

WString WAxis::label(double u) const
{
#ifndef WT_TARGET_JAVA
  char buf[30];
#else
  char *buf = 0;
#endif // WT_TARGET_JAVA

  WString text;

  if (scale_ == CategoryScale) {
    if (chart_->XSeriesColumn() != -1) {
      text = asString(chart_->model()->data((int)u, chart_->XSeriesColumn()));
    } else {
#ifdef WT_TARGET_JAVA
      buf =
#endif // WT_TARGET_JAVA
	sprintf(buf, "%.4g", u+1);
      text = WString::fromUTF8(buf);
    }
  } else if (scale_ == DateScale) {
    WDate d = WDate::fromJulianDay(static_cast<int>(u));
    WString format = labelFormat_;

    if (format.empty()) {
      return d.toString("dd/MM/yyyy");
    } else
      return d.toString(format);
  } else {
    std::string format = labelFormat_.toUTF8();

    if (format.empty())
      format = "%.4g";

#ifdef WT_TARGET_JAVA
    buf =
#endif // WT_TARGET_JAVA
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
    double v = s.renderMinimum > 0 ? s.renderMinimum : 0.0001;
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
  case DateTimeScale:
  case DateScale: {
    double daysRange = 0.0;
    WDateTime dt;
    switch (scale_) {
    case DateScale:
      daysRange = static_cast<double>(s.renderMaximum - s.renderMinimum);
      dt.setDate(WDate::fromJulianDay(static_cast<int>(s.renderMinimum)));
      if (!dt.isValid()) {
	std::string exception = "Invalid julian day: ";
	exception += boost::lexical_cast<std::string>(s.renderMinimum);
	throw WtException(exception);
      }
      break;
    case DateTimeScale:
      daysRange = static_cast<double>((s.renderMaximum - s.renderMinimum) 
				    / (60.0 * 60.0 * 24));
      dt = WDateTime::fromTime_t((time_t)s.renderMinimum);
      break;
    default:
      assert(false); // CategoryScale, LinearScale
    }

    double numLabels = calcAutoNumLabels(s);

    double days = daysRange / numLabels;

    enum { Days, Months, Years, Hours, Minutes } unit;
    int interval;

    if (days > 200) {
      unit = Years;
      interval = std::max(1, static_cast<int>(round125(days / 365)));
	
      if (dt.date().day() != 1 && dt.date().month() != 1)
	dt.date().setDate(dt.date().year(), 1, 1);
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
	
      if (dt.date().day() != 1) {
	dt.date().setDate(dt.date().year(), dt.date().month(), 1);
      }
	
      if ((dt.date().month() - 1) % interval != 0) {
	int m = (((dt.date().month() - 1) / interval) * interval) + 1;
	dt.date().setDate(dt.date().year(), m, 1);
      }
    } else if (days > 0.6) {
      unit = Days;
      if (days < 1.3)
	interval = 1;
      else
	interval = 7 * std::max(1, static_cast<int>((days + 5) / 7));
    } else {
      double minutes = days * 24 * 60;
      if (minutes > 40) {
	unit = Hours;
	double i = minutes / 60;
	if (i < 1.3)
	  interval = 1;
	else if (i < 2.3)
	  interval = 2;
	else if (i < 3.3)
	  interval = 3;
	else if (i < 4.3)
	  interval = 4;
	else if (i < 6.3)
	  interval = 6;
	else
	  interval = 12;
      } else {
	unit = Minutes;
	  if (minutes < 1.3)
	    interval = 1;
	  else if (minutes < 2.3)
	    interval = 2;
	  else if (minutes < 5.3)
	    interval = 5;
	  else if (minutes < 10.3)
	    interval = 10;
	  else if (minutes < 15.3)
	    interval = 15;
	  else if (minutes < 20.3)
	    interval = 20;
	  else
	    interval = 30;
      }
    }

    bool atTick = (interval > 1) || (unit <= Days);

    for (;;) {
      long dl = getDateNumber(dt);
      
      if (dl > s.renderMaximum)
	break;

      WDateTime next;
      switch (unit) {
      case Years:
	next = dt.addYears(interval); break;
      case Months:
	next = dt.addMonths(interval); break;
      case Days:
	next = dt.addDays(interval); break;
      case Hours:
	next = dt.addSecs(interval * 60 * 60); break;
      case Minutes:
	next = dt.addSecs(interval * 60); break;
      }

      WString text;

      if (!labelFormat_.empty())
	text = dt.toString(labelFormat_);
      else {
	if (atTick) 
	  switch (unit) {
	  case Months:
	  case Years:
	  case Days:
	    text = dt.toString("dd/MM/yy"); break;
	  case Hours:
	    text = dt.toString("h'h' dd/MM"); break;
	  case Minutes:
	    text = dt.toString("hh:mm"); break;
	  default:
	    break;
	  }
	else
	  switch (unit) {
	  case Months:
	    text = dt.toString("MMM yy"); break;
	  case Years:
	    text = dt.toString("yyyy"); break;
	  case Hours:
	    text = dt.toString("h'h' dd/MM"); break;
	  case Minutes:
	    text = dt.toString("hh:mm"); break;
	  default:
	    break;
	  }
      }

      if (dl >= s.renderMinimum)
	ticks.push_back(TickLabel(static_cast<double>(dl),
				  TickLabel::Long,
				  atTick ? text : WString()));

      if (!atTick) {
	double tl = (getDateNumber(next) + dl)/2;

	if (tl >= s.renderMinimum && tl <= s.renderMaximum) {
	  ticks.push_back(TickLabel(static_cast<double>(tl), TickLabel::Zero,
				    text));
	}
      }

      dt = next;
    }

    break;
  }
  }
}

long WAxis::getDateNumber(WDateTime dt) const
{
  switch (scale_) {
  case DateScale:
    return static_cast<long>(dt.date().toJulianDay());
  case DateTimeScale:
    return static_cast<long>(dt.toTime_t());
  default:
    return 1;
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
