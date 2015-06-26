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
  namespace Chart {

WAxisSliderWidget::WAxisSliderWidget(WContainerWidget *parent)
  : WPaintedWidget(parent),
    chart_(0),
    seriesColumn_(-1),
    margin_(10),
    selectedSeriesPen_(&seriesPen_),
    handleBrush_(WColor(0,0,200)),
    background_(WColor(230, 230, 230))
{
  init();
}

WAxisSliderWidget::WAxisSliderWidget(WCartesianChart *chart, int seriesColumn, WContainerWidget *parent)
  : WPaintedWidget(parent),
    chart_(chart),
    seriesColumn_(seriesColumn),
    margin_(10),
    selectedSeriesPen_(&seriesPen_),
    handleBrush_(WColor(0,0,200)),
    background_(WColor(230, 230, 230))
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

void WAxisSliderWidget::render(WFlags<RenderFlag> flags)
{
  WPaintedWidget::render(flags);

  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WAxisSliderWidget.js", "WAxisSliderWidget", wtjs1);
}

void WAxisSliderWidget::paintEvent(WPaintDevice *paintDevice)
{
  // Don't paint anything, unless we're associated to a chart,
  // and the chart has been painted.
  if (!chart_ || !chart_->cObjCreated_) return;

  WPainter painter(paintDevice);

  double w = width().value(),
	 h = height().value();

  WRectF chartArea = chart_->chartArea_;
  WRectF selectionRect;
  {
    // Determine initial position based on xTransform of chart
    double maxW = w - 2 * margin_;
    double u = -chart_->xTransform_.value().dx() / (chartArea.width() * chart_->xTransform_.value().m11());
    selectionRect = WRectF(0, 0, maxW, h - 25);
    transform_.setValue(WTransform(1 / chart_->xTransform_.value().m11(), 0, 0, 1, u * maxW, 0));
  }
  WRectF drawArea(margin_, 0, w - 2 * margin_, h);
  WRectF seriesArea(margin_, 5, w - 2 * margin_, h - 30);
  WTransform selectionTransform = WTransform(1,0,0,1,margin_,0) * transform_.value();
  WRectF rect = selectionTransform.map(selectionRect);

  painter.fillRect(WRectF(margin_, 0, w - 2 * margin_, h - 25), background_);
  painter.fillRect(rect, WBrush(WColor(255,255,255)));

  chart_->axis(XAxis).prepareRender(Horizontal, drawArea.width());
  chart_->axis(XAxis).render(
      painter,
      Labels | Line,
      WPointF(drawArea.left(), h - 25),
      WPointF(drawArea.right(), h - 25),
      0, 5, 5,
      AlignCenter | AlignTop);
  WPainterPath line;
  line.moveTo(drawArea.left() + 0.5, h - 24.5);
  line.lineTo(drawArea.right(), h - 24.5);
  painter.strokePath(line, chart_->axis(XAxis).pen());

  WTransform t = WTransform(1,0,0,1,seriesArea.left(),seriesArea.top()) *
    WTransform(seriesArea.width() / chartArea.width(), 0, 0, seriesArea.height() / chartArea.height(), 0, 0) *
    WTransform(1,0,0,1,-chartArea.left(),-chartArea.top());
  WPainterPath curve = t.map(chart_->pathForSeries(seriesColumn_));

  WRectF leftHandle = WRectF(-5, 0, 5, h - 25);
  painter.fillRect((WTransform(1,0,0,1,margin_,0) * (WTransform().translate(transform_.value().map(selectionRect.topLeft())))).map(leftHandle), handleBrush_);

  WRectF rightHandle = WRectF(0, 0, 5, h - 25);
  painter.fillRect(
      (WTransform(1,0,0,1,margin_,0) *
      (WTransform().translate(transform_.value().map(selectionRect.topRight())))).map(rightHandle),
      handleBrush_
  );

  if (selectedSeriesPen_ != &seriesPen_ && *selectedSeriesPen_ != seriesPen_) {
    WPainterPath clipPath;
    clipPath.addRect(selectionRect);
    painter.setClipPath(selectionTransform.map(clipPath));
    painter.setClipping(true);

    painter.setPen(selectedSeriesPen());
    painter.drawPath(curve);

    WPainterPath leftClipPath;
    leftClipPath.addRect(WTransform(1,0,0,1,-selectionRect.width(),0).map(selectionRect));
    painter.setClipPath((
	  WTransform(1,0,0,1,margin_,0) *
	  (WTransform().translate(transform_.value().map(selectionRect.topLeft())))
	).map(leftClipPath));

    painter.setPen(seriesPen());
    painter.drawPath(curve);

    WPainterPath rightClipPath;
    rightClipPath.addRect(WTransform(1,0,0,1,selectionRect.width(),0).map(selectionRect));
    painter.setClipPath((
	  WTransform(1,0,0,1,margin_ - selectionRect.right(),0) *
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
       "seriesArea:" << seriesArea.jsRef() << ","
       "series:" << seriesColumn_ <<
       "});";
    doJavaScript(ss.str());
  }
}

std::string WAxisSliderWidget::sObjJsRef() const
{
  return "jQuery.data(" + jsRef() + ",'sobj')";
}

  }
}
