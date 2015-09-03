/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>
#include <cmath>

#include "Wt/Chart/WAxisSliderWidget"
#include "Wt/Chart/WChart2DImplementation"
#include "Wt/Chart/WDataSeries"
#include "Wt/Chart/WCartesianChart"
#include "Wt/Chart/WStandardPalette"

#include "Wt/WAbstractArea"
#include "Wt/WAbstractItemModel"
#include "Wt/WApplication"
#include "Wt/WCircleArea"
#include "Wt/WException"
#include "Wt/WJavaScriptHandle"
#include "Wt/WJavaScriptObjectStorage"
#include "Wt/WMeasurePaintDevice"
#include "Wt/WPainter"
#include "Wt/WPolygonArea"
#include "Wt/WRectArea"
#include "Wt/WText"

#include "DomElement.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WCartesianChart.min.js"
#endif

namespace {
const int TICK_LENGTH = 5;

inline int toZoomLevel(double zoomFactor)
{
  return ((int)(std::floor(std::log(zoomFactor) / std::log(2.0) + 0.5))) + 1;
}
}

namespace Wt {

LOGGER("WCartesianChart");

  namespace Chart {

SeriesIterator::~SeriesIterator()
{ }

void SeriesIterator::startSegment(int currentXSegment, int currentYSegment,
				  const WRectF& currentSegmentArea)
{
  currentXSegment_ = currentXSegment;
  currentYSegment_ = currentYSegment;
}

void SeriesIterator::endSegment()
{
}

bool SeriesIterator::startSeries(const WDataSeries& series, double groupWidth,
				 int numBarGroups, int currentBarGroup)
{
  return true;
}

void SeriesIterator::endSeries()
{ }

void SeriesIterator::newValue(const WDataSeries& series,
			      double x, double y, double stackY,
			      const WModelIndex& xIndex,
			      const WModelIndex& yIndex)
{ }


void SeriesIterator::setPenColor(WPen& pen,
				 const WModelIndex& xIndex,
				 const WModelIndex& yIndex,
				 int colorRole)
{
  boost::any color;

  if (yIndex.isValid())
    color = yIndex.data(colorRole);

  if (color.empty() && xIndex.isValid())
    color = xIndex.data(colorRole);

  if (!color.empty())
    pen.setColor(boost::any_cast<WColor>(color));
}

void SeriesIterator::setBrushColor(WBrush& brush,
				   const WModelIndex& xIndex,
				   const WModelIndex& yIndex,
				   int colorRole) 
{
  boost::any color;

  if (yIndex.isValid())
    color = yIndex.data(colorRole);

  if (color.empty() && xIndex.isValid())
    color = xIndex.data(colorRole);

  if (!color.empty())
    brush.setColor(boost::any_cast<WColor>(color));
}

class SeriesRenderer;

class SeriesRenderIterator : public SeriesIterator
{
public:
  SeriesRenderIterator(const WCartesianChart& chart, WPainter& painter);

  virtual void startSegment(int currentXSegment, int currentYSegment,
			    const WRectF& currentSegmentArea);
  virtual void endSegment();

  virtual bool startSeries(const WDataSeries& series, double groupWidth,
			   int numBarGroups, int currentBarGroup);
  virtual void endSeries();

  virtual void newValue(const WDataSeries& series, double x, double y,
			double stackY,
			const WModelIndex& xIndex,
			const WModelIndex& yIndex);

  double breakY(double y);

private:
  const WCartesianChart& chart_;
  WPainter& painter_;
  const WDataSeries *series_;
  SeriesRenderer *seriesRenderer_;
  double minY_, maxY_;
};

class SeriesRenderer {
public:
  virtual ~SeriesRenderer() { }
  virtual void addValue(double x, double y, double stacky,
			const WModelIndex& xIndex, const WModelIndex& yIndex)
    = 0;
  virtual void paint() = 0;

protected:
  const WCartesianChart& chart_;
  WPainter& painter_;
  const WDataSeries& series_;
  SeriesRenderIterator& it_;

  SeriesRenderer(const WCartesianChart& chart, WPainter& painter,
		 const WDataSeries& series, SeriesRenderIterator& it)
    : chart_(chart),
      painter_(painter),
      series_(series),
      it_(it)
  { }

  WPointF hv(const WPointF& p) {
    return chart_.hv(p);
  }

  WPointF hv(double x, double y) {
    return chart_.hv(x, y);
  }
};

class LineSeriesRenderer : public SeriesRenderer {
public:
  LineSeriesRenderer(const WCartesianChart& chart, WPainter& painter,
		     const WDataSeries& series,
		     SeriesRenderIterator& it)
    : SeriesRenderer(chart, painter, series, it),
      curveLength_(0)
  { }

  virtual void addValue(double x, double y, double stacky,
			const WModelIndex& xIndex, const WModelIndex& yIndex) {
    WPointF p = chart_.map(x, y, series_.axis(),
			   it_.currentXSegment(), it_.currentYSegment());

    if (curveLength_ == 0) {
      curve_.moveTo(hv(p));

      if (series_.fillRange() != NoFill
	  && series_.brush() != NoBrush) {
	fill_.moveTo(hv(fillOtherPoint(x)));
	fill_.lineTo(hv(p));
      }
    } else {
      if (series_.type() == LineSeries) {
	curve_.lineTo(hv(p));
	fill_.lineTo(hv(p));
      } else {
	if (curveLength_ == 1) {
	  computeC(p0, p, c_);
	} else {
	  WPointF c1, c2;
	  computeC(p_1, p0, p, c1, c2);
	  curve_.cubicTo(hv(c_), hv(c1), hv(p0));
	  fill_.cubicTo(hv(c_), hv(c1), hv(p0));
	  c_ = c2;
	}
      }
    }

    p_1 = p0;
    p0 = p;
    lastX_ = x;
    ++curveLength_;
  }

  virtual void paint() {
    WCartesianChart &chart = const_cast<WCartesianChart &>(chart_);
    WJavaScriptHandle<WPainterPath>& curveHandle = chart.curvePaths_[series_.modelColumn()];

    WTransform transform = chart.combinedTransform();

    if (curveLength_ > 1) {
      if (series_.type() == CurveSeries) {
	WPointF c1;
	computeC(p0, p_1, c1);
	curve_.cubicTo(hv(c_), hv(c1), hv(p0));
	fill_.cubicTo(hv(c_), hv(c1), hv(p0));
      }

      if (series_.fillRange() != NoFill
	  && series_.brush() != NoBrush) {
	fill_.lineTo(hv(fillOtherPoint(lastX_)));
	fill_.closeSubPath();
	painter_.setShadow(series_.shadow());
	painter_.fillPath(transform.map(fill_), series_.brush());
      }

      if (series_.fillRange() == NoFill)
	painter_.setShadow(series_.shadow());
      else
	painter_.setShadow(WShadow());

      curveHandle.setValue(curve_);
      painter_.strokePath(transform.map(curveHandle.value()), series_.pen());
    }

    curveLength_ = 0;
    curve_ = WPainterPath();
    fill_ = WPainterPath();
  }

private:
  int curveLength_;
  WPainterPath curve_;
  WPainterPath fill_;

  double  lastX_;
  WPointF p_1, p0, c_;

  static double dist(const WPointF& p1, const WPointF& p2) {
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return std::sqrt (dx*dx + dy*dy);
  }

  static void computeC(const WPointF& p, const WPointF& p1, WPointF& c) {
    c.setX(p.x() + 0.3 * (p1.x() - p.x()));
    c.setY(p.y() + 0.3 * (p1.y() - p.y()));
  }

  static void computeC(const WPointF& p_1, const WPointF& p0,
		       const WPointF& p1,
		       WPointF& c1, WPointF& c2) {
    double m1x = (p_1.x() + p0.x())/2.0;
    double m1y = (p_1.y() + p0.y())/2.0;

    double m2x = (p0.x() + p1.x())/2.0;
    double m2y = (p0.y() + p1.y())/2.0;

    double L1 = dist(p_1, p0);
    double L2 = dist(p0, p1);
    double r = L1/(L1 + L2);

    c1.setX(p0.x() - r * (m2x - m1x));
    c1.setY(p0.y() - r * (m2y - m1y));

    r = 1-r;

    c2.setX(p0.x() - r * (m1x - m2x));
    c2.setY(p0.y() - r * (m1y - m2y));
  }

  WPointF fillOtherPoint(double x) const {
    FillRangeType fr = series_.fillRange();

    switch (fr) {
    case MinimumValueFill:
      return WPointF(chart_.map(x, 0, series_.axis(),
				it_.currentXSegment(),
				it_.currentYSegment()).x(),
		     chart_.chartArea_.bottom());
    case MaximumValueFill:
      return WPointF(chart_.map(x, 0, series_.axis(),
				it_.currentXSegment(),
				it_.currentYSegment()).x(),
		     chart_.chartArea_.top());
    case ZeroValueFill:
      return WPointF(chart_.map(x, 0, series_.axis(),
				it_.currentXSegment(),
				it_.currentYSegment()));
    default:
      return WPointF();
    }
  }
};

class BarSeriesRenderer : public SeriesRenderer {
public:
  BarSeriesRenderer(const WCartesianChart& chart, WPainter& painter,
		    const WDataSeries& series,
		    SeriesRenderIterator& it,
		    double groupWidth, int numGroups, int group)
    : SeriesRenderer(chart, painter, series, it),
      groupWidth_(groupWidth),
      numGroups_(numGroups),
      group_(group)
  { }

  virtual void addValue(double x, double y, double stacky,
			const WModelIndex& xIndex, const WModelIndex& yIndex) {
    WPainterPath bar;
    const WAxis& yAxis = chart_.axis(series_.axis());

    WPointF topMid = chart_.map(x, y, yAxis.id(),
				it_.currentXSegment(),
				it_.currentYSegment());
    WPointF bottomMid = chart_.map(x, stacky, yAxis.id(),
				   it_.currentXSegment(),
				   it_.currentYSegment());

    FillRangeType fr = series_.fillRange();
    switch (fr) {
    case MinimumValueFill:
      bottomMid = WPointF(chart_.map(x, stacky, yAxis.id(),
				     it_.currentXSegment(),
				     it_.currentYSegment()).x(),
			  chart_.chartArea_.bottom());
      break;
    case MaximumValueFill:
      bottomMid = WPointF(chart_.map(x, stacky, yAxis.id(),
				     it_.currentXSegment(),
				     it_.currentYSegment()).x(),
			  chart_.chartArea_.top());
      break;
    default:
      break;
    }

    double g = numGroups_ + (numGroups_ - 1) * chart_.barMargin();

    double width = groupWidth_ / g;
    double left = topMid.x() - groupWidth_ / 2
      + group_ * width * (1 + chart_.barMargin());

    bar.moveTo(hv(left, topMid.y()));
    bar.lineTo(hv(left + width, topMid.y()));
    bar.lineTo(hv(left + width, bottomMid.y()));
    bar.lineTo(hv(left, bottomMid.y()));
    bar.closeSubPath();

    painter_.setShadow(series_.shadow());

    WCartesianChart &chart = const_cast<WCartesianChart &>(chart_);
    WTransform transform = chart.combinedTransform();

    WBrush brush = WBrush(series_.brush());
    SeriesIterator::setBrushColor(brush, xIndex, yIndex, BarBrushColorRole);
    painter_.fillPath(transform.map(bar).crisp(), brush);

    painter_.setShadow(WShadow());

    WPen pen = WPen(series_.pen());
    SeriesIterator::setPenColor(pen, xIndex, yIndex, BarPenColorRole);
    painter_.strokePath(transform.map(bar).crisp(), pen);

    boost::any toolTip = yIndex.data(ToolTipRole);
    if (!toolTip.empty()) {
      WTransform t = painter_.worldTransform();

      WPointF tl = t.map(segmentPoint(bar, 0));
      WPointF tr = t.map(segmentPoint(bar, 1));
      WPointF br = t.map(segmentPoint(bar, 2));
      WPointF bl = t.map(segmentPoint(bar, 3));

      double tlx = 0, tly = 0, brx = 0, bry = 0;
      bool useRect = false;
      if (fequal(tl.y(), tr.y())) {
	tlx = std::min(tl.x(), tr.x());
	brx = std::max(tl.x(), tr.x());
	tly = std::min(tl.y(), bl.y());
	bry = std::max(tl.y(), br.y());

	useRect = true;
      } else if (fequal(tl.x(), tr.x())) {
	tlx = std::min(tl.x(), bl.x());
	brx = std::max(tl.x(), bl.x());
	tly = std::min(tl.y(), tr.y());
	bry = std::max(tl.y(), tr.y());

	useRect = true;
      }

      WAbstractArea *area;
      if (useRect)
	area = new WRectArea(tlx, tly, (brx - tlx), (bry - tly));
      else {
	WPolygonArea *poly = new WPolygonArea();
	poly->addPoint(tl.x(), tl.y());
	poly->addPoint(tr.x(), tr.y());
	poly->addPoint(br.x(), br.y());
	poly->addPoint(bl.x(), bl.y());
	area = poly;
      }

      area->setToolTip(asString(toolTip));

      const_cast<WCartesianChart&>(chart_)
	.addDataPointArea(series_, xIndex, area);
    }

    double bTopMidY = it_.breakY(topMid.y());
    double bBottomMidY = it_.breakY(bottomMid.y());

    if (bTopMidY > topMid.y() && bBottomMidY <= bottomMid.y()) {
      WPainterPath breakPath;
      breakPath.moveTo(hv(left - 10, bTopMidY + 10));
      breakPath.lineTo(hv(left + width + 10, bTopMidY + 1));
      breakPath.lineTo(hv(left + width + 10, bTopMidY - 1));
      breakPath.lineTo(hv(left - 10, bTopMidY - 1));
      painter_.setPen(NoPen);
      painter_.setBrush(chart_.background());
      painter_.drawPath(transform.map(breakPath).crisp());
      painter_.setPen(WPen());
      WPainterPath line;
      line.moveTo(hv(left - 10, bTopMidY + 10));
      line.lineTo(hv(left + width + 10, bTopMidY + 1));
      painter_.drawPath(transform.map(line).crisp());
    }

    if (bBottomMidY < bottomMid.y() && bTopMidY >= topMid.y()) {
      WPainterPath breakPath;
      breakPath.moveTo(hv(left + width + 10, bBottomMidY - 10));
      breakPath.lineTo(hv(left - 10, bBottomMidY - 1));
      breakPath.lineTo(hv(left - 10, bBottomMidY + 1));
      breakPath.lineTo(hv(left + width + 10, bBottomMidY + 1));
      painter_.setBrush(chart_.background());
      painter_.setPen(NoPen);
      painter_.drawPath(transform.map(breakPath).crisp());
      painter_.setPen(WPen());
      WPainterPath line;
      line.moveTo(hv(left - 10, bBottomMidY - 1));
      line.lineTo(hv(left + width + 10, bBottomMidY - 10));
      painter_.drawPath(transform.map(line).crisp());
    }
  }

  void paint() { }

private:
  static Wt::WPointF segmentPoint(const Wt::WPainterPath& path, int segment)
  {
    const Wt::WPainterPath::Segment& s = path.segments()[segment];
    return Wt::WPointF(s.x(), s.y());
  }

  static bool fequal(double d1, double d2) {
    return std::fabs(d1 - d2) < 1E-5;
  }

private:
  double groupWidth_;
  int numGroups_;
  int group_;
};

SeriesRenderIterator::SeriesRenderIterator(const WCartesianChart& chart,
					   WPainter& painter)
  : chart_(chart),
    painter_(painter),
    series_(0)
{ }

void SeriesRenderIterator::startSegment(int currentXSegment,
					int currentYSegment,
					const WRectF& currentSegmentArea)
{
  SeriesIterator::startSegment(currentXSegment, currentYSegment,
			       currentSegmentArea);

  const WAxis& yAxis = chart_.axis(series_->axis());

  if (currentYSegment == 0)
    maxY_ = DBL_MAX;
  else
    maxY_ = currentSegmentArea.bottom();

  if (currentYSegment == yAxis.segmentCount() - 1)
    minY_ = -DBL_MAX;
  else
    minY_ = currentSegmentArea.top();
}

void SeriesRenderIterator::endSegment()
{
  SeriesIterator::endSegment();

  seriesRenderer_->paint();
}

bool SeriesRenderIterator::startSeries(const WDataSeries& series,
				       double groupWidth,
				       int numBarGroups, int currentBarGroup)
{
  seriesRenderer_ = 0;

  switch (series.type()) {
  case LineSeries:
  case CurveSeries:
    seriesRenderer_ = new LineSeriesRenderer(chart_, painter_, series, *this);
    break;
  case BarSeries:
    seriesRenderer_ = new BarSeriesRenderer(chart_, painter_, series, *this,
					    groupWidth,
					    numBarGroups, currentBarGroup);
  default:
    break;
  }

  series_ = &series;

  if (seriesRenderer_ != 0) painter_.save();

  return seriesRenderer_ != 0;
}

void SeriesRenderIterator::endSeries()
{
  seriesRenderer_->paint();
  painter_.restore();

  delete seriesRenderer_;
  series_ = 0;
}

void SeriesRenderIterator::newValue(const WDataSeries& series,
				    double x, double y,
				    double stackY,
				    const WModelIndex& xIndex,
				    const WModelIndex& yIndex)
{
  if (Utils::isNaN(x) || Utils::isNaN(y))
    seriesRenderer_->paint();
  else
    seriesRenderer_->addValue(x, y, stackY, xIndex, yIndex);
}

double SeriesRenderIterator::breakY(double y)
{
  if (y < minY_)
    return minY_;
  else if (y > maxY_)
    return maxY_;
  else
    return y;
}

class LabelRenderIterator : public SeriesIterator
{
public:
  LabelRenderIterator(const WCartesianChart& chart, WPainter& painter)
    : chart_(chart),
      painter_(painter)
  { }

  virtual bool startSeries(const WDataSeries& series, double groupWidth,
			   int numBarGroups, int currentBarGroup)
  {
    if (series.isLabelsEnabled(XAxis) || series.isLabelsEnabled(YAxis)) {
      groupWidth_ = groupWidth;
      numGroups_ = numBarGroups;
      group_ = currentBarGroup;
      return true;
    } else
      return false;
  }

  virtual void newValue(const WDataSeries& series, double x, double y,
			double stackY,
			const WModelIndex& xIndex,
			const WModelIndex& yIndex)
  {
    if (Utils::isNaN(x) || Utils::isNaN(y))
      return;

    WString text;

    if (series.isLabelsEnabled(XAxis)) {
      text = chart_.axis(XAxis).label(x);
    }

    if (series.isLabelsEnabled(YAxis)) {
      if (!text.empty())
	text += ": ";
      text += chart_.axis(series.axis()).label(y - stackY);
    }

    if (!text.empty()) {
      WPointF p = chart_.map(x, y, series.axis(),
			     currentXSegment(), currentYSegment());
      if (series.type() == BarSeries) {
	double g = numGroups_ + (numGroups_ - 1) * chart_.barMargin();

	double width = groupWidth_ / g;
	double left = p.x() - groupWidth_ / 2 
	  + group_ * width * (1 + chart_.barMargin());

	p = WPointF(left + width/2, p.y());
      }

      WFlags<AlignmentFlag> alignment;
      if (series.type() == BarSeries) {
	if (y < 0)
	  alignment = AlignCenter | AlignBottom;
	else
	  alignment = AlignCenter | AlignTop;
      } else {
	alignment = AlignCenter | AlignBottom;
	p.setY(p.y() - 3);
      }

      WCartesianChart &chart = const_cast<WCartesianChart &>(chart_);
      WPen oldPen = WPen(chart.textPen_);
      chart.textPen_.setColor(series.labelColor());
      WTransform t = WTransform(1,0,0,-1,chart.chartArea_.left(),chart.chartArea_.bottom()) *
	chart.xTransform_ * chart.yTransform_ *
	WTransform(1,0,0,-1,-chart.chartArea_.left(),chart.chartArea_.bottom());
      chart.renderLabel(painter_, text, t.map(p), alignment, 0, 3);
      chart.textPen_ = oldPen;
    }
  }

private:
  const WCartesianChart& chart_;
  WPainter& painter_;

  double groupWidth_;
  int numGroups_;
  int group_;
};

class MarkerRenderIterator : public SeriesIterator
{
public:
  MarkerRenderIterator(const WCartesianChart& chart, WPainter& painter)
    : chart_(chart),
      painter_(painter)
  { }

  virtual bool startSeries(const WDataSeries& series, double groupWidth,
			   int numBarGroups, int currentBarGroup)
  {
    marker_ = WPainterPath();

    if (series.marker() != NoMarker) {
      chart_.drawMarker(series, marker_);
      painter_.save();
      needRestore_ = true;
    } else
      needRestore_ = false;

    return true;
  }

  virtual void endSeries()
  {
    if (needRestore_)
      painter_.restore();
  }

  virtual void newValue(const WDataSeries& series, double x, double y,
			double stackY,
			const WModelIndex& xIndex,
			const WModelIndex& yIndex)
  {
    if (!Utils::isNaN(x) && !Utils::isNaN(y)) {
      WPointF p = chart_.map(x, y, series.axis(),
			     currentXSegment(), currentYSegment());

      WCartesianChart &chart = const_cast<WCartesianChart &>(chart_);
      
      if (!marker_.isEmpty()) {
	painter_.save();

	WPen pen = WPen(series.markerPen());
	setPenColor(pen, xIndex, yIndex, MarkerPenColorRole);

	WBrush brush = WBrush(series.markerBrush());
	setBrushColor(brush, xIndex, yIndex, MarkerBrushColorRole);
	setMarkerSize(painter_, xIndex, yIndex, series.markerSize());

	WTransform currentTransform = ((WTransform()).translate(chart.combinedTransform().map(hv(p)))) * scale_;
	painter_.setWorldTransform(currentTransform, false);

	painter_.setShadow(series.shadow());
	if (series.marker() != CrossMarker &&
	    series.marker() != XCrossMarker) {
	  painter_.fillPath(marker_, brush);
	  painter_.setShadow(WShadow());
	}
	painter_.strokePath(marker_, pen);

	painter_.restore();
      }

      if (series.type() != BarSeries) {
	boost::any toolTip = yIndex.data(ToolTipRole);
	if (!toolTip.empty()) {
	  WTransform t = painter_.worldTransform();

	  p = t.map(hv(p));

	  WCircleArea *circleArea = new WCircleArea();
	  circleArea->setCenter(WPointF(p.x(), p.y()));
	  circleArea->setRadius(5);
	  circleArea->setToolTip(asString(toolTip));

	  const_cast<WCartesianChart&>(chart_)
	    .addDataPointArea(series, xIndex, circleArea);
	}
      }
    }
  }

  WPointF hv(const WPointF& p) {
    return chart_.hv(p);
  }

  WPointF hv(double x, double y) {
    return chart_.hv(x, y);
  }

private:
  const WCartesianChart& chart_;
  WPainter& painter_;
  WPainterPath marker_;
  bool needRestore_;
  WTransform scale_;

  void setMarkerSize(WPainter& painter,
	  	     const WModelIndex& xIndex,
		     const WModelIndex& yIndex,
		     double markerSize)
  {
    boost::any scale;
    double dScale = 1;
    if (yIndex.isValid())
      scale = yIndex.data(MarkerScaleFactorRole);

    if (scale.empty() && xIndex.isValid())
      scale = xIndex.data(MarkerScaleFactorRole);

    if(!scale.empty())
      dScale = asNumber(scale);

  dScale = markerSize / 6 * dScale;

  scale_ = WTransform(dScale, 0, 0, dScale, 0, 0);
}

};

WCartesianChart::WCartesianChart(WContainerWidget *parent)
  : WAbstractChart(parent),
    interface_(new WChart2DImplementation(this)),
    orientation_(Vertical),
    XSeriesColumn_(-1),
    type_(CategoryChart),
    barMargin_(0),
    axisPadding_(5),
    borderPen_(NoPen),
    zoomEnabled_(false),
    panEnabled_(false),
    rubberBandEnabled_(true),
    crosshairEnabled_(false),
    followCurve_(-1),
    cObjCreated_(false)
{
  init();
}

WCartesianChart::WCartesianChart(ChartType type, WContainerWidget *parent)
  : WAbstractChart(parent),
    interface_(new WChart2DImplementation(this)),
    orientation_(Vertical),
    XSeriesColumn_(-1),
    type_(type),
    barMargin_(0),
    axisPadding_(5),
    borderPen_(NoPen),
    zoomEnabled_(false),
    panEnabled_(false),
    rubberBandEnabled_(true),
    crosshairEnabled_(false),
    followCurve_(-1),
    cObjCreated_(false)
{
  init();
}

WCartesianChart::~WCartesianChart()
{
  for (int i = 2; i > -1; i--)
    delete axes_[i];
  delete interface_;
  std::vector<WAxisSliderWidget *> copy = std::vector<WAxisSliderWidget *>(axisSliderWidgets_);
  axisSliderWidgets_.clear();
  for (std::size_t i = 0; i < copy.size(); ++i) {
    copy[i]->setChart(0);
  }
}

void WCartesianChart::init()
{
  setPalette(new WStandardPalette(WStandardPalette::Muted));

  for (int i = 0; i < 3; ++i)
    axes_[i] = new WAxis();

  axes_[XAxis]->init(interface_, XAxis);
  axes_[YAxis]->init(interface_, YAxis);
  axes_[Y2Axis]->init(interface_, Y2Axis);

  axes_[XAxis]->setPadding(axisPadding_);
  axes_[YAxis]->setPadding(axisPadding_);
  axes_[Y2Axis]->setPadding(axisPadding_);
  
  setPlotAreaPadding(40, Left | Right);
  setPlotAreaPadding(30, Top | Bottom);

  xTransformHandle_ = createJSTransform();
  yTransformHandle_ = createJSTransform();
  xTransform_ = xTransformHandle_.value();
  yTransform_ = yTransformHandle_.value();

  if (WApplication::instance() != 0) {
    mouseWentDown().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.mouseDown(o, e);}}");
    mouseWentUp().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.mouseUp(o, e);}}");
    mouseDragged().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.mouseDrag(o, e);}}");
    mouseMoved().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.mouseMove(o, e);}}");
    mouseWheel().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.mouseWheel(o, e);}}");
    touchStarted().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.touchStart(o, e);}}");
    touchEnded().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.touchEnd(o, e);}}");
    touchMoved().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.touchMoved(o, e);}}");
  }

  wheelActions_[NoModifier] = PanMatching;
  wheelActions_[AltModifier | ControlModifier] = ZoomX;
  wheelActions_[ControlModifier | ShiftModifier] = ZoomY;
  wheelActions_[ControlModifier] = ZoomXY;
  wheelActions_[AltModifier | ControlModifier | ShiftModifier] = ZoomXY;
}

void WCartesianChart::setOrientation(Orientation orientation)
{
  if (orientation_ != orientation) {
    orientation_ = orientation;
    update();
  }
}

void WCartesianChart::setXSeriesColumn(int modelColumn)
{
  if (XSeriesColumn_ != modelColumn) {
    XSeriesColumn_ = modelColumn;
    update();
  }
}

void WCartesianChart::setType(ChartType type)
{
  if (type_ != type) {
    type_ = type;
    axes_[XAxis]->init(interface_, XAxis);
    update();
  }
}

void WCartesianChart::setTextPen(const WPen& pen)
{
  if(pen == textPen_)
    return;
  
  textPen_ = pen;

  for(int i = 0; i < 3; ++i) 
    axes_[i]->setTextPen(pen); 
}

void WCartesianChart::addSeries(const WDataSeries& series)
{
  series_.push_back(series);
  series_.back().setChart(this);

  if (series.type() == LineSeries || series.type() == CurveSeries) {
    assignJSPathsForSeries(series);
  }

  update();
}

void WCartesianChart::assignJSPathsForSeries(const WDataSeries& series) {
  WJavaScriptHandle<WPainterPath> handle;
  if (freePainterPaths_.size() > 0) {
    handle = freePainterPaths_.back();
    freePainterPaths_.pop_back();
  } else {
    handle = createJSPainterPath();
  }
  curvePaths_[series.modelColumn()] = handle;
}

void WCartesianChart::removeSeries(int modelColumn)
{
  int index = seriesIndexOf(modelColumn);

  if (index != -1) {
    if (series_[index].type() == LineSeries || series_[index].type() == CurveSeries) {
      freeJSPathsForSeries(modelColumn);
    }

    series_.erase(series_.begin() + index);
    update();
  }
}

void WCartesianChart::freeJSPathsForSeries(int modelColumn) {
  freePainterPaths_.push_back(curvePaths_[modelColumn]);
  curvePaths_.erase(modelColumn);
}

int WCartesianChart::seriesIndexOf(int modelColumn) const
{
  for (unsigned i = 0; i < series_.size(); ++i)
    if (series_[i].modelColumn() == modelColumn)
      return i;

  return -1;
}

WDataSeries& WCartesianChart::series(int modelColumn)
{
  int index = seriesIndexOf(modelColumn);

  if (index != -1)
    return series_[index];

  throw WException("Column " + boost::lexical_cast<std::string>(modelColumn)
		   + " not in plot");
}

const WDataSeries& WCartesianChart::series(int modelColumn) const
{
  int index = seriesIndexOf(modelColumn);

  if (index != -1)
    return series_[index];

  throw WException("Column " + boost::lexical_cast<std::string>(modelColumn)
		   + " not in plot");
}

void WCartesianChart::setSeries(const std::vector<WDataSeries>& series)
{
  series_ = series;

  freeAllJSPaths();

  for (std::size_t i = 0; i < series_.size(); ++i) {
    const WDataSeries &s = series_[i];
    if (s.type() == LineSeries || s.type() == CurveSeries) {
      assignJSPathsForSeries(s);
    }
  }

  for (unsigned i = 0; i < series_.size(); ++i)
    series_[i].setChart(this);

  update();
}

void WCartesianChart::freeAllJSPaths()
{
  for (PainterPathMap::const_iterator it = curvePaths_.begin();
       it != curvePaths_.end(); ++it) {
    freePainterPaths_.push_back(it->second);
  }
  curvePaths_.clear();
}

WAxis& WCartesianChart::axis(Axis axis)
{
  return *axes_[axis];
}

const WAxis& WCartesianChart::axis(Axis axis) const
{
  return *axes_[axis];
}

void WCartesianChart::setAxis(WAxis *waxis, Axis axis)
{
  axes_[axis] = waxis;
  axes_[axis]->init(interface_, axis);
}

void WCartesianChart::setBarMargin(double margin)
{
  if (barMargin_ != margin) {
    barMargin_ = margin;

    update();
  }
}

void WCartesianChart::setLegendEnabled(bool enabled)
{
  legend_.setLegendEnabled(enabled);

  update();
}

void WCartesianChart::setLegendLocation(LegendLocation location, 
					Side side,
					AlignmentFlag alignment)
{
  legend_.setLegendLocation(location, side, alignment);

  update();
}

void WCartesianChart::setLegendColumns(int columns, const WLength& columnWidth)
{
  legend_.setLegendColumns(columns);
  legend_.setLegendColumnWidth(columnWidth);

  update();
}

void WCartesianChart::setLegendStyle(const WFont& font,
				     const WPen& border,
				     const WBrush& background)
{
  legend_.setLegendStyle(font, border, background);

  update();
}

WWidget *WCartesianChart::createLegendItemWidget(int index)
{
  WContainerWidget *legendItem = new WContainerWidget();

  legendItem->addWidget(new IconWidget(this, index));
  WText *label = new WText(asString(model()->headerData(index)));
  label->setVerticalAlignment(AlignTop);
  legendItem->addWidget(label);

  return legendItem;
}

void WCartesianChart::addDataPointArea(const WDataSeries& series,
				       const WModelIndex& xIndex,
				       WAbstractArea *area)
{
  if (areas().empty())
    addAreaMask();
  addArea(area);
}

WPointF WCartesianChart::mapFromDevice(const WPointF& point, Axis ordinateAxis)
  const
{
  const WAxis& xAxis = axis(XAxis);
  const WAxis& yAxis = axis(ordinateAxis);

  WPointF p = inverseHv(point.x(), point.y(), width().toPixels());

  return WPointF(xAxis.mapFromDevice(p.x() - chartArea_.left()),
		 yAxis.mapFromDevice(chartArea_.bottom() - p.y()));
}

WPointF WCartesianChart::mapToDevice(const boost::any& xValue,
				     const boost::any& yValue,
				     Axis ordinateAxis, int xSegment,
				     int ySegment) const
{
  const WAxis& xAxis = axis(XAxis);
  const WAxis& yAxis = axis(ordinateAxis);
  
  double x = chartArea_.left() + xAxis.mapToDevice(xValue, xSegment);
  double y = chartArea_.bottom() - yAxis.mapToDevice(yValue, ySegment);

  return hv(x, y, width().toPixels());
}

WPointF WCartesianChart::hv(double x, double y,
			    double width) const
{
  if (orientation_ == Vertical)
    return WPointF(x, y);
  else
    return WPointF(width - y, x);
}

WPointF WCartesianChart::inverseHv(double x, double y, double width) const
{
   if (orientation_ == Vertical)
    return WPointF(x, y);
  else
    return WPointF(y, width - x); 
}

void WCartesianChart::render(WFlags<RenderFlag> flags)
{
  WAbstractChart::render(flags);

  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WCartesianChart.js", "WCartesianChart", wtjs1);
}

void WCartesianChart::setFormData(const FormData& formData)
{
  WPaintedWidget::setFormData(formData);

  if (!axis(XAxis).zoomDirty_) {
    double z = xTransformHandle_.value().m11();
    if (z != axis(XAxis).zoom()) axis(XAxis).setZoom(z);
  }
  if (!axis(Y1Axis).zoomDirty_) {
    double z = yTransformHandle_.value().m22();
    if (z != axis(Y1Axis).zoom()) axis(Y1Axis).setZoom(z);
  }
  WPointF devicePan =
    WPointF(xTransformHandle_.value().dx() / xTransformHandle_.value().m11(),
	    yTransformHandle_.value().dy() / yTransformHandle_.value().m22());
  WPointF modelPan = WPointF(axis(XAxis).mapFromDevice(-devicePan.x()),
		       axis(Y1Axis).mapFromDevice(-devicePan.y()));
  if (!axis(XAxis).panDirty_) {
    double x = modelPan.x();
    if (x != axis(XAxis).pan()) axis(XAxis).setPan(x);
  }
  if (!axis(Y1Axis).panDirty_) {
    double y = modelPan.y();
    if (y != axis(Y1Axis).pan()) axis(Y1Axis).setPan(y);
  }
}

void WCartesianChart::modelColumnsInserted(const WModelIndex& parent,
					   int start, int end)
{
  for (unsigned i = 0; i < series_.size(); ++i)
    if (series_[i].modelColumn() >= start)
      series_[i].modelColumn_ += (end - start + 1);
}

void WCartesianChart::modelColumnsRemoved(const WModelIndex& parent,
					  int start, int end)
{
  bool needUpdate = false;

  for (unsigned i = 0; i < series_.size(); ++i)
    if (series_[i].modelColumn() >= start) {
      if (series_[i].modelColumn() <= end) {
	series_.erase(series_.begin() + i);
	needUpdate = true;
	--i;
      } else {
	series_[i].modelColumn_ -= (end - start + 1);
      }
    }

  if (needUpdate)
    update();
}

void WCartesianChart::modelRowsInserted(const WModelIndex& parent,
					int start, int end)
{
  update();
}

void WCartesianChart::modelRowsRemoved(const WModelIndex& parent,
				       int start, int end)
{
  update();
}

void WCartesianChart::modelDataChanged(const WModelIndex& topLeft,
				       const WModelIndex& bottomRight)
{
  if (XSeriesColumn_ >= topLeft.column() &&
      XSeriesColumn_ <= bottomRight.column()) {
    update();
    return;
  }

  for (unsigned i = 0; i < series_.size(); ++i) {
    if ((series_[i].modelColumn() >= topLeft.column() &&
	 series_[i].modelColumn() <= bottomRight.column()) ||
	(series_[i].XSeriesColumn() >= topLeft.column() &&
	 series_[i].XSeriesColumn() <= bottomRight.column())) {
      update();
      break;
    }
  }
}

void WCartesianChart::modelHeaderDataChanged(Orientation orientation,
					     int start, int end)
{
  if (orientation == Horizontal) {
    if (XSeriesColumn_ >= start && XSeriesColumn_ <= end) {
      update();
      return;
    }

    for (unsigned i = 0; i < series_.size(); ++i) {
      if ((series_[i].modelColumn() >= start &&
	   series_[i].modelColumn() <= end) ||
	  (series_[i].XSeriesColumn() >= start &&
	   series_[i].XSeriesColumn() <= end)) {
	update();
	break;
      }
    }
  }
}

void WCartesianChart::modelChanged()
{
  XSeriesColumn_ = -1;
  series_.clear();

  update();
}

void WCartesianChart::modelReset()
{
  update();
}

WCartesianChart::IconWidget::IconWidget(WCartesianChart *chart, 
					int index, 
					WContainerWidget *parent) 
  : WPaintedWidget(parent),
    chart_(chart),
    index_(index)
{
  setInline(true);
  resize(20, 20);
}

void WCartesianChart::IconWidget::paintEvent(Wt::WPaintDevice *paintDevice) 
{
  Wt::WPainter painter(paintDevice);
  chart_->renderLegendIcon(painter, 
			   WPointF(2.5, 10.0), 
			   chart_->series(index_));
}

void WCartesianChart::setAxisPadding(int padding)
{
  axisPadding_ = padding;
  for (int i = 0; i < 2; ++i) {
    axes_[i]->setPadding(padding);
  }
}

void WCartesianChart::setBorderPen(const WPen& pen)
{
  if (borderPen_ != pen) {
    borderPen_ = pen;
    update();
  }
}

bool WCartesianChart::isInteractive() const
{
  return (zoomEnabled_ || panEnabled_ || crosshairEnabled_ || followCurve_ >= 0 || axisSliderWidgets_.size() > 0) && getMethod() == HtmlCanvas;
}

WPainterPath WCartesianChart::pathForSeries(int modelColumn) const
{
  for (std::size_t i = 0; i < series_.size(); ++i) {
    if (series_[i].type() == LineSeries || series_[i].type() == CurveSeries) {
      if (series_[i].modelColumn() == modelColumn) {
	return const_cast<PainterPathMap&>(curvePaths_)[series_[i].modelColumn()].value();
      }
    }
  }
  return WPainterPath();
}

DomElement *WCartesianChart::createDomElement(WApplication *app)
{
  if (isInteractive()) {
    createPensForAxis(XAxis);
    createPensForAxis(YAxis);
  }

  DomElement *res = WAbstractChart::createDomElement(app);

  return res;
}

void WCartesianChart::getDomChanges(std::vector<DomElement *>& result,
				    WApplication *app)
{
  if (isInteractive()) {
    clearPens();
    createPensForAxis(XAxis);
    createPensForAxis(YAxis);
  }

  WAbstractChart::getDomChanges(result, app);
}

void WCartesianChart::updateJSPensForAxis(WStringStream& js, Axis axis) const
{
  PenMap& pens = const_cast<PenMap&>(pens_);
  js << "[";
  for (std::size_t i = 0; i < pens[axis].size(); ++i) {
    if (i != 0) {
      js << ",";
    }
    PenAssignment& assignment = pens[axis][i];
    js << "[";
    js << assignment.pen.jsRef();
    js << ",";
    js << assignment.textPen.jsRef();
    js << "]";
  }
  js << "]";
}

void WCartesianChart::updateJSPens(WStringStream& js) const
{
  // pens[axis][level][]
  js << "pens:{x:";
  updateJSPensForAxis(js, XAxis);
  js << ",y:";
  updateJSPensForAxis(js, YAxis);
  js << "},";
  js << "penAlpha:{x:[";
  js << axis(XAxis).pen().color().alpha() << ',';
  js << axis(XAxis).textPen().color().alpha();
  js << "],y:[";
  js << axis(YAxis).pen().color().alpha() << ',';
  js << axis(YAxis).textPen().color().alpha() << "]},";
}

int WCartesianChart::calcNumBarGroups() const
{
  int numBarGroups = 0;

  bool newGroup = true;
  for (unsigned i = 0; i < series_.size(); ++i)
    if (series_[i].type() == BarSeries) {
      if (newGroup || !series_[i].isStacked())
	++numBarGroups;
      newGroup = false;
    } else
      newGroup = true;

  return numBarGroups;
}

void WCartesianChart::iterateSeries(SeriesIterator *iterator,
				    WPainter *painter,
				    bool reverseStacked) const
{
  WAbstractItemModel *chart_model = model();
  unsigned rows = chart_model ? chart_model->rowCount() : 0;

  double groupWidth = 0.0;
  int numBarGroups;
  int currentBarGroup;

#ifndef WT_TARGET_JAVA
  std::vector<double> stackedValuesInit(rows);
#else
  std::vector<double> stackedValuesInit;
  stackedValuesInit.insert(stackedValuesInit.begin(), rows, 0.0);
#endif // WT_TARGET_JAVA

  const bool scatterPlot = type_ == ScatterPlot;

  if (scatterPlot) {
    numBarGroups = 1;
    currentBarGroup = 0;
  } else {
    numBarGroups = calcNumBarGroups();
    currentBarGroup = 0;
  }

  bool containsBars = false;

  for (unsigned g = 0; g < series_.size(); ++g) {
    if (series_[g].isHidden())
      continue;

    groupWidth = series_[g].barWidth() * (map(2, 0).x() - map(1, 0).x());

    if (containsBars)
      ++currentBarGroup;
    containsBars = false;

    int startSeries, endSeries;

    if (scatterPlot) {
      startSeries = endSeries = g;
    } else {
      for (unsigned i = 0; i < rows; ++i)
	stackedValuesInit[i] = 0.0;

      if (reverseStacked) {
	endSeries = g;

	Axis a = series_[g].axis();

	for (;;) {
	  if (g < series_.size()
	      && (((int)g == endSeries) || series_[g].isStacked())
	      && (series_[g].axis() == a)) {
	    if (series_[g].type() == BarSeries)
	      containsBars = true;

	    for (unsigned row = 0; row < rows; ++row) {
	      double y
		= asNumber(chart_model->data(row, series_[g].modelColumn()));

	      if (!Utils::isNaN(y))
		stackedValuesInit[row] += y;
	    }

	    ++g;
	  } else
	    break;
	}

	--g;
	startSeries = g;
      } else {
	startSeries = g;

	Axis a = series_[g].axis();

	if (series_[g].type() == BarSeries)
	  containsBars = true;
	++g;

	for (;;) {
	  if (g < series_.size() && series_[g].isStacked()
	      && series_[g].axis() == a) {
	    if (series_[g].type() == BarSeries)
	      containsBars = true;
	    ++g;
	  } else
	    break;
	}

	--g;

	endSeries = g;
      }
    }

    int i = startSeries;
    for (;;) {
      bool doSeries = 
	iterator->startSeries(series_[i], groupWidth, numBarGroups,
			      currentBarGroup);

      std::vector<double> stackedValues;

      if (doSeries ||
	  (!scatterPlot && i != endSeries)) {

	for (int currentXSegment = 0;
	     currentXSegment < axis(XAxis).segmentCount();
	     ++currentXSegment) {

	  for (int currentYSegment = 0;
	       currentYSegment < axis(series_[i].axis()).segmentCount();
	       ++currentYSegment) {

	    stackedValues.clear();
	    Utils::insert(stackedValues, stackedValuesInit);

	    if (painter) {
	      WRectF csa = chartSegmentArea(axis(series_[i].axis()),
					    currentXSegment, 
					    currentYSegment);
	      iterator->startSegment(currentXSegment, currentYSegment, csa);
	      
	      painter->save();
	      
	      if (!isInteractive()) {
		WPainterPath clipPath;
		
		clipPath.addRect(hv(csa));
		painter->setClipPath(clipPath);
		painter->setClipping(true);
	      }
	    } else {
	      iterator->startSegment(currentXSegment, currentYSegment, 
				     WRectF());
	    }

	    for (unsigned row = 0; row < rows; ++row) {
	      WModelIndex xIndex, yIndex;

	      double x;
	      if (scatterPlot) {
		int c = series_[i].XSeriesColumn();
		if (c == -1)
		  c = XSeriesColumn();
		if (c != -1) {
		  xIndex = chart_model->index(row, c);
		  x = asNumber(chart_model->data(xIndex));
		} else
		  x = row;
	      } else
		x = row;

	      yIndex = chart_model->index(row, series_[i].modelColumn());
	      double y = asNumber(chart_model->data(yIndex));

	      double prevStack;

	      if (scatterPlot)
		iterator->newValue(series_[i], x, y, 0, xIndex, yIndex);
	      else {
		prevStack = stackedValues[row];

		double nextStack = stackedValues[row];

		bool hasValue = !Utils::isNaN(y);

		if (hasValue) {
		  if (reverseStacked)
		    nextStack -= y;
		  else
		    nextStack += y;
		}

		stackedValues[row] = nextStack;

		if (doSeries) {
		  if (reverseStacked)
		    iterator->newValue(series_[i], x, hasValue ? prevStack : y,
				       nextStack, xIndex, yIndex);
		  else
		    iterator->newValue(series_[i], x, hasValue ? nextStack : y,
				       prevStack, xIndex, yIndex);
		}
	      }
	    }

	    iterator->endSegment();

	    if (painter)
	      painter->restore();
	  }
	}

	stackedValuesInit.clear();
	Utils::insert(stackedValuesInit, stackedValues);
      }

      if (doSeries)
	iterator->endSeries();

      if (i == endSeries)
	break;
      else {
	if (endSeries < startSeries)
	  --i;
	else
	  ++i;
      }
    }
  }
}

/*
 * ------ Rendering logic.
 */

void WCartesianChart::paint(WPainter& painter, const WRectF& rectangle) const
{

  while (!areas().empty())
    delete const_cast<WCartesianChart *>(this)->areas().front();

  if (!painter.isActive())
    throw WException("WCartesianChart::paint(): painter is not active.");

  WRectF rect = rectangle;

  if (rect.isNull() || rect.isEmpty())
    rect = painter.window();

  render(painter, rect);
}

void WCartesianChart::setZoomAndPan()
{
  double xPan = -axis(XAxis).mapToDevice(axis(XAxis).pan(), 0);
  double yPan = -axis(YAxis).mapToDevice(axis(YAxis).pan(), 0);
  double xZoom = axis(XAxis).zoom();
  if (xZoom > axis(XAxis).maxZoom()) xZoom = axis(XAxis).maxZoom();
  double yZoom = axis(YAxis).zoom();
  if (yZoom > axis(YAxis).maxZoom()) yZoom = axis(YAxis).maxZoom();
  WTransform xTransform = WTransform(xZoom, 0, 0, 1, xZoom * xPan, 0);
  WTransform yTransform = WTransform(1, 0, 0, yZoom, 0, yZoom * yPan);

  // Enforce limits
  WRectF chartArea = hv(chartArea_);
  WRectF transformedArea = combinedTransform(xTransform, yTransform).map(chartArea);
  if (transformedArea.left() > chartArea.left()) {
    double diff = chartArea.left() - transformedArea.left();
    if (orientation() == Vertical)
      xTransform = WTransform(1, 0, 0, 1, diff, 0) * xTransform;
    else
      yTransform = WTransform(1, 0, 0, 1, 0, diff) * yTransform;
    transformedArea = combinedTransform(xTransform, yTransform).map(chartArea);
  }
  if (transformedArea.right() < chartArea.right()) {
    double diff = chartArea.right() - transformedArea.right();
    if (orientation() == Vertical)
      xTransform = WTransform(1, 0, 0, 1, diff, 0) * xTransform;
    else
      yTransform = WTransform(1, 0, 0, 1, 0, diff) * yTransform;
    transformedArea = combinedTransform(xTransform, yTransform).map(chartArea);
  }
  if (transformedArea.top() > chartArea.top()) {
    double diff = chartArea.top() - transformedArea.top();
    if (orientation() == Vertical)
      yTransform = WTransform(1, 0, 0, 1, 0, -diff) * yTransform;
    else
      xTransform = WTransform(1, 0, 0, 1, diff, 0) * xTransform;
    transformedArea = combinedTransform(xTransform, yTransform).map(chartArea);
  }
  if (transformedArea.bottom() < chartArea.bottom()) {
    double diff = chartArea.bottom() - transformedArea.bottom();
    if (orientation() == Vertical)
      yTransform = WTransform(1, 0, 0, 1, 0, -diff) * yTransform;
    else
      xTransform = WTransform(1, 0, 0, 1, diff, 0) * xTransform;
    transformedArea = combinedTransform(xTransform, yTransform).map(chartArea);
  }

  xTransformHandle_.setValue(xTransform);
  yTransformHandle_.setValue(yTransform);

  axis(XAxis).zoomDirty_ = false;
  axis(XAxis).panDirty_ = false;
  axis(Y1Axis).zoomDirty_ = false;
  axis(Y1Axis).panDirty_ = false;
}

void WCartesianChart::paintEvent(WPaintDevice *paintDevice)
{
  WPainter painter(paintDevice);
  painter.setRenderHint(WPainter::Antialiasing);
  paint(painter);

  if (isInteractive()) {
    setZoomAndPan();

    double modelBottom = axis(Y1Axis).mapFromDevice(0);
    double modelTop = axis(Y1Axis).mapFromDevice(chartArea_.height());
    double modelLeft = axis(XAxis).mapFromDevice(0);
    double modelRight = axis(XAxis).mapFromDevice(chartArea_.width());

    WRectF modelArea(modelLeft, modelBottom, modelRight - modelLeft, modelTop - modelBottom);

    int coordPaddingX = 5,
	coordPaddingY = 5;

    if (orientation() == Vertical) {
      if (axis(XAxis).isVisible() && axis(XAxis).tickDirection() == Inwards &&
	  (axis(XAxis).location() == MaximumValue || axis(XAxis).location() == BothSides))
	coordPaddingY = 25;
      if ((axis(Y1Axis).isVisible() && axis(Y1Axis).tickDirection() == Inwards &&
	    (axis(Y1Axis).location() == MaximumValue || axis(Y1Axis).location() == BothSides)) ||
	  (axis(Y2Axis).isVisible() && axis(Y2Axis).tickDirection() == Inwards))
	coordPaddingX = 40;
    } else {
      if (axis(XAxis).isVisible() && axis(XAxis).tickDirection() == Inwards &&
	  (axis(XAxis).location() == MaximumValue || axis(XAxis).location() == BothSides))
	coordPaddingX = 40;
      if (axis(Y1Axis).isVisible() && axis(Y1Axis).tickDirection() == Inwards &&
	  (axis(Y1Axis).location() == MinimumValue || axis(Y1Axis).location() == BothSides))
	coordPaddingY = 25;
    }

    char buf[30];
    WApplication *app = WApplication::instance();
    WStringStream ss;
    ss << "new " WT_CLASS ".WCartesianChart("
       << app->javaScriptClass() << ","
       << jsRef() << ","
       << objJsRef() << ","
	"{"
	  "isHorizontal:" << asString(orientation() == Horizontal).toUTF8() << ","
	  "zoom:" << asString(zoomEnabled_).toUTF8() << ","
	  "pan:" << asString(panEnabled_).toUTF8() << ","
	  "crosshair:" << asString(crosshairEnabled_).toUTF8() << ","
	  "followCurve:" << followCurve_ << ","
	  "xTransform:" << xTransformHandle_.jsRef() << ","
	  "yTransform:" << yTransformHandle_.jsRef() << ","
	  "area:" << hv(chartArea_).jsRef() << ","
	  "modelArea:" << modelArea.jsRef() << ",";
    updateJSPens(ss);
    ss << "series:{";
    for (std::size_t i = 0; i < series_.size(); ++i) {
      if (series_[i].type() == LineSeries || series_[i].type() == CurveSeries) {
	ss << series_[i].modelColumn() << ":" << curvePaths_[series_[i].modelColumn()].jsRef() << ",";
      }
    }
    ss << "},";
    ss << "maxZoom:[" << Utils::round_js_str(axis(XAxis).maxZoom(), 3, buf) << ",";
    ss <<		 Utils::round_js_str(axis(Y1Axis).maxZoom(), 3, buf) << "],";
    ss << "rubberBand:" << rubberBandEnabled_ << ',';
    ss << "sliders:[";
    for (std::size_t i = 0; i < axisSliderWidgets_.size(); ++i) {
      if (i != 0) ss << ',';
      ss << '"' << axisSliderWidgets_[i]->id() << '"';
    }
    ss << "],";
    ss << "wheelActions:" << wheelActionsToJson(wheelActions_) << ",";
    ss << "coordinateOverlayPadding:[" << coordPaddingX << ",";
    ss                                 << coordPaddingY << "]";
    ss << "});";


    doJavaScript(ss.str());

    cObjCreated_ = true;

    for (std::size_t i = 0; i < axisSliderWidgets_.size(); ++i) {
      axisSliderWidgets_[i]->update();
    }
  }
}

void WCartesianChart::render(WPainter& painter, const WRectF& rectangle) const
{
  painter.save();
  painter.translate(rectangle.topLeft());

  if (initLayout(rectangle, painter.device())) {
    renderBackground(painter);
    renderGrid(painter, axis(XAxis));
    renderGrid(painter, axis(Y1Axis));
    renderGrid(painter, axis(Y2Axis));
    renderSeries(painter);              // render the data series
    renderAxes(painter, Line | Labels); // render the axes (lines & labels)
    renderBorder(painter);
    renderLegend(painter);
  }

  painter.restore();
}

bool WCartesianChart::initLayout(const WRectF& rectangle, WPaintDevice *device)
  const
{
  WRectF rect = rectangle;
  if (rect.isNull() || rect.isEmpty())
    rect = WRectF(0.0, 0.0, width().toPixels(), height().toPixels());

  if (orientation() == Vertical) {
    width_ = (int)rect.width();
    height_ = (int)rect.height();
  } else {
    width_ = (int)rect.height();
    height_ = (int)rect.width();
  }

  for (int i = 0; i < 3; ++i)
    location_[i] = MinimumValue;

  bool autoLayout = isAutoLayoutEnabled();
  if (autoLayout && 
      ((device->features() & WPaintDevice::HasFontMetrics) == 0)) {
    LOG_ERROR("setAutoLayout(): device does not have font metrics "
      "(not even server-side font metrics).");
    autoLayout = false;
  }

  if (autoLayout) {
    WCartesianChart *self = const_cast<WCartesianChart *>(this);
    self->setPlotAreaPadding(40, Left | Right);
    self->setPlotAreaPadding(30, Top | Bottom);

    calcChartArea();

    xTransform_ = WTransform();
    yTransform_ = WTransform();

    if (chartArea_.width() <= 5 || chartArea_.height() <= 5 || !prepareAxes()) {
      xTransform_ = xTransformHandle_.value();
      yTransform_ = yTransformHandle_.value();
      return false;
    }

    WPaintDevice *d = device;
    if (!d)
      d = createPaintDevice();
    {
      WMeasurePaintDevice md(d);
      WPainter painter(&md);

      renderAxes(painter, Line | Labels);
      renderLegend(painter);

      WRectF bounds = md.boundingRect();

      /* bounds should be within rect with a 5 pixel margin */
      const int MARGIN = 5;
      int corrLeft = (int)std::max(0.0, rect.left() - bounds.left() + MARGIN);
      int corrRight = (int)std::max(0.0, bounds.right() - rect.right() + MARGIN);
      int corrTop = (int)std::max(0.0, rect.top() - bounds.top() + MARGIN);
      int corrBottom = (int)std::max(0.0, bounds.bottom() - rect.bottom()
				     + MARGIN);

      self->setPlotAreaPadding(plotAreaPadding(Left) + corrLeft, Left);
      self->setPlotAreaPadding(plotAreaPadding(Right) + corrRight, Right);
      self->setPlotAreaPadding(plotAreaPadding(Top) + corrTop, Top);
      self->setPlotAreaPadding(plotAreaPadding(Bottom) + corrBottom, Bottom);
    }

    if (!device)
      delete d;
  }

  calcChartArea();

  bool result = chartArea_.width() > 5 && chartArea_.height() > 5 && prepareAxes();

  xTransform_ = xTransformHandle_.value();
  yTransform_ = yTransformHandle_.value();

  return result;
}

void WCartesianChart::drawMarker(const WDataSeries& series,
				 WPainterPath& result) const
{
  const double size = 6.0;
  const double hsize = size/2;

  switch (series.marker()) {
  case CircleMarker:
    result.addEllipse(-hsize, -hsize, size, size);
    break;
  case SquareMarker:
    result.addRect(WRectF(-hsize, -hsize, size, size));
    break;
  case CrossMarker:
    result.moveTo(-1.3 * hsize, 0);
    result.lineTo(1.3 * hsize, 0);
    result.moveTo(0, -1.3 * hsize);
    result.lineTo(0, 1.3 * hsize);
    break;
  case XCrossMarker:
    result.moveTo(-hsize, -hsize);
    result.lineTo(hsize, hsize);
    result.moveTo(-hsize, hsize);
    result.lineTo(hsize, -hsize);
    break;
  case TriangleMarker:
    result.moveTo(0, 0.6 * hsize);
    result.lineTo(-hsize, 0.6 * hsize);
    result.lineTo(0, -hsize);
    result.lineTo(hsize, 0.6 * hsize);
    result.closeSubPath();
    break;
  case CustomMarker:
    result = series.customMarker();
    break;
  default:
    ;
  }
}

void WCartesianChart::renderLegendIcon(WPainter& painter,
				       const WPointF& pos,
				       const WDataSeries& series) const
{
  WShadow shadow = painter.shadow();
  switch (series.type()) {
  case BarSeries: {
    WPainterPath path;
    path.moveTo(-6, 8);
    path.lineTo(-6, -8);
    path.lineTo(6, -8);
    path.lineTo(6, 8);
    painter.translate(pos.x() + 7.5, pos.y());  
    painter.setShadow(series.shadow());
    painter.fillPath(path, series.brush());
    painter.setShadow(shadow);
    painter.strokePath(path, series.pen());
    painter.translate(-(pos.x() + 7.5), -pos.y());
    break;
  }
  case LineSeries:
  case CurveSeries: {
#ifdef WT_TARGET_JAVA
    painter.setPen(WPen(series.pen()));
#else
    painter.setPen(series.pen());
#endif
    double offset = (series.pen().width() == 0 ? 0.5 : 0);
    painter.setShadow(series.shadow());
    painter.drawLine(pos.x(), pos.y() + offset, pos.x() + 16, pos.y() + offset);
    painter.setShadow(shadow);
  }
    // no break;
  case PointSeries: {
    WPainterPath path;
    drawMarker(series, path);
    if (!path.isEmpty()) {
      painter.translate(pos.x() + 8, pos.y());  
      painter.setShadow(series.shadow());
      painter.fillPath(path, series.markerBrush());
      painter.setShadow(shadow);
      painter.strokePath(path, series.markerPen());
      painter.translate(- (pos.x() + 8), -pos.y());
    }

    break;
  }
  }
}

void WCartesianChart::renderLegendItem(WPainter& painter,
				       const WPointF& pos,
				       const WDataSeries& series) const
{
  WPen fontPen = painter.pen();

  renderLegendIcon(painter, pos, series);

#ifdef WT_TARGET_JAVA
  painter.setPen(WPen(fontPen));
#else
  painter.setPen(fontPen);
#endif
  painter.drawText(pos.x() + 23, pos.y() - 9, 100, 20,
		   AlignLeft | AlignMiddle,
		   asString(model()->headerData(series.modelColumn())));
}

bool WCartesianChart::prepareAxes() const
{
  const WAxis& xAxis = axis(XAxis);
  const WAxis& yAxis = axis(YAxis);
  const WAxis& y2Axis = axis(Y2Axis);

  Orientation yDir = orientation_;
  Orientation xDir = orientation_ == Vertical ? Horizontal : Vertical;

  if (!xAxis.prepareRender(xDir, chartArea_.width()))
    return false;

  if (!yAxis.prepareRender(yDir, chartArea_.height()))
    return false;

  if (!y2Axis.prepareRender(yDir, chartArea_.height()))
    return false;

  if (xAxis.scale() == CategoryScale) {
    switch (xAxis.location()) {
    case MinimumValue:
    case ZeroValue:
      location_[XAxis] = MinimumValue;
      break;
    case MaximumValue:
      location_[XAxis] = MaximumValue;
      break;
    case BothSides:
      location_[XAxis] = BothSides;
    }
  }

  for (int i = 0; i < 2; ++i) {
    WAxis axis = i == 0 ? xAxis : yAxis;
    WAxis other = i == 0 ? yAxis : xAxis;
    AxisValue location = axis.location();

    if (location == ZeroValue) {
      if (other.segments_.front().renderMaximum < 0)
	location = MaximumValue;
      else if (other.segments_.front().renderMinimum > 0)
	location = MinimumValue;
    } else if (location == MinimumValue) {
      if (other.segments_.front().renderMinimum == 0 && axis.tickDirection() == Outwards)
	location = ZeroValue;
    } else if (location != BothSides) {
      if (other.segments_.front().renderMaximum == 0)
	location = MaximumValue;
    }

    location_[axis.id()] = location;
  }

  // force Y axes to the sides when dual Y axes
  if (y2Axis.isVisible()) {
    if (location_[Y1Axis] == BothSides &&
	xAxis.segments_.front().renderMinimum == 0)
      location_[Y1Axis] = ZeroValue;
    if (location_[Y1Axis] == BothSides ||
	!(location_[Y1Axis] == ZeroValue
	  && (xAxis.segments_.front().renderMinimum == 0)))
      location_[Y1Axis] = MinimumValue;

    location_[Y2Axis] = MaximumValue;
  } else
    location_[Y2Axis] = MaximumValue;

  // adjust axis borders to make them look neat and polished
  xAxis.setOtherAxisLocation(location_[YAxis]);
  yAxis.setOtherAxisLocation(location_[XAxis]);
  y2Axis.setOtherAxisLocation(location_[XAxis]);

  return true;
}

WPointF WCartesianChart::map(double xValue, double yValue,
			     Axis yAxis, int currentXSegment,
			     int currentYSegment) const
{
  const WAxis& xAx = axis(XAxis);
  const WAxis& yAx = axis(yAxis);

  double x = chartArea_.left() + xAx.mapToDevice(xValue, currentXSegment);
  double y = chartArea_.bottom() - yAx.mapToDevice(yValue, currentYSegment);

  return WPointF(x, y);
}

void WCartesianChart::calcChartArea() const
{
  if (orientation_ == Vertical)
    chartArea_ = WRectF(plotAreaPadding(Left),
			plotAreaPadding(Top),
			std::max(10, width_ - plotAreaPadding(Left)
				 - plotAreaPadding(Right)),
			std::max(10, height_ - plotAreaPadding(Top)
				 - plotAreaPadding(Bottom)));
  else
    chartArea_ = WRectF(plotAreaPadding(Top),
			plotAreaPadding(Right),
			std::max(10, width_ - plotAreaPadding(Top)
				 - plotAreaPadding(Bottom)),
			std::max(10, height_ - plotAreaPadding(Right)
				 - plotAreaPadding(Left)));
}

WRectF WCartesianChart::chartSegmentArea(WAxis yAxis, int xSegment,
					 int ySegment) const
{
  const WAxis& xAxis = axis(XAxis);

  const WAxis::Segment& xs = xAxis.segments_[xSegment];
  const WAxis::Segment& ys = yAxis.segments_[ySegment];

  // margin used when clipping, see also WAxis::prepareRender(),
  // when the renderMinimum/maximum is 0, clipping is done exact

  double x1 = chartArea_.left() + xs.renderStart
    + (xSegment == 0
       ? (xs.renderMinimum == 0 ? 0 : -axisPadding())
       : -xAxis.segmentMargin() / 2);
  double x2 = chartArea_.left() + xs.renderStart + xs.renderLength
    + (xSegment == xAxis.segmentCount() - 1
       ? (xs.renderMaximum == 0 ? 0 : axisPadding())
       : xAxis.segmentMargin() / 2);

  double y1 = chartArea_.bottom() - ys.renderStart - ys.renderLength
    - (ySegment == yAxis.segmentCount() - 1
       ? (ys.renderMaximum == 0 ? 0 : axisPadding())
       : yAxis.segmentMargin() / 2);
  double y2 = chartArea_.bottom() - ys.renderStart
    + (ySegment == 0
       ? (ys.renderMinimum == 0 ? 0 : axisPadding())
       : yAxis.segmentMargin() / 2);

  return WRectF(std::floor(x1 + 0.5), std::floor(y1 + 0.5),
		std::floor(x2 - x1), std::floor(y2 - y1));
}

void WCartesianChart::renderBackground(WPainter& painter) const
{
  if (background().style() != NoBrush)
    painter.fillRect(hv(chartArea_), background());
}

void WCartesianChart::renderGrid(WPainter& painter, const WAxis& ax) const
{
  if (!ax.isGridLinesEnabled())
    return;

  bool vertical = ax.id() != XAxis;

  const WAxis& other = vertical ? axis(XAxis) : axis(Y1Axis);
  const WAxis::Segment& s0 = other.segments_.front();
  const WAxis::Segment& sn = other.segments_.back();

  double ou0 = s0.renderStart;
  double oun = sn.renderStart + sn.renderLength;

  // Adjust for potentially different axis padding on second Y-axis
  if (!vertical && axis(Y2Axis).isGridLinesEnabled()) {
    const WAxis& other2 = axis(Y2Axis);
    const WAxis::Segment& s0_2 = other2.segments_.front();
    const WAxis::Segment& sn_2 = other2.segments_.back();
    if (!axis(YAxis).isGridLinesEnabled() || s0_2.renderStart < ou0)
      ou0 = s0_2.renderStart;
    if (!axis(YAxis).isGridLinesEnabled() || sn_2.renderStart + sn_2.renderLength > oun)
      oun = sn_2.renderStart + sn_2.renderLength;
  }
  
  bool otherVertical = !vertical;

  if (otherVertical) {
    ou0 = chartArea_.bottom() - ou0;
    oun = chartArea_.bottom() - oun;
  } else {
    ou0 = chartArea_.left() + ou0;
    oun = chartArea_.left() + oun;
  }

  WPainterPath gridPath;

  std::vector<double> gridPos = ax.gridLinePositions();
  
  for (unsigned i = 0; i < gridPos.size(); ++i) {
    double u = gridPos[i];

    if (vertical) {
      u = chartArea_.bottom() - u;
      gridPath.moveTo(hv(ou0, u));
      gridPath.lineTo(hv(oun, u));
    } else {
      u = chartArea_.left() + u;
      gridPath.moveTo(hv(u, ou0));
      gridPath.lineTo(hv(u, oun));
    }
  }

  if (isInteractive()) {
    painter.save();
    WPainterPath clipPath;
    clipPath.addRect(hv(chartArea_));
    painter.setClipPath(clipPath);
    painter.setClipping(true);
  }
  painter.strokePath(combinedTransform().map(gridPath).crisp(), ax.gridLinesPen());
  if (isInteractive()) {
    painter.restore();
  }
}

void WCartesianChart::renderAxis(WPainter& painter, const WAxis& axis,
				 WFlags<AxisProperty> properties) const
{
  if (!axis.isVisible())
    return;

  bool vertical = axis.id() != XAxis;

  if (isInteractive()) {
    WRectF clipRect;
    WRectF area = hv(chartArea_);
    if (axis.location() == ZeroValue && location_[axis.id()] == ZeroValue) {
      clipRect = area;
    } else if (vertical != /*XOR*/ (orientation() == Horizontal)) {
      double h = area.height();
      if (location_[XAxis] == ZeroValue &&
	  orientation() == Vertical) {
	h += 1; // prevent clipping off of zero tick
      }
      clipRect = WRectF(0.0, area.top(), vertical ? width_ : height_, h);
    } else {
      clipRect = WRectF(area.left(), 0.0, area.width(), vertical ? height_ : width_);
    }
    WPainterPath clipPath;
    clipPath.addRect(clipRect);
    painter.save();
    painter.setClipPath(clipPath);
    painter.setClipping(true);
  }

  std::vector<AxisValue> locations;
  if (location_[axis.id()] == BothSides) {
    locations.push_back(MinimumValue);
    locations.push_back(MaximumValue);
  } else
    locations.push_back(location_[axis.id()]);

  for (std::size_t l = 0; l < locations.size(); ++l) {
    WPointF axisStart, axisEnd;
    double tickStart = 0.0, tickEnd = 0.0, labelPos = 0.0;
    AlignmentFlag labelHFlag = AlignCenter, labelVFlag = AlignMiddle;

    if (vertical) {
      labelVFlag = AlignMiddle;
      axisStart.setY(chartArea_.bottom());
      axisEnd.setY(chartArea_.top());
    } else {
      labelHFlag = AlignCenter;
      axisStart.setX(chartArea_.left());
      axisEnd.setX(chartArea_.right());
    }

    switch (locations[l]) {
    case MinimumValue:
      if (vertical) {
	if (axis.tickDirection() == Inwards) {
	  tickStart = 0;
	  tickEnd = TICK_LENGTH;
	  labelPos = TICK_LENGTH;
	  labelHFlag = AlignLeft;

	  double x = chartArea_.left();
	  axisStart.setX(x);
	  axisEnd.setX(x);
	} else {
	  tickStart = -TICK_LENGTH;
	  tickEnd = 0;
	  labelPos = -TICK_LENGTH;
	  labelHFlag = AlignRight;

	  double x = chartArea_.left() - axis.margin();
	  axisStart.setX(x);
	  axisEnd.setX(x);
	}
      } else {
	if (axis.tickDirection() == Inwards) {
	  tickStart = -TICK_LENGTH;
	  tickEnd = 0;
	  labelPos = -TICK_LENGTH;
	  labelVFlag = AlignBottom;

	  double y = chartArea_.bottom() - 1;
	  axisStart.setY(y);
	  axisEnd.setY(y);
	} else {
	  tickStart = 0;
	  tickEnd = TICK_LENGTH;
	  labelPos = TICK_LENGTH;
	  labelVFlag = AlignTop;

	  double y = chartArea_.bottom() + axis.margin();
	  axisStart.setY(y);
	  axisEnd.setY(y);
	}
      }

      break;
    case MaximumValue:
      if (vertical) {
	if (axis.tickDirection() == Inwards) {
	  tickStart = -TICK_LENGTH;
	  tickEnd = 0;
	  labelPos = -TICK_LENGTH;
	  labelHFlag = AlignRight;

	  double x = chartArea_.right() - 1;
	  axisStart.setX(x);
	  axisEnd.setX(x);
	} else {
	  tickStart = 0;
	  tickEnd = TICK_LENGTH;
	  labelPos = TICK_LENGTH;
	  labelHFlag = AlignLeft;

	  double x = chartArea_.right() + axis.margin();
	  axisStart.setX(x);
	  axisEnd.setX(x);
	}
      } else {
	if (axis.tickDirection() == Inwards) {
	  tickStart = 0;
	  tickEnd = TICK_LENGTH;
	  labelPos = TICK_LENGTH;
	  labelVFlag = AlignTop;

	  double y = chartArea_.top();
	  axisStart.setY(y);
	  axisEnd.setY(y);
	} else {
	  tickStart = -TICK_LENGTH;
	  tickEnd = 0;
	  labelPos = -TICK_LENGTH;
	  labelVFlag = AlignBottom;

	  double y = chartArea_.top() - axis.margin();
	  axisStart.setY(y);
	  axisEnd.setY(y);
	}
      }

      break;
    case ZeroValue:
      tickStart = -TICK_LENGTH;
      tickEnd = TICK_LENGTH;

      if (vertical) {
	double x = map(0, 0, YAxis).x();
	axisStart.setX(x);
	axisEnd.setX(x);

	labelHFlag = AlignRight;

	/* force labels left even if axis is in middle */
	if (type() == CategoryChart)
	  labelPos = chartArea_.left() - axisStart.x() - TICK_LENGTH;
	else
	  labelPos = -TICK_LENGTH;

      } else {
	double y = map(0, 0, YAxis).y();
	axisStart.setY(y);
	axisEnd.setY(y);

	labelVFlag = AlignTop;

	/* force labels bottom even if axis is in middle */
	if (type() == CategoryChart)
	  labelPos = chartArea_.bottom() - axisStart.y() + TICK_LENGTH;
	else
	  labelPos = TICK_LENGTH;
      }

      break;
    case BothSides:
      assert(false);
      break;
    }

    if ((properties & Labels) && !axis.title().empty()) {
      if (isInteractive()) painter.setClipping(false);

      WFont oldFont2 = painter.font();
      WFont titleFont = axis.titleFont();
      painter.setFont(titleFont);

      bool chartVertical = orientation() == Vertical;

      if (vertical) {
	/* Y Axes */
	double u = axisStart.x();
	if (chartVertical) {
	  if(axis.titleOrientation() == Horizontal) {
	    renderLabel(painter, axis.title(),
		WPointF(u + (labelHFlag == AlignRight ? 15 : -15),
		  chartArea_.top() - 8),
		labelHFlag | AlignBottom, 0, 10);
	  } else {
	    WPaintDevice *device = painter.device();
	    double size = 0, titleSizeW = 0;
	    if (device->features() & WPaintDevice::HasFontMetrics) {
	      if (axis.tickDirection() == Outwards) size = axis.calcMaxTickLabelSize(device, Horizontal);
	      titleSizeW = axis.calcTitleSize(device, Vertical);
	      if (axis.tickDirection() == Inwards) titleSizeW = -titleSizeW;
	    } else {
	      size = 35;
	      if (axis.tickDirection() == Inwards) size = -20;
	    }

	    renderLabel(painter, axis.title(),
		WPointF(u + (labelHFlag == AlignRight ? -( size + titleSizeW + 5)  : +( size + titleSizeW + 5)),
		  chartArea_.center().y()), AlignCenter | AlignMiddle, locations[l] == MaximumValue ? -90 : 90, 10);
	  }
	} else {
	  double extraMargin = 0;
	  WPaintDevice *device = painter.device();
	  if (axis.tickDirection() == Outwards) extraMargin = axis.calcMaxTickLabelSize(device, Vertical);
	  if (locations[l] != MaximumValue) extraMargin = -extraMargin;
	  WFlags<AlignmentFlag> alignment = (locations[l] == MaximumValue ? AlignLeft : AlignRight) | AlignMiddle;
	  renderLabel(painter, axis.title(),
		      WPointF(u + extraMargin, chartArea_.center().y()), alignment, 0, 10);
        }
      } else {
	/* X Axes */
	double u = axisStart.y();
	if (chartVertical) {
	  double extraMargin = 0;
	  WPaintDevice *device = painter.device();
	  if (device->features() & WPaintDevice::HasFontMetrics) {
	    if (axis.tickDirection() == Outwards) extraMargin = axis.calcMaxTickLabelSize(device, Vertical);
	  } else {
	    if (axis.tickDirection() == Outwards) extraMargin = 15;
	  }
	  if (locations[l] == MaximumValue) extraMargin = -extraMargin;
	  WFlags<AlignmentFlag> alignment = (locations[l] == MaximumValue ? AlignBottom : AlignTop) | AlignCenter;
	  renderLabel(painter, axis.title(),
		      WPointF(chartArea_.center().x(), u + extraMargin),
		      alignment, 0, 10);
	} else {
	  if (axis.titleOrientation() == Vertical) {
	    // Vertical X axis
	    WPaintDevice *device = painter.device();
	    double extraMargin = 0;
	    if (device->features() & WPaintDevice::HasFontMetrics) {
	      if (axis.tickDirection() == Outwards) extraMargin = axis.calcMaxTickLabelSize(device, Horizontal);
	      extraMargin += axis.calcTitleSize(device, Vertical);
	    } else {
	      extraMargin = 40;
	    }
	    if (locations[l] == MaximumValue) extraMargin = -extraMargin;

	    renderLabel(painter, axis.title(),
			WPointF(chartArea_.center().x(), u + extraMargin),
			AlignMiddle | AlignCenter, locations[l] == MaximumValue ? -90 : 90, 10);
	  } else {
	    WFlags<AlignmentFlag> alignment = (locations[l] == MaximumValue ? AlignBottom : AlignTop) | AlignLeft;
	    renderLabel(painter, axis.title(), WPointF(chartArea_.right(), u), alignment, 0, 8);
	  }
	}
      }

      painter.setFont(oldFont2);

      if (isInteractive()) painter.setClipping(true);
    }

    const double ANGLE1 = 15;
    const double ANGLE2 = 80;

    /* Adjust alignment when rotating the labels */
    if (vertical) {
      if (axis.labelAngle() > ANGLE1) {
	labelVFlag = labelPos < 0 ? AlignBottom : AlignTop;
	if (axis.labelAngle() > ANGLE2)
	  labelHFlag = AlignCenter;
      } else if (axis.labelAngle() < -ANGLE1) {
	labelVFlag = labelPos < 0 ? AlignTop : AlignBottom;
	if (axis.labelAngle() < -ANGLE2)
	  labelHFlag = AlignCenter;
      }
    } else {
      if (axis.labelAngle() > ANGLE1) {
	labelHFlag = labelPos > 0 ? AlignRight : AlignLeft;
	if (axis.labelAngle() > ANGLE2)
	  labelVFlag = AlignMiddle;
      } else if (axis.labelAngle() < -ANGLE1) {
	labelHFlag = labelPos > 0 ? AlignLeft : AlignRight;
	if (axis.labelAngle() < -ANGLE2)
	  labelVFlag = AlignMiddle;
      }
    }

    /* perform hv() if necessary */
    if (orientation() == Horizontal) {
      axisStart = hv(axisStart);
      axisEnd = hv(axisEnd);

      AlignmentFlag rHFlag = AlignCenter, rVFlag = AlignMiddle;

      switch (labelHFlag) {
      case AlignLeft: rVFlag = AlignTop; break;
      case AlignCenter: rVFlag = AlignMiddle; break;
      case AlignRight: rVFlag = AlignBottom; break;
      default: break;
      }

      switch (labelVFlag) {
      case AlignTop: rHFlag = AlignRight; break;
      case AlignMiddle: rHFlag = AlignCenter; break;
      case AlignBottom: rHFlag = AlignLeft; break;
      default: break;
      }

      labelHFlag = rHFlag;
      labelVFlag = rVFlag;

      bool invertTicks = !vertical;
      if (invertTicks) {
	tickStart = -tickStart;
	tickEnd = -tickEnd;
	labelPos = -labelPos;
      }
    }

    std::vector<WPen> pens;
    std::vector<WPen> textPens;
    PenMap& penMap = const_cast<PenMap&>(pens_);
    if (isInteractive() && (axis.id() == XAxis || axis.id() == YAxis)) {
      for (std::size_t i = 0; i < penMap[axis.id()].size(); ++i) {
	pens.push_back(penMap[axis.id()][i].pen.value());
	textPens.push_back(penMap[axis.id()][i].textPen.value());
      }
    }

    WTransform transform;
    WRectF area = hv(chartArea_);
    if (axis.location() == ZeroValue) {
      transform =
	WTransform(1,0,0,-1,area.left(),area.bottom()) *
	  xTransform_ * yTransform_ *
	WTransform(1,0,0,-1,-area.left(),area.bottom());
    } else if (vertical && orientation() == Vertical) {
      transform = WTransform(1,0,0,-1,0,area.bottom()) * yTransform_ * WTransform(1,0,0,-1,0,area.bottom());
    } else if (vertical && orientation() == Horizontal) {
      transform = WTransform(0,1,1,0,area.left(),0) * yTransform_ * WTransform(0,1,1,0,0,-area.left());
    } else if (orientation() == Horizontal) {
      transform = WTransform(0,1,1,0,0,area.top()) * xTransform_ * WTransform(0,1,1,0,-area.top(),0);
    } else {
      transform = WTransform(1,0,0,1,area.left(),0) * xTransform_ * WTransform(1,0,0,1,-area.left(),0);
    }

    axis.render(painter, properties, axisStart, axisEnd, tickStart, tickEnd,
		labelPos, labelHFlag | labelVFlag, transform, pens, textPens);
  }

  if (isInteractive()) {
    painter.restore();
  }
}

void WCartesianChart::renderAxes(WPainter& painter,
				 WFlags<AxisProperty> properties) const
{
  renderAxis(painter, axis(XAxis), properties);
  renderAxis(painter, axis(Y1Axis), properties);
  renderAxis(painter, axis(Y2Axis), properties);
}

void WCartesianChart::renderBorder(WPainter& painter) const
{
  WPainterPath area;
  int horizontalShift = 0,
      verticalShift = 0;
  if ((axis(Y1Axis).isVisible() && axis(Y1Axis).tickDirection() == Inwards && (location_[Y1Axis] == BothSides || location_[Y1Axis] == MaximumValue)) ||
      (axis(Y2Axis).isVisible() && axis(Y2Axis).tickDirection() == Inwards))
    horizontalShift = -1;
  if (axis(XAxis).isVisible() && axis(XAxis).tickDirection() == Inwards)
    verticalShift = -1;
  area.addRect(hv(WRectF(chartArea_.left(), chartArea_.top(), chartArea_.width() + horizontalShift, chartArea_.height() + verticalShift)));
  painter.strokePath(area.crisp(), borderPen_);
}

void WCartesianChart::renderSeries(WPainter& painter) const
{
  if (isInteractive()) {
    painter.save();
    WPainterPath clipPath;
    clipPath.addRect(hv(chartArea_));
    painter.setClipPath(clipPath);
    painter.setClipping(true);
  }
  {
    SeriesRenderIterator iterator(*this, painter);
    iterateSeries(&iterator, &painter, true);
  }

  {
    LabelRenderIterator iterator(*this, painter);
    iterateSeries(&iterator, &painter);
  }

  {
    MarkerRenderIterator iterator(*this, painter);
    iterateSeries(&iterator, &painter);
  }
  if (isInteractive()) {
    painter.restore();
  }
}

int WCartesianChart::calcNumBarGroups()
{
  int numBarGroups = 0;

  bool newGroup = true;
  for (unsigned i = 0; i < series().size(); ++i)
    if (series()[i].type() == BarSeries) {
      if (newGroup || !series()[i].isStacked())
	++numBarGroups;
      newGroup = false;
    } else
      newGroup = true;

  return numBarGroups;
}
  

void WCartesianChart::renderLegend(WPainter& painter) const
{
  bool vertical = orientation() == Vertical;

  int w = vertical ? width_ : height_;
  int h = vertical ? height_ : width_;

  // Calculate margin based on layout
  int margin;
  if (isLegendEnabled()) {
	painter.save();

	WPaintDevice  *device = painter.device();
	WAxis *caxis = 0;
	Orientation titleOrientation = Horizontal;
	if(legendSide() == Right) {
	  if (axes_[Y2Axis]->isVisible()) {
	    caxis = axes_[Y2Axis];
	  } else if (axes_[Y1Axis]->isVisible() && (axes_[Y1Axis]->location() == BothSides || axes_[Y1Axis]->location() == MaximumValue)) {
	    caxis = axes_[Y1Axis];
	  }
	  if (caxis && caxis->titleOrientation() == Vertical)
	    titleOrientation = Vertical;
	} else if(legendSide() == Left) {
	  caxis =  axes_[YAxis];
	  if(caxis->titleOrientation() == Vertical) 
		titleOrientation = Vertical;
	} 

	bool fontMetrics = device->features() & WPaintDevice::HasFontMetrics;

	if (titleOrientation == Vertical && caxis) {
	  if (fontMetrics) {
	    margin 
	      = (int)(caxis->calcTitleSize(device, Vertical)
		      + axes_[Y2Axis]->calcMaxTickLabelSize(device, Horizontal));
	  } else {
	    margin = 30;
	  }
	} else
	  margin = 20;

	if(caxis && titleOrientation == Horizontal) {
	  if (fontMetrics) {
	    margin += caxis->calcMaxTickLabelSize(device, Horizontal);
	  } else {
	    margin += 20;
	  }
	}


    int numSeriesWithLegend = 0;

    for (unsigned i = 0; i < series().size(); ++i)
      if (series()[i].isLegendEnabled())
	++numSeriesWithLegend;

    painter.setFont(legendFont());
    WFont f = painter.font();

    if (isAutoLayoutEnabled() &&
	((painter.device()->features() & WPaintDevice::HasFontMetrics) != 0)) {
      int columnWidth = 0;
      for (unsigned i = 0; i < series().size(); ++i)
	if (series()[i].isLegendEnabled()) {
	  WString s = asString(model()->headerData(series()[i].modelColumn()));
	  WTextItem t = painter.device()->measureText(s);
	  columnWidth = std::max(columnWidth, (int)t.width());
	}

      WCartesianChart *self = const_cast<WCartesianChart *>(this);
      self->legend_.setLegendColumnWidth(columnWidth + 25);
    }

    int numLegendRows = (numSeriesWithLegend - 1) / legendColumns() + 1;
    double lineHeight = f.sizeLength().toPixels() * 1.5;

    int legendWidth = (int)legendColumnWidth().toPixels()
      * std::min(legendColumns(), numSeriesWithLegend);
    int legendHeight = (int) (numLegendRows * lineHeight);

    int x = 0;
    int y = 0;

    switch (legendSide()) {
    case Left:
      if (legendLocation() == LegendInside)
	x = plotAreaPadding(Left) + margin;
      else
	x = plotAreaPadding(Left) - margin - legendWidth;
      break;
    case Right:
      x = w - plotAreaPadding(Right);
      if (legendLocation() == LegendInside)
	x -= margin + legendWidth;
      else
	x += margin;
      break;
    case Top:
      if (legendLocation() == LegendInside)
	y = plotAreaPadding(Top) + margin;
      else
	y = plotAreaPadding(Top) - margin - legendHeight;
      break;
    case Bottom:
      y = h - plotAreaPadding(Bottom);
      if (legendLocation() == LegendInside)
	y -= margin + legendHeight;
      else
	y += margin;
    default:
      break;
    }

    switch (legendAlignment()) {
    case AlignTop:
      y = plotAreaPadding(Top) + margin;
      break;
    case AlignMiddle:
      {
	double middle = plotAreaPadding(Top)
	  + (h - plotAreaPadding(Top) - plotAreaPadding(Bottom)) / 2; 

	y = (int) (middle - legendHeight/2);
      }
      break;
    case AlignBottom:
      y = h - plotAreaPadding(Bottom) - margin - legendHeight;
      break;
    case AlignLeft:
      x = plotAreaPadding(Left) + margin;
      break;
    case AlignCenter:
      {
	double center = plotAreaPadding(Left)
	  + (w - plotAreaPadding(Left) - plotAreaPadding(Right)) / 2; 

	x = (int) (center - legendWidth/2);
      } 
      break;
    case AlignRight:
      x = w - plotAreaPadding(Right) - margin - legendWidth;
      break;
    default:
      break;
    }

    // FIXME: Actually calculate the proper size of these shifts?
    if (legendLocation() == LegendOutside) {
      if (legendSide() == Top && !vertical && axis(Y1Axis).isVisible())
	y -= 16;

      if (legendSide() == Right && vertical && (axis(Y2Axis).isVisible() ||
	    (axis(Y1Axis).isVisible() && (axis(Y1Axis).location() == BothSides || axis(Y1Axis).location() == MaximumValue))))
	x += 40;

      if (legendSide() == Right && !vertical && axis(XAxis).isVisible() && (axis(XAxis).location() == MaximumValue || axis(XAxis).location() == BothSides))
	x += 40;

      if (legendSide() == Bottom
	  && ((vertical && axis(XAxis).isVisible()) ||
	      (!vertical && (axis(Y2Axis).isVisible() ||
			     (axis(Y1Axis).isVisible() && (axis(Y1Axis).location() == BothSides || axis(Y1Axis).location() == MaximumValue))))))
	y += 16;

      if (legendSide() == Left
	  && ((vertical && axis(Y1Axis).isVisible()) ||
	      (!vertical && axis(XAxis).isVisible())))
	x -= 40;
    }

#ifdef WT_TARGET_JAVA
    painter.setPen(WPen(legendBorder()));
#else
    painter.setPen(legendBorder());
#endif
    painter.setBrush(legendBackground());

	painter.drawRect(x - margin/2, y - margin/2, legendWidth + margin,
		legendHeight + margin);

    painter.setPen(WPen());

    painter.setFont(legendFont());

    int item = 0;
    for (unsigned i = 0; i < series().size(); ++i)
      if (series()[i].isLegendEnabled()) {
	int col = item % legendColumns();
	int row = item / legendColumns();
	double itemX = x + col * legendColumnWidth().toPixels();
	double itemY = y + row * lineHeight;

	renderLegendItem(painter, WPointF(itemX, itemY + lineHeight/2),
			 series()[i]);

	++item;
      }

    painter.restore();
  }

  if (!title().empty()) {
    int x = plotAreaPadding(Left) 
      + (w - plotAreaPadding(Left) - plotAreaPadding(Right)) / 2 ;
    painter.save();
    painter.setFont(titleFont());
    const int TITLE_HEIGHT = 50;
    const int TITLE_PADDING = 10;
    painter.drawText(x - 500,
		     plotAreaPadding(Top) - TITLE_HEIGHT - TITLE_PADDING,
		     1000, TITLE_HEIGHT, AlignCenter | AlignTop, title());
    painter.restore();
  }
}

void WCartesianChart::renderLabel(WPainter& painter, const WString& text,
				  const WPointF& p, 
				  WFlags<AlignmentFlag> flags,
				  double angle, int margin) const
{
  AlignmentFlag horizontalAlign = flags & AlignHorizontalMask;
  AlignmentFlag verticalAlign = flags & AlignVerticalMask;

  AlignmentFlag rHorizontalAlign = horizontalAlign;
  AlignmentFlag rVerticalAlign = verticalAlign;

  double width = 1000;
  double height = 20;

  WPointF pos = hv(p);

  if (orientation() == Horizontal) {
    switch (horizontalAlign) {
    case AlignLeft:
      rVerticalAlign = AlignTop; break;
    case AlignCenter:
      rVerticalAlign = AlignMiddle; break;
    case AlignRight:
      rVerticalAlign = AlignBottom; break;
    default:
      break;
    }

    switch (verticalAlign) {
    case AlignTop:
      rHorizontalAlign = AlignRight; break;
    case AlignMiddle:
      rHorizontalAlign = AlignCenter; break;
    case AlignBottom:
      rHorizontalAlign = AlignLeft; break;
    default:
      break;
    }
  }

  double left = 0;
  double top = 0;

  switch (rHorizontalAlign) {
  case AlignLeft:
    left += margin; break;
  case AlignCenter:
    left -= width/2; break;
  case AlignRight:
    left -= width + margin;
  default:
    break;
  }

  switch (rVerticalAlign) {
  case AlignTop:
    top += margin; break;
  case AlignMiddle:
    top -= height/2; break;
  case AlignBottom:
    top -= height + margin; break;
  default:
    break;
  }

  WPen oldPen = painter.pen();
#ifdef WT_TARGET_JAVA
  painter.setPen(WPen(textPen_));
#else
  painter.setPen(textPen_);
#endif
  WTransform oldTransform = WTransform(painter.worldTransform());
  painter.translate(pos);

  if (angle == 0) {
    painter.drawText(WRectF(left, top, width, height),
		     rHorizontalAlign | rVerticalAlign, text);
  } else {
    painter.rotate(-angle);
    painter.drawText(WRectF(left, top, width, height),
		     rHorizontalAlign | rVerticalAlign, text);
  }

  painter.setWorldTransform(oldTransform, false);
  painter.setPen(oldPen);
}

WPointF WCartesianChart::hv(const WPointF& p) const
{
  if (p.isJavaScriptBound()) {
    if (orientation() == Vertical) {
      return p;
    } else {
      return p.swapHV(height_);
    }
  }
  return hv(p.x(), p.y());
}

WPointF WCartesianChart::hv(double x, double y) const
{
  return hv(x, y, height_);
}

WRectF WCartesianChart::hv(const WRectF& r) const
{
  if (orientation() == Vertical)
    return r;
  else {
    WPointF tl = hv(r.bottomLeft());
    return WRectF(tl.x(), tl.y(), r.height(), r.width());
  }
}

void WCartesianChart::updateJSConfig(const std::string &key, boost::any value)
{
  if (getMethod() == HtmlCanvas) {
    if (!cObjCreated_) {
      update();
    } else {
      doJavaScript(cObjJsRef() + ".updateConfig({" + key + ":" + 
	  asString(value).toUTF8() + "});");
    }
  }
}

void WCartesianChart::setZoomEnabled(bool zoomEnabled)
{
  if (zoomEnabled_ != zoomEnabled) {
    zoomEnabled_ = zoomEnabled;
    updateJSConfig("zoom", zoomEnabled_);
  }
}

bool WCartesianChart::zoomEnabled() const
{
  return zoomEnabled_;
}

void WCartesianChart::setPanEnabled(bool panEnabled)
{
  if (panEnabled_ != panEnabled) {
    panEnabled_ = panEnabled;
    updateJSConfig("pan", panEnabled_);
  }
}

bool WCartesianChart::panEnabled() const
{
  return panEnabled_;
}

void WCartesianChart::setCrosshairEnabled(bool crosshair)
{
  if (crosshairEnabled_ != crosshair) {
    crosshairEnabled_ = crosshair;
    updateJSConfig("crosshair", crosshairEnabled_);
  }
}

bool WCartesianChart::crosshairEnabled() const
{
  return crosshairEnabled_;
}

void WCartesianChart::setFollowCurve(int followCurve)
{
  if (followCurve_ != followCurve) {
    followCurve_ = followCurve;
    updateJSConfig("followCurve", followCurve_);
  }
}

void WCartesianChart::disableFollowCurve()
{
  setFollowCurve(-1);
}

int WCartesianChart::followCurve() const
{
  return followCurve_;
}

void WCartesianChart::setRubberBandEffectEnabled(bool rubberBandEnabled)
{
  if (rubberBandEnabled_ != rubberBandEnabled) {
    rubberBandEnabled_ = rubberBandEnabled;
    updateJSConfig("rubberBand", rubberBandEnabled_);
  }
}

bool WCartesianChart::rubberBandEffectEnabled() const
{
  return rubberBandEnabled_;
}

std::string WCartesianChart::wheelActionsToJson(WheelActions wheelActions) {
  WStringStream ss;
  ss << '{';
  bool first = true;
  for (WheelActions::iterator it = wheelActions.begin(); it != wheelActions.end(); ++it) {
    if (first) first = false;
    else ss << ',';
    ss << it->first.value() << ':' << (int)it->second;
  }
  ss << '}';
  return ss.str();
}

void WCartesianChart::setWheelActions(WheelActions wheelActions)
{
  wheelActions_ = wheelActions;

  updateJSConfig("wheelActions", wheelActionsToJson(wheelActions_));
}

void WCartesianChart::clearPens()
{
  for (PenMap::iterator it = pens_.begin();
       it != pens_.end(); ++it) {
    std::vector<PenAssignment>& assignments = it->second;
    for (std::size_t i = 0; i < assignments.size(); ++i) {
      PenAssignment& assignment = assignments[i];
      freePens_.push_back(assignment.pen);
      freePens_.push_back(assignment.textPen);
    }
  }
  pens_.clear();
}

void WCartesianChart::createPensForAxis(Axis ax)
{
  if (!axis(ax).isVisible()) return;

  double zoom = axis(ax).zoom();
  if (zoom > axis(ax).maxZoom()) {
    zoom = axis(ax).maxZoom();
  }
  int level = toZoomLevel(zoom);

  std::vector<PenAssignment> assignments;
  for (int i = 1;;++i) {
    double z = std::pow(2.0, i-1);
    if (z > axis(ax).maxZoom()) break;
    WJavaScriptHandle<WPen> pen;
    if (freePens_.size() > 0) {
      pen = freePens_.back();
      freePens_.pop_back();
    } else {
      pen = createJSPen();
    }
    WPen p = WPen(axis(ax).pen());
    p.setColor(WColor(p.color().red(), p.color().green(), p.color().blue(),
	  (i == level ? p.color().alpha() : 0)));
    pen.setValue(p);
    WJavaScriptHandle<WPen> textPen;
    if (freePens_.size() > 0) {
      textPen = freePens_.back();
      freePens_.pop_back();
    } else {
      textPen = createJSPen();
    }
    p = WPen(axis(ax).textPen());
    p.setColor(WColor(p.color().red(), p.color().green(), p.color().blue(),
	  (i == level ? p.color().alpha() : 0)));
    textPen.setValue(p);
    assignments.push_back(PenAssignment(pen, textPen));
  }
  pens_[ax] = assignments;
}

WTransform WCartesianChart::combinedTransform() const
{
  return combinedTransform(xTransform_, yTransform_);
}

WTransform WCartesianChart::combinedTransform(WTransform xTransform, WTransform yTransform) const
{
  if (orientation() == Vertical) {
    return WTransform(1,0,0,-1,chartArea_.left(),chartArea_.bottom()) *
	xTransform * yTransform *
      WTransform(1,0,0,-1,-chartArea_.left(),chartArea_.bottom());
  } else {
    WRectF area = hv(chartArea_);
    return WTransform(0,1,1,0,area.left(),area.top()) *
	xTransform * yTransform *
      WTransform(0,1,1,0,-area.top(),-area.left());
  }
}

std::string WCartesianChart::cObjJsRef() const
{
  return "jQuery.data(" + jsRef() + ",'cobj')";
}

void WCartesianChart::addAxisSliderWidget(WAxisSliderWidget *slider)
{
  axisSliderWidgets_.push_back(slider);
  WStringStream ss;
  ss << '[';
  for (std::size_t i = 0; i < axisSliderWidgets_.size(); ++i) {
    if (i != 0) ss << ',';
    ss << '"' << axisSliderWidgets_[i]->id() << '"';
  }
  ss << ']';
  updateJSConfig("sliders", ss.str());
}

void WCartesianChart::removeAxisSliderWidget(WAxisSliderWidget *slider)
{
  for (std::size_t i = 0; i < axisSliderWidgets_.size(); ++i) {
    if (slider == axisSliderWidgets_[i]) {
      axisSliderWidgets_.erase(axisSliderWidgets_.begin() + i);
      WStringStream ss;
      ss << '[';
      for (std::size_t j = 0; j < axisSliderWidgets_.size(); ++j) {
	if (j != 0) ss << ',';
	ss << '"' << axisSliderWidgets_[j]->id() << '"';
      }
      ss << ']';
      updateJSConfig("sliders", ss.str());
      return;
    }
  }
}

void WCartesianChart::addAreaMask()
{
  WRectF all = hv(WRectF(0, 0, width_, height_));
  WRectF chart = hv(chartArea_);
  std::vector<WRectF> rects;

  rects.push_back(WRectF(all.topLeft(), WPointF(all.right(), chart.top())));
  rects.push_back(WRectF(WPointF(all.left(), chart.bottom()), all.bottomRight()));
  rects.push_back(WRectF(WPointF(all.left(), chart.top()), chart.bottomLeft()));
  rects.push_back(WRectF(chart.topRight(), WPointF(all.right(), chart.bottom())));

  for (std::size_t i = 0; i < rects.size(); ++i) {
    if (rects[i].height() > 0 && rects[i].width() > 0) {
      WRectArea *rect = new WRectArea(rects[i]);
      rect->setHole(true);
      rect->setTransformable(false);
      addArea(rect);
    }
  }
}

  }
}
