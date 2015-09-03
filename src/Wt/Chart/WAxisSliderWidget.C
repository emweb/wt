/*
 * Copyright (C) 2015 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Chart/WAxisSliderWidget"

#include "Wt/WApplication"
#include "Wt/WBrush"
#include "Wt/WColor"
#include "Wt/WJavaScript"
#include "Wt/WPainter"
#include "Wt/WStringStream"
#include "Wt/WTransform"

#include "Wt/Chart/WCartesianChart"

#ifndef WT_DEBUG_JS
#include "js/WAxisSliderWidget.min.js"
#endif

namespace Wt {

LOGGER("Chart.WAxisSliderWidget");

  namespace Chart {

WAxisSliderWidget::WAxisSliderWidget(WContainerWidget *parent)
  : WPaintedWidget(parent),
    chart_(0),
    seriesColumn_(-1),
    selectedSeriesPen_(&seriesPen_),
    handleBrush_(WColor(0,0,200)),
    background_(WColor(230, 230, 230)),
    selectedAreaBrush_(WColor(255, 255, 255)),
    autoPadding_(false),
    labelsEnabled_(true)
{
  init();
}

WAxisSliderWidget::WAxisSliderWidget(WCartesianChart *chart, int seriesColumn, WContainerWidget *parent)
  : WPaintedWidget(parent),
    chart_(chart),
    seriesColumn_(seriesColumn),
    selectedSeriesPen_(&seriesPen_),
    handleBrush_(WColor(0,0,200)),
    background_(WColor(230, 230, 230)),
    selectedAreaBrush_(WColor(255, 255, 255)),
    autoPadding_(false),
    labelsEnabled_(true)
{
  init();
}

void WAxisSliderWidget::init()
{
  transform_ = createJSTransform();

  mouseWentDown().connect("function(o, e){var o=" + this->sObjJsRef() + ";if(o){o.mouseDown(o, e);}}");
  mouseWentUp().connect("function(o, e){var o=" + this->sObjJsRef() + ";if(o){o.mouseUp(o, e);}}");
  mouseDragged().connect("function(o, e){var o=" + this->sObjJsRef() + ";if(o){o.mouseDrag(o, e);}}");
  mouseMoved().connect("function(o, e){var o=" + this->sObjJsRef() + ";if(o){o.mouseMoved(o, e);}}");
  touchStarted().connect("function(o, e){var o=" + this->sObjJsRef() + ";if(o){o.touchStarted(o, e);}}");
  touchEnded().connect("function(o, e){var o=" + this->sObjJsRef() + ";if(o){o.touchEnded(o, e);}}");
  touchMoved().connect("function(o, e){var o=" + this->sObjJsRef() + ";if(o){o.touchMoved(o, e);}}");

  setSelectionAreaPadding(0, Top);
  setSelectionAreaPadding(20, Left | Right);
  setSelectionAreaPadding(30, Bottom);

  if (chart_) chart_->addAxisSliderWidget(this);
}

WAxisSliderWidget::~WAxisSliderWidget()
{
  if (chart_) chart_->removeAxisSliderWidget(this);
  if (selectedSeriesPen_ != &seriesPen_) {
    delete selectedSeriesPen_;
  }
}

void WAxisSliderWidget::setChart(WCartesianChart *chart)
{
  if (chart != chart_) {
    if (chart_) chart_->removeAxisSliderWidget(this);
    chart_ = chart;
    if (chart_) chart_->addAxisSliderWidget(this);
    update();
  }
}

void WAxisSliderWidget::setSeriesColumn(int seriesColumn)
{
  if (seriesColumn != seriesColumn_) {
    seriesColumn_ = seriesColumn;
    update();
  }
}

void WAxisSliderWidget::setSeriesPen(const WPen& pen)
{
  if (pen != seriesPen_) {
    seriesPen_ = pen;
    update();
  }
}

void WAxisSliderWidget::setSelectedSeriesPen(const WPen& pen)
{
  if (selectedSeriesPen_ != &seriesPen_) {
    delete selectedSeriesPen_;
    if (*selectedSeriesPen_ != pen) {
      selectedSeriesPen_ = new WPen(pen);
    }
  } else {
    selectedSeriesPen_ = new WPen(pen);
  }
  update();
}

void WAxisSliderWidget::setHandleBrush(const WBrush& brush)
{
  if (brush != handleBrush_) {
    handleBrush_ = brush;
    update();
  }
}

void WAxisSliderWidget::setBackground(const WBrush& brush)
{
  if (brush != background_) {
    background_ = brush;
    update();
  }
}

void WAxisSliderWidget::setSelectedAreaBrush(const WBrush& brush)
{
  if (brush != selectedAreaBrush_) {
    selectedAreaBrush_ = brush;
    update();
  }
}

void WAxisSliderWidget::render(WFlags<RenderFlag> flags)
{
  WPaintedWidget::render(flags);

  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WAxisSliderWidget.js", "WAxisSliderWidget", wtjs1);
}

void WAxisSliderWidget::setSelectionAreaPadding(int padding, WFlags<Side> sides)
{
  if (sides & Top)
    padding_[0] = padding;
  if (sides & Right)
    padding_[1] = padding;
  if (sides & Bottom)
    padding_[2] = padding;
  if (sides & Left)
    padding_[3] = padding;
}

int WAxisSliderWidget::selectionAreaPadding(Side side) const
{
  switch (side) {
  case Top:
    return padding_[0];
  case Right:
    return padding_[1];
  case Bottom:
    return padding_[2];
  case Left:
    return padding_[3];
  default:
    LOG_ERROR("selectionAreaPadding(): improper side.");
    return 0;
  }
}

void WAxisSliderWidget::setAutoLayoutEnabled(bool enabled)
{
  autoPadding_ = enabled;
}

void WAxisSliderWidget::setLabelsEnabled(bool enabled)
{
  if (enabled != labelsEnabled_) {
    labelsEnabled_ = enabled;
    update();
  }
}

WRectF WAxisSliderWidget::hv(const WRectF& rect) const
{
  bool horizontal = chart_->orientation() == Vertical; // yes, vertical chart means horizontal X axis slider

  if (horizontal) {
    return rect;
  } else {
    return WRectF(width().value() - rect.y() - rect.height(), rect.x(), rect.height(), rect.width());
  }
}

WTransform WAxisSliderWidget::hv(const WTransform& t) const
{
  bool horizontal = chart_->orientation() == Vertical; // yes, vertical chart means horizontal X axis slider

  if (horizontal) {
    return t;
  } else {
    return WTransform(0,1,1,0,0,0) * t * WTransform(0,1,1,0,0,0);
  }
}

void WAxisSliderWidget::paintEvent(WPaintDevice *paintDevice)
{
  // Don't paint anything, unless we're associated to a chart,
  // and the chart has been painted.
  if (!chart_ || !chart_->cObjCreated_) return;

  if (chart_->series(seriesColumn_).type() != LineSeries &&
      chart_->series(seriesColumn_).type() != CurveSeries) {
    if (getMethod() == HtmlCanvas) {
      WStringStream ss;
      ss << "jQuery.removeData(" << jsRef() << ",'sobj');";
      ss << "\nif (" << objJsRef() << ") {"
	   << objJsRef() << ".canvas.style.cursor = 'auto';"
	   << "setTimeout(" << objJsRef() << ".repaint,0);"
	    "}\n";
      doJavaScript(ss.str());
    }
    LOG_ERROR("WAxisSliderWidget is not associated with a line or curve series.");
    return;
  }

  WPainter painter(paintDevice);

  bool horizontal = chart_->orientation() == Vertical; // yes, vertical chart means horizontal X axis slider

  double w = horizontal ? width().value() : height().value(),
	 h = horizontal ? height().value() : width().value();

  bool autoPadding = autoPadding_;
  if (autoPadding && ((paintDevice->features() & WPaintDevice::HasFontMetrics) == 0) && labelsEnabled_) {
    LOG_ERROR("setAutoLayout(): device does not have font metrics "
	"(not even server-side font metrics).");
    autoPadding = false;
  }

  if (autoPadding) {
    if (horizontal) {
      if (labelsEnabled_) {
	setSelectionAreaPadding(0, Top);
	setSelectionAreaPadding(
	    static_cast<int>(chart_->axis(XAxis).calcMaxTickLabelSize(
	      paintDevice,
	      Vertical
	    ) + 10), Bottom);
	setSelectionAreaPadding(
	    static_cast<int>(std::max(chart_->axis(XAxis).calcMaxTickLabelSize(
	      paintDevice,
	      Horizontal
	    ) / 2, 10.0)), Left | Right);
      } else {
	setSelectionAreaPadding(0, Top);
	setSelectionAreaPadding(5, Left | Right | Bottom);
      }
    } else {
      if (labelsEnabled_) {
	setSelectionAreaPadding(0, Right);
	setSelectionAreaPadding(
	    static_cast<int>(std::max(chart_->axis(XAxis).calcMaxTickLabelSize(
	      paintDevice,
	      Vertical
	    ) / 2, 10.0)), Top | Bottom);
	setSelectionAreaPadding(
	    static_cast<int>(chart_->axis(XAxis).calcMaxTickLabelSize(
	      paintDevice,
	      Horizontal
	    ) + 10), Left);
      } else {
	setSelectionAreaPadding(0, Right);
	setSelectionAreaPadding(5, Top | Bottom | Left);
      }
    }
  }

  double left = horizontal ? selectionAreaPadding(Left) : selectionAreaPadding(Top);
  double right = horizontal ? selectionAreaPadding(Right) : selectionAreaPadding(Bottom);
  double top = horizontal ? selectionAreaPadding(Top) : selectionAreaPadding(Right);
  double bottom = horizontal ? selectionAreaPadding(Bottom) : selectionAreaPadding(Left);

  double maxW = w - left - right;
  WRectF drawArea(left, 0, maxW, h);
  std::vector<WAxis::Segment> segmentsBak = chart_->axis(XAxis).segments_;
  double renderIntervalBak = chart_->axis(XAxis).renderInterval_;
  chart_->axis(XAxis).prepareRender(horizontal ? Horizontal : Vertical, drawArea.width());

  const WRectF& chartArea = chart_->chartArea_;
  WRectF selectionRect;
  {
    // Determine initial position based on xTransform of chart
    double u = -chart_->xTransformHandle_.value().dx() / (chartArea.width() * chart_->xTransformHandle_.value().m11());
    selectionRect = WRectF(0, top, maxW, h - (top + bottom));
    transform_.setValue(WTransform(1 / chart_->xTransformHandle_.value().m11(), 0, 0, 1, u * maxW, 0));
  }
  WRectF seriesArea(left, top + 5, maxW, h - (top + bottom + 5));
  WTransform selectionTransform = hv(WTransform(1,0,0,1,left,0) * transform_.value());
  WRectF rect = selectionTransform.map(hv(selectionRect));

  painter.fillRect(hv(WRectF(left, top, maxW, h - top - bottom)), background_);
  painter.fillRect(rect, selectedAreaBrush_);

  // FIXME: Refactor this code? We have very similar code now in WCartesianChart and
  //	    WCartesian3DChart too.
  const double TICK_LENGTH = 5;
  const double ANGLE1 = 15;
  const double ANGLE2 = 80;

  double tickStart = 0.0, tickEnd = 0.0, labelPos = 0.0;
  AlignmentFlag labelHFlag = AlignCenter, labelVFlag = AlignMiddle;

  WAxis &axis = chart_->axis(XAxis);

  if (horizontal) {
    tickStart = 0;
    tickEnd = TICK_LENGTH;
    labelPos = TICK_LENGTH;
    labelVFlag = AlignTop;
  } else {
    tickStart = -TICK_LENGTH;
    tickEnd = 0;
    labelPos = -TICK_LENGTH;
    labelHFlag = AlignRight;
  }

  if (horizontal) {
    if (axis.labelAngle() > ANGLE1) {
      labelHFlag = AlignRight;
      if (axis.labelAngle() > ANGLE2)
	labelVFlag = AlignMiddle;
    } else if (axis.labelAngle() < -ANGLE1) {
      labelHFlag = AlignLeft;
      if (axis.labelAngle() < -ANGLE2)
	labelVFlag = AlignMiddle;
    }
  } else {
    if (axis.labelAngle() > ANGLE1) {
      labelVFlag = AlignBottom;
      if (axis.labelAngle() > ANGLE2)
	labelHFlag = AlignCenter;
    } else if (axis.labelAngle() < -ANGLE1) {
      labelVFlag = AlignTop;
      if (axis.labelAngle() < -ANGLE2)
	labelHFlag = AlignCenter;
    }
  }
  
  WFlags<AxisProperty> axisProperties = Line;
  if (labelsEnabled_) {
    axisProperties |= Labels;
  }

  if (horizontal) {
    axis.render(
	painter,
	axisProperties,
	WPointF(drawArea.left(), h - bottom),
	WPointF(drawArea.right(), h - bottom),
	tickStart, tickEnd, labelPos,
	labelHFlag | labelVFlag);
    WPainterPath line;
    line.moveTo(drawArea.left() + 0.5, h - (bottom - 0.5));
    line.lineTo(drawArea.right(), h - (bottom - 0.5));
    painter.strokePath(line, chart_->axis(XAxis).pen());
  } else {
    axis.render(
	painter,
	axisProperties,
	WPointF(selectionAreaPadding(Left) - 1, drawArea.left()),
	WPointF(selectionAreaPadding(Left) - 1, drawArea.right()),
	tickStart, tickEnd, labelPos,
	labelHFlag | labelVFlag);
    WPainterPath line;
    line.moveTo(selectionAreaPadding(Left) - 0.5, drawArea.left() + 0.5);
    line.lineTo(selectionAreaPadding(Left) - 0.5, drawArea.right());
    painter.strokePath(line, chart_->axis(XAxis).pen());
  }

  WPainterPath curve;
  {
    WTransform t = WTransform(1,0,0,1,seriesArea.left(),seriesArea.top()) *
      WTransform(seriesArea.width() / chartArea.width(), 0, 0, seriesArea.height() / chartArea.height(), 0, 0) *
      WTransform(1,0,0,1,-chartArea.left(),-chartArea.top());
    if (!horizontal) {
      t = WTransform(0,1,1,0,selectionAreaPadding(Left) - selectionAreaPadding(Right) - 5,0) * t * WTransform(0,1,1,0,0,0);
    }
    curve = t.map(chart_->pathForSeries(seriesColumn_));
  }

  {
    WRectF leftHandle = hv(WRectF(-5, top, 5, h - top - bottom));
    WTransform t = (WTransform(1,0,0,1,left,-top) *
	(WTransform().translate(transform_.value().map(selectionRect.topLeft()))));
    painter.fillRect(hv(t).map(leftHandle), handleBrush_);
  }

  {
    WRectF rightHandle = hv(WRectF(0, top, 5, h - top - bottom));
    WTransform t = (WTransform(1,0,0,1,left,-top) *
	(WTransform().translate(transform_.value().map(selectionRect.topRight()))));
    painter.fillRect(hv(t).map(rightHandle), handleBrush_);
  }

  if (selectedSeriesPen_ != &seriesPen_ && *selectedSeriesPen_ != seriesPen_) {
    WPainterPath clipPath;
    clipPath.addRect(hv(selectionRect));
    painter.setClipPath(selectionTransform.map(clipPath));
    painter.setClipping(true);

    painter.setPen(selectedSeriesPen());
    painter.drawPath(curve);

    WPainterPath leftClipPath;
    leftClipPath.addRect(hv(WTransform(1,0,0,1,-selectionRect.width(),0).map(selectionRect)));
    painter.setClipPath(hv(
	  WTransform(1,0,0,1,left,-top) *
	  (WTransform().translate(transform_.value().map(selectionRect.topLeft())))
	).map(leftClipPath));

    painter.setPen(seriesPen());
    painter.drawPath(curve);

    WPainterPath rightClipPath;
    rightClipPath.addRect(hv(WTransform(1,0,0,1,selectionRect.width(),0).map(selectionRect)));
    painter.setClipPath(hv(
	  WTransform(1,0,0,1,left - selectionRect.right(),-top) *
	  (WTransform().translate(transform_.value().map(selectionRect.topRight())))
	).map(rightClipPath));

    painter.drawPath(curve);

    painter.setClipping(false);
  } else {
    painter.setPen(seriesPen());
    painter.drawPath(curve);
  }

  if (getMethod() == HtmlCanvas) {
    WApplication *app = WApplication::instance();
    WStringStream ss;
    ss << "new " WT_CLASS ".WAxisSliderWidget("
       << app->javaScriptClass() << ","
       << jsRef() << ","
       << objJsRef() << ","
       << "{"
       "chart:" << chart_->cObjJsRef() << ","
       "transform:" << transform_.jsRef() << ","
       "rect:function(){return " << rect.jsRef() << "},"
       "drawArea:" << drawArea.jsRef() << ","
       "series:" << seriesColumn_ <<
       "});";
    doJavaScript(ss.str());
  }

  chart_->axis(XAxis).segments_ = segmentsBak;
  chart_->axis(XAxis).renderInterval_ = renderIntervalBak;
}

std::string WAxisSliderWidget::sObjJsRef() const
{
  return "jQuery.data(" + jsRef() + ",'sobj')";
}

  }
}
