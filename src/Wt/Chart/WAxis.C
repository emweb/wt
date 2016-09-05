/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cmath>
#include <limits>
#include <cstdio>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "Wt/WAbstractItemModel"
#include "Wt/WColor"
#include "Wt/WDate"
#include "Wt/WException"
#include "Wt/WLogger"
#include "Wt/WPainter"
#include "Wt/WPainterPath"
#include "Wt/WRectF"
#include "Wt/WTime"
#include "Wt/WMeasurePaintDevice"

#include "Wt/Chart/WAbstractChartImplementation"
#include "Wt/Chart/WAxis"
#include "Wt/Chart/WCartesianChart"

#include "WebUtils.h"

namespace {
  const int AUTO_V_LABEL_PIXELS = 25;
  const int AUTO_H_LABEL_PIXELS = 80;

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

  int roundUp(int v, int factor) {
    return ((v - 1) / factor + 1) * factor;
  }

  Wt::WPointF interpolate(const Wt::WPointF& p1, const Wt::WPointF& p2,
			  double u) {
    double x = p1.x();
    if (p2.x() - p1.x() > 0)
      x += u;
    else if (p2.x() - p1.x() < 0)
      x -= u;

    double y = p1.y();
    if (p2.y() - p1.y() > 0)
      y += u;
    else if (p2.y() - p1.y() < 0)
      y -= u;

    return Wt::WPointF(x, y);
  }

  class TildeStartMarker : public Wt::WPainterPath {
  public:
    TildeStartMarker(int segmentMargin) {
      moveTo(0, 0);
      lineTo(0, segmentMargin - 25);
      moveTo(-15, segmentMargin - 10);
      lineTo(15, segmentMargin - 20);
    }
  };

  class TildeEndMarker : public Wt::WPainterPath {
  public:
    TildeEndMarker(int segmentMargin) {
      moveTo(0, 0);
      lineTo(0, -(segmentMargin - 25));
      moveTo(-15, -(segmentMargin - 20));
      lineTo(15, -(segmentMargin - 10));
    }
  };
}

namespace Wt {

LOGGER("Chart.WAxis");

  namespace Chart {

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
    renderStart(AUTO_MAXIMUM),
    dateTimeRenderUnit(Days),
    dateTimeRenderInterval(0)
{ }

WAxis::Segment::Segment(const Segment &other)
  : minimum(other.minimum),
    maximum(other.maximum),
    renderMinimum(other.renderMinimum),
    renderMaximum(other.renderMaximum),
    renderLength(other.renderLength),
    renderStart(other.renderStart),
    dateTimeRenderUnit(other.dateTimeRenderUnit),
    dateTimeRenderInterval(other.dateTimeRenderInterval)
{ }

WAxis::WAxis()
  : chart_(0),
    axis_(XAxis),
    visible_(true),
    location_(MinimumValue),
    scale_(LinearScale),
    resolution_(0.0),
    labelInterval_(0),
    labelBasePoint_(0),
    defaultLabelFormat_(true),
    gridLines_(false),
    gridLinesPen_(gray),
    margin_(0),
    labelAngle_(0),
    roundLimits_(MinimumValue | MaximumValue),
    segmentMargin_(40),
    titleOffset_(0),
    textPen_(black),
    titleOrientation_(Horizontal),
    maxZoom_(4.0),
    minimumZoomRange_(AUTO_MINIMUM),
    zoomMin_(AUTO_MINIMUM),
    zoomMax_(AUTO_MAXIMUM),
    zoomRangeDirty_(true),
    padding_(0),
    tickDirection_(Outwards),
    partialLabelClipping_(true),
    inverted_(false),
    renderingMirror_(false)
{
  titleFont_.setFamily(WFont::SansSerif, "Arial");
  titleFont_.setSize(WFont::FixedSize, WLength(12, WLength::Point));
  labelFont_.setFamily(WFont::SansSerif, "Arial");
  labelFont_.setSize(WFont::FixedSize, WLength(10, WLength::Point));

  segments_.push_back(Segment());
}

WAxis::~WAxis()
{ }

void WAxis::init(WAbstractChartImplementation* chart,
		 Axis axis)
{
  chart_ = chart;
  axis_ = axis;

  if (axis == XAxis || axis_ == XAxis_3D || axis_ == YAxis_3D) {
    if (chart_->chartType() == CategoryChart) {
      scale_ = CategoryScale;
    } else if (scale_ == CategoryScale)
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
  update();
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
  update();
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

void WAxis::setLabelBasePoint(double labelBasePoint)
{
  set(labelBasePoint_, labelBasePoint);
}

void WAxis::setLabelFormat(const WString& format)
{
  set(labelFormat_, format);
  defaultLabelFormat_ = false;
}

WString WAxis::labelFormat() const
{
  switch (scale_) {
  case CategoryScale:
    return WString();
  case DateScale:
  case DateTimeScale:
    if (defaultLabelFormat_) {
      if (!segments_.empty()) {
	const Segment& s = segments_[0];
	return defaultDateTimeFormat(s);
      } else {
	return labelFormat_;
      }
    } else
      return labelFormat_;
  default:
    return defaultLabelFormat_ ? WString::fromUTF8("%.4g") : labelFormat_;
  }
}

void WAxis::setLabelAngle(double angle)
{
  if (renderingMirror_)
    labelAngle_ = angle;
  else
    set(labelAngle_, angle);
}
  
void WAxis::setTitleOrientation(const Orientation& orientation)
{
  set(titleOrientation_, orientation);
}

void WAxis::setGridLinesEnabled(bool enabled)
{
  set(gridLines_, enabled);
}

void WAxis::setPen(const WPen& pen)
{
  set(pen_, pen);
}

void WAxis::setTextPen(const WPen& pen)
{
  set(textPen_, pen);
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

double WAxis::calcTitleSize(WPaintDevice *d, Orientation orientation) const 
{
  WMeasurePaintDevice device(d);

  WPainter painter(&device);

  // Set Painter props
  painter.setFont(titleFont_);

  painter.drawText( 0, 0, 100, 100, AlignCenter, title());
  
  return orientation == Vertical ? 
    device.boundingRect().height() : device.boundingRect().width();


}

double WAxis::calcMaxTickLabelSize(WPaintDevice *d, Orientation orientation)
  const
{
  WMeasurePaintDevice device(d);

  WPainter painter(&device);

  // Set Painter props
  painter.setFont(labelFont_);

  std::vector<TickLabel> ticks;

  // Get all the ticks for the axis
  for(int i = 0; i< segmentCount(); ++i) {
    AxisConfig cfg;
    cfg.zoomLevel = 1;
    if (location() == MinimumValue || location() == BothSides) {
      cfg.side = MinimumValue;
      getLabelTicks(ticks, i, cfg);
    }
    if (location() == MaximumValue || location() == BothSides) {
      cfg.side = MaximumValue;
      getLabelTicks(ticks, i, cfg);
    }
    if (location() == ZeroValue) {
      cfg.side = ZeroValue;
      getLabelTicks(ticks, i, cfg);
    }
  }

  painter.rotate(-labelAngle_);
  for (unsigned int i = 0; i < ticks.size(); ++i) {
    painter.drawText(0, 0, 100, 100, AlignRight, ticks[i].label);
  }

  return orientation == Vertical ? 
    device.boundingRect().height() : device.boundingRect().width();
}

void WAxis::update()
{
  if (chart_)
    chart_->update();
}

bool WAxis::prepareRender(Orientation orientation, double length) const
{
  fullRenderLength_ = length;
  double totalRenderRange = 0;

  for (unsigned i = 0; i < segments_.size(); ++i) {
    const Segment& s = segments_[i];
    computeRange(s);
    totalRenderRange += s.renderMaximum - s.renderMinimum;
  }

  double clipMin = 0;
  double clipMax = 0;
  if (scale_ == CategoryScale || scale_ == LogScale) {
    clipMin = clipMax = padding();
  } else {
    if (inverted()) {
      clipMin = segments_.back().renderMaximum == 0 ?
	  0 : padding();
      clipMax = segments_.front().renderMinimum == 0 ?
	  0 : padding();
    } else {
      clipMin = segments_.front().renderMinimum == 0 ?
	  0 : padding();
      clipMax = segments_.back().renderMaximum == 0 ?
	  0 : padding();
    }
  }

  double totalRenderLength = length;
  double totalRenderStart = clipMin;
  
  const double SEGMENT_MARGIN = 40;

  // 6 pixels additional margin to avoid clipping lines that render
  // the extreme values
  totalRenderLength
    -= SEGMENT_MARGIN * (segments_.size() - 1) + clipMin + clipMax;

  if (totalRenderLength <= 0) {
    renderInterval_ = 1.0;
    return false;
  }

  /*
   * Iterate twice, since we adjust the render extrema based on the size
   * and vice-versa
   */
  unsigned numIterations = 2;
  for (unsigned it = 0; it < numIterations; ++it) {
    double rs = totalRenderStart; 
    double TRR = totalRenderRange;
    totalRenderRange = 0;

    for (unsigned i = 0; i < segments_.size(); ++i) {
      const Segment& s = segments_[i];

      bool roundMinimumLimit = i == 0 && roundLimits_ & MinimumValue;
      bool roundMaximumLimit = i == segments_.size() - 1 && roundLimits_ & MaximumValue;

      double diff = s.renderMaximum - s.renderMinimum;
      s.renderStart = rs;
      s.renderLength = diff / TRR * totalRenderLength;

      if (i == 0 && it != 2) {
	double oldRenderInterval = renderInterval_;
	renderInterval_ = labelInterval_;
	if (renderInterval_ == 0) {
	  if (scale_ == CategoryScale) {
	    double numLabels = calcAutoNumLabels(orientation, s) / 1.5;
	    int rc = chart_->numberOfCategories(axis_);
	    renderInterval_ = std::max(1.0, std::floor(rc / numLabels));
	  } else if (scale_ == LogScale) {
	    renderInterval_ = 1; // does not apply
	  } else {
	    double numLabels = calcAutoNumLabels(orientation, s);

	    renderInterval_ = round125(diff / numLabels);

	    if (it == 1 && renderInterval_ != oldRenderInterval) {
	      // If render interval changes in the second iteration,
	      // iterate once more
	      numIterations = 3;
	    }
	  }
	}
      }

      if (renderInterval_ == 0) {
	renderInterval_ = 1;
	return false;
      }

      if (scale_ == LinearScale) {
	if (it < numIterations - 1) {
	  if (roundMinimumLimit) {
	    s.renderMinimum
	      = roundDown125(s.renderMinimum, renderInterval_);

	    if (s.renderMinimum <= labelBasePoint_ &&
		s.renderMaximum >= labelBasePoint_) {
	      double interv = labelBasePoint_ - s.renderMinimum;
	      interv = renderInterval_ * 2
		* std::ceil(interv / (renderInterval_ * 2));
	      s.renderMinimum = labelBasePoint_ - interv;
	    }
	  }
	  
	  if (roundMaximumLimit)
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

	  if (roundMinimumLimit)
	    if (min.date().day() != 1 && min.date().month() != 1)
	      min = WDateTime(WDate(min.date().year(), 1, 1));

	  if (roundMaximumLimit)
	    if (max.date().day() != 1 && max.date().month() != 1)
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
	  if (roundMinimumLimit) {
	    if ((min.date().month() - 1) % interval != 0) {
	      int m = roundDown(min.date().month() - 1, interval) + 1;
	      min = WDateTime(WDate(min.date().year(), m, 1));
	    } else if (min.date().day() != 1)
	      min = WDateTime(WDate(min.date().year(), min.date().month(), 1));
	  }

	  if (roundMaximumLimit) {
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

	  if (daysInterval < 1.3) {
	    interval = 1;

	    /* push min and max to midnight */
	    if (roundMinimumLimit)
	      min.setTime(WTime(0, 0));

	    if (roundMaximumLimit) {
	      if (max.time() != WTime(0, 0))
		max = WDateTime(max.date().addDays(1));
	    }
	  } else {
	    interval = 7 * std::max(1,
				    static_cast<int>((daysInterval + 5) / 7));
	   
	    /* push min to midnight start of the week */
	    if (roundMinimumLimit) {
	      int dw = min.date().dayOfWeek();
	      min = WDateTime(min.date().addDays(-(dw - 1)));
	    }

	    /*
	      push max to midgnight start of the week, at interval days
	      from min
	     */
	    if (roundMaximumLimit) {
	      int days = min.date().daysTo(max.date());
	      if (max.time() != WTime(0, 0))
		++days;

	      days = roundUp(days, interval);
	      
	      max = WDateTime(min.addDays(days));
	    }	    
	  }

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
	    if (roundMinimumLimit) {
	      if (min.time().hour() % interval != 0) {
		int h = roundDown(min.time().hour(), interval);
		min.setTime(WTime(h, 0));
	      } else if (min.time().minute() != 0)
		min.setTime(WTime(min.time().hour(), 0));
	    }

	    if (roundMaximumLimit) {
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
	  } else if (minutes > 0.8) {
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

	    if (roundMinimumLimit) {
	      /* push min and max to a round minute (at interval boundary) */
	      if (min.time().minute() % interval != 0) {
		int m = roundDown(min.time().minute(), interval);
		min.setTime(WTime(min.time().hour(), m));
	      } else if (min.time().second() != 0)
		min.setTime(WTime(min.time().hour(), min.time().minute()));
	    }

	    if (roundMaximumLimit) {
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
	    if (roundMinimumLimit) {
	      if (min.time().second() % interval != 0) {
		int sec = roundDown(min.time().second(), interval);
		min.setTime(WTime(min.time().hour(), min.time().minute(), sec));
	      } else if (min.time().msec() != 0)
		min.setTime(WTime(min.time().hour(), min.time().minute(),
				  min.time().second()));
	    }

	    if (roundMaximumLimit) {
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
      rs += s.renderLength + SEGMENT_MARGIN;
    }
  }

  return true;
}

void WAxis::computeRange(const Segment& segment) const
{
  segment.renderMinimum = segment.minimum;
  segment.renderMaximum = segment.maximum;

  const bool findMinimum = segment.renderMinimum == AUTO_MINIMUM;
  const bool findMaximum = segment.renderMaximum == AUTO_MAXIMUM;

  if (scale_ == CategoryScale) {
    int rc = chart_->numberOfCategories(axis_);
    rc = std::max(1, rc);
    if (findMinimum)
      segment.renderMinimum = -0.5;
    if (findMaximum)
      segment.renderMaximum = rc - 0.5;
  } else {
    if (findMinimum || findMaximum) {
      double minimum = std::numeric_limits<double>::max();
      double maximum = -std::numeric_limits<double>::max();

      WAbstractChartImplementation::RenderRange rr =
	chart_->computeRenderRange(axis_, scale_);
      minimum = rr.minimum;
      maximum = rr.maximum;

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

double WAxis::mapToDevice(const boost::any &value) const
{
  return mapToDevice(getValue(value));
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

double WAxis::mapToDevice(double value) const
{
  if (Utils::isNaN(value))
      return value;

  for (int i = 0; i < segments_.size(); ++i) {
    if (value <= segments_[i].renderMaximum ||
	i == segments_.size() - 1) {
      return mapToDevice(value, i);
    }
  }

  assert(false);
  return std::numeric_limits<double>::signaling_NaN();
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

  if (inverted()) {
    const Segment& firstSegment = segments_[0];
    const Segment& lastSegment = segments_[segments_.size() - 1];
    return lastSegment.renderStart + lastSegment.renderLength - (s.renderStart + d) + firstSegment.renderStart;
  } else {
    return s.renderStart + d;
  }
}

bool WAxis::isOnAxis(double d) const
{
  for (int i = 0; i < segments_.size(); ++i) {
    if (d >= segments_[i].renderMinimum &&
	d <= segments_[i].renderMaximum) {
      return true;
    }
  }
  return false;
}

double WAxis::mapFromDevice(double d) const
{
  const Segment& firstSegment = segments_[0];
  const Segment& lastSegment = segments_[segments_.size() - 1];
  if (inverted()) {
    d = lastSegment.renderStart + lastSegment.renderLength - d + firstSegment.renderStart;
  }
  for (unsigned i = 0; i < segments_.size(); ++i) {
    const Segment& s = segments_[i];

    bool isLastSegment = (i == segments_.size() - 1);

    if (isLastSegment || (!inverted() && d < mapToDevice(s.renderMaximum, i)) ||
	(inverted() && d < - (mapToDevice(s.renderMaximum, i) - lastSegment.renderStart - lastSegment.renderLength - firstSegment.renderStart))) {
      d = d - s.renderStart;

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
    text = chart_->categoryLabel((int)u, axis_);
    if (text.empty())
      text = WLocale::currentLocale().toString(u);
  } else if (scale_ == DateScale) {
    WDate d = WDate::fromJulianDay(static_cast<int>(u));
    WString format = labelFormat();
    return d.toString(format);
  } else {
    std::string format = labelFormat().toUTF8();

    if (format.empty())
      text = WLocale::currentLocale().toString(u);
    else {
#ifdef WT_TARGET_JAVA
      buf =
#endif // WT_TARGET_JAVA
	std::sprintf(buf, format.c_str(), u);

      text = WString::fromUTF8(buf);
    }
  }

  return text;
}

double WAxis::drawnMinimum() const
{
  if (!inverted()) {
    return mapFromDevice(0.0);
  } else {
    return mapFromDevice(fullRenderLength_);
  }
}

double WAxis::drawnMaximum() const
{
  if (!inverted()) {
    return mapFromDevice(fullRenderLength_);
  } else {
    return mapFromDevice(0.0);
  }
}

void WAxis::setZoomRange(double minimum, double maximum)
{
  if (maximum < minimum) {
    double temp = maximum;
    maximum = minimum;
    minimum = temp;
  }
  if (minimum <= this->minimum()) {
    minimum = AUTO_MINIMUM;
  }
  if (maximum >= this->maximum()) {
    maximum = AUTO_MAXIMUM;
  }
  if (minimum != AUTO_MINIMUM &&
      maximum != AUTO_MAXIMUM &&
      (maximum - minimum) < minimumZoomRange()) {
    minimum = (minimum + maximum) / 2.0 - minimumZoomRange() / 2.0;
    maximum = (minimum + maximum) / 2.0 + minimumZoomRange() / 2.0;
  }
  set(zoomMin_, minimum);
  set(zoomMax_, maximum);
  zoomRangeDirty_ = true;
}

double WAxis::zoomMinimum() const
{
  double min = drawnMinimum();
  if (zoomMin_ <= min) {
    return min;
  }
  return zoomMin_;
}

double WAxis::zoomMaximum() const
{
  double max = drawnMaximum();
  if (zoomMax_ >= max) {
    return max;
  }
  return zoomMax_;
}

void WAxis::setZoom(double zoom)
{
  double min = drawnMinimum();
  double max = drawnMaximum();
  setZoomRange(zoomMinimum(), zoomMinimum() + (max - min) / zoom);
}

double WAxis::zoom() const
{
  if (zoomMin_ == AUTO_MINIMUM && zoomMax_ == AUTO_MAXIMUM) {
    return 1.0;
  }
  double min = drawnMinimum();
  double max = drawnMaximum();
  return (max - min) / (zoomMaximum() - zoomMinimum());
}

void WAxis::setPan(double pan)
{
  setZoomRange(pan, zoomMaximum() + pan - zoomMinimum());
}

double WAxis::pan() const
{
  if (!inverted()) {
    return zoomMinimum();
  } else {
    return zoomMaximum();
  }
}

void WAxis::setZoomRangeFromClient(double minimum, double maximum)
{
  if (minimum > maximum) {
    double temp = minimum;
    minimum = maximum;
    maximum = temp;
  }
  double min = drawnMinimum();
  double max = drawnMaximum();
  double zoom = (max - min) / (maximum - minimum);
  if (minimum <= min || !(zoom > 1.01)) {
    minimum = AUTO_MINIMUM;
  }
  if (maximum >= max || !(zoom > 1.01)) {
    maximum = AUTO_MAXIMUM;
  }
  zoomMin_ = minimum;
  zoomMax_ = maximum;
}

void WAxis::setPadding(int padding)
{
  set(padding_, padding);
}

void WAxis::setTickDirection(TickDirection direction)
{
  if (direction == Inwards) {
    setPadding(25);
  }
  set(tickDirection_, direction);
}

void WAxis::setSoftLabelClipping(bool enabled)
{
  set(partialLabelClipping_, !enabled);
}

void WAxis::setMaxZoom(double maxZoom)
{
  if (maxZoom < 1)
    maxZoom = 1;
  if (minimumZoomRange_ != AUTO_MINIMUM) {
    setMinimumZoomRange((maximum() - minimum()) / maxZoom);
  }
  set(maxZoom_, maxZoom);
}

double WAxis::maxZoom() const
{
  double min = drawnMinimum();
  double max = drawnMaximum();
  double zoom = (max - min) / minimumZoomRange();
  if (zoom < 1.0)
    return 1.0;
  else
    return (max - min) / minimumZoomRange();
}

void WAxis::setMinimumZoomRange(double size)
{
  set(minimumZoomRange_, size);
}

double WAxis::minimumZoomRange() const
{
  double min = drawnMinimum();
  double max = drawnMaximum();
  if (minimumZoomRange_ == AUTO_MINIMUM) {
    return (max - min) / maxZoom_;
  } else {
    return minimumZoomRange_;
  }
}

void WAxis::getLabelTicks(std::vector<TickLabel>& ticks, int segment, AxisConfig config) const
{
  static double EPSILON = 1E-3;
  double divisor = std::pow(2.0, config.zoomLevel - 1);

  const Segment& s = segments_[segment];

  switch (scale_) {
  case CategoryScale: {
    int renderInterval = std::max(1, 
				  static_cast<int>(renderInterval_ / divisor));
    if (renderInterval == 1) {
      ticks.push_back(TickLabel(s.renderMinimum, TickLabel::Long));
      for (int i = (int)(s.renderMinimum + 0.5); i < s.renderMaximum; ++i) {
	ticks.push_back(TickLabel(i + 0.5, TickLabel::Long));
	ticks.push_back(TickLabel(i, TickLabel::Zero,
				  label(static_cast<double>(i))));
      }
    } else {
      /*
       * We could do a special effort for date X series here...
       */
      for (int i = (int)(s.renderMinimum); i < s.renderMaximum;
	   i += renderInterval) {
	ticks.push_back(TickLabel(i, TickLabel::Long,
				  label(static_cast<double>(i))));
      }
    }
    break;
  }
  case LinearScale: {
    double interval = renderInterval_ / divisor;
    // Start labels at a round minimum
    double minimum = roundUp125(s.renderMinimum, interval);
    bool firstTickIsLong = true;
    if (labelBasePoint_ >= minimum &&
	labelBasePoint_ <= s.renderMaximum) {
      // Make sure the base point label is included as a long tick
      int n = (int)((minimum - labelBasePoint_) / (- 2.0 * interval));
      minimum = labelBasePoint_ - n * 2.0 * interval;
      if (minimum - interval >= s.renderMinimum) {
	// We can still put a short tick before the first long tick
	minimum -= interval;
	firstTickIsLong = false;
      }
    }
    for (int i = 0;; ++i) {
      double v = minimum + interval * i;

      if (v - s.renderMaximum > EPSILON * interval)
	break;

      WString t;

      if (i % 2 == (firstTickIsLong ? 0 : 1)) {
	if (hasLabelTransformOnSide(config.side)) {
#ifndef WT_TARGET_JAVA
	  t = label(labelTransform(config.side)(v));
#else
	  t = label(labelTransform(config.side).apply(v));
#endif
	} else {
	  t = label(v);
	}
      }
 
      ticks.push_back
	(TickLabel(v, i % 2 == (firstTickIsLong ? 0 : 1) ? TickLabel::Long : TickLabel::Short, t));
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

      if (i == 0) {
	WString text = label(v);
	if (hasLabelTransformOnSide(config.side)) {
#ifndef WT_TARGET_JAVA
	  text = label(labelTransform(config.side)(v));
#else
	  text = label(labelTransform(config.side).apply(v));
#endif
	}
	ticks.push_back(TickLabel(v, TickLabel::Long, text));
      } else {
	ticks.push_back(TickLabel(v, TickLabel::Short));
      }

      v += p;
    }

    break;
  }
  case DateTimeScale:
  case DateScale: {
    WString format = labelFormat();

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

    DateTimeUnit unit;
    int interval;
    if (config.zoomLevel == 1) {
      unit = s.dateTimeRenderUnit;
      interval = s.dateTimeRenderInterval;
    } else {
      // FIXME: this duplicates code in prepareRender
      double daysInterval = 0.0;
      if (scale_ == DateScale) {
	daysInterval = renderInterval_;
      } else {
	daysInterval = renderInterval_ / (60.0 * 60.0 * 24);
      }
      daysInterval /= divisor;
      if (daysInterval > 200) {
	unit = Years;
	interval = std::max(1,
	    static_cast<int>(round125(daysInterval / 365)));
      } else if (daysInterval > 20) {
	unit = Months;
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
      } else if (daysInterval > 0.6) {
	unit = Days;

	if (daysInterval < 1.3) {
	  interval = 1;
	} else {
	  interval = 7 * std::max(1,
				  static_cast<int>((daysInterval + 5) / 7));
	}
      } else {
	double minutes = daysInterval * 24 * 60;

	if (minutes > 40) {
	  unit = Hours;

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
	} else if (minutes > 0.8) {
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
	} else {
	  unit = Seconds;

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
	}
      }
    }

    bool atTick = (interval > 1) ||
      (unit <= Days) || 
      !(roundLimits_ & MinimumValue);

    for (;;) {
      long long dl = getDateNumber(dt);

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

      WString text;
      {
	WDateTime transformedDt = dt;
	if (hasLabelTransformOnSide(config.side)) {
#ifndef WT_TARGET_JAVA
	  transformedDt = WDateTime::fromTime_t(static_cast<std::time_t>(labelTransform(config.side)(static_cast<double>(dt.toTime_t()))));
#else
	  transformedDt = WDateTime::fromTime_t(static_cast<std::time_t>(labelTransform(config.side).apply(static_cast<double>(dt.toTime_t()))));
#endif
	}
	text = transformedDt.toString(format);
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

WString WAxis::autoDateFormat(const WDateTime& dt, DateTimeUnit unit, bool atTick) const 
{
  if (atTick) {
    switch (unit) {
    case Months:
    case Years:
    case Days:
      if (dt.time().second() != 0)
	return WString::fromUTF8("dd/MM/yy hh:mm:ss");
      else if (dt.time().hour() != 0)
	return WString::fromUTF8("dd/MM/yy hh:mm");
      else
	return WString::fromUTF8("dd/MM/yy");
    case Hours:
      if (dt.time().second() != 0)
	return WString::fromUTF8("dd/MM hh:mm:ss");
      else if (dt.time().minute() != 0)
	return WString::fromUTF8("dd/MM hh:mm");
      else
	return WString::fromUTF8("h'h' dd/MM");
    case Minutes:
      if (dt.time().second() != 0)
	return WString::fromUTF8("hh:mm:ss");
      else
	return WString::fromUTF8("hh:mm");
    case Seconds:
      return WString::fromUTF8("hh:mm:ss");
    }
  } else {
    switch (unit) {
    case Years:
      return WString::fromUTF8("yyyy");
    case Months:
      return WString::fromUTF8("MMM yy");
    case Days:
      return WString::fromUTF8("dd/MM/yy");
    case Hours:
      return WString::fromUTF8("h'h' dd/MM");
    case Minutes:
      return WString::fromUTF8("hh:mm");
    case Seconds:
      return WString::fromUTF8("hh:mm:ss");
    default:
      break;
    }
  }
  return WString::Empty;
}

WString WAxis::defaultDateTimeFormat(const Segment& s) const
{
  if (scale_ != DateScale && scale_ != DateTimeScale)
    return WString::Empty;

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

  return autoDateFormat(dt, unit, atTick);
}

long long WAxis::getDateNumber(WDateTime dt) const
{
  switch (scale_) {
  case DateScale:
    return static_cast<long long>(dt.date().toJulianDay());
  case DateTimeScale:
    return static_cast<long long>(dt.toTime_t());
  default:
    return 1;
  }
}


double WAxis::calcAutoNumLabels(Orientation orientation, const Segment& s) const
{
  if (orientation == Horizontal) {
    if (std::fabs(labelAngle_) <= 15) {
      return s.renderLength
	/ std::max((double)AUTO_H_LABEL_PIXELS,
		   WLength(defaultDateTimeFormat(s).value().size(),
			   WLength::FontEm).toPixels());
    } else if (std::fabs(labelAngle_) <= 40) {
      return s.renderLength / (2 * AUTO_V_LABEL_PIXELS);
    } else {
      return s.renderLength / AUTO_V_LABEL_PIXELS;
    }
  } else
    return s.renderLength / AUTO_V_LABEL_PIXELS;
}

namespace {
  static std::vector<WString> splitLabel(WString text)
  {
    std::string s = text.toUTF8();
    std::vector<std::string> splitText;
    boost::split(splitText, s, boost::is_any_of("\n"));
    std::vector<WString> result;
    for (std::size_t i = 0; i < splitText.size(); ++i) {
      result.push_back(splitText[i]);
    }
    return result;
  }

  static double calcYOffset(int lineNb,
			    int nbLines,
			    double lineHeight,
			    WFlags<AlignmentFlag> verticalAlign)
  {
    if (verticalAlign == AlignMiddle) {
      return - ((nbLines - 1) * lineHeight / 2.0) + lineNb * lineHeight;
    } else if (verticalAlign == AlignTop) {
      return lineNb * lineHeight;
    } else if (verticalAlign == AlignBottom) {
      return - (nbLines - 1 - lineNb) * lineHeight;
    } else {
      return 0;
    }
  }
}

void WAxis::render(WPainter& painter,
		   WFlags<AxisProperty> properties,
		   const WPointF& axisStart,
		   const WPointF& axisEnd,
		   double tickStart, double tickEnd, double labelPos,
		   WFlags<AlignmentFlag> labelFlags,
		   const WTransform& transform,
		   AxisValue side,
		   std::vector<WPen> pens,
		   std::vector<WPen> textPens) const
{
  WFont oldFont1 = painter.font();
  painter.setFont(labelFont_);
  

  bool vertical = axisStart.x() == axisEnd.x();

  WPointF axStart, axEnd;
  if (inverted()) {
    axStart = axisEnd;
    axEnd = axisStart;
  } else {
    axStart = axisStart;
    axEnd = axisEnd;
  }

  for (int segment = 0; segment < segmentCount(); ++segment) {
    const WAxis::Segment& s = segments_[segment];

    if (properties & Line) { 
#ifdef WT_TARGET_JAVA
      painter.setPen(WPen(pen()));
#else
      painter.setPen(pen());
#endif

      WPointF begin = interpolate(axisStart, axisEnd, mapToDevice(s.renderMinimum, segment));
      WPointF end = interpolate(axisStart, axisEnd, mapToDevice(s.renderMaximum, segment));

      {
	WPainterPath path;
	path.moveTo(begin);
	path.lineTo(end);
	painter.drawPath(transform.map(path).crisp());
      }

      bool rotate = vertical;

      if (segment != 0) {
  	painter.save();
  	painter.translate(begin);
  	if (rotate)
  	  painter.rotate(90);
  	painter.drawPath(TildeStartMarker((int)segmentMargin_));
  	painter.restore();
      }

      if (segment != segmentCount() - 1) {
  	painter.save();
  	painter.translate(end);
  	if (rotate)
  	  painter.rotate(90);
  	painter.drawPath(TildeEndMarker((int)segmentMargin_));
  	painter.restore();	
      }
    }

    if (pens.empty()) {
      pens.push_back(pen());
      textPens.push_back(textPen());
    }
    for (unsigned level = 1; level <= pens.size(); ++level) {
      WPainterPath shortTicksPath;
      WPainterPath longTicksPath;

      std::vector<WAxis::TickLabel> ticks;
      AxisConfig cfg;
      cfg.zoomLevel = level;
      cfg.side = side;
      getLabelTicks(ticks, segment, cfg);

      std::vector<WString> labels;
      WPainterPath path;
      for (unsigned i = 0; i < ticks.size(); ++i) {
	double u = mapToDevice(ticks[i].u, segment);
	WPointF p = interpolate(axisStart, axisEnd, u);

	if ((properties & Line) &&
	    ticks[i].tickLength != WAxis::TickLabel::Zero) {
	  if (ticks[i].tickLength == WAxis::TickLabel::Short) {
	    shortTicksPath.moveTo(p);
	  } else { // Long
	    longTicksPath.moveTo(p);
	  }
	}

	if ((properties & Labels) && !ticks[i].label.empty()) {
	  path.moveTo(p);
	  labels.push_back(ticks[i].label);
	}
      }
      WTransform t = vertical ? WTransform(1,0,0,1, labelPos, 0) : WTransform(1,0,0,1,0, labelPos);
      renderLabels(painter, labels, path, labelFlags, labelAngle(), 3,
		   t * transform, textPens[level-1]);

      WPen oldPen = painter.pen();
      painter.setPen(pens[level-1]);
      if (shortTicksPath.segments().size() != 0) {
	WPainterPath stencil;
	if (vertical) {
	  stencil.moveTo(tickStart / 2, 0);
	  stencil.lineTo(tickEnd / 2, 0);
	} else {
	  stencil.moveTo(0, tickStart / 2);
	  stencil.lineTo(0, tickEnd / 2);
	}
	painter.drawStencilAlongPath(stencil, transform.map(shortTicksPath).crisp(), false);
      }
      if (longTicksPath.segments().size() != 0) {
	WPainterPath stencil;
	if (vertical) {
	  stencil.moveTo(tickStart, 0);
	  stencil.lineTo(tickEnd, 0);
	} else {
	  stencil.moveTo(0, tickStart);
	  stencil.lineTo(0, tickEnd);
	}
	painter.drawStencilAlongPath(stencil, transform.map(longTicksPath).crisp(), false);
      }
      painter.setPen(oldPen);
    }
  }

  painter.setFont(oldFont1);
}

void WAxis::renderLabels(WPainter &painter,
			const std::vector<WString> &labels,
			const WPainterPath &path,
			WFlags<AlignmentFlag> flags,
			double angle, int margin,
			const WTransform &transform,
			const WPen &pen) const
{
  if (path.segments().size() == 0)
    return;
  AlignmentFlag horizontalAlign = flags & AlignHorizontalMask;
  AlignmentFlag verticalAlign = flags & AlignVerticalMask;

  double width = 1000;
  double height = 14;

  double left = 0.0;
  double top = 0.0;

  switch (horizontalAlign) {
  case AlignLeft:
    left += margin; break;
  case AlignCenter:
    left -= width/2; break;
  case AlignRight:
    left -= width + margin;
  default:
    break;
  }

  switch (verticalAlign) {
  case AlignTop:
    top += margin; break;
  case AlignMiddle:
    top -= height/2; break;
  case AlignBottom:
    top -= height + margin; break;
  default:
    break;
  }

#ifdef WT_TARGET_JAVA
  WPen oldPen = WPen(painter.pen());
  painter.setPen(WPen(pen));
#else
  WPen oldPen = painter.pen();
  painter.setPen(pen);
#endif

  double lineHeight = height;
  if (painter.device()->features() & WPaintDevice::HasFontMetrics) {
    WMeasurePaintDevice device(painter.device());
    WPainter measPainter(&device);
    measPainter.drawText(WRectF(0,0,100,100), AlignMiddle | AlignCenter, TextSingleLine, "Sfjh", 0);
    lineHeight = device.boundingRect().height();
  }

  bool clipping = painter.hasClipping();
  if (!partialLabelClipping_ && clipping && tickDirection() == Outwards && location() != ZeroValue) {
    painter.setClipping(false);
  }

  painter.drawTextOnPath(WRectF(left, top, width, height),
			 horizontalAlign | verticalAlign,
			 labels, transform,
			 path,
			 angle, lineHeight,
			 clipping && !partialLabelClipping_);

  painter.setClipping(clipping);

  painter.setPen(oldPen);
}

std::vector<double> WAxis::gridLinePositions(AxisConfig config) const
{
  std::vector<double> pos;

  for (unsigned segment = 0; segment < segments_.size(); ++segment) {
    std::vector<WAxis::TickLabel> ticks;
    getLabelTicks(ticks, segment, config);

    for (unsigned i = 0; i < ticks.size(); ++i)
      if (ticks[i].tickLength == WAxis::TickLabel::Long)
	pos.push_back(mapToDevice(ticks[i].u, segment));
  }
  
  return pos;
}

void WAxis::setInverted(bool inverted)
{
  set(inverted_, inverted);
}

void WAxis::setLabelTransform(const LabelTransform& transform, AxisValue side)
{
  labelTransforms_[side] = transform;
  update();
}

#ifndef WT_TARGET_JAVA
namespace {
  double identity(double d) { return d; }
}
#endif

bool WAxis::hasLabelTransformOnSide(AxisValue side) const
{
  return labelTransforms_.find(side) != labelTransforms_.end();
}

WAxis::LabelTransform WAxis::labelTransform(AxisValue side) const
{
  boost::unordered_map<AxisValue, LabelTransform >::const_iterator it = labelTransforms_.find(side);
  if (it != labelTransforms_.end()) {
    return it->second;
  } else {
#ifndef WT_TARGET_JAVA
    return &identity;
#else
    return IdentityLabelTransform();
#endif
  }
}

  }
}
