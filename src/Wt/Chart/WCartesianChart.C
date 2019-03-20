/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>
#include <cmath>

#include "Wt/Chart/WAbstractChartModel"
#include "Wt/Chart/WAxisSliderWidget"
#include "Wt/Chart/WChart2DImplementation"
#include "Wt/Chart/WDataSeries"
#include "Wt/Chart/WCartesianChart"
#include "Wt/Chart/WStandardPalette"

#include "Wt/WAbstractArea"
#include "Wt/WAbstractItemModel"
#include "Wt/WApplication"
#include "Wt/WCanvasPaintDevice"
#include "Wt/WCircleArea"
#include "Wt/WException"
#include "Wt/WEnvironment"
#include "Wt/WJavaScriptHandle"
#include "Wt/WJavaScriptPreamble"
#include "Wt/WJavaScriptObjectStorage"
#include "Wt/WMeasurePaintDevice"
#include "Wt/WPainter"
#include "Wt/WPolygonArea"
#include "Wt/WRectArea"
#include "Wt/WText"
#include "Wt/WTheme"

#include "DomElement.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/ChartCommon.min.js"

#ifndef WT_TARGET_JAVA
namespace skeletons {
  extern std::vector<const char*> WCartesianChart_js();
}

namespace {
  using namespace Wt;
  std::string WCartesianChart_js_str()
  {
    std::vector<const char *> v = skeletons::WCartesianChart_js();
    WStringStream ss;
    for (std::size_t i = 0; i < v.size(); ++i) {
      ss << std::string(v[i]);
    }
    return ss.str();
  }

  WJavaScriptPreamble wtjs1() {
    static std::string js = WCartesianChart_js_str();
    return WJavaScriptPreamble(WtClassScope, JavaScriptConstructor, "WCartesianChart", js.c_str());
  }
}
#else
#include "js/WCartesianChart.min.js"
#endif
#endif

namespace {
  using namespace Wt::Chart;
  std::string locToJsString(AxisValue loc) {
    switch (loc) {
    case MinimumValue:
      return "min";
    case MaximumValue:
      return "max";
    case ZeroValue:
      return "zero";
    case BothSides:
      return "both";
    }
    assert(false);
    return "";
  }

  int binarySearchRow(const Wt::Chart::WAbstractChartModel &model, int xColumn, double d, int minRow, int maxRow)
  {
    if (minRow == maxRow)
      return minRow;
    double min = model.data(minRow, xColumn);
    double max = model.data(maxRow, xColumn);
    if (d <= min)
      return minRow;
    if (d >= max)
      return maxRow;
    double start = minRow + (d - min) / (max - min) * (maxRow - minRow);
    double data = model.data(static_cast<int>(start), xColumn);
    if (data < d) {
      return binarySearchRow(model, xColumn, d, static_cast<int>(start) + 1, maxRow);
    } else if (data > d) {
      return binarySearchRow(model, xColumn, d, minRow, static_cast<int>(start) - 1);
    } else {
      return static_cast<int>(start);
    }
  }
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
const int TICK_LENGTH = 5;
const int CURVE_LABEL_PADDING = 10;
const int DEFAULT_CURVE_LABEL_WIDTH = 100;
const int CURVE_SELECTION_DISTANCE_SQUARED = 400; // Maximum selection distance in pixels, squared

inline int toZoomLevel(double zoomFactor)
{
  return ((int)(std::floor(std::log(zoomFactor) / std::log(2.0) + 0.5))) + 1;
}
}

namespace Wt {

LOGGER("WCartesianChart");

  namespace Chart {

WColor WCartesianChart::lightenColor(const WColor& in)
{
  double hsl[3];
  in.toHSL(hsl);
  double h = hsl[0], s = hsl[1], l = hsl[2];
  return WColor::fromHSL(h, std::max(s - 0.2, 0.0), std::min(l + 0.2, 1.0), in.alpha());
}

CurveLabel::CurveLabel(const WDataSeries &series, const WPointF &point, const WT_USTRING &label)
  : series_(&series),
#ifndef WT_TARGET_JAVA
    x_(point.x()),
    y_(point.y()),
#endif // WT_TARGET_JAVA
    label_(label),
    offset_(60, -20),
    width_(0),
    linePen_(WColor(0,0,0)),
    textPen_(WColor(0,0,0)),
    boxBrush_(WColor(255,255,255)),
    markerBrush_(WColor(0,0,0))
{
#ifdef WT_TARGET_JAVA
  x_ = point.x();
  y_ = point.y();
#endif // WT_TARGET_JAVA
}

CurveLabel::CurveLabel(const WDataSeries &series, const boost::any &x, const boost::any &y,const WT_USTRING &label)
  : series_(&series),
    x_(x),
    y_(y),
    label_(label),
    offset_(60, -20),
    width_(0),
    linePen_(WColor(0,0,0)),
    textPen_(WColor(0,0,0)),
    boxBrush_(WColor(255,255,255)),
    markerBrush_(WColor(0,0,0))
{ }

void CurveLabel::setSeries(const WDataSeries &series)
{
  series_ = &series;
}

void CurveLabel::setPoint(const WPointF &point)
{
  x_ = point.x();
  y_ = point.y();
}

void CurveLabel::setPoint(const boost::any &x, const boost::any &y)
{
  x_ = x;
  y_ = y;
}

const WPointF CurveLabel::point() const
{
  return WPointF(asNumber(x_), asNumber(y_));
}

void CurveLabel::setLabel(const WT_USTRING &label)
{
  label_ = label;
}

void CurveLabel::setOffset(const WPointF &offset)
{
  offset_ = offset;
}

void CurveLabel::setWidth(int width)
{
  width_ = width;
}

void CurveLabel::setLinePen(const WPen &pen)
{
  linePen_ = pen;
}

void CurveLabel::setTextPen(const WPen &pen)
{
  textPen_ = pen;
}

void CurveLabel::setBoxBrush(const WBrush &brush)
{
  boxBrush_ = brush;
}

void CurveLabel::setMarkerBrush(const WBrush &brush)
{
  markerBrush_ = brush;
}

// Checks if edge [p1,p2] intersects the horizontal edge [(minX,y),(maxX,y)]
bool CurveLabel::checkIntersectHorizontal(const WPointF &p1, const WPointF &p2, double minX, double maxX, double y)
{
  if (p1.y() == p2.y()) return p1.y() == y;
  double t = (y - p1.y()) / (p2.y() - p1.y());
  if (t <= 0 || t >= 1) return false; // The edge does not reach y, we do not consider the ends
  double x = p1.x() * (1 - t) + p2.x() * t;
  return x > minX && x < maxX; // do not consider the ends
}

// Checks if edge [p1,p2] intersects the vertical edge [(x,minY),(x,maxY)]
bool CurveLabel::checkIntersectVertical(const WPointF &p1, const WPointF &p2, double minY, double maxY, double x)
{
  return checkIntersectHorizontal(WPointF(p1.y(), p1.x()), WPointF(p2.y(), p2.x()), minY, maxY, x);
}

void CurveLabel::render(WPainter &painter) const
{
  WRectF rect;
  {
    // Determine rectangle width, width if nonzero, otherwise calculated using font metrics or, if unavailable, DEFAULT_CURVE_LABEL_WIDTH
    double rectWidth = DEFAULT_CURVE_LABEL_WIDTH;
    if (width() != 0) {
      rectWidth = width();
    } else if (painter.device()->features() & WPaintDevice::HasFontMetrics) {
      WMeasurePaintDevice device(painter.device());
      WPainter measPainter(&device);
      measPainter.drawText(WRectF(0,0,100,100),AlignMiddle | AlignCenter, TextSingleLine,label(), 0);
      rectWidth = device.boundingRect().width() + CURVE_LABEL_PADDING / 2;
    }
    rect = WRectF(offset().x() - rectWidth / 2, offset().y() - 10, rectWidth, 20).normalized();
  }
  WPointF closestAnchor;
  {
    // Find the closest point on the side of the rectangle to connect to,
    // that does not cause the line to it from the starting point to intersect
    // with an edge of the rectangle.
    std::vector<WPointF> anchorPoints;
    anchorPoints.push_back(WPointF(rect.left(), rect.center().y()));
    anchorPoints.push_back(WPointF(rect.right(), rect.center().y()));
    anchorPoints.push_back(WPointF(rect.center().x(), rect.top()));
    anchorPoints.push_back(WPointF(rect.center().x(), rect.bottom()));
    double minSquareDist = std::numeric_limits<double>::infinity();
    for (std::size_t k = 0; k < anchorPoints.size(); ++k) {
      const WPointF &anchorPoint = anchorPoints[k];
      double d = anchorPoint.x() * anchorPoint.x() + anchorPoint.y() * anchorPoint.y();
      if (d < minSquareDist &&
	  (k == 0 || !checkIntersectVertical(WPointF(), anchorPoint, rect.top(), rect.bottom(), rect.left())) &&
	  (k == 1 || !checkIntersectVertical(WPointF(), anchorPoint, rect.top(), rect.bottom(), rect.right())) &&
	  (k == 2 || !checkIntersectHorizontal(WPointF(), anchorPoint, rect.left(), rect.right(), rect.top())) &&
	  (k == 3 || !checkIntersectHorizontal(WPointF(), anchorPoint, rect.left(), rect.right(), rect.bottom()))) {
	closestAnchor = anchorPoint;
	minSquareDist = d;
      }
    }
  }
  WTransform translation = painter.worldTransform();
  painter.setWorldTransform(WTransform());
  WPainterPath connectorLine;
  connectorLine.moveTo(0, 0);
  connectorLine.lineTo(closestAnchor);
  painter.strokePath(translation.map(connectorLine).crisp(), linePen());
  WPainterPath circle;
  circle.addEllipse(-2.5, -2.5, 5, 5);
  painter.fillPath(translation.map(circle), markerBrush());
  WPainterPath rectPath;
  rectPath.addRect(rect);
  painter.fillPath(translation.map(rectPath), boxBrush());
  painter.strokePath(translation.map(rectPath).crisp(), linePen());
  painter.setPen(textPen());
  painter.drawText(translation.map(rect), AlignMiddle | AlignCenter, TextSingleLine, label(), 0);
}

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
			      int xRow, int xColumn,
			      int yRow, int yColumn)
{ }


void SeriesIterator::setPenColor(WPen& pen, const WDataSeries &series,
				 int xRow, int xColumn,
				 int yRow, int yColumn,
				 int colorRole)
{
  const WColor *color = 0;

  if (yRow >= 0 && yColumn >= 0) {
    if (colorRole == MarkerPenColorRole) {
      color = series.model()->markerPenColor(yRow, yColumn);
    } else if (colorRole == MarkerBrushColorRole) {
      color = series.model()->markerBrushColor(yRow, yColumn);
    }
  }

  if (!color && xRow >= 0 && xColumn >= 0) {
    if (colorRole == MarkerPenColorRole) {
      color = series.model()->markerPenColor(xRow, xColumn);
    } else if (colorRole == MarkerBrushColorRole) {
      color = series.model()->markerBrushColor(xRow, xColumn);
    }
  }

  if (color)
    pen.setColor(*color);
}

void SeriesIterator::setBrushColor(WBrush& brush, const WDataSeries &series,
				   int xRow, int xColumn,
				   int yRow, int yColumn,
				   int colorRole) 
{
  const WColor *color = 0;

  if (yRow >= 0 && yColumn >= 0) {
    if (colorRole == MarkerBrushColorRole) {
      color = series.model()->markerBrushColor(yRow, yColumn);
    } else if (colorRole == BarBrushColorRole) {
      color = series.model()->barBrushColor(yRow, yColumn);
    }
  }

  if (!color && xRow >= 0 && xColumn >= 0) {
    if (colorRole == MarkerBrushColorRole) {
      color = series.model()->markerBrushColor(xRow, xColumn);
    } else if (colorRole == BarBrushColorRole) {
      color = series.model()->barBrushColor(xRow, xColumn);
    }
  }

  if (color)
    brush.setColor(*color);
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
			int xRow, int xColumn,
			int yRow, int yColumn);

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
  virtual void addBreak() = 0;
  virtual void addValue(double x, double y, double stacky,
			int xRow, int xColumn, int yRow, int yColumn)
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

  static double crisp(double u) {
    return std::floor(u) + 0.5;
  }

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
      curveLength_(0),
      curveFragmentLength_(0)
  {
    curve_.setOpenSubPathsEnabled(true);
  }

  virtual void addValue(double x, double y, double stacky,
			int xRow, int xColumn, int yRow, int yColumn) {
    WPointF p = chart_.map(x, y, series_.yAxis(),
			   it_.currentXSegment(), it_.currentYSegment());

    if (curveFragmentLength_ == 0) {
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
	if (curveFragmentLength_ == 1) {
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
    ++curveFragmentLength_;
  }

  virtual void addBreak()
  {
    if (curveFragmentLength_ > 1) {
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
      }
    }
    curveFragmentLength_ = 0;
  }

  virtual void paint() {
    WCartesianChart::PainterPathMap::iterator curveHandle =
        chart_.curvePaths_.find(&series_);
    WCartesianChart::TransformMap::iterator transformHandle =
        chart_.curveTransforms_.find(&series_);

    WTransform transform = chart_.zoomRangeTransform(series_.yAxis());

    if (curveLength_ > 1) {
      if (series_.type() == CurveSeries) {
	WPointF c1;
	computeC(p0, p_1, c1);
	curve_.cubicTo(hv(c_), hv(c1), hv(p0));
	fill_.cubicTo(hv(c_), hv(c1), hv(p0));
      }

      if (series_.fillRange() != NoFill
	  && series_.brush() != NoBrush && !series_.isHidden()) {
	fill_.lineTo(hv(fillOtherPoint(lastX_)));
	fill_.closeSubPath();
	painter_.setShadow(series_.shadow());
	WBrush brush = series_.brush();
	if (chart_.seriesSelectionEnabled() &&
	    chart_.selectedSeries() != 0 &&
	    chart_.selectedSeries() != &series_) {
	  brush.setColor(WCartesianChart::lightenColor(brush.color()));
	}
	painter_.fillPath(transform.map(fill_), brush); // FIXME: support curve transforms
      }

      if (series_.fillRange() == NoFill)
	painter_.setShadow(series_.shadow());
      else
	painter_.setShadow(WShadow());

      WTransform ct;
      WTransform t = chart_.calculateCurveTransform(series_);
      if (transformHandle != chart_.curveTransforms_.end()) {
	transformHandle->second.setValue(t);
	ct = chart_.curveTransform(series_);
      } else {
	ct = t;
	if (chart_.orientation() == Horizontal) {
	  ct = WTransform(0,1,1,0,0,0) * ct * WTransform(0,1,1,0,0,0);
	}
      }
      series_.scaleDirty_ = false;
      series_.offsetDirty_ = false;

      const WPainterPath *curve = 0;
      if (curveHandle != chart_.curvePaths_.end()) {
	curveHandle->second.setValue(curve_);
	curve = &curveHandle->second.value();
      } else {
	curve = &curve_;
      }
      if (!series_.isHidden()) {
	WPen pen = series_.pen();
	if (chart_.seriesSelectionEnabled() &&
	    chart_.selectedSeries() != 0 &&
	    chart_.selectedSeries() != &series_) {
	  pen.setColor(WCartesianChart::lightenColor(pen.color()));
	}
	painter_.strokePath((transform * ct).map(*curve), pen);
      }
    }

    curveLength_ = 0;
    curveFragmentLength_ = 0;
    curve_ = WPainterPath();
    fill_ = WPainterPath();
  }

private:
  int curveLength_;
  int curveFragmentLength_;
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
      return WPointF(chart_.map(x, 0, series_.yAxis(),
				it_.currentXSegment(),
				it_.currentYSegment()).x(),
		     chart_.chartArea_.bottom());
    case MaximumValueFill:
      return WPointF(chart_.map(x, 0, series_.yAxis(),
				it_.currentXSegment(),
				it_.currentYSegment()).x(),
		     chart_.chartArea_.top());
    case ZeroValueFill:
      return WPointF(chart_.map(x, 0, series_.yAxis(),
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
			int xRow, int xColumn, int yRow, int yColumn) {
    WPainterPath bar;
    const WAxis& yAxis = chart_.yAxis(series_.yAxis());

    WPointF topMid = chart_.map(x, y, yAxis.yAxisId(),
				it_.currentXSegment(),
				it_.currentYSegment());
    WPointF bottomMid = chart_.map(x, stacky, yAxis.yAxisId(),
				   it_.currentXSegment(),
				   it_.currentYSegment());

    FillRangeType fr = series_.fillRange();
    switch (fr) {
    case MinimumValueFill:
      bottomMid = WPointF(chart_.map(x, stacky, yAxis.yAxisId(),
				     it_.currentXSegment(),
				     it_.currentYSegment()).x(),
			  chart_.chartArea_.bottom());
      break;
    case MaximumValueFill:
      bottomMid = WPointF(chart_.map(x, stacky, yAxis.yAxisId(),
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

    bool nonZeroWidth = chart_.isInteractive() || crisp(left) != crisp(left + width);

    bar.moveTo(hv(left, topMid.y()));
    if (nonZeroWidth) {
      bar.lineTo(hv(left + width, topMid.y()));
      bar.lineTo(hv(left + width, bottomMid.y()));
    }
    bar.lineTo(hv(left, bottomMid.y()));
    if (nonZeroWidth) {
      bar.closeSubPath();
    }

    painter_.setShadow(series_.shadow());

    WTransform transform = chart_.zoomRangeTransform(series_.yAxis());

    if (nonZeroWidth) {
      WBrush brush = WBrush(series_.brush());
      SeriesIterator::setBrushColor(brush, series_, xRow, xColumn, yRow, yColumn, BarBrushColorRole);
      painter_.fillPath(transform.map(bar), brush);
    }

    painter_.setShadow(WShadow());

    WPen pen = WPen(series_.pen());
    SeriesIterator::setPenColor(pen, series_, xRow, xColumn, yRow, yColumn, BarPenColorRole);
    painter_.strokePath(transform.map(bar).crisp(), pen);

    WString toolTip = series_.model()->toolTip(yRow, yColumn);
    if (!toolTip.empty()) {
      WTransform t = painter_.worldTransform();

      WPointF tl = t.map(segmentPoint(bar, 0));
      WPointF tr = t.map(segmentPoint(bar, 1));
      WPointF br = t.map(segmentPoint(bar, 2));
      WPointF bl = t.map(segmentPoint(bar, 3));

      if (series_.model()->flags(yRow, yColumn) & ItemHasDeferredTooltip ||
	  // Force deferred tooltips if XHTML text
	  series_.model()->flags(yRow, yColumn) & ItemIsXHTMLText) {
	chart_.hasDeferredToolTips_ = true;
	WCartesianChart::BarTooltip btt(series_, xRow, xColumn, yRow, yColumn);
	btt.xs[0] = tl.x();
	btt.ys[0] = tl.y();
	btt.xs[1] = tr.x();
	btt.ys[1] = tr.y();
	btt.xs[2] = br.x();
	btt.ys[2] = br.y();
	btt.xs[3] = bl.x();
	btt.ys[3] = bl.y();
	chart_.barTooltips_.push_back(btt);
      } else {
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

	area->setToolTip(toolTip);

	const_cast<WCartesianChart&>(chart_)
	  .addDataPointArea(series_, xRow, xColumn, area);
      }
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

  void addBreak() { }
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

  const WAxis& yAxis = chart_.yAxis(series_->yAxis());

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
				    int xRow, int xColumn,
				    int yRow, int yColumn)
{
  if (Utils::isNaN(x) || Utils::isNaN(y))
    seriesRenderer_->addBreak();
  else
    seriesRenderer_->addValue(x, y, stackY, xRow, xColumn, yRow, yColumn);
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
			int xRow, int xColumn,
			int yRow, int yColumn)
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
      text += chart_.yAxis(series.yAxis()).label(y - stackY);
    }

    if (!text.empty()) {
      WPointF point = chart_.map(x, y, series.axis(),
			     currentXSegment(), currentYSegment());
      WPointF p = point;
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
      WTransform t = chart_.zoomRangeTransform(series.yAxis());
      WTransform ct;
      WCartesianChart::TransformMap::const_iterator transformHandle = chart_.curveTransforms_.find(&series);
      if (transformHandle != chart_.curveTransforms_.end()) {
	ct = chart_.curveTransform(series);
      }
      if (series.type() == BarSeries) {
	chart.renderLabel(painter_, text, chart_.inverseHv((t * ct).map(chart_.hv(p))), alignment, 0, 3);
      } else {
	double dx = p.x() - point.x();
	double dy = p.y() - point.y();
	chart.renderLabel(painter_, text, chart_.inverseHv((t * ct).translate(WPointF(dx, dy)).map(chart_.hv(point))), alignment, 0, 3);
      }
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
      painter_(painter),
      currentMarkerType_(NoMarker),
      currentScale_(0),
      series_(0)
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
    if (series_)
      finishPathFragment(*series_);
    series_ = 0;

    if (needRestore_)
      painter_.restore();
  }

  virtual void newValue(const WDataSeries& series, double x, double y,
			double stackY,
			int xRow, int xColumn,
			int yRow, int yColumn)
  {
    if (!Utils::isNaN(x) && !Utils::isNaN(y)) {
      WPointF p = chart_.map(x, y, series.yAxis(),
			     currentXSegment(), currentYSegment());

      const MarkerType *pointMarker = series.model()->markerType(yRow, yColumn);
      if (!pointMarker) {
        pointMarker = series.model()->markerType(xRow, xColumn);
      }
      MarkerType markerType = series.marker();
      if (pointMarker) {
        markerType = *pointMarker;
      }
      if (markerType != NoMarker) {
	WPen pen = WPen(series.markerPen());
	SeriesIterator::setPenColor(pen, series, xRow, xColumn, yRow, yColumn, MarkerPenColorRole);
	if (chart_.seriesSelectionEnabled() &&
	    chart_.selectedSeries() != 0 &&
	    chart_.selectedSeries() != &series) {
	  pen.setColor(WCartesianChart::lightenColor(pen.color()));
	}

	WBrush brush = WBrush(series.markerBrush());
	SeriesIterator::setBrushColor(brush, series, xRow, xColumn, yRow, yColumn, MarkerBrushColorRole);
	double scale = calculateMarkerScale(series, xRow, xColumn, yRow, yColumn, series.markerSize());
	if (chart_.seriesSelectionEnabled() &&
	    chart_.selectedSeries() != 0 &&
	    chart_.selectedSeries() != &series) {
	  brush.setColor(WCartesianChart::lightenColor(brush.color()));
	}

	if (!series_ ||
	    brush != currentBrush_ ||
	    pen != currentPen_ ||
            scale != currentScale_ ||
            markerType != currentMarkerType_) {
	  if (series_) {
	    finishPathFragment(*series_);
	  }

	  series_ = &series;
	  currentBrush_ = brush;
	  currentPen_ = pen;
	  currentScale_ = scale;

          if (markerType != currentMarkerType_) {
            marker_ = WPainterPath();
            currentMarkerType_ = markerType;
            if (pointMarker) {
              chart_.drawMarker(series, markerType, marker_);
            } else {
              chart_.drawMarker(series, marker_);
            }
            if (!needRestore_) {
              painter_.save();
              needRestore_ = true;
            }
          }
	}

	pathFragment_.moveTo(hv(p));
      }

      if (series.type() != BarSeries) {
	WString toolTip = series.model()->toolTip(yRow, yColumn);
	if (!toolTip.empty()) {
	  if (!(series.model()->flags(yRow, yColumn) & ItemHasDeferredTooltip ||
		// We force deferred tooltips if it is XHTML
		series.model()->flags(yRow, yColumn) & ItemIsXHTMLText)) {
	    WTransform t = painter_.worldTransform();

	    p = t.map(hv(p));

	    WCircleArea *circleArea = new WCircleArea();
	    circleArea->setCenter(WPointF(p.x(), p.y()));
	    const double *scaleFactorP = series.model()
	      ->markerScaleFactor(yRow, yColumn);
	    double scaleFactor = scaleFactorP != 0 ? *scaleFactorP : 1.0;
	    if (scaleFactor < 1.0)
	      scaleFactor = 1.0;
	    circleArea->setRadius(int(scaleFactor * 5.0));
	    circleArea->setToolTip(toolTip);

	    const_cast<WCartesianChart&>(chart_)
	      .addDataPointArea(series, xRow, xColumn, circleArea);
	  } else {
	    chart_.hasDeferredToolTips_ = true;
	  }
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

  WPainterPath pathFragment_;
  WPen currentPen_;
  WBrush currentBrush_;
  MarkerType currentMarkerType_;
  double currentScale_;
  const WDataSeries *series_;

  double calculateMarkerScale(const WDataSeries &series,
		     int xRow, int xColumn,
		     int yRow, int yColumn,
		     double markerSize)
  {
    const double *scale = 0;
    double dScale = 1;
    if (yRow >= 0 && yColumn >= 0)
      scale = series.model()->markerScaleFactor(yRow, yColumn);

    if (!scale && xRow >= 0 && xColumn >= 0)
      scale = series.model()->markerScaleFactor(xRow, xColumn);

    if (scale)
      dScale = *scale;

    dScale = markerSize / 6 * dScale;

    return dScale;
  }

  void finishPathFragment(const WDataSeries &series)
  {
    if (pathFragment_.segments().empty())
      return;

    painter_.save();

    painter_.setWorldTransform(WTransform(currentScale_, 0, 0, currentScale_, 0, 0));

    WTransform currentTransform = WTransform(1.0 / currentScale_, 0, 0, 1.0 / currentScale_, 0, 0) * chart_.zoomRangeTransform(series.yAxis());

    painter_.setPen(NoPen);
    painter_.setBrush(NoBrush);
    painter_.setShadow(series.shadow());
    if (currentMarkerType_ != CrossMarker &&
        currentMarkerType_ != XCrossMarker &&
        currentMarkerType_ != AsteriskMarker &&
        currentMarkerType_ != StarMarker) {
      painter_.setBrush(currentBrush_);

      if (!series.shadow().none())
	painter_.drawStencilAlongPath(marker_, currentTransform.map(pathFragment_), false);

      painter_.setShadow(WShadow());
    }
    painter_.setPen(currentPen_);
    if (!series.shadow().none())
      painter_.setBrush(NoBrush);

    painter_.drawStencilAlongPath(marker_, currentTransform.map(pathFragment_), false);

    painter_.restore();

    pathFragment_ = WPainterPath();
  }
};

// Used to find if a given point matches a marker, for tooltips
class MarkerMatchIterator : public SeriesIterator {
public:
  static const double MATCH_RADIUS;

  MarkerMatchIterator(const WCartesianChart &chart, double x, std::vector<double> ys, double rx, std::vector<double> rys)
    : chart_(chart),
      matchX_(x),
      rX_(rx),
      matchYs_(ys),
      rYs_(rys),
      matchedSeries_(0),
      matchedXRow_(-1),
      matchedXColumn_(-1),
      matchedYRow_(-1),
      matchedYColumn_(-1)
  { }

  bool startSeries(const WDataSeries &series, double groupWidth, int numBarGroups, int currentBarGroup)
  {
    return matchedSeries_ == 0 && (series.type() == PointSeries || series.type() == LineSeries || series.type() == CurveSeries);
  }

  void newValue(const WDataSeries &series, double x, double y, double stackY, int xRow, int xColumn, int yRow, int yColumn)
  {
    if (matchedSeries_)
      return; // we already have a match
    if (!Utils::isNaN(x) && !Utils::isNaN(y)) {
      const double *scaleFactorP = series.model()->markerScaleFactor(yRow, yColumn);
      double scaleFactor = scaleFactorP != 0 ? *scaleFactorP : 1.0;
      if (scaleFactor < 1.0)
	scaleFactor = 1.0;
      double scaledRx = scaleFactor * rX_;
      double scaledRy = scaleFactor * rYs_[series.yAxis()];
      
      WPointF p = chart_.map(x, y, series.yAxis(), currentXSegment(), currentYSegment());
      double dx = p.x() - matchX_;
      double dy = p.y() - matchYs_[series.yAxis()];
      double dx2 = dx * dx;
      double dy2 = dy * dy;
      double rx2 = scaledRx * scaledRx;
      double ry2 = scaledRy * scaledRy;
      if (dx2/rx2 + dy2/ry2 <= 1) {
	matchedXRow_ = xRow;
	matchedXColumn_ = xColumn;
	matchedYRow_ = yRow;
	matchedYColumn_ = yColumn;
	matchedSeries_ = &series;
      }
    }
  }

  const WDataSeries *matchedSeries() const { return matchedSeries_; }
  int xRow() const { return matchedXRow_; }
  int xColumn() const { return matchedXColumn_; }
  int yRow() const { return matchedYRow_; }
  int yColumn() const { return matchedYColumn_; }

private:
  const WCartesianChart &chart_;
  double matchX_, rX_;
  std::vector<double> matchYs_;
  std::vector<double> rYs_;
  const WDataSeries *matchedSeries_;
  int matchedXRow_, matchedXColumn_, matchedYRow_, matchedYColumn_;
};

const double MarkerMatchIterator::MATCH_RADIUS = 5;

WCartesianChart::AxisStruct::AxisStruct()
  : axis(new WAxis()),
    calculatedWidth(0),
    transformChanged(0)
{ }

WCartesianChart::AxisStruct::AxisStruct(WAxis *ax)
  : axis(ax),
    calculatedWidth(0),
    transformChanged(0)
{ }

WCartesianChart::AxisStruct::~AxisStruct()
{
  delete axis;
  delete transformChanged;
}

WCartesianChart::WCartesianChart(WContainerWidget *parent)
  : WAbstractChart(parent),
    interface_(new WChart2DImplementation(this)),
    orientation_(Vertical),
    XSeriesColumn_(-1),
    type_(CategoryChart),
    barMargin_(0),
    axisPadding_(5),
    borderPen_(NoPen),
    hasDeferredToolTips_(false),
    jsDefined_(false),
    zoomEnabled_(false),
    panEnabled_(false),
    rubberBandEnabled_(true),
    crosshairEnabled_(false),
    crosshairColor_(black),
    crosshairYAxis_(0),
    seriesSelectionEnabled_(false),
    selectedSeries_(0),
    followCurve_(0),
    curveManipulationEnabled_(false),
    onDemandLoadingEnabled_(false),
    loadingBackground_(lightGray),
    cObjCreated_(false),
    jsSeriesSelected_(this, "seriesSelected"),
    loadTooltip_(this, "loadTooltip")
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
    hasDeferredToolTips_(false),
    jsDefined_(false),
    zoomEnabled_(false),
    panEnabled_(false),
    rubberBandEnabled_(true),
    crosshairEnabled_(false),
    crosshairColor_(black),
    crosshairYAxis_(0),
    seriesSelectionEnabled_(false),
    selectedSeries_(0),
    followCurve_(0),
    curveManipulationEnabled_(false),
    onDemandLoadingEnabled_(false),
    loadingBackground_(lightGray),
    cObjCreated_(false),
    jsSeriesSelected_(this, "seriesSelected"),
    loadTooltip_(this, "loadTooltip")
{
  init();
}

WCartesianChart::~WCartesianChart()
{
  while (!yAxes_.empty()) {
    delete yAxes_.back();
    yAxes_.pop_back();
  }
  delete interface_;
  std::vector<WAxisSliderWidget *> copy = std::vector<WAxisSliderWidget *>(axisSliderWidgets_);
  axisSliderWidgets_.clear();
  for (std::size_t i = 0; i < copy.size(); ++i) {
    copy[i]->setSeries(0);
  }
  for (std::size_t i = 0; i < series_.size(); ++i) {
    delete series_[i];
  }
}

void WCartesianChart::init()
{
  setPalette(new WStandardPalette(WStandardPalette::Muted));

  yAxes_.push_back(new AxisStruct());
  yAxes_.push_back(new AxisStruct());
  yAxes_.back()->axis->setLocation(MaximumValue);

  axis(XAxis).init(interface_, XAxis);
  axis(YAxis).init(interface_, YAxis);
  axis(Y2Axis).init(interface_, Y2Axis);
  axis(Y2Axis).setVisible(false);

  axis(XAxis).setPadding(axisPadding_);
  axis(YAxis).setPadding(axisPadding_);
  axis(Y2Axis).setPadding(axisPadding_);

  axis(XAxis).setSoftLabelClipping(true);
  axis(YAxis).setSoftLabelClipping(true);
  axis(Y2Axis).setSoftLabelClipping(true);
  
  setPlotAreaPadding(40, Left | Right);
  setPlotAreaPadding(30, Top | Bottom);

  xAxis_.transformHandle = createJSTransform();
  xAxis_.transformChanged = new JSignal<>(this, "xTransformChanged");

  for (int i = 0; i < 2; ++i) {
    yAxes_[i]->transformHandle = createJSTransform();
    yAxes_[i]->transformChanged = new JSignal<>(this, "yTransformChanged" + boost::lexical_cast<std::string>(i));
  }

  if (WApplication::instance() != 0 && WApplication::instance()->environment().ajax()) {
    mouseWentDown().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.mouseDown(o, e);}}");
    mouseWentUp().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.mouseUp(o, e);}}");
    mouseDragged().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.mouseDrag(o, e);}}");
    mouseMoved().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.mouseMove(o, e);}}");
    mouseWheel().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.mouseWheel(o, e);}}");
    mouseWentOut().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.mouseOut(o, e);}}");
    touchStarted().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.touchStart(o, e);}}");
    touchEnded().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.touchEnd(o, e);}}");
    touchMoved().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.touchMoved(o, e);}}");
    clicked().connect("function(o, e){var o=" + this->cObjJsRef() + ";if(o){o.clicked(o, e);}}");
    jsSeriesSelected_.connect(this, &WCartesianChart::jsSeriesSelected);
    loadTooltip_.connect(this, &WCartesianChart::loadTooltip);
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
    xAxis_.axis->init(interface_, XAxis);
    update();
  }
}

void WCartesianChart::setTextPen(const WPen& pen)
{
  if(pen == textPen_)
    return;
  
  textPen_ = pen;

  xAxis_.axis->setTextPen(pen);
  for (std::size_t i = 0; i < yAxes_.size(); ++i)
    yAxes_[i]->axis->setTextPen(pen);
}

#ifndef WT_TARGET_JAVA
void WCartesianChart::addSeries(const WDataSeries& series)
{
  addSeries(new WDataSeries(series));
}
#endif

void WCartesianChart::addSeries(WDataSeries *series)
{
  series_.push_back(series);
  series_.back()->setChart(this);

  if (series->type() == LineSeries || series->type() == CurveSeries) {
    assignJSPathsForSeries(*series);
    assignJSTransformsForSeries(*series);
  }

  update();
}

void WCartesianChart::assignJSHandlesForAllSeries()
{
  if (!isInteractive()) return;
  for (std::size_t i = 0; i < series_.size(); ++i) {
    const WDataSeries &s = *series_[i];
    if (s.type() == LineSeries || s.type() == CurveSeries) {
      assignJSPathsForSeries(s);
      assignJSTransformsForSeries(s);
    }
  }
}

void WCartesianChart::freeJSHandlesForAllSeries()
{
  freeAllJSPaths();
  freeAllJSTransforms();
}

void WCartesianChart::assignJSPathsForSeries(const WDataSeries& series)
{
  WJavaScriptHandle<WPainterPath> handle;
  if (freePainterPaths_.size() > 0) {
    handle = freePainterPaths_.back();
    freePainterPaths_.pop_back();
  } else {
    handle = createJSPainterPath();
  }
  curvePaths_[&series] = handle;
}

void WCartesianChart::assignJSTransformsForSeries(const WDataSeries &series)
{
  WJavaScriptHandle<WTransform> handle;
  if (freeTransforms_.size() > 0) {
    handle = freeTransforms_.back();
    freeTransforms_.pop_back();
  } else {
    handle = createJSTransform();
  }
  curveTransforms_[&series] = handle;
}

void WCartesianChart::removeSeries(int modelColumn)
{
  for (std::size_t i = 0; i < series_.size(); ++i) {
    if (series_[i]->modelColumn() == modelColumn) {
      removeSeries(series_[i]);
      return;
    }
  }
}

void WCartesianChart::removeSeries(WDataSeries *series)
{
  int index = seriesIndexOf(*series);

  if (index != -1) {
    for (std::size_t i = 0; i < axisSliderWidgets_.size(); ++i) {
      if (axisSliderWidgets_[i]->series() == series) {
	axisSliderWidgets_[i]->setSeries(0);
      }
    }
    if (series->type() == LineSeries || series->type() == CurveSeries) {
      freeJSPathsForSeries(*series);
      freeJSTransformsForSeries(*series);
    }

    delete series_[index];
    series_.erase(series_.begin() + index);
    update();
  }
}

void WCartesianChart::freeJSPathsForSeries(const WDataSeries &series)
{
  freePainterPaths_.push_back(curvePaths_[&series]);
  curvePaths_.erase(&series);
}

void WCartesianChart::freeJSTransformsForSeries(const WDataSeries &series)
{
  freeTransforms_.push_back(curveTransforms_[&series]);
  curveTransforms_.erase(&series);
}

int WCartesianChart::seriesIndexOf(int modelColumn) const
{
  for (unsigned i = 0; i < series_.size(); ++i)
    if (series_[i]->modelColumn() == modelColumn)
      return i;

  return -1;
}

int WCartesianChart::seriesIndexOf(const WDataSeries &series) const
{
  for (unsigned i = 0; i < series_.size(); ++i)
    if (series_[i] == &series)
      return i;

  return -1;
}

WDataSeries& WCartesianChart::series(int modelColumn)
{
  int index = seriesIndexOf(modelColumn);

  if (index != -1)
    return *series_[index];

  throw WException("Column " + boost::lexical_cast<std::string>(modelColumn)
		   + " not in plot");
}

const WDataSeries& WCartesianChart::series(int modelColumn) const
{
  int index = seriesIndexOf(modelColumn);

  if (index != -1)
    return *series_[index];

  throw WException("Column " + boost::lexical_cast<std::string>(modelColumn)
		   + " not in plot");
}

#ifndef WT_TARGET_JAVA
void WCartesianChart::setSeries(const std::vector<WDataSeries>& series)
{
  std::vector<WDataSeries *> seriesCopy;
  seriesCopy.reserve(series.size());
  for (std::size_t i = 0; i < series.size(); ++i) {
    seriesCopy.push_back(new WDataSeries(series[i]));
  }
  setSeries(seriesCopy);
}
#endif

void WCartesianChart::setSeries(const std::vector<WDataSeries *> &series)
{
  for (std::size_t i = 0; i < series_.size(); ++i) {
    delete series_[i];
  }

  series_ = series;

  freeJSHandlesForAllSeries();
  assignJSHandlesForAllSeries();

  for (unsigned i = 0; i < series_.size(); ++i)
    series_[i]->setChart(this);

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

void WCartesianChart::freeAllJSTransforms()
{
  for (TransformMap::const_iterator it = curveTransforms_.begin();
       it != curveTransforms_.end(); ++it) {
    freeTransforms_.push_back(it->second);
  }
  curveTransforms_.clear();
}

WAxis& WCartesianChart::axis(Axis axis)
{
  if (axis == XAxis)
    return *xAxis_.axis;
  else
    return yAxis(axis == Y1Axis ? 0 : 1);
}

const WAxis& WCartesianChart::axis(Axis axis) const
{
  if (axis == XAxis)
    return *xAxis_.axis;
  else
    return yAxis(axis == Y1Axis ? 0 : 1);
}

void WCartesianChart::setAxis(WAxis *waxis, Axis axis)
{
  if (&this->axis(axis) == waxis)
    return;

  if (axis == XAxis) {
    delete xAxis_.axis;
    xAxis_.axis = waxis;
    xAxis_.axis->init(interface_, axis);
  } else {
    int yIndex = axis == Y1Axis ? 0 : 1;
    delete yAxes_[yIndex]->axis;
    yAxes_[yIndex]->axis = waxis;
    yAxes_[yIndex]->axis->init(interface_, axis);
  }
}

std::vector<WAxis*> WCartesianChart::yAxes()
{
  std::vector<WAxis*> result;
  result.reserve(yAxes_.size());
  for (std::size_t i = 0; i < yAxes_.size(); ++i)
    result.push_back(yAxes_[i]->axis);
  return result;
}

std::vector<const WAxis*> WCartesianChart::yAxes() const
{
  std::vector<const WAxis*> result;
  result.reserve(yAxes_.size());
  for (std::size_t i = 0; i < yAxes_.size(); ++i)
    result.push_back(yAxes_[i]->axis);
  return result;
}

int WCartesianChart::yAxisCount() const
{
  return static_cast<int>(yAxes_.size());
}

WAxis &WCartesianChart::yAxis(int i)
{
  return *yAxes_[i]->axis;
}

const WAxis &WCartesianChart::yAxis(int i) const
{
  return *yAxes_[i]->axis;
}

int WCartesianChart::addYAxis(WAxis *waxis)
{
  int idx = static_cast<int>(yAxes_.size());
  yAxes_.push_back(new AxisStruct(waxis));
  yAxes_[idx]->axis->initYAxis(interface_, idx);
  yAxes_[idx]->axis->setPadding(axisPadding());
  yAxes_[idx]->axis->setSoftLabelClipping(true);

  yAxes_[idx]->transformHandle = createJSTransform();
  yAxes_[idx]->transformChanged =
      new JSignal<>(this, "yTransformChanged" + boost::lexical_cast<std::string>(idx));

  update();

  return idx;
}

void WCartesianChart::removeYAxis(int yAxisId)
{
  {
    std::size_t i = 0;
    while (i < series_.size()) {
      if (series_[i]->yAxis() == yAxisId) {
        removeSeries(series_[i]);
      } else {
        if (series_[i]->yAxis() > yAxisId) {
          series_[i]->bindToYAxis(series_[i]->yAxis() - 1);
        }
        ++i;
      }
    }
  }
  if (crosshairYAxis() > yAxisId) {
    setCrosshairYAxis(crosshairYAxis() - 1);
  }
  clearPensForAxis(YAxis, yAxisId);
  delete yAxes_[yAxisId];
  yAxes_.erase(yAxes_.begin() + yAxisId);
  for (std::size_t i = 0; i < yAxes_.size(); ++i) {
    yAxes_[i]->axis->yAxis_ = static_cast<int>(i);
    yAxes_[i]->axis->axis_ = i == 1 ? Y2Axis : Y1Axis;
  }

  update();
}

void WCartesianChart::clearYAxes()
{
  while (!series_.empty())
    removeSeries(series_[series_.size() - 1]);
  clearPens();
  for (std::size_t i = 0; i < yAxes_.size(); ++i)
    delete yAxes_[i];
  yAxes_.clear();

  update();
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
  WText *label = new WText(series(index).model()->headerData(index));
  label->setVerticalAlignment(AlignTop);
  legendItem->addWidget(label);

  return legendItem;
}

void WCartesianChart::addDataPointArea(const WDataSeries& series,
				       int xRow, int xColumn,
				       WAbstractArea *area)
{
  if (areas().empty())
    addAreaMask();
  addArea(area);
}

WPointF WCartesianChart::mapFromDevice(const WPointF &point, Axis ordinateAxis) const
{
  return mapFromDevice(point, ordinateAxis == Y1Axis ? 0 : 1);
}

WPointF WCartesianChart::mapFromDevice(const WPointF &point, int ordinateAxis) const
{
  if (isInteractive()) {
    return mapFromDeviceWithoutTransform(
          zoomRangeTransform(xAxis_.transformHandle.value(),
                             yAxes_[ordinateAxis]->transformHandle.value())
          .inverted()
          .map(point),
          ordinateAxis);
  } else {
    return mapFromDeviceWithoutTransform(point, ordinateAxis);
  }
}

WPointF WCartesianChart::mapFromDeviceWithoutTransform(const WPointF& point, Axis ordinateAxis)
  const
{
  return mapFromDeviceWithoutTransform(point, ordinateAxis == Y1Axis ? 0 : 1);
}

WPointF WCartesianChart::mapFromDeviceWithoutTransform(const WPointF& point, int ordinateAxis)
  const
{
  const WAxis& xAxis = axis(XAxis);
  const WAxis& yAxis = this->yAxis(ordinateAxis);

  WPointF p = inverseHv(point.x(), point.y(), width().toPixels());

  return WPointF(xAxis.mapFromDevice(p.x() - chartArea_.left()),
                 yAxis.mapFromDevice(chartArea_.bottom() - p.y()));
}

WPointF WCartesianChart::mapToDevice(const boost::any &xValue,
				     const boost::any &yValue,
				     Axis axis, int xSegment, int ySegment) const
{
  return mapToDevice(xValue, yValue, axis == Y1Axis ? 0 : 1, xSegment, ySegment);
}

WPointF WCartesianChart::mapToDevice(const boost::any &xValue,
                                     const boost::any &yValue,
                                     int axis, int xSegment, int ySegment) const
{
  if (isInteractive()) {
    return zoomRangeTransform(
            xAxis_.transformHandle.value(),
            yAxes_[axis]->transformHandle.value()
          ).map(mapToDeviceWithoutTransform(xValue, yValue, axis, xSegment, ySegment));
  } else {
    return mapToDeviceWithoutTransform(xValue, yValue, axis, xSegment, ySegment);
  }
}

WPointF WCartesianChart::mapToDeviceWithoutTransform(const boost::any& xValue,
				     const boost::any& yValue,
				     Axis ordinateAxis, int xSegment,
				     int ySegment) const
{
  return mapToDeviceWithoutTransform(xValue, yValue, ordinateAxis == Y1Axis ? 0 : 1, xSegment, ySegment);
}

WPointF WCartesianChart::mapToDeviceWithoutTransform(const boost::any& xValue,
                                     const boost::any& yValue,
                                     int ordinateAxis, int xSegment,
                                     int ySegment) const
{
  const WAxis& xAxis = axis(XAxis);
  const WAxis& yAxis = this->yAxis(ordinateAxis);

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

void WCartesianChart::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  if (app && (isInteractive() || hasDeferredToolTips_)) {
    LOAD_JAVASCRIPT(app, "js/ChartCommon.js", "ChartCommon", wtjs2);
    app->doJavaScript(std::string("if (!" WT_CLASS ".chartCommon) {"
				  WT_CLASS ".chartCommon = new ") +
		      WT_CLASS ".ChartCommon(" + app->javaScriptClass() + "); }", false);

    LOAD_JAVASCRIPT(app, "js/WCartesianChart.js", "WCartesianChart", wtjs1);
    jsDefined_ = true;
  } else {
    jsDefined_ = false;
  }
}

void WCartesianChart::render(WFlags<RenderFlag> flags)
{
  WAbstractChart::render(flags);

  if (flags & RenderFull || !jsDefined_) {
    defineJavaScript();
  }
}

void WCartesianChart::setFormData(const FormData& formData)
{
  WPaintedWidget::setFormData(formData);

  const WTransform &xTransform = xAxis_.transformHandle.value();
  std::vector<const WTransform *> yTransforms;
  for (std::size_t i = 0; i < yAxes_.size(); ++i) {
    yTransforms.push_back(&yAxes_[i]->transformHandle.value());
  }

  if (!axis(XAxis).zoomRangeDirty_) {
    WPointF devicePan =
      WPointF(xTransform.dx() / xTransform.m11(), 0.0);
    WPointF modelPan = WPointF(axis(XAxis).mapFromDevice(-devicePan.x()), 0.0);
    if (xTransform.isIdentity()) {
      axis(XAxis).setZoomRangeFromClient(WAxis::AUTO_MINIMUM, WAxis::AUTO_MAXIMUM);
    } else {
      double z = xTransform.m11();
      double x = modelPan.x();
      double min = axis(XAxis).mapFromDevice(0.0);
      double max = axis(XAxis).mapFromDevice(axis(XAxis).fullRenderLength_);
      double x2 = x + ((max - min)/ z);
      axis(XAxis).setZoomRangeFromClient(x, x2);
    }
  }
  for (int i = 0; i < yAxisCount(); ++i) {
    if (!yAxis(i).zoomRangeDirty_) {
      WPointF devicePan(0.0, yTransforms[i]->dy() / yTransforms[i]->m22());
      WPointF modelPan(0.0, yAxis(i).mapFromDevice(-devicePan.y()));
      if (yTransforms[i]->isIdentity()) {
        yAxis(i).setZoomRangeFromClient(WAxis::AUTO_MINIMUM, WAxis::AUTO_MAXIMUM);
      } else {
        double z = yTransforms[i]->m22();
        double y = modelPan.y();
        double min = yAxis(i).mapFromDevice(0.0);
        double max = yAxis(i).mapFromDevice(yAxis(i).fullRenderLength_);
        double y2 = y + ((max - min) / z);
        yAxis(i).setZoomRangeFromClient(y, y2);
      }
    }
  }
  if (curveTransforms_.size() != 0) {
    for (std::size_t i = 0; i < series_.size(); ++i) {
      WDataSeries &s = *series_[i];
      int yAxis = s.yAxis();
      if (yAxis >= 0 && yAxis < yAxisCount()) {
        if ((s.type() == LineSeries || s.type() == CurveSeries) && !s.isHidden()) {
          if (!s.scaleDirty_) {
            s.scale_ = curveTransforms_[&s].value().m22();
          }
          if (!s.offsetDirty_) {
            double origin;
            if (orientation() == Horizontal) {
              origin = mapToDeviceWithoutTransform(0.0, 0.0, yAxis).x();
            } else {
              origin = mapToDeviceWithoutTransform(0.0, 0.0, yAxis).y();
            }
            double dy = curveTransforms_[&s].value().dy();
            double scale = curveTransforms_[&s].value().m22();
            double offset = - dy + origin * (1 - scale) + this->yAxis(yAxis).mapToDevice(0.0, 0);
            if (orientation() == Horizontal) {
              s.offset_ = - this->yAxis(yAxis).mapFromDevice(offset);
            } else {
              s.offset_ = this->yAxis(yAxis).mapFromDevice(offset);
            }
          }
	}
      }
    }
  }
}

void WCartesianChart::modelChanged()
{
  XSeriesColumn_ = -1;
  while (axisSliderWidgets_.size() > 0) {
    axisSliderWidgets_[axisSliderWidgets_.size() - 1]->setSeries(0);
  }
  freeAllJSPaths();
  freeAllJSTransforms();
  for (std::size_t i = 0; i < series_.size(); ++i) {
    delete series_[i];
  }
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
  xAxis_.axis->setPadding(padding);
  for (std::size_t i = 0; i < yAxes_.size(); ++i)
    yAxes_[i]->axis->setPadding(padding);
}

void WCartesianChart::setBorderPen(const WPen& pen)
{
  if (borderPen_ != pen) {
    borderPen_ = pen;
    update();
  }
}

void WCartesianChart::setSeriesSelectionEnabled(bool enabled)
{
  if (seriesSelectionEnabled_ != enabled) {
    seriesSelectionEnabled_ = enabled;
    updateJSConfig("seriesSelection", seriesSelectionEnabled_);
  }
}

void WCartesianChart::setSelectedSeries(const WDataSeries *series)
{
  if (selectedSeries_ != series) {
    selectedSeries_ = series;
    update();
  }
}

void WCartesianChart::setCurveManipulationEnabled(bool enabled)
{
  if (curveManipulationEnabled_ != enabled) {
    curveManipulationEnabled_ = enabled;
    updateJSConfig("curveManipulation", curveManipulationEnabled_);
  }
}

void WCartesianChart::addCurveLabel(const CurveLabel &label)
{
  curveLabels_.push_back(label);
  update();
}

void WCartesianChart::setCurveLabels(const std::vector<CurveLabel> &labels)
{
  curveLabels_ = labels;
  update();
}

void WCartesianChart::clearCurveLabels()
{
  curveLabels_.clear();
  update();
}

void WCartesianChart::setOnDemandLoadingEnabled(bool enabled)
{
  if (onDemandLoadingEnabled_ != enabled) {
    onDemandLoadingEnabled_ = enabled;
    update();
  }
}

void WCartesianChart::setLoadingBackground(const WBrush &brush)
{
  if (loadingBackground_ != brush) {
    loadingBackground_ = brush;
    if (onDemandLoadingEnabled())
      update();
  }
}

bool WCartesianChart::isInteractive() const
{
  return !yAxes_.empty() && (zoomEnabled_ || panEnabled_ || crosshairEnabled_ || followCurve_ != 0 ||
	  axisSliderWidgets_.size() > 0 || seriesSelectionEnabled_ || curveManipulationEnabled_) && getMethod() == HtmlCanvas;
}

WPainterPath WCartesianChart::pathForSeries(const WDataSeries &series) const
{
  PainterPathMap::const_iterator it =  curvePaths_.find(&series);
  if (it == curvePaths_.end()) {
    return WPainterPath();
  } else {
    return it->second.value();
  }
}

void WCartesianChart::updateJSPensForAxis(WStringStream& js, Axis axis, int yAxis) const
{
  const std::vector<PenAssignment> pens =
      axis == XAxis ? xAxis_.pens : yAxes_[yAxis]->pens;
  js << "[";
  for (std::size_t i = 0; i < pens.size(); ++i) {
    if (i != 0) {
      js << ",";
    }
    const PenAssignment& assignment = pens[i];
    js << "[";
    js << assignment.pen.jsRef();
    js << ",";
    js << assignment.textPen.jsRef();
    js << ",";
    js << assignment.gridPen.jsRef();
    js << "]";
  }
  js << "]";
}

void WCartesianChart::updateJSPens(WStringStream& js) const
{
  // pens[axis][level][]
  js << "pens:{x:";
  updateJSPensForAxis(js, XAxis, -1);
  js << ",y:[";
  for (int i = 0; i < yAxisCount(); ++i) {
    if (i != 0)
      js << ',';
    updateJSPensForAxis(js, YAxis, i);
  }
  js << "]},";
  js << "penAlpha:{x:[";
  js << axis(XAxis).pen().color().alpha() << ',';
  js << axis(XAxis).textPen().color().alpha() << ',';
  js << axis(XAxis).gridLinesPen().color().alpha();
  js << "],y:[";
  for (int i = 0; i < yAxisCount(); ++i) {
    if (i != 0)
      js << ',';
    js << '[' << yAxis(i).pen().color().alpha() << ',';
    js << yAxis(i).textPen().color().alpha() << ',';
    js << yAxis(i).gridLinesPen().color().alpha() << ']';
  }
  js << "]},";
}

int WCartesianChart::calcNumBarGroups() const
{
  int numBarGroups = 0;

  bool newGroup = true;
  for (unsigned i = 0; i < series_.size(); ++i)
    if (series_[i]->type() == BarSeries) {
      if (newGroup || !series_[i]->isStacked())
	++numBarGroups;
      newGroup = false;
    } else
      newGroup = true;

  return numBarGroups;
}

void WCartesianChart::setSoftLabelClipping(bool enabled)
{
  xAxis_.axis->setSoftLabelClipping(enabled);
  for (std::size_t i = 0; i < yAxes_.size(); ++i)
    yAxes_[i]->axis->setSoftLabelClipping(enabled);
}

bool WCartesianChart::axisSliderWidgetForSeries(WDataSeries *series) const
{
  for (std::size_t i = 0; i < axisSliderWidgets_.size(); ++i) {
    if (axisSliderWidgets_[i]->series() == series)
      return true;
  }
  return false;
}

void WCartesianChart::iterateSeries(SeriesIterator *iterator,
				    WPainter *painter,
                                    bool reverseStacked,
                                    bool extremesOnly) const
{
  double groupWidth = 0.0;
  int numBarGroups;
  int currentBarGroup;

  int rowCount = model() ? model()->rowCount() : 0;
  std::vector<double> posStackedValuesInit, minStackedValuesInit;

  const bool scatterPlot = type_ == ScatterPlot;

  if (scatterPlot) {
    numBarGroups = 1;
    currentBarGroup = 0;
  } else {
    numBarGroups = calcNumBarGroups();
    currentBarGroup = 0;
    posStackedValuesInit.insert(posStackedValuesInit.begin(), rowCount, 0.0);
    minStackedValuesInit.insert(minStackedValuesInit.begin(), rowCount, 0.0);
  }

  bool containsBars = false;

  for (unsigned g = 0; g < series_.size(); ++g) {
    if (series_[g]->isHidden() &&
	!(axisSliderWidgetForSeries(series_[g]) &&
	  (dynamic_cast<SeriesRenderIterator *>(iterator) ||
	   dynamic_cast<ExtremesIterator *>(iterator))))
      continue;

    groupWidth = series_[g]->barWidth() * (axis(XAxis).mapToDevice(2) - axis(XAxis).mapToDevice(1));

    if (containsBars)
      ++currentBarGroup;
    containsBars = false;

    int startSeries, endSeries;

    if (scatterPlot) {
      startSeries = endSeries = g;
    } else if (series_[g]->model() == model()) {
      for (int i = 0; i < rowCount; ++i)
        posStackedValuesInit[i] = minStackedValuesInit[i] = 0.0;

      if (reverseStacked) {
	endSeries = g;

        int a = series_[g]->yAxis();

	for (;;) {
	  if (g < series_.size()
	      && (((int)g == endSeries) || series_[g]->isStacked())
              && (series_[g]->yAxis() == a)) {
	    if (series_[g]->type() == BarSeries)
	      containsBars = true;

            for (int row = 0; row < rowCount; ++row) {
	      double y = asNumber(model()->data(row, series_[g]->modelColumn()));

	      if (!Utils::isNaN(y)) {
		if (y > 0)
		  posStackedValuesInit[row] += y;
		else
		  minStackedValuesInit[row] += y;
	      }
	    }

	    ++g;
	  } else
	    break;
	}

	--g;
	startSeries = g;
      } else {
	startSeries = g;

        int a = series_[g]->yAxis();

	if (series_[g]->type() == BarSeries)
	  containsBars = true;
	++g;

	for (;;) {
	  if (g < series_.size() && series_[g]->isStacked()
              && series_[g]->yAxis() == a) {
	    if (series_[g]->type() == BarSeries)
	      containsBars = true;
	    ++g;
	  } else
	    break;
	}

	--g;

	endSeries = g;
      }
    } else {
      throw WException("Configuring different models for WDataSeries are unsupported "
		       "for category charts!");
    }

    int i = startSeries;
    for (;;) {
      bool doSeries = series_[i]->yAxis() >= 0 && series_[i]->yAxis() < yAxisCount() &&
	iterator->startSeries(*series_[i], groupWidth, numBarGroups,
			      currentBarGroup);

      std::vector<double> posStackedValues, minStackedValues;

      if (doSeries ||
	  (!scatterPlot && i != endSeries)) {

	for (int currentXSegment = 0;
	     currentXSegment < axis(XAxis).segmentCount();
	     ++currentXSegment) {

	  for (int currentYSegment = 0;
               currentYSegment < yAxis(series_[i]->yAxis()).segmentCount();
	       ++currentYSegment) {

	    posStackedValues.clear();
	    Utils::insert(posStackedValues, posStackedValuesInit);
	    minStackedValues.clear();
	    Utils::insert(minStackedValues, minStackedValuesInit);

	    if (painter) {
              WRectF csa = chartSegmentArea(yAxis(series_[i]->yAxis()),
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

            int startRow = 0;
            int endRow = series_[i]->model() ? series_[i]->model()->rowCount() : 0;

            if (isInteractive() &&
                !extremesOnly &&
                onDemandLoadingEnabled() &&
                series_[i]->model() &&
                !axisSliderWidgetForSeries(series_[i])) {
              int xColumn = series_[i]->XSeriesColumn() == -1 ? XSeriesColumn() : series_[i]->XSeriesColumn();
              double zoomMin = axis(XAxis).zoomMinimum();
              double zoomMax = axis(XAxis).zoomMaximum();
              double zoomRange = zoomMax - zoomMin;
              if (xColumn == -1) {
                startRow = std::max(0, static_cast<int>(zoomMin - zoomRange));
                endRow = std::min(endRow, static_cast<int>(std::ceil(zoomMax + zoomRange)) + 1);
              } else {
                startRow = std::max(binarySearchRow(*series_[i]->model(),
                                                    xColumn,
                                                    zoomMin - zoomRange,
                                                    0, series_[i]->model()->rowCount() - 1) - 1, startRow);
                endRow = std::min(binarySearchRow(*series_[i]->model(),
                                                  xColumn,
                                                  zoomMax + zoomRange,
                                                  0, series_[i]->model()->rowCount() - 1) + 1, endRow);
              }
            }

            for (int row = startRow; row < endRow; ++row) {
	      int xIndex[] = {-1, -1};
	      int yIndex[] = {-1, -1};

	      double x;
	      if (scatterPlot) {
		int c = series_[i]->XSeriesColumn();
		if (c == -1)
		  c = XSeriesColumn();
		if (c != -1) {
		  xIndex[0] = row;
		  xIndex[1] = c;
		  x = series_[i]->model()->data(xIndex[0], xIndex[1]);
		} else
		  x = row;
	      } else
		x = row;

	      yIndex[0] = row;
	      yIndex[1] = series_[i]->modelColumn();
	      double y = series_[i]->model()->data(yIndex[0], yIndex[1]);

	      if (scatterPlot)
		iterator->newValue(*series_[i], x, y, 0, xIndex[0], xIndex[1], yIndex[0], yIndex[1]);
	      else {
		double prevStack = 0, nextStack = 0;

		bool hasValue = !Utils::isNaN(y);

		if (hasValue) {
		  if (y > 0)
		    prevStack = nextStack = posStackedValues[row];
		  else
		    prevStack = nextStack = minStackedValues[row];

		  if (reverseStacked)
		    nextStack -= y;
		  else
		    nextStack += y;

		  if (y > 0)
		    posStackedValues[row] = nextStack;
		  else
		    minStackedValues[row] = nextStack;
		}

		if (doSeries) {
		  if (reverseStacked)
		    iterator->newValue(*series_[i], x, hasValue ? prevStack : y,
				       nextStack, xIndex[0], xIndex[1], yIndex[0], yIndex[1]);
		  else
		    iterator->newValue(*series_[i], x, hasValue ? nextStack : y,
				       prevStack, xIndex[0], xIndex[1], yIndex[0], yIndex[1]);
		}
	      }

              if (extremesOnly && onDemandLoadingEnabled())
                row = std::max(endRow - 2, row);
	    }

	    iterator->endSegment();

	    if (painter)
	      painter->restore();
	  }
	}

	posStackedValuesInit.clear();
	Utils::insert(posStackedValuesInit, posStackedValues);
	minStackedValuesInit.clear();
	Utils::insert(minStackedValuesInit, minStackedValues);
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

WRectF WCartesianChart::insideChartArea() const
{
  const WAxis& xAxis = axis(XAxis);

  const WAxis::Segment& xs = xAxis.segments_[0];

  // margin used when clipping, see also WAxis::prepareRender(),
  // when the renderMinimum/maximum is 0, clipping is done exact
  double xRenderStart = xAxis.inverted() ? xAxis.mapToDevice(xs.renderMaximum, 0) : xs.renderStart;
  double xRenderEnd = xAxis.inverted() ? xAxis.mapToDevice(xs.renderMinimum, 0) : xs.renderStart + xs.renderLength;

  double yRenderStart = 0;
  double yRenderEnd = 0;
  if (yAxisCount() >= 1) {
    const WAxis& yAxis = axis(YAxis);
    const WAxis::Segment& ys = yAxis.segments_[0];
    yRenderStart = yAxis.inverted() ? yAxis.mapToDevice(ys.renderMaximum, 0) : ys.renderStart;
    yRenderEnd = yAxis.inverted() ? yAxis.mapToDevice(ys.renderMinimum, 0) : ys.renderStart + ys.renderLength;
  }

  double x1 = chartArea_.left() + xRenderStart;
  double x2 = chartArea_.left() + xRenderEnd;
  double y1 = chartArea_.bottom() - yRenderEnd;
  double y2 = chartArea_.bottom() - yRenderStart;

  return WRectF(x1, y1, x2 - x1, y2 - y1);
}

void WCartesianChart::setZoomAndPan()
{
  WTransform xTransform;
  if (axis(XAxis).zoomMin_ != WAxis::AUTO_MINIMUM ||
      axis(XAxis).zoomMax_ != WAxis::AUTO_MAXIMUM) {
    double xPan = -axis(XAxis).mapToDevice(axis(XAxis).pan(), 0);
    double xZoom = axis(XAxis).zoom();
    if (xZoom > axis(XAxis).maxZoom())
      xZoom = axis(XAxis).maxZoom();
    if (xZoom < axis(XAxis).minZoom())
      xZoom = axis(XAxis).minZoom();
    xTransform = WTransform(xZoom, 0, 0, 1, xZoom * xPan, 0);
  } else {
    double xZoom = axis(XAxis).minZoom();
    xTransform = WTransform(xZoom, 0, 0, 1, 0, 0);
  }

  std::vector<WTransform> yTransforms;
  for (int i = 0; i < yAxisCount(); ++i) {
    if (yAxis(i).zoomMin_ != WAxis::AUTO_MINIMUM ||
        yAxis(i).zoomMax_ != WAxis::AUTO_MAXIMUM) {
      double yPan = -yAxis(i).mapToDevice(yAxis(i).pan(), 0);
      double yZoom = yAxis(i).zoom();
      if (yZoom > yAxis(i).maxZoom())
        yZoom = yAxis(i).maxZoom();
      if (yZoom < yAxis(i).minZoom())
        yZoom = yAxis(i).minZoom();
      yTransforms.push_back(WTransform(1, 0, 0, yZoom, 0, yZoom * yPan));
    } else {
      double yZoom = yAxis(i).minZoom();
      yTransforms.push_back(WTransform(1, 0, 0, yZoom, 0, 0));
    }
  }

  // Enforce limits
  WRectF chartArea = hv(insideChartArea());
  for (int i = 0; i < yAxisCount(); ++i) {
    WRectF transformedArea = zoomRangeTransform(xTransform, yTransforms[i]).map(chartArea);
    if (transformedArea.left() > chartArea.left()) {
      double diff = chartArea.left() - transformedArea.left();
      if (orientation() == Vertical)
        xTransform = WTransform(1, 0, 0, 1, diff, 0) * xTransform;
      else
        yTransforms[i] = WTransform(1, 0, 0, 1, 0, diff) * yTransforms[i];
      transformedArea = zoomRangeTransform(xTransform, yTransforms[i]).map(chartArea);
    }
    if (transformedArea.right() < chartArea.right()) {
      double diff = chartArea.right() - transformedArea.right();
      if (orientation() == Vertical)
        xTransform = WTransform(1, 0, 0, 1, diff, 0) * xTransform;
      else
        yTransforms[i] = WTransform(1, 0, 0, 1, 0, diff) * yTransforms[i];
      transformedArea = zoomRangeTransform(xTransform, yTransforms[i]).map(chartArea);
    }
    if (transformedArea.top() > chartArea.top()) {
      double diff = chartArea.top() - transformedArea.top();
      if (orientation() == Vertical)
        yTransforms[i] = WTransform(1, 0, 0, 1, 0, -diff) * yTransforms[i];
      else
        xTransform = WTransform(1, 0, 0, 1, diff, 0) * xTransform;
      transformedArea = zoomRangeTransform(xTransform, yTransforms[i]).map(chartArea);
    }
    if (transformedArea.bottom() < chartArea.bottom()) {
      double diff = chartArea.bottom() - transformedArea.bottom();
      if (orientation() == Vertical)
        yTransforms[i] = WTransform(1, 0, 0, 1, 0, -diff) * yTransforms[i];
      else
        xTransform = WTransform(1, 0, 0, 1, diff, 0) * xTransform;
      transformedArea = zoomRangeTransform(xTransform, yTransforms[i]).map(chartArea);
    }
  }

  xAxis_.transformHandle.setValue(xTransform);
  for (int i = 0; i < yAxisCount(); ++i) {
    yAxes_[i]->transformHandle.setValue(yTransforms[i]);
  }

  axis(XAxis).zoomRangeDirty_ = false;
  for (int i = 0; i < yAxisCount(); ++i) {
    yAxis(i).zoomRangeDirty_ = false;
  }
}

void WCartesianChart::paintEvent(WPaintDevice *paintDevice)
{
  hasDeferredToolTips_ = false;

  WPainter painter(paintDevice);
  painter.setRenderHint(WPainter::Antialiasing);
  paint(painter);

  if (hasDeferredToolTips_ && !jsDefined_) {
    // We need to define the JavaScript after all, because we need to be able
    // to load deferred tooltips.
    defineJavaScript();
  }

  if (isInteractive() || hasDeferredToolTips_) {
    setZoomAndPan();

    std::vector<WRectF> modelAreas;

    for (int i = 0; i < yAxisCount(); ++i) {
      double modelBottom = yAxis(i).mapFromDevice(0);
      double modelTop = yAxis(i).mapFromDevice(chartArea_.height());
      double modelLeft = axis(XAxis).mapFromDevice(0);
      double modelRight = axis(XAxis).mapFromDevice(chartArea_.width());
      WRectF modelArea(modelLeft, modelBottom, modelRight - modelLeft, modelTop - modelBottom);
      modelAreas.push_back(modelArea);
    }

    WRectF insideArea = insideChartArea();

    int coordPaddingX = 5,
	coordPaddingY = 5;

    if (orientation() == Vertical) {
      if (axis(XAxis).isVisible() && axis(XAxis).tickDirection() == Inwards &&
          (axis(XAxis).location() == MaximumValue || axis(XAxis).location() == BothSides))
        coordPaddingY = 25;
      for (int i = 0; i < yAxisCount(); ++i) {
        if (yAxis(i).isVisible() &&
            (yAxes_[i]->location.initLoc == MaximumValue ||
             yAxes_[i]->location.initLoc == BothSides)) {
          if (yAxis(i).tickDirection() == Inwards)
            coordPaddingX = 40;
          break;
        }
      }
    } else {
      if (axis(XAxis).isVisible() && axis(XAxis).tickDirection() == Inwards &&
          (axis(XAxis).location() == MaximumValue || axis(XAxis).location() == BothSides))
        coordPaddingX = 40;
      for (int i = 0; i < yAxisCount(); ++i) {
        if (yAxis(i).isVisible() &&
            (yAxes_[i]->location.initLoc == MinimumValue ||
             yAxes_[i]->location.initLoc == BothSides)) {
          if (yAxis(i).tickDirection() == Inwards)
            coordPaddingY = 25;
          break;
        }
      }
    }

    if ((axis(XAxis).zoomRangeChanged().isConnected() ||
         onDemandLoadingEnabled()) &&
        !xAxis_.transformChanged->isConnected()) {
      xAxis_.transformChanged->connect(this, &WCartesianChart::xTransformChanged);
    }
    for (int i = 0; i < yAxisCount(); ++i) {
      if ((yAxis(i).zoomRangeChanged().isConnected() ||
           onDemandLoadingEnabled()) &&
          !yAxes_[i]->transformChanged->isConnected()) {
        const int axis = i; // Fix for JWt
        yAxes_[i]->transformChanged->connect(boost::bind(&WCartesianChart::yTransformChanged, this, axis));
      }
    }

    char buf[30];
    WApplication *app = WApplication::instance();
    WStringStream ss;
    int selectedCurve = selectedSeries_ ? seriesIndexOf(*selectedSeries_) : -1;
    int followCurve = followCurve_ ? seriesIndexOf(*followCurve_) : -1;
    ss << "new " WT_CLASS ".WCartesianChart("
       << app->javaScriptClass() << ","
       << jsRef() << ","
       << objJsRef() << ","
	"{"
	  "curveManipulation:" << asString(curveManipulationEnabled_).toUTF8() << ","
	  "seriesSelection:" << asString(seriesSelectionEnabled_).toUTF8() << ","
	  "selectedCurve:" << selectedCurve << ","
	  "isHorizontal:" << asString(orientation() == Horizontal).toUTF8() << ","
	  "zoom:" << asString(zoomEnabled_).toUTF8() << ","
	  "pan:" << asString(panEnabled_).toUTF8() << ","
	  "crosshair:" << asString(crosshairEnabled_).toUTF8() << ","
          "crosshairAxis:" << crosshairYAxis_ << ","
          "crosshairColor:" << jsStringLiteral(crosshairColor_.cssText(true)) << ","
	  "followCurve:" << followCurve << ","
          "xTransform:" << xAxis_.transformHandle.jsRef() << ","
          "yTransforms:[";
    for (int i = 0; i < yAxisCount(); ++i) {
      if (i != 0)
        ss << ',';
      ss << yAxes_[i]->transformHandle.jsRef();
    }
    ss << "],"
	  "area:" << hv(chartArea_).jsRef() << ","
	  "insideArea:" << hv(insideArea).jsRef() << ","
          "modelAreas:[";
    for (int i = 0; i < yAxisCount(); ++i) {
      if (i != 0)
        ss << ',';
      ss << modelAreas[i].jsRef();
    }
    ss << "],";
    ss << "hasToolTips:" << asString(hasDeferredToolTips_).toUTF8() << ","
          "notifyTransform:{x:" << asString(axis(XAxis).zoomRangeChanged().isConnected() ||
                                            onDemandLoadingEnabled()).toUTF8() << ","
                           "y:[";
    for (int i = 0; i < yAxisCount(); ++i) {
      if (i != 0)
        ss << ',';
      ss << asString(yAxis(i).zoomRangeChanged().isConnected() ||
                     onDemandLoadingEnabled()).toUTF8();
    }
    ss << "]},"
	  "ToolTipInnerStyle:" << jsStringLiteral(app->theme()->utilityCssClass(ToolTipInner)) << ","
	  "ToolTipOuterStyle:" << jsStringLiteral(app->theme()->utilityCssClass(ToolTipOuter)) << ",";
    updateJSPens(ss);
    ss << "series:{";
    {
      bool firstCurvePath = true;
      for (std::size_t i = 0; i < series_.size(); ++i) {
	if (curvePaths_.find(series_[i]) != curvePaths_.end()) {
	  if (firstCurvePath) {
	    firstCurvePath = false;
	  } else {
	    ss << ",";
	  }
	  ss << (int)i << ":{";
	  ss << "curve:" << curvePaths_[series_[i]].jsRef() << ",";
	  ss << "transform:" << curveTransforms_[series_[i]].jsRef() << ",";
          ss << "axis:" << series_[i]->yAxis();
	  ss << "}";
	}
      }
    }
    ss << "},";
    ss << "minZoom:{x:" << Utils::round_js_str(axis(XAxis).minZoom(), 16, buf) << ","
                   "y:[";
    for (int i = 0; i < yAxisCount(); ++i) {
      if (i != 0)
        ss << ',';
      ss << Utils::round_js_str(yAxis(i).minZoom(), 16, buf);
    }
    ss << "]},";
    ss << "maxZoom:{x:" << Utils::round_js_str(axis(XAxis).maxZoom(), 16, buf) << ","
                   "y:[";
    for (int i = 0; i < yAxisCount(); ++i) {
      if (i != 0)
        ss << ',';
      ss << Utils::round_js_str(yAxis(i).maxZoom(), 16, buf);
    }
    ss << "]},";
    ss << "rubberBand:" << rubberBandEnabled_ << ',';
    ss << "sliders:[";
    for (std::size_t i = 0; i < axisSliderWidgets_.size(); ++i) {
      if (i != 0) ss << ',';
      ss << '"' << axisSliderWidgets_[i]->id() << '"';
    }
    ss << "],";
    ss << "wheelActions:" << wheelActionsToJson(wheelActions_) << ",";
    ss << "coordinateOverlayPadding:[" << coordPaddingX << ",";
    ss                                 << coordPaddingY << "],";
    ss << "xAxis:{";
    ss << "width:" << Utils::round_js_str(xAxis_.calculatedWidth, 16, buf) << ',';
    ss << "side:'" << locToJsString(xAxis_.location.initLoc) << '\'';
    ss << "},";
    ss << "yAxes:[";
    for (int i = 0; i < yAxisCount(); ++i) {
      if (i != 0)
        ss << ',';
      ss << '{';
      ss << "width:" << Utils::round_js_str(yAxes_[i]->calculatedWidth, 16, buf) << ',';
      ss << "side:'" << locToJsString(yAxes_[i]->location.initLoc) << "',";
      ss << "minOffset:" << yAxes_[i]->location.minOffset << ',';
      ss << "maxOffset:" << yAxes_[i]->location.maxOffset;
      ss << '}';
    }
    ss << "]});";

    doJavaScript(ss.str());

    cObjCreated_ = true;
  }
}

void WCartesianChart::getDomChanges(std::vector<DomElement *>& result, WApplication *app)
{
  WAbstractChart::getDomChanges(result, app);
  for (std::size_t i = 0; i < axisSliderWidgets_.size(); ++i) {
    axisSliderWidgets_[i]->update();
  }
}

void WCartesianChart::render(WPainter& painter, const WRectF& rectangle) const
{
  painter.save();
  painter.translate(rectangle.topLeft());

  if (initLayout(rectangle, painter.device())) {
    renderBackground(painter);
    renderGrid(painter, axis(XAxis));
    for (int i = 0; i < yAxisCount(); ++i)
      renderGrid(painter, yAxis(i));
    renderAxes(painter, Line); // render the axes (lines)
    renderSeries(painter);     // render the data series
    renderAxes(painter, Labels); // render the axes (labels)
    renderBorder(painter);
    renderCurveLabels(painter);
    renderLegend(painter);
    renderOther(painter);
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

  xAxis_.location.initLoc = MinimumValue;
  xAxis_.location.finLoc = MinimumValue;
  for (std::size_t i = 0; i < yAxes_.size(); ++i) {
    yAxes_[i]->location.initLoc = MinimumValue;
    yAxes_[i]->location.finLoc = MinimumValue;
  }

  WPaintDevice *d = device;
  if (!d)
    d = createPaintDevice();

  bool autoLayout = isAutoLayoutEnabled();
  if (autoLayout &&
      ((d->features() & WPaintDevice::HasFontMetrics) == 0)) {
    LOG_ERROR("setAutoLayout(): device does not have font metrics "
      "(not even server-side font metrics).");
    autoLayout = false;
  }

  // FIXME: eliminate this const_cast!
  WCartesianChart *self = const_cast<WCartesianChart *>(this);
  self->clearPens();
  if (isInteractive()) {
    self->createPensForAxis(XAxis, -1);
    for (int i = 0; i < yAxisCount(); ++i) {
      self->createPensForAxis(YAxis, i);
    }
  }

  if (autoLayout) {
    self->setPlotAreaPadding(40, Left | Right);
    self->setPlotAreaPadding(30, Top | Bottom);

    calcChartArea();

    xAxis_.transform = WTransform();
    for (int i = 0; i < yAxisCount(); ++i)
      yAxes_[i]->transform = WTransform();

    if (chartArea_.width() <= 5 || chartArea_.height() <= 5 || !prepareAxes(device)) {
      if (isInteractive()) {
        xAxis_.transform = xAxis_.transformHandle.value();
        for (int i = 0; i < yAxisCount(); ++i) {
          yAxes_[i]->transform = yAxes_[i]->transformHandle.value();
        }
      }
      return false;
    }

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
  }

  if (!device)
    delete d;

  calcChartArea();

  bool result = chartArea_.width() > 5 && chartArea_.height() > 5 && prepareAxes(device);

  if (isInteractive()) {
    xAxis_.transform = xAxis_.transformHandle.value();
    for (int i = 0; i < yAxisCount(); ++i)
      yAxes_[i]->transform = yAxes_[i]->transformHandle.value();
  } else {
    xAxis_.transform = WTransform();
    for (int i = 0; i < yAxisCount(); ++i)
      yAxes_[i]->transform = WTransform();
  }

  if (isInteractive()) {
    if (curvePaths_.empty()) {
      self->assignJSHandlesForAllSeries();
    }
  }

  return result;
}

void WCartesianChart::drawMarker(const WDataSeries& series,
				 WPainterPath& result) const
{
  drawMarker(series, series.marker(), result);
}

void WCartesianChart::drawMarker(const WDataSeries &series,
                                 MarkerType marker,
                                 WPainterPath &result) const
{
  const double size = 6.0;
  const double hsize = size/2;

  switch (marker) {
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
  case StarMarker: {
      double angle = M_PI / 2.0;
      for (int i = 0; i < 5; ++i) {
	double x = std::cos(angle) * hsize;
	double y = -std::sin(angle) * hsize;
	result.moveTo(0, 0);
	result.lineTo(x, y);
	angle += M_PI * 2.0 / 5.0;
      }
    }
    break;
  case InvertedTriangleMarker:
    result.moveTo(0, -0.6 * hsize);
    result.lineTo(-hsize, -0.6 * hsize);
    result.lineTo(0, hsize);
    result.lineTo(hsize, -0.6 * hsize);
    result.closeSubPath();
    break;
  case DiamondMarker: {
      double s = std::sqrt(2.0) * hsize;
      result.moveTo(0, s);
      result.lineTo(s, 0);
      result.lineTo(0, -s);
      result.lineTo(-s, 0);
      result.closeSubPath();
    }
    break;
  case AsteriskMarker: {
      double angle = M_PI / 2.0;
      for (int i = 0; i < 6; ++i) {
	double x = std::cos(angle) * hsize;
	double y = -std::sin(angle) * hsize;
	result.moveTo(0, 0);
	result.lineTo(x, y);
	angle += M_PI / 3.0;
      }
    }
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

  int width = (int)legendColumnWidth().toPixels();
  if (width < 100)
    width = 100;
  painter.drawText(pos.x() + 23, pos.y() - 9, width, 20,
		   AlignLeft | AlignMiddle,
		   series.model()->headerData(series.modelColumn()));
}

bool WCartesianChart::prepareAxes(WPaintDevice *device) const
{
  // No axes
  if (yAxes_.empty())
    return true;

  const WAxis& xAxis = axis(XAxis);

  Orientation yDir = orientation_;
  Orientation xDir = orientation_ == Vertical ? Horizontal : Vertical;

  if (!xAxis.prepareRender(xDir, chartArea_.width()))
    return false;

  for (std::size_t i = 0; i < yAxes_.size(); ++i)
    if (!yAxes_[i]->axis->prepareRender(yDir, chartArea_.height()))
      return false;

  if (xAxis.scale() == CategoryScale) {
    switch (xAxis.location()) {
    case MinimumValue:
    case ZeroValue:
      xAxis_.location.initLoc = MinimumValue;
      xAxis_.location.finLoc = MinimumValue;
      break;
    case MaximumValue:
      xAxis_.location.initLoc = MaximumValue;
      xAxis_.location.finLoc = MaximumValue;
      break;
    case BothSides:
      xAxis_.location.initLoc = BothSides;
      xAxis_.location.finLoc = BothSides;
    }
  } else {
    AxisValue xLocation = xAxis_.axis->location();
    xAxis_.location.initLoc = xLocation;

    if (xLocation == ZeroValue) {
      if (yAxes_[0]->axis->segments_.back().renderMaximum < 0)
        xLocation = MaximumValue;
      else if (yAxes_[0]->axis->segments_.back().renderMinimum > 0)
        xLocation = MinimumValue;
      else if (!yAxes_[0]->axis->isOnAxis(0.0))
        xLocation = MinimumValue;
      xAxis_.location.initLoc = xLocation;
    } else if (xLocation == MinimumValue) {
      if (yAxes_[0]->axis->segments_.front().renderMinimum == 0 && yAxes_[0]->axis->tickDirection() == Outwards)
        xLocation = ZeroValue;
    } else if (xLocation == MaximumValue)
      if (yAxes_[0]->axis->segments_.back().renderMaximum == 0)
        xLocation = ZeroValue;

    xAxis_.location.finLoc = xLocation;
  }

  xAxis_.calculatedWidth = calcAxisSize(xAxis, device) + 10;

  for (std::size_t i = 0; i < yAxes_.size(); ++i)
    yAxes_[i]->location.initLoc = yAxes_[i]->axis->location();

  std::vector<const WAxis*> minimumYaxes = collectYAxesAtLocation(MinimumValue);
  int offset = 0;
  for (std::size_t i = 0; i < minimumYaxes.size(); ++i) {
    const WAxis &axis = *minimumYaxes[i];
    if (axis.location() != BothSides)
      yAxes_[axis.yAxisId()]->location.initLoc = MinimumValue;
    yAxes_[axis.yAxisId()]->location.minOffset = offset;
    yAxes_[axis.yAxisId()]->calculatedWidth =
        calcAxisSize(axis, device) + 10;
    if (i == 0 && axis.tickDirection() == Inwards)
      offset += 10;
    else
      offset += yAxes_[axis.yAxisId()]->calculatedWidth;
  }

  std::vector<const WAxis*> maximumYaxes = collectYAxesAtLocation(MaximumValue);
  offset = 0;
  for (std::size_t i = 0; i < maximumYaxes.size(); ++i) {
    const WAxis &axis = *maximumYaxes[i];
    if (axis.location() != BothSides)
      yAxes_[axis.yAxisId()]->location.initLoc = MaximumValue;
    yAxes_[axis.yAxisId()]->location.maxOffset = offset;
    yAxes_[axis.yAxisId()]->calculatedWidth =
        calcAxisSize(axis, device) + 10;
    if (i == 0 && axis.tickDirection() == Inwards)
      offset += 10;
    else
      offset += yAxes_[axis.yAxisId()]->calculatedWidth;
  }

  for (int i = 0; i < yAxisCount(); ++i)
    yAxes_[i]->location.finLoc = yAxes_[i]->location.initLoc;

  if (!minimumYaxes.empty() &&
      minimumYaxes[0]->location() == MinimumValue &&
      (axis(XAxis).inverted() ?
       xAxis_.axis->segments_.back().renderMaximum == 0 :
       xAxis_.axis->segments_.front().renderMinimum == 0) &&
      minimumYaxes[0]->tickDirection() == Outwards) {
    yAxes_[minimumYaxes[0]->yAxisId()]->location.finLoc = ZeroValue;
  }

  if (!maximumYaxes.empty() &&
      maximumYaxes[0]->location() == MaximumValue &&
      (axis(XAxis).inverted() ?
       xAxis_.axis->segments_.front().renderMinimum == 0 :
       xAxis_.axis->segments_.back().renderMaximum == 0)) {
    yAxes_[maximumYaxes[0]->yAxisId()]->location.finLoc = ZeroValue;
  }

  return true;
}

// Collects Y axes on minimum or maximum side
// The vector is ordered from innermost to outermost
// NOTE: should be called after prepareRender is called for all axes!
std::vector<const WAxis*> WCartesianChart::collectYAxesAtLocation(AxisValue side) const
{
  std::vector<const WAxis*> result;

  // Innermost axes are axes set to location ZeroValue
  // and there is no 0 on the X axis
  for (std::size_t i = 0; i < yAxes_.size(); ++i) {
    const WAxis &axis = *yAxes_[i]->axis;
    if (!axis.isVisible())
      continue;
    if (axis.location() == ZeroValue) {
      if (side == MinimumValue) {
        if (xAxis_.axis->segments_.front().renderMinimum >= 0 ||
            (!xAxis_.axis->isOnAxis(0.0) &&
             xAxis_.axis->segments_.back().renderMaximum > 0))
          result.push_back(&axis);
      } else if (side == MaximumValue) {
        if (xAxis_.axis->segments_.back().renderMaximum <= 0)
          result.push_back(&axis);
      }
    }
  }

  for (std::size_t i = 0; i < yAxes_.size(); ++i) {
    const WAxis &axis = *yAxes_[i]->axis;
    if (!axis.isVisible())
      continue;
    if (axis.location() == side || axis.location() == BothSides)
      result.push_back(&axis);
  }

  return result;
}

int WCartesianChart::calcAxisSize(const WAxis &axis, WPaintDevice *device) const
{
  if (device->features() & Wt::WPaintDevice::HasFontMetrics) {
    // TICK_LENGTH + axis labels + (optional) title
    if ((orientation() == Horizontal) != /*XOR*/ (axis.id() == XAxis)) {
      WMeasurePaintDevice md(device);
      double h = TICK_LENGTH;
      h += axis.calcMaxTickLabelSize(&md, Vertical);
      if (!axis.title().empty()) {
        h += axis.calcTitleSize(&md, Vertical);
      }
      return static_cast<int>(std::ceil(h));
    } else {
      WMeasurePaintDevice md(device);
      double w = TICK_LENGTH;
      w += axis.calcMaxTickLabelSize(&md, Horizontal);
      if (!axis.title().empty() &&
           axis.titleOrientation() == Vertical) {
        w += axis.calcTitleSize(&md, Vertical) + 10;
      }
      return static_cast<int>(std::ceil(w));
    }
  } else {
    if ((orientation() == Horizontal) != /*XOR*/ (axis.id() == XAxis)) {
      return TICK_LENGTH + 20 + (axis.title().empty() ? 0 : 20);
    } else {
      return TICK_LENGTH + 30 + (axis.title().empty() ? 0 : 15);
    }
  }
}

WPointF WCartesianChart::map(double xValue, double yValue,
			     Axis yAxis, int currentXSegment,
			     int currentYSegment) const
{
  return map(xValue, yValue, yAxis == Y1Axis ? 0 : 1, currentXSegment, currentYSegment);
}

WPointF WCartesianChart::map(double xValue, double yValue,
                             int yAxis, int currentXSegment,
                             int currentYSegment) const
{
  const WAxis& xAx = axis(XAxis);
  const WAxis& yAx = this->yAxis(yAxis);

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

WRectF WCartesianChart::chartSegmentArea(const WAxis& yAxis, int xSegment,
					 int ySegment) const
{
  const WAxis& xAxis = axis(XAxis);

  const WAxis::Segment& xs = xAxis.segments_[xSegment];
  const WAxis::Segment& ys = yAxis.segments_[ySegment];

  // margin used when clipping, see also WAxis::prepareRender(),
  // when the renderMinimum/maximum is 0, clipping is done exact

  double xRenderStart = xAxis.inverted() ? xAxis.mapToDevice(xs.renderMaximum, xSegment) : xs.renderStart;
  double xRenderEnd = xAxis.inverted() ? xAxis.mapToDevice(xs.renderMinimum, xSegment) : xs.renderStart + xs.renderLength;
  double yRenderStart = yAxis.inverted() ? yAxis.mapToDevice(ys.renderMaximum, ySegment) : ys.renderStart;
  double yRenderEnd = yAxis.inverted() ? yAxis.mapToDevice(ys.renderMinimum, ySegment) : ys.renderStart + ys.renderLength;

  double x1 = chartArea_.left() + xRenderStart
    + (xSegment == 0
       ? (xs.renderMinimum == 0 ? 0 : -axisPadding())
       : -xAxis.segmentMargin() / 2);
  double x2 = chartArea_.left() + xRenderEnd
    + (xSegment == xAxis.segmentCount() - 1
       ? (xs.renderMaximum == 0 ? 0 : axisPadding())
       : xAxis.segmentMargin() / 2);

  double y1 = chartArea_.bottom() - yRenderEnd
    - (ySegment == yAxis.segmentCount() - 1
       ? (ys.renderMaximum == 0 ? 0 : axisPadding())
       : yAxis.segmentMargin() / 2);
  double y2 = chartArea_.bottom() - yRenderStart
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

  if (onDemandLoadingEnabled()) {
    painter.save();
    WPainterPath clipPath;
    clipPath.addRect(hv(chartArea_));
    painter.setClipPath(clipPath);
    painter.setClipping(true);

    double zoomRange = axis(XAxis).zoomMaximum() - axis(XAxis).zoomMinimum();
    double zoomStart = axis(XAxis).zoomMinimum() - zoomRange;
    double zoomEnd = axis(XAxis).zoomMaximum() + zoomRange;
    double minX = std::max(chartArea_.left() + axis(XAxis).mapToDevice(zoomStart), chartArea_.left());
    double maxX = std::min(chartArea_.left() + axis(XAxis).mapToDevice(zoomEnd), chartArea_.right());
    painter.fillRect(zoomRangeTransform(0).map(hv(WRectF(chartArea_.left(), chartArea_.top(), minX - chartArea_.left(), chartArea_.height()))), loadingBackground());
    painter.fillRect(zoomRangeTransform(0).map(hv(WRectF(maxX, chartArea_.top(), chartArea_.right() - maxX, chartArea_.height()))), loadingBackground());

    painter.restore();
  }
}

void WCartesianChart::renderGrid(WPainter& painter, const WAxis& ax) const
{
  if (!ax.isGridLinesEnabled())
    return;

  bool isYAxis = ax.id() != XAxis;
  
  if (!isYAxis && yAxes_.empty())
    return;

  const WAxis& other = isYAxis ? axis(XAxis) : axis(Y1Axis);
  const WAxis::Segment& s0 = other.segments_.front();
  const WAxis::Segment& sn = other.segments_.back();

  double ou0 = s0.renderStart;
  double oun = sn.renderStart + sn.renderLength;

  // Adjust for potentially different axis padding on other Y axes
  if (!isYAxis) {
    bool gridLines = axis(YAxis).isGridLinesEnabled();
    for (int i = 1; i < yAxisCount(); ++i) {
      if (!(yAxis(i).isVisible() && yAxis(i).isGridLinesEnabled()))
        continue;
      const WAxis::Segment &s0_2 = yAxis(i).segments_.front();
      const WAxis::Segment &sn_2 = yAxis(i).segments_.front();
      if (!gridLines || s0_2.renderStart < ou0)
        ou0 = s0_2.renderStart;
      if (!gridLines || sn_2.renderStart + sn_2.renderLength > oun)
        oun = sn_2.renderStart + sn_2.renderLength;
      gridLines = true;
    }
  }
  
  if (!isYAxis) {
    ou0 = chartArea_.bottom() - ou0;
    oun = chartArea_.bottom() - oun;
  } else {
    ou0 = chartArea_.left() + ou0;
    oun = chartArea_.left() + oun;
  }

  if (isInteractive()) {
    painter.save();
    WPainterPath clipPath;
    clipPath.addRect(hv(chartArea_));
    painter.setClipPath(clipPath);
    painter.setClipping(true);
  }

  std::vector<WPen> pens;
  const std::vector<PenAssignment> &assignments =
      ax.id() == XAxis ? xAxis_.pens : yAxes_[ax.yAxisId()]->pens;
  if (assignments.empty()) {
    pens.push_back(ax.gridLinesPen());
  } else {
    for (std::size_t i = 0; i < assignments.size(); ++i) {
      pens.push_back(assignments[i].gridPen.value());
    }
  }

  AxisConfig axisConfig;
  if (ax.location() == BothSides)
    axisConfig.side = MinimumValue;
  else
    axisConfig.side = ax.location();

  for (unsigned level = 1; level <= pens.size(); ++level) {
    WPainterPath gridPath;

    axisConfig.zoomLevel = level;
    std::vector<double> gridPos = ax.gridLinePositions(axisConfig);

    for (unsigned i = 0; i < gridPos.size(); ++i) {
      double u = gridPos[i];

      if (isYAxis) {
	u = chartArea_.bottom() - u;
	gridPath.moveTo(hv(ou0, u));
	gridPath.lineTo(hv(oun, u));
      } else {
	u = chartArea_.left() + u;
	gridPath.moveTo(hv(u, ou0));
	gridPath.lineTo(hv(u, oun));
      }
    }

    painter.strokePath(zoomRangeTransform(isYAxis ? ax.yAxisId() : 0).map(gridPath).crisp(), pens[level - 1]);
  }

  if (isInteractive()) {
    painter.restore();
  }
}

void WCartesianChart::renderAxis(WPainter& painter, const WAxis& axis,
				 WFlags<AxisProperty> properties) const
{
  if (!axis.isVisible())
    return;

  bool isYAxis = axis.id() != XAxis;

  const AxisValue location = axis.id() == XAxis ? xAxis_.location.finLoc : yAxes_[axis.yAxisId()]->location.finLoc;

  if (isInteractive() && dynamic_cast<WCanvasPaintDevice*>(painter.device())) {
    WRectF clipRect;
    WRectF area = hv(chartArea_);
    if (axis.location() == ZeroValue && location == ZeroValue) {
      clipRect = area;
    } else if (isYAxis != /*XOR*/ (orientation() == Horizontal)) {
      double h = area.height();
      if (xAxis_.location.finLoc == ZeroValue &&
	  orientation() == Vertical) {
	h += 1; // prevent clipping off of zero tick
      }
      clipRect = WRectF(0.0, area.top(), isYAxis ? width_ : height_, h);
    } else {
      clipRect = WRectF(area.left(), 0.0, area.width(), isYAxis ? height_ : width_);
    }
    if (properties == Labels) {
      clipRect = WRectF(clipRect.left() - 1, clipRect.top() - 1, clipRect.width() + 2, clipRect.height() + 2);
    }
    WPainterPath clipPath;
    clipPath.addRect(clipRect);
    painter.save();
    painter.setClipPath(clipPath);
    painter.setClipping(true);
  }

  std::vector<AxisValue> locations;
  if (location == BothSides) {
    locations.push_back(MinimumValue);
    locations.push_back(MaximumValue);
  } else
    locations.push_back(location);

  for (std::size_t l = 0; l < locations.size(); ++l) {
    WPointF axisStart, axisEnd;
    double tickStart = 0.0, tickEnd = 0.0, labelPos = 0.0;
    AlignmentFlag labelHFlag = AlignCenter, labelVFlag = AlignMiddle;

    if (isYAxis) {
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
      if (isYAxis) {
        double x = chartArea_.left() - yAxes_[axis.yAxisId()]->location.minOffset;
        if (axis.tickDirection() == Inwards) {
	  tickStart = 0;
	  tickEnd = TICK_LENGTH;
	  labelPos = TICK_LENGTH;
	  labelHFlag = AlignLeft;

	  axisStart.setX(x);
	  axisEnd.setX(x);
	} else {
	  tickStart = -TICK_LENGTH;
	  tickEnd = 0;
	  labelPos = -TICK_LENGTH;
	  labelHFlag = AlignRight;

          x -= axis.margin();
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
      if (isYAxis) {
        double x = chartArea_.right() + yAxes_[axis.yAxisId()]->location.maxOffset;
        if (axis.tickDirection() == Inwards) {
	  tickStart = -TICK_LENGTH;
	  tickEnd = 0;
	  labelPos = -TICK_LENGTH;
	  labelHFlag = AlignRight;

          x -= 1;
	  axisStart.setX(x);
	  axisEnd.setX(x);
	} else {
	  tickStart = 0;
	  tickEnd = TICK_LENGTH;
	  labelPos = TICK_LENGTH;
	  labelHFlag = AlignLeft;

          x += axis.margin();
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

      if (isYAxis) {
	double x = chartArea_.left() + this->axis(XAxis).mapToDevice(0.0);
	axisStart.setX(x);
	axisEnd.setX(x);

	labelHFlag = AlignRight;

	/* force labels left even if axis is in middle */
	if (type() == CategoryChart)
	  labelPos = chartArea_.left() - axisStart.x() - TICK_LENGTH;
	else
	  labelPos = -TICK_LENGTH;
      } else {
	double y = chartArea_.bottom() - this->axis(YAxis).mapToDevice(0.0);
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

      if (isYAxis) {
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
              if (axis.tickDirection() == Outwards)
                size = axis.calcMaxTickLabelSize(device, Horizontal);
	      titleSizeW = axis.calcTitleSize(device, Vertical);
              if (axis.tickDirection() == Inwards)
                titleSizeW = -titleSizeW;
	    } else {
	      size = 35;
              if (axis.tickDirection() == Inwards)
                size = -20;
	    }

	    renderLabel(painter, axis.title(),
		WPointF(u + (labelHFlag == AlignRight ? -( size + titleSizeW + 5)  : +( size + titleSizeW + 5)),
		  chartArea_.center().y()), AlignCenter | AlignMiddle, locations[l] == MaximumValue ? -90 : 90, 10);
	  }
	} else {
	  double extraMargin = 0;
	  WPaintDevice *device = painter.device();
          if (axis.tickDirection() == Outwards) {
              if (device->features() & WPaintDevice::HasFontMetrics)
                extraMargin = axis.calcMaxTickLabelSize(device, Vertical);
              else
                extraMargin = 15;
          }
          if (locations[l] != MaximumValue)
            extraMargin = -extraMargin;
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
            if (axis.tickDirection() == Outwards)
              extraMargin = axis.calcMaxTickLabelSize(device, Vertical);
	  } else {
            if (axis.tickDirection() == Outwards)
              extraMargin = 15;
	  }
          if (locations[l] == MaximumValue)
            extraMargin = -extraMargin;
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
              if (axis.tickDirection() == Outwards)
                extraMargin = axis.calcMaxTickLabelSize(device, Horizontal);
	      extraMargin += axis.calcTitleSize(device, Vertical);
	    } else {
	      extraMargin = 40;
	    }
            if (locations[l] == MaximumValue)
              extraMargin = -extraMargin;

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
    if (isYAxis) {
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

      bool invertTicks = !isYAxis;
      if (invertTicks) {
	tickStart = -tickStart;
	tickEnd = -tickEnd;
	labelPos = -labelPos;
      }
    }

    std::vector<WPen> pens;
    std::vector<WPen> textPens;
    if (isInteractive()) {
      const std::vector<PenAssignment> &assignment =
          axis.id() == XAxis ? xAxis_.pens : yAxes_[axis.yAxisId()]->pens;
      if (!assignment.empty()) {
        for (std::size_t i = 0; i < assignment.size(); ++i) {
          pens.push_back(assignment[i].pen.value());
          textPens.push_back(assignment[i].textPen.value());
        }
      }
    }

    WTransform transform;
    WRectF area = hv(chartArea_);
    if (axis.location() == ZeroValue) {
      transform =
	WTransform(1,0,0,-1,area.left(),area.bottom()) *
          xAxis_.transform * yAxes_[axis.yAxisId()]->transform *
	WTransform(1,0,0,-1,-area.left(),area.bottom());
    } else if (isYAxis && orientation() == Vertical) {
      transform = WTransform(1,0,0,-1,0,area.bottom()) * yAxes_[axis.yAxisId()]->transform * WTransform(1,0,0,-1,0,area.bottom());
    } else if (isYAxis && orientation() == Horizontal) {
      transform = WTransform(0,1,1,0,area.left(),0) * yAxes_[axis.yAxisId()]->transform * WTransform(0,1,1,0,0,-area.left());
    } else if (orientation() == Horizontal) {
      transform = WTransform(0,1,1,0,0,area.top()) * xAxis_.transform * WTransform(0,1,1,0,-area.top(),0);
    } else {
      transform = WTransform(1,0,0,1,area.left(),0) * xAxis_.transform * WTransform(1,0,0,1,-area.left(),0);
    }

    AxisValue side = location == BothSides ? locations[l] : axis.location();

    axis.render(painter, properties, axisStart, axisEnd, tickStart, tickEnd,
		labelPos, labelHFlag | labelVFlag, transform, side, pens, textPens);
  }

  if (isInteractive()) {
    painter.restore();
  }
}

void WCartesianChart::renderAxes(WPainter& painter,
				 WFlags<AxisProperty> properties) const
{
  renderAxis(painter, *xAxis_.axis, properties);
  for (std::size_t i = 0; i < yAxes_.size(); ++i)
    renderAxis(painter, *yAxes_[i]->axis, properties);
}

// Determines if the first Y axis on the maximum side
// is visible and has its tick direction inwards
//
// This is used to determine whether the border should
// be shifted by one pixel in renderBorder()
bool WCartesianChart::hasInwardsYAxisOnMaximumSide() const
{
  std::vector<const WAxis*> maximumYaxes = collectYAxesAtLocation(MaximumValue);
  return !maximumYaxes.empty() && maximumYaxes[0]->tickDirection() == Inwards;
}

void WCartesianChart::renderBorder(WPainter& painter) const
{
  WPainterPath area;
  int horizontalShift = 0,
      verticalShift = 0;

  if (hasInwardsYAxisOnMaximumSide())
    horizontalShift = -1;
  if (axis(XAxis).isVisible() && axis(XAxis).tickDirection() == Inwards)
    verticalShift = -1;
  area.addRect(hv(WRectF(chartArea_.left(), chartArea_.top(), chartArea_.width() + horizontalShift, chartArea_.height() + verticalShift)));
  painter.strokePath(area.crisp(), borderPen_);
}

void WCartesianChart::renderCurveLabels(WPainter &painter) const
{
  if (isInteractive()) {
    painter.save();
    WPainterPath clipPath;
    clipPath.addRect(hv(chartArea_));
    painter.setClipPath(clipPath);
    painter.setClipping(true);
  }
  for (std::size_t i = 0; i < curveLabels_.size(); ++i) {
    const CurveLabel &label = curveLabels_[i];
    for (std::size_t j = 0; j < series_.size(); ++j) {
      const WDataSeries &series = *series_[j];
      // Don't draw curve labels for hidden series
      if (series.isHidden())
	continue;
      if (&series == &label.series()) {
        WTransform t = zoomRangeTransform(series.yAxis());
	if (series.type() == LineSeries || series.type() == CurveSeries) {
	  t = t * curveTransform(series);
	}
	// Find the right x and y segment
	int xSegment = 0;
        double x = axis(XAxis).getValue(label.x());
        if (!isInteractive())
          while (xSegment < axis(XAxis).segmentCount() && (axis(XAxis).segments_[xSegment].renderMinimum > x
                                    || axis(XAxis).segments_[xSegment].renderMaximum < x))
            ++xSegment;
	int ySegment = 0;
        double y = yAxis(series.yAxis()).getValue(label.y());
        if (!isInteractive())
          while (ySegment < yAxis(series.yAxis()).segmentCount() && (yAxis(series.yAxis()).segments_[ySegment].renderMinimum > y
                                    || yAxis(series.yAxis()).segments_[ySegment].renderMaximum < y))
            ++ySegment;
	// Only draw the label if it is actually on a segment
        if (xSegment < axis(XAxis).segmentCount() && ySegment < yAxis(series.yAxis()).segmentCount()) {
	  // Figure out the device coordinates of the point to draw a label at.
          WPointF devicePoint = mapToDeviceWithoutTransform(label.x(), label.y(), series.yAxis(), xSegment, ySegment);
	  WTransform translation = WTransform().translate(t.map(devicePoint));
	  painter.save();
	  painter.setWorldTransform(translation);

	  label.render(painter);

	  painter.restore();
	}
      }
    }
  }
  if (isInteractive()) {
    painter.restore();
  }
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
  barTooltips_.clear();
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
    if (series()[i]->type() == BarSeries) {
      if (newGroup || !series()[i]->isStacked())
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
  const int legendPadding = 10;
  int legendWidth = 0;
  int legendHeight = 0;
  if (isLegendEnabled()) {
    painter.save();

    int numSeriesWithLegend = 0;

    for (unsigned i = 0; i < series().size(); ++i)
      if (series()[i]->isLegendEnabled())
	++numSeriesWithLegend;

    painter.setFont(legendFont());
    WFont f = painter.font();

    if (isAutoLayoutEnabled() &&
	(painter.device()->features() & WPaintDevice::HasFontMetrics)) {
      int columnWidth = 0;
      for (unsigned i = 0; i < series().size(); ++i)
	if (series()[i]->isLegendEnabled()) {
	  WString s = series()[i]->model()->headerData(series()[i]->modelColumn());
	  WTextItem t = painter.device()->measureText(s);
	  columnWidth = std::max(columnWidth, (int)t.width());
	}

      columnWidth += 25;
      WCartesianChart *self = const_cast<WCartesianChart *>(this);
      self->legend_.setLegendColumnWidth(columnWidth);

      if (legendSide() == Top || legendSide() == Bottom) {
	self->legend_.setLegendColumns(std::max(1, w / columnWidth - 1));
      }
    }

    int numLegendRows = (numSeriesWithLegend - 1) / legendColumns() + 1;
    double lineHeight = f.sizeLength().toPixels() * 1.5;

    legendWidth = (int)legendColumnWidth().toPixels()
      * std::min(legendColumns(), numSeriesWithLegend);
    legendHeight = (int) (numLegendRows * lineHeight);

    int x = 0;
    int y = 0;

    switch (legendSide()) {
    case Left:
      if (legendLocation() == LegendInside)
        x = plotAreaPadding(Left) + legendPadding;
      else
        x = plotAreaPadding(Left) - legendPadding - legendWidth;
      break;
    case Right:
      x = w - plotAreaPadding(Right);
      if (legendLocation() == LegendInside)
        x -= legendPadding + legendWidth;
      else
        x += legendPadding;
      break;
    case Top:
      if (legendLocation() == LegendInside)
        y = plotAreaPadding(Top) + legendPadding;
      else
        y = plotAreaPadding(Top) - legendPadding - legendHeight;
      break;
    case Bottom:
      y = h - plotAreaPadding(Bottom);
      if (legendLocation() == LegendInside)
        y -= legendPadding + legendHeight;
      else
        y += legendPadding;
    default:
      break;
    }

    switch (legendAlignment()) {
    case AlignTop:
      y = plotAreaPadding(Top) + legendPadding;
      break;
    case AlignMiddle:
      {
	double middle = plotAreaPadding(Top)
	  + (h - plotAreaPadding(Top) - plotAreaPadding(Bottom)) / 2; 

	y = (int) (middle - legendHeight/2);
      }
      break;
    case AlignBottom:
      y = h - plotAreaPadding(Bottom) - legendPadding - legendHeight;
      break;
    case AlignLeft:
      x = plotAreaPadding(Left) + legendPadding;
      break;
    case AlignCenter:
      {
	double center = plotAreaPadding(Left)
	  + (w - plotAreaPadding(Left) - plotAreaPadding(Right)) / 2; 

	x = (int) (center - legendWidth/2);
      } 
      break;
    case AlignRight:
      x = w - plotAreaPadding(Right) - legendPadding - legendWidth;
      break;
    default:
      break;
    }

    int xOffset = 0;
    int yOffset = 0;
    if (legendLocation() == LegendOutside) {
      switch (legendSide()) {
      case Top: {
          if (orientation() == Horizontal) {
            for (int i = yAxisCount() - 1; i >= 0; --i) {
              if (yAxis(i).isVisible() &&
                  (yAxes_[i]->location.initLoc == MinimumValue ||
                   yAxes_[i]->location.initLoc == BothSides)) {
                yOffset = - (yAxes_[i]->location.minOffset +
                             yAxes_[i]->calculatedWidth);
                break;
              }
            }
          } else {
            if (axis(XAxis).isVisible() &&
                (xAxis_.location.initLoc == MaximumValue ||
                 xAxis_.location.initLoc == BothSides))
              yOffset = - xAxis_.calculatedWidth;
          }
          yOffset -= 5;
        }
        break;
      case Bottom: {
          if (orientation() == Horizontal) {
            for (int i = yAxisCount() - 1; i >= 0; --i) {
              if (yAxis(i).isVisible() &&
                  (yAxes_[i]->location.initLoc == MaximumValue ||
                   yAxes_[i]->location.initLoc == BothSides)) {
                yOffset = yAxes_[i]->location.maxOffset +
                          yAxes_[i]->calculatedWidth;
                break;
              }
            }
          } else {
            if (axis(XAxis).isVisible() &&
                (xAxis_.location.initLoc == MinimumValue ||
                 xAxis_.location.initLoc == BothSides))
              yOffset = xAxis_.calculatedWidth;
          }
          yOffset += 5;
        }
        break;
      case Left: {
          if (orientation() == Horizontal) {
            if (axis(XAxis).isVisible() &&
                (xAxis_.location.initLoc == MinimumValue ||
                 xAxis_.location.initLoc == BothSides))
              xOffset = - xAxis_.calculatedWidth;
          } else {
            for (int i = yAxisCount() - 1; i >= 0; --i) {
              if (yAxis(i).isVisible() &&
                  (yAxes_[i]->location.initLoc == MinimumValue ||
                   yAxes_[i]->location.initLoc == BothSides)) {
                xOffset = - (yAxes_[i]->location.minOffset +
                             yAxes_[i]->calculatedWidth);
                break;
              }
            }
          }
          xOffset -= 5;
        }
        break;
      case Right: {
          if (orientation() == Horizontal) {
            if (axis(XAxis).isVisible() &&
                (xAxis_.location.initLoc == MaximumValue ||
                 xAxis_.location.initLoc == BothSides))
              xOffset = xAxis_.calculatedWidth;
          } else {
            for (int i = yAxisCount() - 1; i >= 0; --i) {
              if (yAxis(i).isVisible() &&
                  (yAxes_[i]->location.initLoc == MaximumValue ||
                   yAxes_[i]->location.initLoc == BothSides)) {
                xOffset = yAxes_[i]->location.maxOffset +
                          yAxes_[i]->calculatedWidth;
                break;
              }
            }
          }
          xOffset += 5;
        }
        break;
      }
    } else {
      switch (legendSide()) {
      case Top:
        yOffset = 5;
        break;
      case Bottom:
        yOffset = -5;
        break;
      case Left:
        xOffset = 5;
        break;
      case Right:
        xOffset = -5;
        break;
      }
    }

#ifdef WT_TARGET_JAVA
    painter.setPen(WPen(legendBorder()));
#else
    painter.setPen(legendBorder());
#endif
    painter.setBrush(legendBackground());

    painter.drawRect(x + xOffset - legendPadding/2, y + yOffset - legendPadding/2, legendWidth + legendPadding, legendHeight + legendPadding);

    painter.setPen(WPen());

    painter.setFont(legendFont());

    int item = 0;
    for (unsigned i = 0; i < series().size(); ++i)
      if (series()[i]->isLegendEnabled()) {
	int col = item % legendColumns();
	int row = item / legendColumns();
        double itemX = x + xOffset + col * legendColumnWidth().toPixels();
        double itemY = y + yOffset + row * lineHeight;

	renderLegendItem(painter, WPointF(itemX, itemY + lineHeight/2),
			 *series()[i]);

	++item;
      }

    painter.restore();
  }

  if (!title().empty()) {
    int x = plotAreaPadding(Left) 
      + (w - plotAreaPadding(Left) - plotAreaPadding(Right)) / 2 ;
    painter.save();
    painter.setFont(titleFont());
    double titleHeight = titleFont().sizeLength().toPixels();
    const int TITLE_PADDING = 10;
    const int TITLE_WIDTH = 1000;
    // Extra space required for axes above the chart + legend
    int titleOffset = 0;
    if (orientation() == Horizontal) {
      for (int i = yAxisCount() - 1; i >= 0; --i) {
        const WAxis &yAx = yAxis(i);
        if (yAxes_[i]->location.initLoc == MinimumValue ||
            yAxes_[i]->location.initLoc == BothSides) {
          titleOffset = yAxes_[i]->location.minOffset +
                        yAxes_[i]->calculatedWidth;
          break;
        }
      }
    } else if (xAxis_.location.initLoc == MaximumValue ||
               xAxis_.location.initLoc == BothSides) {
      titleOffset = xAxis_.calculatedWidth;
    }
    if (legendSide() == Top &&
        legendLocation() == LegendOutside) {
      titleOffset += legendHeight + legendPadding + 5;
    }
    painter.drawText(x - TITLE_WIDTH / 2,
                     plotAreaPadding(Top) - titleHeight - TITLE_PADDING - titleOffset,
                     TITLE_WIDTH, titleHeight, AlignCenter | AlignTop, title());
    painter.restore();
  }
}

void WCartesianChart::renderOther(WPainter &painter) const
{
  WPainterPath clipPath;
  clipPath.addRect(hv(chartArea_));
  painter.setClipPath(clipPath);
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

WPointF WCartesianChart::inverseHv(const WPointF &p) const
{
  if (p.isJavaScriptBound()) {
    if (orientation() == Vertical) {
      return p;
    } else {
      return p.inverseSwapHV(height_);
    }
  }
  return inverseHv(p.x(), p.y(), height_);
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

void WCartesianChart::setCrosshairColor(const WColor &color)
{
  if (crosshairColor_ != color) {
    crosshairColor_ = color;
    updateJSConfig("crosshairColor", jsStringLiteral(color.cssText(true)));
  }
}

void WCartesianChart::setCrosshairYAxis(int yAxis)
{
  if (crosshairYAxis_ != yAxis) {
    crosshairYAxis_ = yAxis;
    updateJSConfig("crosshairAxis", yAxis);
  }
}

void WCartesianChart::setFollowCurve(int followCurve)
{
  if (followCurve == -1) {
    setFollowCurve((const WDataSeries *) 0);
  } else {
    for (std::size_t i = 0; i < series_.size(); ++i) {
      if (series_[i]->modelColumn() == followCurve)
	setFollowCurve(series_[i]);
    }
  }
}

void WCartesianChart::setFollowCurve(const WDataSeries *series)
{
  if (followCurve_ != series) {
    followCurve_ = series;
    updateJSConfig("followCurve", series ? seriesIndexOf(*series) : -1);
  }
}

void WCartesianChart::disableFollowCurve()
{
  setFollowCurve((const WDataSeries *) 0);
}

const WDataSeries *WCartesianChart::followCurve() const
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
  clearPensForAxis(XAxis, -1);
  for (int i = 0; i < yAxisCount(); ++i) {
    clearPensForAxis(YAxis, i);
  }
}

void WCartesianChart::clearPensForAxis(Axis ax, int yAxis)
{
  std::vector<PenAssignment> &assignments =
      ax == XAxis ? xAxis_.pens : yAxes_[yAxis]->pens;
  for (std::size_t i = 0; i < assignments.size(); ++i) {
    PenAssignment &assignment = assignments[i];
    freePens_.push_back(assignment.pen);
    freePens_.push_back(assignment.textPen);
    freePens_.push_back(assignment.gridPen);
  }
  assignments.clear();
}

void WCartesianChart::createPensForAxis(Axis ax, int yAxis)
{
  WAxis &axis = ax == XAxis ? this->axis(XAxis) : this->yAxis(yAxis);
  if (!axis.isVisible() || axis.scale() == LogScale)
    return;

  AxisStruct &axisStruct = ax == XAxis ? xAxis_ : *yAxes_[yAxis];

  double zoom = axis.zoom();
  if (zoom > axis.maxZoom()) {
    zoom = axis.maxZoom();
  }
  int level = toZoomLevel(zoom);

  std::vector<PenAssignment> assignments;
  bool stop = false;
  for (int i = 1; !stop; ++i) {
    if (onDemandLoadingEnabled() &&
        i > level + 1)
      break;
    double z = std::pow(2.0, i-1);
    stop = z >= axis.maxZoom();
    WJavaScriptHandle<WPen> pen;
    if (freePens_.size() > 0) {
      pen = freePens_.back();
      freePens_.pop_back();
    } else {
      pen = createJSPen();
    }
    WPen p = WPen(axis.pen());
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
    p = WPen(axis.textPen());
    p.setColor(WColor(p.color().red(), p.color().green(), p.color().blue(),
	  (i == level ? p.color().alpha() : 0)));
    textPen.setValue(p);
    WJavaScriptHandle<WPen> gridPen;
    if (freePens_.size() > 0) {
      gridPen = freePens_.back();
      freePens_.pop_back();
    } else {
      gridPen = createJSPen();
    }
    p = WPen(axis.gridLinesPen());
    p.setColor(WColor(p.color().red(), p.color().green(), p.color().blue(),
	  (i == level ? p.color().alpha() : 0)));
    gridPen.setValue(p);
    assignments.push_back(PenAssignment(pen, textPen, gridPen));
  }
  axisStruct.pens = assignments;
}

WTransform WCartesianChart::zoomRangeTransform(int yAxis) const
{
  return zoomRangeTransform(xAxis_.transform, yAxes_[yAxis]->transform);
}

WTransform WCartesianChart::zoomRangeTransform(const WTransform &xTransform, const WTransform &yTransform) const
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

WTransform WCartesianChart::calculateCurveTransform(const WDataSeries &series) const
{
  int yAxis = series.yAxis();
  double origin;
  if (orientation() == Horizontal) {
    origin = mapToDeviceWithoutTransform(0.0, 0.0, yAxis).x();
  } else {
    origin = mapToDeviceWithoutTransform(0.0, 0.0, yAxis).y();
  }
  double offset = this->yAxis(yAxis).mapToDevice(0.0, 0) - this->yAxis(yAxis).mapToDevice(series.offset(), 0);
  if (orientation() == Horizontal) offset = -offset;
  return WTransform(1, 0, 0, series.scale(), 0, origin * (1 - series.scale()) + offset);
}

WTransform WCartesianChart::curveTransform(const WDataSeries &series) const
{
  TransformMap::const_iterator it = curveTransforms_.find(&series);
  WTransform t;
  if (it == curveTransforms_.end()) {
    t = calculateCurveTransform(series);
  } else {
    t = it->second.value();
  }
  if (orientation() == Vertical) {
    return t;
  } else {
    return WTransform(0,1,1,0,0,0) * t * WTransform(0,1,1,0,0,0);
  }
}

std::string WCartesianChart::cObjJsRef() const
{
  return jsRef() + ".wtCObj";
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

void WCartesianChart::xTransformChanged()
{
  if (onDemandLoadingEnabled()) {
    update();
  }

  // setFormData() already assigns the right values
  axis(XAxis).zoomRangeChanged().emit(axis(XAxis).zoomMinimum(),
				     axis(XAxis).zoomMaximum());
}

void WCartesianChart::yTransformChanged(int yAxis)
{
  if (onDemandLoadingEnabled()) {
    update();
  }

  // setFormData() already assigns the right values
  this->yAxis(yAxis).zoomRangeChanged().emit(this->yAxis(yAxis).zoomMinimum(),
                                             this->yAxis(yAxis).zoomMaximum());
}

void WCartesianChart::jsSeriesSelected(double x, double y)
{
  if (!seriesSelectionEnabled())
    return;
  double smallestSqDistance = std::numeric_limits<double>::infinity();
  const WDataSeries *closestSeries = 0;
  WPointF closestPointPx;
  WPointF closestPointBeforeSeriesTransform;
  for (std::size_t i = 0; i < series_.size(); ++i) {
    const WDataSeries &series = *series_[i];
    if (!series.isHidden() && (series.type() == LineSeries || series.type() == CurveSeries)) {
      WTransform transform = zoomRangeTransform(xAxis_.transformHandle.value(),
                                                yAxes_[series.yAxis()]->transformHandle.value());
      WPointF p = transform.inverted().map(WPointF(x,y));
      WPainterPath path = pathForSeries(series);
      WTransform t = curveTransform(series);
      for (std::size_t j = 0; j < path.segments().size(); ++j) {
	const WPainterPath::Segment &seg = path.segments()[j];
	if (seg.type() != WPainterPath::Segment::CubicC1 &&
	    seg.type() != WPainterPath::Segment::CubicC2 &&
	    seg.type() != WPainterPath::Segment::QuadC) {
	  WPointF segP = t.map(WPointF(seg.x(), seg.y()));
	  double dx = p.x() - segP.x();
	  double dy = p.y() - segP.y();
          double d2 = dx * dx + dy * dy;
          if (d2 < smallestSqDistance) {
            smallestSqDistance = d2;
	    closestSeries = &series;
            closestPointPx = segP;
            closestPointBeforeSeriesTransform = WPointF(seg.x(), seg.y());
	  }
	}
      }
    }
  }
  {
    WTransform transform = zoomRangeTransform(xAxis_.transformHandle.value(),
                                              yAxes_[closestSeries ? closestSeries->yAxis() : 0]->transformHandle.value());
    WPointF closestDisplayPoint = transform.map(closestPointPx);
    double dx = closestDisplayPoint.x() - x;
    double dy = closestDisplayPoint.y() - y;
    double d2 = dx * dx + dy * dy;
    if (d2 > CURVE_SELECTION_DISTANCE_SQUARED) {
      return;
    }
  }
  setSelectedSeries(closestSeries);
  if (closestSeries) {
    seriesSelected_.emit(closestSeries,
                   mapFromDeviceWithoutTransform(closestPointBeforeSeriesTransform, closestSeries->axis()));
  } else {
    seriesSelected_.emit(0, mapFromDeviceWithoutTransform(closestPointBeforeSeriesTransform, YAxis));
  }
}

void WCartesianChart::loadTooltip(double x, double y)
{
  double px = zoomRangeTransform(xAxis_.transformHandle.value(), WTransform()).inverted().map(WPointF(x,0.0)).x();
  double rx = MarkerMatchIterator::MATCH_RADIUS / xAxis_.transformHandle.value().m11();
  std::vector<double> pys;
  std::vector<double> rys;
  for (int i = 0; i < yAxisCount(); ++i) {
    WPointF p = zoomRangeTransform(WTransform(), yAxes_[i]->transformHandle.value()).inverted().map(WPointF(0.0,y));
    pys.push_back(p.y());
    rys.push_back(MarkerMatchIterator::MATCH_RADIUS / yAxes_[i]->transformHandle.value().m22());
  }
  MarkerMatchIterator iterator(*this, px, pys, rx, rys);
  iterateSeries(&iterator, 0);

  if (iterator.matchedSeries()) {
    const WDataSeries &series = *iterator.matchedSeries();
    WString tooltip = series.model()->toolTip(iterator.yRow(), iterator.yColumn());
    bool isDeferred = series.model()->flags(iterator.yRow(), iterator.yColumn()) & ItemHasDeferredTooltip;
    bool isXHTML = series.model()->flags(iterator.yRow(), iterator.yColumn()) & ItemIsXHTMLText;
    if (!tooltip.empty() && (isDeferred | isXHTML)) {
      if (isXHTML) {
	bool res = removeScript(tooltip);
	if (!res) {
	  tooltip = escapeText(tooltip);
	}
      } else {
	tooltip = escapeText(tooltip);
      }
      doJavaScript(cObjJsRef() + ".updateTooltip(" + tooltip.jsStringLiteral() + ");");
    }
  } else {
    for (std::size_t btt = 0; btt < barTooltips_.size(); ++btt) {
      const WT_ARRAY double *xs = barTooltips_[btt].xs;
      const WT_ARRAY double *ys = barTooltips_[btt].ys;
      int j = 0;
      int k = 3;
      bool c = false;
      WPointF p = zoomRangeTransform(xAxis_.transformHandle.value(), yAxes_[barTooltips_[btt].series->yAxis()]->transformHandle.value()).inverted().map(WPointF(x,y));
      for (; j < 4; k = j++) {
	  if ((((ys[j]<=p.y()) && (p.y()<ys[k])) ||
	       ((ys[k]<=p.y()) && (p.y()<ys[j]))) &&
	      (p.x() < (xs[k] - xs[j]) * (p.y() - ys[j]) / (ys[k] - ys[j]) + xs[j]))
	    c = !c;
      }
      if (c) {
	WString tooltip = barTooltips_[btt].series->model()->toolTip(barTooltips_[btt].yRow, barTooltips_[btt].yColumn);
	if (!tooltip.empty()) {
	  doJavaScript(cObjJsRef() + ".updateTooltip(" + escapeText(tooltip, false).jsStringLiteral() + ");");
	}
	return;
      }
    }
  }
}

  }
}
