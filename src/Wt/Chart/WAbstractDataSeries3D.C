/*
 * Copyright (C) 2013 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Chart/WAbstractDataSeries3D"

#include "Wt/WAbstractItemModel"
#include "Wt/WCanvasPaintDevice"
#include "Wt/WPainter"
#include "Wt/Chart/WAbstractColorMap"
#include "Wt/Chart/WCartesian3DChart"

using namespace Wt;

namespace {
  void clearConnections(std::vector<Wt::Signals::connection> connections) {
    for (unsigned i=0; i < connections.size(); i++)
      connections[i].disconnect();
    connections.clear();
  }
}

namespace Wt {
  namespace Chart {

WAbstractDataSeries3D::WAbstractDataSeries3D(WAbstractItemModel *model)
  : model_(model),
    chart_(0),
    rangeCached_(false),
    pointSize_(2.0),
    colormap_(0),
    showColorMap_(false),
    colorMapSide_(Right),
    legendEnabled_(true),
    hidden_(false),
    /* in webGL, the z-direction is out of the screen, in model coordinates
     * it's the vertical direction */
    mvMatrix_(1.0f, 0.0f, 0.0f, 0.0f,
	      0.0f, 0.0f, 1.0f, 0.0f,
	      0.0f, 1.0f, 0.0f, 0.0f,
	      0.0f, 0.0f, 0.0f, 1.0f)
{
}

WAbstractDataSeries3D::~WAbstractDataSeries3D()
{
}

void WAbstractDataSeries3D::setModel(WAbstractItemModel *model)
{
  if (model != model_) {
    // handle previous model
    if (model_ && chart_) {
      clearConnections(connections_);
    }
    rangeCached_ = false;

    // set new model
    model_ = model;

    if (model_ && chart_) {
      chart_->updateChart(WCartesian3DChart::GLContext);
      connections_.push_back(model_->modelReset().connect(boost::bind(&WCartesian3DChart::updateChart, chart_, WCartesian3DChart::GLTextures | WCartesian3DChart::GLContext)));
      connections_.push_back(model_->dataChanged().connect(boost::bind(&WCartesian3DChart::updateChart, chart_, WCartesian3DChart::GLTextures | WCartesian3DChart::GLContext)));
      connections_.push_back(model_->rowsInserted().connect(boost::bind(&WCartesian3DChart::updateChart, chart_, WCartesian3DChart::GLTextures | WCartesian3DChart::GLContext)));
      connections_.push_back(model_->columnsInserted().connect(boost::bind(&WCartesian3DChart::updateChart, chart_, WCartesian3DChart::GLTextures | WCartesian3DChart::GLContext)));
    }
  }
}

void WAbstractDataSeries3D::setTitle(const WString& name)
{
  name_ = name;
  if (chart_)
    chart_->updateChart(WCartesian3DChart::GLTextures);
}

void WAbstractDataSeries3D::setPointSize(double size)
{
  if (size != pointSize_) {
    pointSize_ = size;
    if (chart_)
      chart_->updateChart(WCartesian3DChart::GLContext);
  }
}

void WAbstractDataSeries3D::setColorMap(WAbstractColorMap* colormap)
{
  if (colormap != colormap_)
    delete colormap_;

  colormap_ = colormap;
  if (chart_)
    chart_->updateChart(WCartesian3DChart::GLContext |
			WCartesian3DChart::GLTextures);
}

void WAbstractDataSeries3D::setColorMapVisible(bool enabled)
{
  if (showColorMap_ == enabled)
    return;

  showColorMap_ = enabled;
  if (chart_)
    chart_->updateChart(WCartesian3DChart::GLTextures);
}

void WAbstractDataSeries3D::setColorMapSide(Side side)
{
  if (colorMapSide_ == side)
    return;

  colorMapSide_ = side;
  if (chart_)
    chart_->updateChart(WCartesian3DChart::GLTextures);
}

void WAbstractDataSeries3D::setHidden(bool enabled)
{
  if (enabled != hidden_) {
    hidden_ = enabled;
    if (chart_)
      chart_->updateChart(WCartesian3DChart::GLContext);
  }
}

WGLWidget::Texture WAbstractDataSeries3D::colorTexture()
{
  WPaintDevice *cpd = 0;
  if (colormap_ == 0) {
    cpd = chart_->createPaintDevice(WLength(1),WLength(1));
    WColor seriesColor = chartpaletteColor();
    WPainter painter(cpd);
    painter.setPen(WPen(seriesColor));
    painter.drawLine(0,0,1,1);
    painter.end();
  } else {
    cpd = chart_->createPaintDevice(WLength(1),WLength(1024));
    WPainter painter(cpd);
    colormap_->createStrip(&painter);
    painter.end();
  }

  WGLWidget::Texture tex = chart_->createTexture();
  chart_->bindTexture(WGLWidget::TEXTURE_2D, tex);
  chart_->pixelStorei(WGLWidget::UNPACK_FLIP_Y_WEBGL, 1);
  chart_->texImage2D(WGLWidget::TEXTURE_2D, 0, WGLWidget::RGBA, WGLWidget::RGBA, WGLWidget::UNSIGNED_BYTE, cpd);

  return tex;
}

void WAbstractDataSeries3D::setDefaultTitle(int i)
{
  std::string tmp = std::string("dataset ");
  tmp.append(boost::lexical_cast<std::string>(i));
  name_ = WString(tmp);
}

WColor WAbstractDataSeries3D::chartpaletteColor() const
{
  if (colormap_)
    return WColor();

  int index = 0;
  for (unsigned i=0; i < chart_->dataSeries().size(); i++) { // which colorscheme
    if (chart_->dataSeries()[i] == this) {
      break;
    } else if (chart_->dataSeries()[i]->colorMap() == 0) {
      index++;
    }
  }
  return chart_->palette()->brush(index).color();
}

void WAbstractDataSeries3D::setChart(WCartesian3DChart *chart)
{
  if (chart == chart_)
    return;
  else if (chart_)
    chart_->removeDataSeries(this);

  if (chart_ && model_)
    clearConnections(connections_);
  
  chart_ = chart;

  if (chart_ && model_) {
    connections_.push_back(model_->modelReset().connect(boost::bind(&WCartesian3DChart::updateChart, chart_, WCartesian3DChart::GLTextures | WCartesian3DChart::GLContext)));
    connections_.push_back(model_->dataChanged().connect(boost::bind(&WCartesian3DChart::updateChart, chart_, WCartesian3DChart::GLTextures | WCartesian3DChart::GLContext)));
      connections_.push_back(model_->rowsInserted().connect(boost::bind(&WCartesian3DChart::updateChart, chart_, WCartesian3DChart::GLTextures | WCartesian3DChart::GLContext)));
      connections_.push_back(model_->columnsInserted().connect(boost::bind(&WCartesian3DChart::updateChart, chart_, WCartesian3DChart::GLTextures | WCartesian3DChart::GLContext)));
  }
}

  }
}
