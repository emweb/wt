/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/Chart/WChart2DRenderer"
#include "Wt/Chart/WDataSeries"
#include "Wt/Chart/WCartesianChart"
#include "Wt/Chart/WStandardPalette"

#include "Wt/WAbstractArea"
#include "Wt/WAbstractItemModel"
#include "Wt/WPainter"
#include "Wt/WText"

#include "WtException.h"

namespace {
  
class PlotException : public std::exception
{
public:
  PlotException(const std::string what)
    : what_(what) { }
  ~PlotException() throw() { }

  const char *what() const throw() { return what_.c_str(); }
  
private:
  std::string what_;
};

}

namespace Wt {
  namespace Chart {

WCartesianChart::WCartesianChart(WContainerWidget *parent)
  : WAbstractChart(parent),
    orientation_(Vertical),
    XSeriesColumn_(-1),
    type_(CategoryChart),
    barMargin_(0),
    legend_(false)
{
  init();
}

WCartesianChart::WCartesianChart(ChartType type, WContainerWidget *parent)
  : WAbstractChart(parent),
    orientation_(Vertical),
    XSeriesColumn_(-1),
    type_(type),
    barMargin_(0),
    legend_(false)
{
  init();
}

void WCartesianChart::init()
{
  setPalette(new WStandardPalette(WStandardPalette::Muted));
  //setPreferredMethod(InlineSvgVml);
	
#ifdef WT_TARGET_JAVA
  for (int i = 0; i < 3; ++i)
    axes_[i] = WAxis();
#endif //WT_TARGET_JAVA
	
  axes_[XAxis].init(this, XAxis);
  axes_[YAxis].init(this, YAxis);
  axes_[Y2Axis].init(this, Y2Axis);

  setPlotAreaPadding(40, Left | Right);
  setPlotAreaPadding(30, Top | Bottom);
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
    axes_[XAxis].init(this, XAxis);
    update();
  }
}

void WCartesianChart::addSeries(const WDataSeries& series)
{
  series_.push_back(series);
  series_.back().setChart(this);
  update();
}

void WCartesianChart::removeSeries(int modelColumn)
{
  int index = seriesIndexOf(modelColumn);

  if (index != -1) {
    series_.erase(series_.begin() + index);
    update();
  }
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

  throw PlotException("Column " + boost::lexical_cast<std::string>(modelColumn)
		      + " not in plot");
}

const WDataSeries& WCartesianChart::series(int modelColumn) const
{
  int index = seriesIndexOf(modelColumn);

  if (index != -1)
    return series_[index];

  throw PlotException("Column " + boost::lexical_cast<std::string>(modelColumn)
		      + " not in plot");
}

void WCartesianChart::setSeries(const std::vector<WDataSeries>& series)
{
  series_ = series;

  for (unsigned i = 0; i < series_.size(); ++i)
    series_[i].setChart(this);

  update();
}

WAxis& WCartesianChart::axis(Axis axis)
{
  return axes_[axis];
}

const WAxis& WCartesianChart::axis(Axis axis) const
{
  return axes_[axis];
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
  if (legend_ != enabled) {
    legend_ = enabled;

    update();
  }
}

void WCartesianChart::paint(WPainter& painter, const WRectF& rectangle) const
{
  if (!painter.isActive())
    throw WtException("WCartesianChart::paint(): painter is not active.");

  WRectF rect = rectangle;

  if (rect.isEmpty())
    rect = painter.window();

  WChart2DRenderer *renderer = createRenderer(painter, rect);
  renderer->render();
  delete renderer;
}

WChart2DRenderer *WCartesianChart::createRenderer(WPainter& painter,
						  const WRectF& rectangle) const
{
  return new WChart2DRenderer(const_cast<WCartesianChart *>(this),
			      painter, rectangle);
}

void WCartesianChart::paintEvent(WPaintDevice *paintDevice)
{
  while (!areas().empty())
    delete areas().front();

  WPainter painter(paintDevice);
  painter.setRenderHint(WPainter::Antialiasing);
  paint(painter);
}

void WCartesianChart::drawMarker(const WDataSeries& series,
				 WPainterPath& result)
  const
{
  const double size = series.markerSize();
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
    result.moveTo(0, -hsize);
    result.lineTo(hsize, 0.6 * hsize);
    result.lineTo(-hsize, 0.6 * hsize);
    result.closeSubPath();
    break;
  default:
    ;
  }
}

void WCartesianChart::renderLegendIcon(WPainter& painter,
				       const WPointF& pos,
				       const WDataSeries& series) const
{
  switch (series.type()) {
  case BarSeries: {
    WPainterPath path;
    path.moveTo(-6, 8);
    path.lineTo(-6, -8);
    path.lineTo(6, -8);
    path.lineTo(6, 8);
    painter.setPen(series.pen());
    painter.setBrush(series.brush());
    painter.translate(pos.x() + 7.5, pos.y());  
    painter.drawPath(path);
    painter.translate(-(pos.x() + 7.5), -pos.y());
    break;
  }
  case LineSeries:
  case CurveSeries: {
    painter.setPen(series.pen());
    double offset = (series.pen().width() == 0 ? 0.5 : 0);
    painter.drawLine(pos.x(), pos.y() + offset, pos.x() + 16, pos.y() + offset);
  }
    // no break;
  case PointSeries: {
    WPainterPath path;
    drawMarker(series, path);
    if (!path.isEmpty()) {
      painter.translate(pos.x() + 8, pos.y());  
      painter.setPen(series.markerPen());
      painter.setBrush(series.markerBrush());
      painter.drawPath(path);
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

  painter.setPen(fontPen);
  painter.drawText(pos.x() + 17, pos.y() - 10, 100, 20,
		   AlignLeft | AlignMiddle,
		   asString(model()->headerData(series.modelColumn())));
}

WWidget* WCartesianChart::createLegendItemWidget(int index)
{
  WContainerWidget* legendItem = new WContainerWidget();

  legendItem->addWidget(new IconWidget(this, index));
  WText* label = new WText(asString(model()->headerData(index)));
  label->setVerticalAlignment(AlignTop);
  legendItem->addWidget(label);

  return legendItem;
}

void WCartesianChart::addDataPointArea(const WDataSeries& series,
				       const WModelIndex& xIndex,
				       WAbstractArea *area)
{
  addArea(area);
}

void WCartesianChart::initLayout(const WRectF& rectangle)
{
  WRectF rect = rectangle;

  if (rect.isEmpty())
    rect = WRectF(0, 0, width().toPixels(), height().toPixels());

  WPainter painter;
  WChart2DRenderer *renderer = createRenderer(painter, rect);
  renderer->initLayout();
  delete renderer;
}

WPointF WCartesianChart::mapFromDevice(const WPointF& point, Axis ordinateAxis)
  const
{
  const WAxis& xAxis = axis(XAxis);
  const WAxis& yAxis = axis(ordinateAxis);

  WPointF p = inverseHv(point.x(), point.y(), width().toPixels());

  return WPointF(xAxis.mapFromDevice(p.x()), yAxis.mapFromDevice(p.y()));
}

WPointF WCartesianChart::mapToDevice(const boost::any& xValue,
				     const boost::any& yValue,
				     Axis ordinateAxis, int xSegment,
				     int ySegment) const
{
  const WAxis& xAxis = axis(XAxis);
  const WAxis& yAxis = axis(ordinateAxis);
  
  return hv(xAxis.mapToDevice(xValue, xSegment),
	    yAxis.mapToDevice(yValue, ySegment),
	    width().toPixels());
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
  if (XSeriesColumn_ <= topLeft.column()
      && XSeriesColumn_ >= bottomRight.column()) {
    update();
    return;
  }

  for (unsigned i = 0; i < series_.size(); ++i) {
    if (series_[i].modelColumn() >= topLeft.column()
	&& series_[i].modelColumn() <= bottomRight.column()) {
      update();
      break;
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

  }
}
