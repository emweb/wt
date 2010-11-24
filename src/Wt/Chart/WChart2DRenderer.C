/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cmath>

#include "Wt/Chart/WChart2DRenderer"
#include "Wt/Chart/WCartesianChart"

#include "Wt/WAbstractItemModel"
#include "Wt/WCircleArea"
#include "Wt/WDate"
#include "Wt/WPainter"
#include "Wt/WPolygonArea"
#include "Wt/WRectArea"

#include "Utils.h"

#include <limits>
#include <float.h>

namespace {
  const int TICK_LENGTH = 5;
}

namespace Wt {
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

class SeriesRenderer;

class SeriesRenderIterator : public SeriesIterator
{
public:
  SeriesRenderIterator(WChart2DRenderer& renderer);

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
  WChart2DRenderer&  renderer_;
  const WDataSeries *series_;
  SeriesRenderer    *seriesRenderer_;
  double             minY_, maxY_;
};

class SeriesRenderer {
public:
  virtual ~SeriesRenderer() { }
  virtual void addValue(double x, double y, double stacky,
			const WModelIndex& xIndex, const WModelIndex& yIndex) = 0;
  virtual void paint() = 0;

protected:
  WChart2DRenderer&  renderer_;
  const WDataSeries& series_;

  SeriesRenderer(WChart2DRenderer& renderer, const WDataSeries& series,
		 SeriesRenderIterator& it)
    : renderer_(renderer),
      series_(series),
      it_(it)
  { }

  static double crisp(double u) {
    return std::floor(u) + 0.5;
  }

  WPointF hv(const WPointF& p) {
    return renderer_.hv(p);
  }

  WPointF hv(double x, double y) {
    return renderer_.hv(x, y);
  }

protected:
  SeriesRenderIterator& it_;
};

class LineSeriesRenderer : public SeriesRenderer {
public:
  LineSeriesRenderer(WChart2DRenderer& renderer,
		     const WDataSeries& series,
		     SeriesRenderIterator& it)
    : SeriesRenderer(renderer, series, it),
      curveLength_(0)
  { }

  void addValue(double x, double y, double stacky,
		const WModelIndex& xIndex, const WModelIndex& yIndex) {
    WPointF p = renderer_.map(x, y, series_.axis(),
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

  void paint() {
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
	renderer_.painter().setShadow(series_.shadow());
	renderer_.painter().fillPath(fill_, series_.brush());
      }

      if (series_.fillRange() == NoFill)
	renderer_.painter().setShadow(series_.shadow());
      else
	renderer_.painter().setShadow(WShadow());

      renderer_.painter().strokePath(curve_, series_.pen());
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

  static void computeC(const WPointF& p_1, const WPointF& p0, const WPointF& p1,
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
      return WPointF(renderer_.map(x, 0, series_.axis(),
				   it_.currentXSegment(),
				   it_.currentYSegment()).x(),
		     renderer_.chartArea().bottom());
    case MaximumValueFill:
      return WPointF(renderer_.map(x, 0, series_.axis(),
				   it_.currentXSegment(),
				   it_.currentYSegment()).x(),
		     renderer_.chartArea().top());
    case ZeroValueFill:
      return WPointF(renderer_.map(x, 0, series_.axis(),
				   it_.currentXSegment(),
				   it_.currentYSegment()));
    default:
      return WPointF();
    }
  }
};

class BarSeriesRenderer : public SeriesRenderer {
public:
  BarSeriesRenderer(WChart2DRenderer& renderer, const WDataSeries& series,
		    SeriesRenderIterator& it,
		    double groupWidth, int numGroups, int group)
    : SeriesRenderer(renderer, series, it),
      groupWidth_(groupWidth),
      numGroups_(numGroups),
      group_(group)
  { }

  void addValue(double x, double y, double stacky,
		const WModelIndex& xIndex, const WModelIndex& yIndex) {
    WPainterPath bar;
    const WAxis& yAxis = renderer_.chart()->axis(series_.axis());

    WPointF topMid = renderer_.map(x, y, yAxis.id(),
				   it_.currentXSegment(),
				   it_.currentYSegment());
    WPointF bottomMid = renderer_.map(x, stacky, yAxis.id(),
				      it_.currentXSegment(),
				      it_.currentYSegment());

    double g = numGroups_ + (numGroups_ - 1) * renderer_.chart()->barMargin();

    double width = groupWidth_ / g;
    double left = topMid.x() - groupWidth_ / 2
      + group_ * width * (1 + renderer_.chart()->barMargin());

    bar.moveTo(hv(crisp(left), crisp(topMid.y())));
    bar.lineTo(hv(crisp(left + width), crisp(topMid.y())));
    bar.lineTo(hv(crisp(left + width), crisp(bottomMid.y())));
    bar.lineTo(hv(crisp(left), crisp(bottomMid.y())));
    bar.closeSubPath();

    renderer_.painter().setShadow(series_.shadow());
    renderer_.painter().fillPath(bar, series_.brush());
    renderer_.painter().setShadow(WShadow());
    renderer_.painter().strokePath(bar, series_.pen());

    boost::any toolTip = yIndex.data(ToolTipRole);
    if (!toolTip.empty()) {
      WTransform t = renderer_.painter().worldTransform();

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
      renderer_.chart()->addDataPointArea(series_, xIndex, area);
    }

    double bTopMidY = it_.breakY(topMid.y());
    double bBottomMidY = it_.breakY(bottomMid.y());

    if (bTopMidY > topMid.y() && bBottomMidY <= bottomMid.y()) {
      WPainterPath breakPath;
      breakPath.moveTo(hv(left - 10, bTopMidY + 10));
      breakPath.lineTo(hv(left + width + 10, bTopMidY + 1));
      breakPath.lineTo(hv(left + width + 10, bTopMidY - 1));
      breakPath.lineTo(hv(left - 10, bTopMidY - 1));
      renderer_.painter().setPen(NoPen);
      renderer_.painter().setBrush(renderer_.chart()->background());
      renderer_.painter().drawPath(breakPath);
      renderer_.painter().setPen(WPen());
      renderer_.painter().drawLine(hv(left - 10, bTopMidY + 10),
				   hv(left + width + 10, bTopMidY + 1));
    }

    if (bBottomMidY < bottomMid.y() && bTopMidY >= topMid.y()) {
      WPainterPath breakPath;
      breakPath.moveTo(hv(left + width + 10, bBottomMidY - 10));
      breakPath.lineTo(hv(left - 10, bBottomMidY - 1));
      breakPath.lineTo(hv(left - 10, bBottomMidY + 1));
      breakPath.lineTo(hv(left + width + 10, bBottomMidY + 1));
      renderer_.painter().setBrush(renderer_.chart()->background());
      renderer_.painter().setPen(NoPen);
      renderer_.painter().drawPath(breakPath);
      renderer_.painter().setPen(WPen());
      renderer_.painter().drawLine(hv(left - 10, bBottomMidY - 1),
				   hv(left + width + 10, bBottomMidY - 10));
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

SeriesRenderIterator::SeriesRenderIterator(WChart2DRenderer& renderer)
  : renderer_(renderer),
    series_(0)
{ }

void SeriesRenderIterator::startSegment(int currentXSegment,
					int currentYSegment,
					const WRectF& currentSegmentArea)
{
  SeriesIterator::startSegment(currentXSegment, currentYSegment,
			       currentSegmentArea);

  const WAxis& yAxis = renderer_.chart()->axis(series_->axis());

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
    seriesRenderer_ = new LineSeriesRenderer(renderer_, series, *this);
    break;
  case BarSeries:
    seriesRenderer_ = new BarSeriesRenderer(renderer_, series, *this,
					    groupWidth,
					    numBarGroups, currentBarGroup);
  default:
    break;
  }

  series_ = &series;

  renderer_.painter().save();

  return seriesRenderer_ != 0;
}

void SeriesRenderIterator::endSeries()
{
  seriesRenderer_->paint();
  renderer_.painter().restore();

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
  LabelRenderIterator(WChart2DRenderer& renderer)
    : renderer_(renderer)
  { }

  virtual bool startSeries(const WDataSeries& series, double groupWidth,
			   int numBarGroups, int currentBarGroup)
  {
    if (series.isLabelsEnabled(XAxis)
	|| series.isLabelsEnabled(YAxis)) {
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
      text = renderer_.chart()->axis(XAxis).label(x);
    }

    if (series.isLabelsEnabled(YAxis)) {
      if (!text.empty())
	text += ": ";
      text += renderer_.chart()->axis(series.axis()).label(y - stackY);
    }

    if (!text.empty()) {
      WPointF p = renderer_.map(x, y, series.axis(),
				currentXSegment(), currentYSegment());
      if (series.type() == BarSeries) {
	double g = numGroups_ + (numGroups_ - 1)
	  * renderer_.chart()->barMargin();

	double width = groupWidth_ / g;
	double left = p.x() - groupWidth_ / 2 
	  + group_ * width * (1 + renderer_.chart()->barMargin());

	p = WPointF(left + width/2, p.y());
      }

      WColor c(black);

      WFlags<AlignmentFlag> alignment;
      if (series.type() == BarSeries) {
	if (y < 0)
	  alignment = AlignCenter | AlignBottom;
	else
	  alignment = AlignCenter | AlignTop;

	c = series.labelColor();
      } else {
	alignment = AlignCenter | AlignBottom;
	p.setY(p.y() - 3);
      }

      renderer_.renderLabel(text, p, c, alignment, 0, 3);
    }
  }

private:
  WChart2DRenderer& renderer_;

  double groupWidth_;
  int numGroups_;
  int group_;
};

class MarkerRenderIterator : public SeriesIterator
{
public:
  MarkerRenderIterator(WChart2DRenderer& renderer)
    : renderer_(renderer)
  { }

  virtual bool startSeries(const WDataSeries& series, double groupWidth,
			   int numBarGroups, int currentBarGroup)
  {
    marker_ = WPainterPath();

    if (series.marker() != NoMarker) {
      renderer_.chart()->drawMarker(series, marker_);
      renderer_.painter().save();
      renderer_.painter().setShadow(series.shadow());
    }

    return true;
  }

  virtual void endSeries()
  {
    renderer_.painter().restore();
  }

  virtual void newValue(const WDataSeries& series, double x, double y,
			double stackY,
			const WModelIndex& xIndex,
			const WModelIndex& yIndex)
  {
    if (!Utils::isNaN(x) && !Utils::isNaN(y)) {
      WPointF p = renderer_.map(x, y, series.axis(),
				currentXSegment(), currentYSegment());
      
      if (!marker_.isEmpty()) {
	WPainter& painter = renderer_.painter();
	painter.save();
	painter.translate(hv(p));

	WPen pen = WPen(series.markerPen());

	boost::any penColor = yIndex.data(MarkerPenColorRole);
	if (penColor.empty() && xIndex.isValid())
	  penColor = xIndex.data(MarkerPenColorRole);

	if (!penColor.empty())
	  pen.setColor(boost::any_cast<WColor>(penColor));

	painter.setPen(pen);

	WBrush brush = WBrush(series.markerBrush());

	boost::any brushColor = yIndex.data(MarkerBrushColorRole);
	if (brushColor.empty() && xIndex.isValid())
	  brushColor = xIndex.data(MarkerBrushColorRole);

	if (!brushColor.empty())
	  brush.setColor(boost::any_cast<WColor>(brushColor));

	painter.setBrush(brush);

	painter.drawPath(marker_);
	painter.restore();
      }

      if (series.type() != BarSeries) {
	boost::any toolTip = yIndex.data(ToolTipRole);
	if (!toolTip.empty()) {
	  WTransform t = renderer_.painter().worldTransform();

	  p = t.map(hv(p));

	  WCircleArea *circleArea
	    = new WCircleArea(static_cast<int>(p.x()), static_cast<int>(p.y()), 5);
	  circleArea->setToolTip(asString(toolTip));

	  renderer_.chart()->addDataPointArea(series, xIndex, circleArea);
	}
      }
    }
  }

  WPointF hv(const WPointF& p) {
    return renderer_.hv(p);
  }

  WPointF hv(double x, double y) {
    return renderer_.hv(x, y);
  }

private:
  WChart2DRenderer& renderer_;
  WPainterPath      marker_;
};

WChart2DRenderer::WChart2DRenderer(WCartesianChart *chart,
				   WPainter& painter, const WRectF& rectangle)
  : chart_(chart),
    painter_(painter)
{
  segmentMargin_ = 40;

  painter_.save();

  if (chart_->orientation() == Vertical) {
    painter_.translate(rectangle.topLeft());

    width_ = (int)rectangle.width();
    height_ = (int)rectangle.height();
  } else {
    painter_.translate(rectangle.topLeft());

    width_ = (int)rectangle.height();
    height_ = (int)rectangle.width();
  }

  for (int i = 0; i < 3; ++i)
    location_[i] = MinimumValue;
}

WChart2DRenderer::~WChart2DRenderer()
{
  painter_.restore();
}

void WChart2DRenderer::initLayout()
{
  calcChartArea();           // sets chartArea_
  prepareAxes();             // provides logical dimensions to the axes
}

void WChart2DRenderer::render()
{
  tildeStartMarker_ = WPainterPath();
  tildeStartMarker_.moveTo(0, 0);
  tildeStartMarker_.lineTo(0, segmentMargin_ - 25);
  tildeStartMarker_.moveTo(-15, segmentMargin_ - 10);
  tildeStartMarker_.lineTo(15, segmentMargin_ - 20);

  tildeEndMarker_ = WPainterPath();
  tildeEndMarker_.moveTo(0, 0);
  tildeEndMarker_.lineTo(0, -(segmentMargin_ - 25));
  tildeEndMarker_.moveTo(-15, -(segmentMargin_ - 20));
  tildeEndMarker_.lineTo(15, -(segmentMargin_ - 10));

  initLayout();

  renderBackground();        // render the background
  renderAxes(Grid);          // render the grid
  renderSeries();            // render the data series
  renderAxes(Line | Labels); // render the axes (lines & labels)
  renderLegend();            // render legend and titles
}

void WChart2DRenderer::prepareAxes()
{
  chart_->axis(XAxis).prepareRender(*this);
  chart_->axis(Y1Axis).prepareRender(*this);
  chart_->axis(Y2Axis).prepareRender(*this);

  const WAxis& xAxis = chart_->axis(XAxis);
  const WAxis& yAxis = chart_->axis(YAxis);
  const WAxis& y2Axis = chart_->axis(Y2Axis);

  if (xAxis.scale() == CategoryScale) {
    switch (xAxis.location()) {
    case MinimumValue:
    case ZeroValue:
      location_[XAxis] = MinimumValue;
      break;
    case MaximumValue:
      location_[XAxis] = MaximumValue;
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
      if (other.segments_.front().renderMinimum == 0)
	location = ZeroValue;
    } else
      if (other.segments_.front().renderMaximum == 0)
	location = MaximumValue;

    location_[axis.id()] = location;
  }

  // force Y axes to the sides when dual Y axes
  if (y2Axis.isVisible()) {
    if (!(location_[Y1Axis] == ZeroValue
	  && (xAxis.segments_.front().renderMinimum == 0)))
      location_[Y1Axis] = MinimumValue;

    location_[Y2Axis] = MaximumValue;
  } else
    location_[Y2Axis] = MaximumValue;

  // adjust axis borders to make them look neat and polished
  xAxis.setOtherAxisLocation(location_[YAxis]);
  yAxis.setOtherAxisLocation(location_[XAxis]);
  y2Axis.setOtherAxisLocation(location_[XAxis]);
}

WPointF WChart2DRenderer::map(double xValue, double yValue,
			      Axis axis, int currentXSegment,
			      int currentYSegment) const
{
  const WAxis& xAxis = chart_->axis(XAxis);
  const WAxis& yAxis = chart_->axis(axis);

  return WPointF(xAxis.mapToDevice(xValue, currentXSegment),
		 yAxis.mapToDevice(yValue, currentYSegment));
}

void WChart2DRenderer::calcChartArea()
{
  if (chart_->orientation() == Vertical)
    chartArea_ = WRectF(chart_->plotAreaPadding(Left),
			chart_->plotAreaPadding(Top),
			std::max(1, width_ - chart_->plotAreaPadding(Left)
				 - chart_->plotAreaPadding(Right)),
			std::max(1, height_ - chart_->plotAreaPadding(Top)
				 - chart_->plotAreaPadding(Bottom)));
  else
    chartArea_ = WRectF(chart_->plotAreaPadding(Top),
			chart_->plotAreaPadding(Right),
			std::max(1, width_ - chart_->plotAreaPadding(Top)
				 - chart_->plotAreaPadding(Bottom)),
			std::max(1, height_ - chart_->plotAreaPadding(Right)
				 - chart_->plotAreaPadding(Left)));
}

WRectF WChart2DRenderer::chartSegmentArea(WAxis yAxis, int xSegment,
					  int ySegment) const
{
  const WAxis& xAxis = chart_->axis(XAxis);

  const WAxis::Segment& xs = xAxis.segments_[xSegment];
  const WAxis::Segment& ys = yAxis.segments_[ySegment];

  // margin used when clipping, see also WAxis::prepareRender(),
  // when the renderMinimum/maximum is 0, clipping is done exact
  const int CLIP_MARGIN = 5;

  double x1 = xs.renderStart
    + (xSegment == 0
       ? (xs.renderMinimum == 0 ? 0 : -CLIP_MARGIN)
       : -segmentMargin_/2);
  double x2 = xs.renderStart + xs.renderLength
    + (xSegment == xAxis.segmentCount() - 1
       ? (xs.renderMaximum == 0 ? 0 : CLIP_MARGIN)
       : segmentMargin_/2);

  double y1 = ys.renderStart - ys.renderLength
    - (ySegment == yAxis.segmentCount() - 1
       ? (ys.renderMaximum == 0 ? 0 : CLIP_MARGIN)
       : segmentMargin_/2);
  double y2 = ys.renderStart
    + (ySegment == 0
       ? (ys.renderMinimum == 0 ? 0 : CLIP_MARGIN)
       : segmentMargin_/2);

  return WRectF(std::floor(x1 + 0.5), std::floor(y1 + 0.5),
		std::floor(x2 - x1 + 0.5), std::floor(y2 - y1 + 0.5));
}

void WChart2DRenderer::renderBackground()
{
  if (chart_->background().style() != NoBrush)
    painter_.fillRect(hv(chartArea_), chart_->background());
}

void WChart2DRenderer::renderAxis(const WAxis& axis,
				  WFlags<AxisProperty> properties)
{
  bool vertical = axis.id() != XAxis;

  WFont oldFont1 = painter_.font();
  WFont labelFont = axis.labelFont();
  painter_.setFont(labelFont);

  double u = 0;
  enum { Left = 0x1, Right = 0x2, Both = 0x3 } tickPos = Left;
  AlignmentFlag labelHFlag = AlignLeft;

  switch (location_[axis.id()]) {
  case MinimumValue:
    tickPos = Left;

    if (vertical) {
      labelHFlag = AlignRight;
      u = chartArea_.left() - 0.5 - axis.margin();
    } else {
      labelHFlag = AlignTop;
      u = chartArea_.bottom() + 0.5 + axis.margin();
    }

    break;
  case MaximumValue:
    tickPos = Right;

    if (vertical) {
      labelHFlag = AlignLeft;
      u = chartArea_.right() + 0.5 + axis.margin();
    } else {
      labelHFlag = AlignBottom;
      u = chartArea_.top() - 0.5 - axis.margin();
    }
    break;
  case ZeroValue:
    tickPos = Both;

    if (vertical) {
      labelHFlag = AlignRight;
      u = std::floor(map(0, 0, YAxis).x()) + 0.5;
    } else {
      labelHFlag = AlignTop;
      u = std::floor(map(0, 0, YAxis).y()) + 0.5;
    }
    break;
  }

  for (int segment = 0; segment < axis.segmentCount(); ++segment) {
    const WAxis::Segment& s = axis.segments_[segment];

    if ((properties & Line) && axis.isVisible()) { 
      painter_.setPen(axis.pen());

      WPointF begin, end;

      if (vertical) {
	begin = hv(u, s.renderStart);
	end = hv(u, s.renderStart - s.renderLength);
      } else {
	begin = hv(s.renderStart, u);
	end = hv(s.renderStart + s.renderLength, u);
      }

      painter_.drawLine(begin, end);

      bool rotate = (chart_->orientation() == Vertical) != vertical;

      if (segment != 0) {
	painter_.save();
	painter_.translate(begin);
	if (rotate)
	  painter_.rotate(90);
	painter_.drawPath(tildeStartMarker_);
	painter_.restore();
      }

      if (segment != axis.segmentCount() - 1) {
	painter_.save();
	painter_.translate(end);
	if (rotate)
	  painter_.rotate(90);
	painter_.drawPath(tildeEndMarker_);
	painter_.restore();	
      }
    }

    WPainterPath gridPath;
    WPainterPath ticksPath;

    std::vector<WAxis::TickLabel> ticks;
    axis.getLabelTicks(*this, ticks, segment);

    const WAxis& other
      = axis.id() == XAxis ? chart_->axis(Y1Axis) : chart_->axis(XAxis);
    const WAxis::Segment& s0 = other.segments_.front();
    const WAxis::Segment& sn = other.segments_.back();

    for (unsigned i = 0; i < ticks.size(); ++i) {
      double d = ticks[i].u;

      double dd = axis.mapToDevice(d, segment);

      dd = std::floor(dd) + 0.5;

      int tickLength = ticks[i].tickLength == WAxis::TickLabel::Long
	? TICK_LENGTH : TICK_LENGTH / 2;

      WPointF labelPos;

      switch (location_[axis.id()]) {
      case MinimumValue:
	if (vertical)
	  labelPos = WPointF(u - tickLength, dd);
	else
	  labelPos = WPointF(dd, u + tickLength);

	break;
      case MaximumValue:
	if (vertical)
	  labelPos = WPointF(u + tickLength, dd);
	else
	  labelPos = WPointF(dd, u - tickLength);

	break;
      case ZeroValue:
	if (vertical) {
	  /* force labels at bottom and left even if axis is in middle */
	  if (chart_->type() == CategoryChart)
	    labelPos = WPointF(chartArea_.left() - 0.5
			       - axis.margin() - tickLength,
			       dd);
	  else
	    labelPos = WPointF(u - tickLength, dd);
      } else {
	  /* force labels at bottom and left even if axis is in middle */
	  if (chart_->type() == CategoryChart)
	    labelPos = WPointF(dd, chartArea_.bottom() + 0.5
			       + axis.margin() + tickLength);
	  else
	    labelPos = WPointF(dd, u + tickLength);
	}
      }

      if (ticks[i].tickLength != WAxis::TickLabel::Zero) {
	if (vertical) {
	  ticksPath.moveTo(hv(u + (tickPos & Left ? -tickLength : 0), dd));
	  ticksPath.lineTo(hv(u + (tickPos & Right ? +tickLength : 0), dd));
	  if (ticks[i].tickLength == WAxis::TickLabel::Long) {
	    gridPath.moveTo(hv(s0.renderStart, dd));
	    gridPath.lineTo(hv(sn.renderStart + sn.renderLength, dd));
	  }
	} else {
	  ticksPath.moveTo(hv(dd, u + (tickPos & Right ? -tickLength : 0)));
	  ticksPath.lineTo(hv(dd, u + (tickPos & Left ? +tickLength : 0)));
	  if (ticks[i].tickLength == WAxis::TickLabel::Long) {
	    gridPath.moveTo(hv(dd, s0.renderStart));
	    gridPath.lineTo(hv(dd, sn.renderStart - sn.renderLength));
	  }
	}
      }

      if ((properties & Labels) && !ticks[i].label.empty()
	  && axis.isVisible()) {
	WFlags<AlignmentFlag> labelFlags = labelHFlag;

	if (vertical)
	  if (axis.labelAngle() == 0)
	    labelFlags |= AlignMiddle;
	  else if (axis.labelAngle() > 0)
	    labelFlags |= AlignTop;
	  else
	    labelFlags |= AlignBottom;
	else
	  if (axis.labelAngle() == 0)
	    labelFlags |= AlignCenter;
	  else if (axis.labelAngle() > 0)
	    labelFlags |= AlignRight;
	  else
	    labelFlags |= AlignLeft;

	renderLabel(ticks[i].label,
		    labelPos, black, labelFlags, axis.labelAngle(), 3);
      }
    }

    if ((properties & Grid) && axis.isGridLinesEnabled())
      painter_.strokePath(gridPath, axis.gridLinesPen());

    if ((properties & Line) && axis.isVisible())
      painter_.strokePath(ticksPath, axis.pen());

    if (segment == 0 && (properties & Labels) && !axis.title().empty()) {
      WFont oldFont2 = painter_.font();
      WFont titleFont = axis.titleFont();
      painter_.setFont(titleFont);

      bool chartVertical = chart_->orientation() == Vertical;

      if (vertical) {
	if (chartVertical)
	  renderLabel(axis.title(),
		      WPointF(u + (labelHFlag == AlignRight ? 15 : -15),
			      chartArea_.top() - 8),
		      black, labelHFlag | AlignBottom, 0, 0);
	else
	  renderLabel(axis.title(),
		      WPointF(u + (labelHFlag == AlignRight ? -40 : +40),
			      chartArea_.center().y()),
		      black,
		      (labelHFlag == AlignRight ? AlignLeft : AlignRight) |
		      AlignMiddle, 0, 0);
      } else {
	if (chartVertical)
	  renderLabel(axis.title(),
		      WPointF(chartArea_.center().x(), u + 22),
		      black, AlignTop | AlignCenter, 0, 0);
	else
	  renderLabel(axis.title(),
		      WPointF(chartArea_.right(), u),
		      black, AlignTop | AlignLeft, 0, 8);
      }

      painter_.setFont(oldFont2);
    }
  }

  painter_.setFont(oldFont1);
}

void WChart2DRenderer::renderAxes(WFlags<AxisProperty> properties)
{
  renderAxis(chart_->axis(XAxis), properties);
  renderAxis(chart_->axis(Y1Axis), properties);
  renderAxis(chart_->axis(Y2Axis), properties);
}

void WChart2DRenderer::iterateSeries(SeriesIterator *iterator,
				     bool reverseStacked)
{
  const std::vector<WDataSeries>& series = chart_->series();
  WAbstractItemModel *model = chart_->model();
  unsigned rows = model->rowCount();

  double groupWidth;
  int numBarGroups;
  int currentBarGroup;

#ifndef WT_TARGET_JAVA
  std::vector<double> stackedValuesInit(rows);
#else
  std::vector<double> stackedValuesInit;
  stackedValuesInit.insert(stackedValuesInit.begin(), rows, 0.0);
#endif // WT_TARGET_JAVA

  const bool scatterPlot = chart_->type() == ScatterPlot;

  if (scatterPlot) {
    numBarGroups = 1;
    currentBarGroup = 0;
  } else {
    numBarGroups = calcNumBarGroups();
    currentBarGroup = 0;
  }

  bool containsBars = false;
  for (unsigned g = 0; g < series.size(); ++g) {
    if (series[g].isHidden())
      continue;

    groupWidth = series[g].barWidth() * (map(2, 0).x() - map(1, 0).x());

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

	Axis a = series[g].axis();

	for (;;) {
	  if (g < series.size()
	      && (((int)g == endSeries) || series[g].isStacked())
	      && (series[g].axis() == a)) {
	    if (series[g].type() == BarSeries)
	      containsBars = true;

	    for (unsigned row = 0; row < rows; ++row) {
	      double y
		= asNumber(model->data(row, series[g].modelColumn()));

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

	Axis a = series[g].axis();

	if (series[g].type() == BarSeries)
	  containsBars = true;
	++g;

	for (;;) {
	  if (g < series.size() && series[g].isStacked()
	      && series[g].axis() == a) {
	    if (series[g].type() == BarSeries)
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
	iterator->startSeries(series[i], groupWidth, numBarGroups,
			      currentBarGroup);

      std::vector<double> stackedValues;

      if (doSeries ||
	  (!scatterPlot && i != endSeries)) {

	for (int currentXSegment = 0;
	     currentXSegment < chart_->axis(XAxis).segmentCount();
	     ++currentXSegment) {

	  for (int currentYSegment = 0;
	       currentYSegment < chart_->axis(series[i].axis()).segmentCount();
	       ++currentYSegment) {

	    stackedValues.clear();
	    Utils::insert(stackedValues, stackedValuesInit);

	    WRectF csa = chartSegmentArea(chart_->axis(series[i].axis()),
					  currentXSegment, currentYSegment);

	    iterator->startSegment(currentXSegment, currentYSegment, csa);

	    painter_.save();

	    WPainterPath clipPath;
	    clipPath.addRect(hv(csa));
	    painter_.setClipPath(clipPath);
	    painter_.setClipping(true);

	    for (unsigned row = 0; row < rows; ++row) {
	      WModelIndex xIndex, yIndex;

	      double x;
	      if (scatterPlot)
		if (chart_->XSeriesColumn() != -1) {
		  xIndex = model->index(row, chart_->XSeriesColumn());
		  x = asNumber(model->data(xIndex));
		} else
		  x = row;
	      else
		x = row;

	      yIndex = model->index(row, series[i].modelColumn());
	      double y = asNumber(model->data(yIndex));

	      double prevStack;

	      if (scatterPlot)
		iterator->newValue(series[i], x, y, 0, xIndex, yIndex);
	      else {
		prevStack = stackedValues[row];

		double nextStack = stackedValues[row];
		if (!Utils::isNaN(y)) {
		  if (reverseStacked)
		    nextStack -= y;
		  else
		    nextStack += y;
		}

		stackedValues[row] = nextStack;

		if (doSeries) {
		  if (reverseStacked)
		    iterator->newValue(series[i], x, prevStack, nextStack,
				       xIndex, yIndex);
		  else
		    iterator->newValue(series[i], x, nextStack, prevStack,
				       xIndex, yIndex);
		}
	      }
	    }

	    iterator->endSegment();

	    painter_.restore();
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

void WChart2DRenderer::renderSeries()
{
  {
    SeriesRenderIterator iterator(*this);
    iterateSeries(&iterator, true);
  }

  {
    LabelRenderIterator iterator(*this);
    iterateSeries(&iterator);
  }

  {
    MarkerRenderIterator iterator(*this);
    iterateSeries(&iterator);
  }
}

int WChart2DRenderer::calcNumBarGroups()
{
  const std::vector<WDataSeries>& series = chart_->series();

  int numBarGroups = 0;

  bool newGroup = true;
  for (unsigned i = 0; i < series.size(); ++i)
    if (series[i].type() == BarSeries) {
      if (newGroup || !series[i].isStacked())
	++numBarGroups;
      newGroup = false;
    } else
      newGroup = true;

  return numBarGroups;
}

void WChart2DRenderer::renderLegend()
{
  bool vertical = chart_->orientation() == Vertical;

  if (chart_->isLegendEnabled()) {
    int numSeriesWithLegend = 0;

    for (unsigned i = 0; i < chart_->series().size(); ++i)
      if (chart_->series()[i].isLegendEnabled())
	++numSeriesWithLegend;

    const int lineHeight = 25;

    int x = int(vertical ? chartArea_.right()
		: height_ - chartArea_.top()) + 20;

    if (vertical && chart_->axis(Y2Axis).isVisible())
      x += 40;

    int y = (vertical
	     ? int(chartArea_.center().y()) : int(chartArea_.center().x()))
      - lineHeight * numSeriesWithLegend / 2; 

    painter_.setPen(WPen());

    for (unsigned i = 0; i < chart_->series().size(); ++i)
      if (chart_->series()[i].isLegendEnabled()) {
	chart_->renderLegendItem(painter_, WPointF(x, y + lineHeight/2),
				 chart_->series()[i]);
	y += lineHeight;
      }
  }

  if (!chart_->title().empty()) {
    int x = (vertical ? width_/2 : height_/2);
    WFont oldFont = painter_.font();
    WFont titleFont = chart_->titleFont();
    painter_.setFont(titleFont);
    painter_.drawText(x - 50, 5, 100, 50,
		      AlignCenter | AlignTop,
		      chart_->title());
    painter_.setFont(oldFont);
  }
}

void WChart2DRenderer::renderLabel(const WString& text, const WPointF& p,
				   const WColor& color,
				   WFlags<AlignmentFlag> flags,
				   double angle, int margin)
{
  AlignmentFlag horizontalAlign = flags & AlignHorizontalMask;
  AlignmentFlag verticalAlign = flags & AlignVerticalMask;

  AlignmentFlag rHorizontalAlign = horizontalAlign;
  AlignmentFlag rVerticalAlign = verticalAlign;

  double width = 100;
  double height = 20;

  WPointF pos = hv(p);

  double left = pos.x();
  double top = pos.y();

  if (chart_->orientation() == Horizontal) {
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

  switch (rHorizontalAlign) {
  case AlignLeft:
    left = pos.x() + margin; break;
  case AlignCenter:
    left = pos.x() - width/2; break;
  case AlignRight:
    left = pos.x() - width - margin;
  default:
    break;
 }

  switch (rVerticalAlign) {
  case AlignTop:
    top = pos.y() + margin; break;
  case AlignMiddle:
    top = pos.y() - height/2; break;
  case AlignBottom:
    top = pos.y() - height - margin; break;
  default:
    break;
  }

  WPen pen(color);
  WPen oldPen = painter_.pen();
  painter_.setPen(pen);

  if (angle == 0)
    painter_.drawText(WRectF(left, top, width, height),
		      rHorizontalAlign | rVerticalAlign, text);
  else {
    painter_.save();
    painter_.translate(pos);
    painter_.rotate(-angle);
    painter_.drawText(WRectF(left - pos.x(), top - pos.y(), width, height),
		      rHorizontalAlign | rVerticalAlign, text);
    painter_.restore();
  }

  painter_.setPen(oldPen);
}

WPointF WChart2DRenderer::hv(const WPointF& p) const
{
  return hv(p.x(), p.y());
}

WPointF WChart2DRenderer::hv(double x, double y) const
{
  return chart_->hv(x, y, height_);
}

WRectF WChart2DRenderer::hv(const WRectF& r) const
{
  if (chart_->orientation() == Vertical)
    return r;
  else {
    WPointF tl = hv(r.bottomLeft());
    return WRectF(tl.x(), tl.y(), r.height(), r.width());
  }
}

  }
}
