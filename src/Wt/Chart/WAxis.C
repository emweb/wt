/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cmath>
#include <limits>
#include <cstdio>

#include <boost/lexical_cast.hpp>

#include "Wt/WAbstractItemModel"
#include "Wt/WDate"
#include "Wt/WException"
#include "Wt/WLogger"
#include "Wt/WTime"

#include "Wt/Chart/WAxis"
#include "Wt/Chart/WCartesianChart"
#include "Wt/Chart/WChart2DRenderer"

#include "WebUtils.h"

namespace {
  const int AXIS_MARGIN = 4;
  const int AUTO_V_LABEL_PIXELS = 25;
  const int AUTO_H_LABEL_PIXELS = 80;

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

  int roundDown(int v, int factor) {
    return (v / factor) * factor;
  }
}

namespace Wt {

LOGGER("Chart.WAxis");

  namespace Chart {

class ExtremesIterator : public SeriesIterator
{
public:
  ExtremesIterator(Axis axis, AxisScale scale)
    : axis_(axis), scale_(scale),
      minimum_(DBL_MAX),
      maximum_(-DBL_MAX)
  { }

  virtual bool startSeries(const WDataSeries& series, double groupWidth,
			   int numBarGroups, int currentBarGroup)
  {
    return axis_ == XAxis || series.axis() == axis_;
  }

  virtual void newValue(const WDataSeries& series, double x, double y,
			double stackY, const WModelIndex& xIndex,
			const WModelIndex& yIndex)
  {
    double v = axis_ == XAxis ? x : y;

    if (!Utils::isNaN(v) && (scale_ != LogScale || v > 0.0)) {
      maximum_ = std::max(v, maximum_);
      minimum_ = std::min(v, minimum_);
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

const double WAxis::AUTO_MINIMUM = -DBL_MAX;
const double WAxis::AUTO_MAXIMUM = DBL_MAX;

WAxis::Segment::Segment()
  : minimum(AUTO_MINIMUM),
    maximum(AUTO_MAXIMUM),
    renderMinimum(AUTO_MINIMUM),
    renderMaximum(AUTO_MAXIMUM),
    renderLength(AUTO_MAXIMUM),
    renderStart(AUTO_MAXIMUM)
{ }

WAxis::WAxis()
  : chart_(0),
    axis_(XAxis),
    visible_(true),
    location_(MinimumValue),
    scale_(LinearScale),
    resolution_(0.0),
    labelInterval_(0),
    gridLines_(false),
    gridLinesPen_(gray),
    margin_(0),
    labelAngle_(0),
    roundLimits_(MinimumValue | MaximumValue)
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

  roundLimits_.clear(MinimumValue);
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

  roundLimits_.clear(MaximumValue);
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

    roundLimits_ = 0;

    update();
  }
}

void WAxis::setRoundLimits(WFlags<AxisValue> locations)
{
  roundLimits_ = locations;
}

void WAxis::setResolution(const double resolution)
{
  resolution_ = resolution;
  update();
}

void WAxis::setAutoLimits(WFlags<AxisValue> locations)
{
  if (locations & MinimumValue) {
    set(segments_.front().minimum, AUTO_MINIMUM);
    roundLimits_ |= MinimumValue;
  }

  if (locations & MaximumValue) {
    set(segments_.back().maximum, AUTO_MAXIMUM);
    roundLimits_ |= MaximumValue;
  }
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

bool WAxis::prepareRender(WChart2DRenderer& renderer) const
{
  double totalRenderRange = 0;

  for (unsigned i = 0; i < segments_.size(); ++i) {
    const Segment& s = segments_[i];

    computeRange(renderer, s);
    totalRenderRange += s.renderMaximum - s.renderMinimum;
  }

  bool vertical = axis_ != XAxis;

  double clipMin = segments_.front().renderMinimum == 0 ? 0 : chart_->axisPadding();
  double clipMax = segments_.back().renderMaximum == 0 ? 0 : chart_->axisPadding();

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

  if (totalRenderLength <= 0) {
    renderInterval_ = 1.0;
    return false;
  }

  int rc = 0;
  if (chart_->model())
    rc = chart_->model()->rowCount();

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

	    renderInterval_ = std::max(1.0, std::floor(rc / numLabels));
	  } else if (scale_ == LogScale) {
	    renderInterval_ = 1; // does not apply
	  } else {
	    double numLabels = calcAutoNumLabels(s);

	    renderInterval_ = round125(diff / numLabels);
	  }
	}
      }

      if (renderInterval_ == 0) {
	renderInterval_ = 1;
	return false;
      }

      if (scale_ == LinearScale) {
	if (it == 0) {
	  if (roundLimits_ & MinimumValue)
	    s.renderMinimum
	      = roundDown125(s.renderMinimum, renderInterval_);
	  
	  if (roundLimits_ & MaximumValue)
	    s.renderMaximum
	      = roundUp125(s.renderMaximum, renderInterval_);
	}
      } else if (scale_ == DateScale || scale_ == DateTimeScale) {
	double daysInterval = 0.0;

	WDateTime min, max;
	int interval;

	if (scale_ == DateScale) {
	  daysInterval = renderInterval_;
	  min = WDateTime(WDate::fromJulianDay
			  (static_cast<int>(s.renderMinimum)));
	  max = WDateTime(WDate::fromJulianDay
			  (static_cast<int>(s.renderMaximum)));
	} else if (scale_ == DateTimeScale) {
	  daysInterval = renderInterval_ / (60.0 * 60.0 * 24);
	  min = WDateTime::fromTime_t((std::time_t)s.renderMinimum);
	  max = WDateTime::fromTime_t((std::time_t)s.renderMaximum);
	}

	LOG_DEBUG("Range: " << min.toString() << ", " << max.toString());

	if (daysInterval > 200) {
	  s.dateTimeRenderUnit = Years;
	  interval = std::max(1, 
			      static_cast<int>(round125(daysInterval / 365)));

	  if (roundLimits_ & MinimumValue)
	    if (min.date().day() != 1 && min.date().month() != 1)
	      min = WDateTime(WDate(min.date().year(), 1, 1));

	  if (roundLimits_ & MaximumValue)
	    if (max.date().day() != 1 && max.date().day() != 1)
	      max = WDateTime(WDate(max.date().year() + 1, 1, 1));
	} else if (daysInterval > 20) {
	  s.dateTimeRenderUnit = Months;

	  double d = daysInterval / 30;
	  if (d < 1.3)
	    interval = 1;
	  else if (d < 2.3)
	    interval = 2;
	  else if (d < 3.3)
	    interval = 3;
	  else if (d < 4.3)
	    interval = 4;
	  else
	    interval = 6;
	
	  /* push min and max to a round month (at interval boundary) */
	  if (roundLimits_ & MinimumValue) {
	    if ((min.date().month() - 1) % interval != 0) {
	      int m = roundDown(min.date().month() - 1, interval) + 1;
	      min = WDateTime(WDate(min.date().year(), m, 1));
	    } else if (min.date().day() != 1)
	      min = WDateTime(WDate(min.date().year(), min.date().month(), 1));
	  }

	  if (roundLimits_ & MaximumValue) {
	    if (max.date().day() != 1)
	      max = WDateTime
		(WDate(max.date().year(), max.date().month(), 1).addMonths(1));

	    if ((max.date().month() - 1) % interval != 0) {
	      int m = roundDown(max.date().month() - 1, interval) + 1;
	      max = WDateTime(WDate(max.date().year(), m, 1)
			      .addMonths(interval));
	    }
	  }
	} else if (daysInterval > 0.6) {
	  s.dateTimeRenderUnit = Days;

	  if (daysInterval < 1.3)
	    interval = 1;
	  else
	    interval = 7 * std::max(1,
				    static_cast<int>((daysInterval + 5) / 7));
	} else {
	  double minutes = daysInterval * 24 * 60;

	  if (minutes > 40) {
	    s.dateTimeRenderUnit = Hours;

	    double d = minutes / 60;
	    if (d < 1.3)
	      interval = 1;
	    else if (d < 2.3)
	      interval = 2;
	    else if (d < 3.3)
	      interval = 3;
	    else if (d < 4.3)
	      interval = 4;
	    else if (d < 6.3)
	      interval = 6;
	    else
	      interval = 12;

	    /* push min and max to a round hour (at interval boundary) */
	    if (roundLimits_ & MinimumValue) {
	      if (min.time().hour() % interval != 0) {
		int h = roundDown(min.time().hour(), interval);
		min.setTime(WTime(h, 0));
	      } else if (min.time().minute() != 0)
		min.setTime(WTime(min.time().hour(), 0));
	    }

	    if (roundLimits_ & MaximumValue) {
	      if (max.time().minute() != 0) {
		max.setTime(WTime(max.time().hour(), 0));
		max = max.addSecs(60 * 60);
	      }

	      if (max.time().hour() % interval != 0) {
		int h = roundDown(max.time().hour(), interval);
		max.setTime(WTime(h, 0));
		max = max.addSecs(interval * 60 * 60);
	      }
	    }
	  } else if (minutes > 2) {
	    s.dateTimeRenderUnit = Minutes;

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

	    if (roundLimits_ & MinimumValue) {
	      /* push min and max to a round minute (at interval boundary) */
	      if (min.time().minute() % interval != 0) {
		int m = roundDown(min.time().minute(), interval);
		min.setTime(WTime(min.time().hour(), m));
	      } else if (min.time().second() != 0)
		min.setTime(WTime(min.time().hour(), min.time().minute()));
	    }

	    if (roundLimits_ & MaximumValue) {
	      if (max.time().second() != 0) {
		max.setTime(WTime(max.time().hour(), max.time().minute()));
		max = max.addSecs(60);
	      }

	      if (max.time().minute() % interval != 0) {
		int m = roundDown(max.time().minute(), interval);
		max.setTime(WTime(max.time().hour(), m));
		max = max.addSecs(interval * 60);
	      }
	    }
	  } else {
	    s.dateTimeRenderUnit = Seconds;

	    double seconds = minutes * 60;

	    if (seconds < 1.3)
	      interval = 1;
	    else if (seconds < 2.3)
	      interval = 2;
	    else if (seconds < 5.3)
	      interval = 5;
	    else if (seconds < 10.3)
	      interval = 10;
	    else if (seconds < 15.3)
	      interval = 15;
	    else if (seconds < 20.3)
	      interval = 20;
	    else
	      interval = 30;

	    /* push min and max to a round second (at interval boundary) */
	    if (roundLimits_ & MinimumValue) {
	      if (min.time().second() % interval != 0) {
		int sec = roundDown(min.time().second(), interval);
		min.setTime(WTime(min.time().hour(), min.time().minute(), sec));
	      } else if (min.time().msec() != 0)
		min.setTime(WTime(min.time().hour(), min.time().minute(),
				  min.time().second()));
	    }

	    if (roundLimits_ & MaximumValue) {
	      if (max.time().msec() != 0) {
		max.setTime(WTime(max.time().hour(), max.time().minute(),
				  max.time().second()));
		max = max.addSecs(1);
	      }

	      if (max.time().second() % interval != 0) {
		int sec = roundDown(max.time().second(), interval);
		max.setTime(WTime(max.time().hour(), max.time().minute(), sec));
		max = max.addSecs(interval);
	      }
	    }
	  }
	}

	s.dateTimeRenderInterval = interval;

	if (scale_ == DateScale) {
	  s.renderMinimum = min.date().toJulianDay();
	  s.renderMaximum = max.date().toJulianDay();
	} else if (scale_ == DateTimeScale) {
	  s.renderMinimum = min.toTime_t();
	  s.renderMaximum = max.toTime_t();
	}
      }

      totalRenderRange += s.renderMaximum - s.renderMinimum;

      if (axis_ == XAxis)
	rs += s.renderLength + SEGMENT_MARGIN;
      else
	rs -= s.renderLength + SEGMENT_MARGIN;
    }
  }
  return true;

}

void WAxis::setOtherAxisLocation(AxisValue otherLocation) const
{
  if (scale_ != LogScale) {
    for (unsigned i = 0; i < segments_.size(); ++i) {
      const Segment& s = segments_[i];

      int borderMin, borderMax;

      if (scale_ == CategoryScale){
	borderMax = borderMin = chart_->axisPadding();
      }else {
	borderMin = (s.renderMinimum == 0 && otherLocation == ZeroValue)
	  ? 0 : chart_->axisPadding();
	borderMax = (s.renderMinimum == 0 && otherLocation == ZeroValue)
	  ? 0 : chart_->axisPadding();
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
  int rc = 0;
  if (chart_->model())
    rc = chart_->model()->rowCount();

  if (scale_ == CategoryScale) {
    rc = std::max(1, rc);
    segment.renderMinimum = -0.5;
    segment.renderMaximum = rc - 0.5;
  } else {
    segment.renderMinimum = segment.minimum;
    segment.renderMaximum = segment.maximum;

    const bool findMinimum = segment.renderMinimum == AUTO_MINIMUM;
    const bool findMaximum = segment.renderMaximum == AUTO_MAXIMUM;

    if (findMinimum || findMaximum) {
      double minimum = std::numeric_limits<double>::max();
      double maximum = -std::numeric_limits<double>::max();

      ExtremesIterator iterator(axis_, scale_);
      renderer.iterateSeries(&iterator);

      minimum = iterator.minimum();
      maximum = iterator.maximum();

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

    if (scale_ == LogScale) {
      /*
       * For LogScale, resolution is ignored, and we always
       * show at least one log range (if it's up to us).
       */

      double minLog10 = std::log10(segment.renderMinimum);
      double maxLog10 = std::log10(segment.renderMaximum);

      if (findMinimum && findMaximum) {
	segment.renderMinimum = std::pow(10, std::floor(minLog10));
	segment.renderMaximum = std::pow(10, std::ceil(maxLog10));
	if (segment.renderMinimum == segment.renderMaximum)
	  segment.renderMaximum = std::pow(10, std::ceil(maxLog10) + 1);
      } else if (findMinimum) {
	segment.renderMinimum = std::pow(10, std::floor(minLog10));
	if (segment.renderMinimum == segment.renderMaximum)
	  segment.renderMinimum = std::pow(10, std::floor(minLog10) - 1);
      } else if (findMaximum) {
	segment.renderMaximum = std::pow(10, std::ceil(maxLog10));
	if (segment.renderMinimum == segment.renderMaximum)
	  segment.renderMaximum = std::pow(10, std::ceil(maxLog10) + 1);
      }
    } else {
      double resolution = resolution_;

      /*
       * Old behaviour, we ignore a resolution set.
       */
      if (resolution == 0) {
	if (scale_ == LinearScale)
	  resolution = std::max(1E-3,
				std::fabs(1E-3 * segment.renderMinimum));
	else if (scale_ == DateScale)
	  resolution = 1;
	else if (scale_ == DateTimeScale)
	  resolution = 120;
      }

      if (std::fabs(diff) < resolution) {
	double average = (segment.renderMaximum + segment.renderMinimum) / 2.0;

	double d = resolution;

        if (findMinimum && findMaximum) {
          segment.renderMaximum = average + d / 2.0;
          segment.renderMinimum = average - d / 2.0;
        } else if (findMinimum) {
          segment.renderMinimum = segment.renderMaximum - d;
        } else if (findMaximum) {
          segment.renderMaximum = segment.renderMinimum + d;
        }

	diff = segment.renderMaximum - segment.renderMinimum;
      }

      /*
       * Heuristic to extend range to include 0 or to span at least one
       * log
       */
      if (findMinimum && segment.renderMinimum >= 0
	  && (segment.renderMinimum - 0.50 * diff <= 0))
	segment.renderMinimum = 0;

      if (findMaximum && segment.renderMaximum <= 0
	  && (segment.renderMaximum + 0.50 * diff >= 0))
	segment.renderMaximum = 0;
    }
  }

  assert(segment.renderMinimum < segment.renderMaximum);
}

double WAxis::mapToDevice(const boost::any& value, int segment) const
{
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
	std::sprintf(buf, "%.4g", u+1);
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
      std::sprintf(buf, format.c_str(), u);

    text = WString::fromUTF8(buf);
  }

  return text;
}

void WAxis::getLabelTicks(WChart2DRenderer& renderer,
			  std::vector<TickLabel>& ticks, int segment) const
{
  static double EPSILON = 1E-3;

  const Segment& s = segments_[segment];

  int rc = 0;
  if (chart_->model())
    rc = chart_->model()->rowCount();

  switch (scale_) {
  case CategoryScale: {
    int renderInterval = std::max(1, static_cast<int>(renderInterval_));
    if (renderInterval == 1) {
      ticks.push_back(TickLabel(-0.5, TickLabel::Long));
      for (int i = 0; i < rc; ++i) {
	ticks.push_back(TickLabel(i + 0.5, TickLabel::Long));
	ticks.push_back(TickLabel(i, TickLabel::Zero,
				  label(static_cast<double>(i))));
      }
    } else {
      /*
       * We could do a special effort for date X series here...
       */
      for (int i = 0; i < rc; i += renderInterval) {
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
    WDateTime dt;

    if (scale_ == DateScale) {
      dt.setDate(WDate::fromJulianDay(static_cast<int>(s.renderMinimum)));
      if (!dt.isValid()) {
	std::string exception = "Invalid julian day: "
	  + boost::lexical_cast<std::string>(s.renderMinimum);
	throw WException(exception);
      }
    } else
      dt = WDateTime::fromTime_t((std::time_t)s.renderMinimum);

    int interval = s.dateTimeRenderInterval;
    DateTimeUnit unit = s.dateTimeRenderUnit;

    bool atTick = (interval > 1) ||
      (unit <= Days) || 
      !(roundLimits_ & MinimumValue);

    WString format = labelFormat_;

    if (format.empty()) {
      if (atTick) {
	switch (unit) {
	case Months:
	case Years:
	case Days:
	  if (dt.time().second() != 0)
	    format = WString::fromUTF8("dd/MM/yy hh:mm:ss");
	  else if (dt.time().hour() != 0)
	    format = WString::fromUTF8("dd/MM/yy hh:mm");
	  else
	    format = WString::fromUTF8("dd/MM/yy");

	  break;
	case Hours:
	  if (dt.time().second() != 0)
	    format = WString::fromUTF8("dd/MM hh:mm:ss");
	  else if (dt.time().minute() != 0)
	    format = WString::fromUTF8("dd/MM hh:mm");
	  else
	    format = WString::fromUTF8("h'h' dd/MM");

	  break;
	case Minutes:
	  if (dt.time().second() != 0)
	    format = WString::fromUTF8("hh:mm:ss");
	  else
	    format = WString::fromUTF8("hh:mm");

	  break;
	case Seconds:
	  format = WString::fromUTF8("hh:mm:ss");

	  break;
	}
      } else {
	switch (unit) {
	case Years:
	  format = WString::fromUTF8("yyyy"); break;
	case Months:
	  format = WString::fromUTF8("MMM yy"); break;
	case Days:
	  format = WString::fromUTF8("dd/MM/yy"); break;
	case Hours:
	  format = WString::fromUTF8("h'h' dd/MM"); break;
	case Minutes:
	  format = WString::fromUTF8("hh:mm"); break;
	case Seconds:
	  format = WString::fromUTF8("hh:mm:ss"); break;
	default:
	  break;
	}
      }
    }

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
      case Seconds:
	next = dt.addSecs(interval); break;
      }

      WString text = dt.toString(format);

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
