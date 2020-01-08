/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Chart/WAbstractGridData.h"

#include "Wt/Chart/WAbstractColorMap.h"
#include "Wt/WAbstractItemModel.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WMemoryResource.h"
#include "Wt/WPainter.h"
#include "Wt/WPaintDevice.h"
#include "Wt/WStandardItemModel.h"
#include "Wt/WStandardItem.h"
#include "Wt/WVector3.h"
#include "Wt/WVector4.h"
#include "WebUtils.h"

#include <cmath>

#include "gridDataShaders"

namespace {
  using namespace Wt;

  inline void push(FloatBuffer& vec,
		   float x, float y, float z)
  {
    vec.push_back(x);
    vec.push_back(y);
    vec.push_back(z);
  }

  inline void push(IntBuffer& vec,
		   int x, int y, int z)
  {
    vec.push_back(x);
    vec.push_back(y);
    vec.push_back(z);
  }

  using namespace Wt::Chart;

  inline std::size_t axisToIndex(Axis axis)
  {
    if (axis == Axis::X3D) {
      return 0;
    } else if (axis == Axis::Y3D) {
      return 1;
    } else if (axis == Axis::Z3D) {
      return 2;
    } else {
      throw WException("Invalid axis for 3D chart");
    }
  }
}

namespace Wt {
  namespace Chart {

const float WAbstractGridData::zeroBarCompensation = 0.001f;

WAbstractGridData::WAbstractGridData(std::shared_ptr<WAbstractItemModel> model)
  : WAbstractDataSeries3D(model),
    seriesType_(Series3DType::Point),
    surfaceMeshEnabled_(false),
    colorRoleEnabled_(false),
    barWidthX_(0.5f),
    barWidthY_(0.5f),
    isoLineColorMap_(nullptr),
    jsMinPt_(3),
    jsMaxPt_(3),
    minPtChanged_(true),
    maxPtChanged_(true),
    clippingLinesEnabled_(false),
    clippingLinesColor_(0, 0, 0)
{
  for (unsigned i = 0; i < 3; ++i) {
    minPt_.push_back(-std::numeric_limits<float>::infinity());
    maxPt_.push_back(std::numeric_limits<float>::infinity());
  }
}

WAbstractGridData::~WAbstractGridData()
{
  for (unsigned i = 0; i < binaryResources_.size(); i++) {
    delete binaryResources_[i];
  }
}

void WAbstractGridData::setType(Series3DType type)
{
  using Wt::operator|; // for CNOR

  if (seriesType_ != type) {
    seriesType_ = type;
    if (chart_)
      chart_->updateChart(WFlags<ChartUpdates>(ChartUpdates::GLContext) | ChartUpdates::GLTextures);
  }
}

void WAbstractGridData::setSurfaceMeshEnabled(bool enabled)
{
  using Wt::operator|; // for CNOR

  if (enabled != surfaceMeshEnabled_) {
    surfaceMeshEnabled_ = enabled;
    if (seriesType_ == Series3DType::Surface)
      if (chart_)
        chart_->updateChart(WFlags<ChartUpdates>(ChartUpdates::GLContext) | ChartUpdates::GLTextures);
  }
}

void WAbstractGridData::setBarWidth(double xWidth, double yWidth)
{
  using Wt::operator|; // for CNOR

  if (xWidth != barWidthX_ || yWidth != barWidthY_) {
    barWidthX_ = xWidth;
    barWidthY_ = yWidth;
    if (chart_)
      chart_->updateChart(WFlags<ChartUpdates>(ChartUpdates::GLContext) | ChartUpdates::GLTextures);
  }
}

void WAbstractGridData::setPen(const WPen &pen)
{
  using Wt::operator|; // for CNOR

  meshPen_ = pen;

  if (chart_)
    chart_->updateChart(WFlags<ChartUpdates>(ChartUpdates::GLContext) | ChartUpdates::GLTextures);
}

float WAbstractGridData::stackAllValues(std::vector<WAbstractGridData*> dataseries,
					int i, int j) const
{
  float value = 0;
  for (unsigned k = 0; k<dataseries.size(); k++) {
    float plotCubeVal = (float)chart_->toPlotCubeCoords
      (Wt::asNumber(dataseries[k]->data(i, j)), Axis::Z3D);
    if (plotCubeVal <= 0)
      plotCubeVal = zeroBarCompensation;
    value += plotCubeVal;
  }

  return value;
}

void WAbstractGridData::setClippingLinesEnabled(bool clippingLinesEnabled)
{
  clippingLinesEnabled_ = clippingLinesEnabled;
  if (chart_)
    chart_->updateChart(WFlags<ChartUpdates>(ChartUpdates::GLContext) | ChartUpdates::GLTextures);
}

void WAbstractGridData::setClippingLinesColor(WColor clippingLinesColor)
{
  clippingLinesColor_ = clippingLinesColor;
  if (chart_)
    chart_->updateChart(WFlags<ChartUpdates>(ChartUpdates::GLContext) | ChartUpdates::GLTextures);
}

void WAbstractGridData::updateGL()
{
  // compatible chart-type check
  switch (seriesType_) {
  case Series3DType::Point:
    if (chart_->type() != ChartType::Scatter)
      return;
    initializePointSeriesBuffers();
    break;
  case Series3DType::Surface:
    if (chart_->type() != ChartType::Scatter)
      return;
    initializeSurfaceSeriesBuffers();
    break;
  case Series3DType::Bar:
    if (chart_->type() != ChartType::Category)
      return;
    initializeBarSeriesBuffers();
    break;
  }

  // initialize texture (for colormap)
  colormapTexture_ = colorTexture();

  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_MAG_FILTER,
			WGLWidget::NEAREST);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_MIN_FILTER,
			WGLWidget::NEAREST);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_WRAP_S,
			WGLWidget::CLAMP_TO_EDGE);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_WRAP_T,
			WGLWidget::CLAMP_TO_EDGE);

  isoLineColorMapTexture_ = isoLineColorMapTexture();

  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_MAG_FILTER,
			WGLWidget::NEAREST);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_MIN_FILTER,
			WGLWidget::NEAREST);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_WRAP_S,
			WGLWidget::CLAMP_TO_EDGE);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_WRAP_T,
			WGLWidget::CLAMP_TO_EDGE);

  pointSpriteTexture_ = pointSpriteTexture();

  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_MAG_FILTER,
			WGLWidget::NEAREST);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_MIN_FILTER,
			WGLWidget::NEAREST);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_WRAP_S,
			WGLWidget::CLAMP_TO_EDGE);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_WRAP_T,
			WGLWidget::CLAMP_TO_EDGE);

  initShaders();
  
  if (!jsMinPt_.initialized()) {
    chart_->initJavaScriptVector(jsMinPt_);
  }

  if (!jsMaxPt_.initialized()) {
    chart_->initJavaScriptVector(jsMaxPt_);
  }

  if (minPtChanged_) {
    chart_->setJavaScriptVector(jsMinPt_, minPt_);
    minPtChanged_ = false;
  }
  if (maxPtChanged_) {
    chart_->setJavaScriptVector(jsMaxPt_, maxPt_);
    maxPtChanged_ = false;
  }

  // initialize colormap settings
  double min, max;
  switch (seriesType_) {
  case Series3DType::Bar:
    break; // coordinate-buffers made in initializeBarSeriesBuffers
  case Series3DType::Point:
  case Series3DType::Surface:
    chart_->useProgram(seriesProgram_);
    if (colormap_ != nullptr) {
      min = chart_->toPlotCubeCoords(colormap_->minimum(), Axis::Z3D);
      max = chart_->toPlotCubeCoords(colormap_->maximum(), Axis::Z3D);
      chart_->uniform1f(offset_, min);
      chart_->uniform1f(scaleFactor_, 1.0/(max-min));
    } else {
      chart_->uniform1f(offset_, 0.0);
      chart_->uniform1f(scaleFactor_, 1.0);
    }
    if (isoLineHeights_.size() > 0) {
      chart_->useProgram(isoLineProgram_);
      if (isoLineColorMap_ != nullptr) {
	min = chart_->toPlotCubeCoords(isoLineColorMap_->minimum(), 
				       Axis::Z3D);
	max = chart_->toPlotCubeCoords(isoLineColorMap_->maximum(), 
				       Axis::Z3D);
	chart_->uniform1f(isoLine_offset_, min);
	chart_->uniform1f(isoLine_scaleFactor_, 1.0/(max-min));
      } else if (colormap_ != nullptr) {
	min = chart_->toPlotCubeCoords(colormap_->minimum(), Axis::Z3D);
	max = chart_->toPlotCubeCoords(colormap_->maximum(), Axis::Z3D);
	chart_->uniform1f(isoLine_offset_, min);
	chart_->uniform1f(isoLine_scaleFactor_, 1.0/(max-min));
      } else {
	chart_->uniform1f(isoLine_offset_, 0.0);
	chart_->uniform1f(isoLine_scaleFactor_, 1.0);
      }
    }
    break;
  };

  // additional uniform initializations
  chart_->useProgram(seriesProgram_);
  chart_->uniformMatrix4(mvMatrixUniform_, mvMatrix_);
  chart_->uniformMatrix4(pMatrix_, chart_->pMatrix());
  switch (seriesType_) {
  case Series3DType::Bar:
    chart_->useProgram(meshProgram_);
    chart_->uniformMatrix4(mesh_mvMatrixUniform_, mvMatrix_);
    chart_->uniformMatrix4(mesh_pMatrix_, chart_->pMatrix());
  case Series3DType::Point:
    chart_->useProgram(colSeriesProgram_);
    chart_->uniformMatrix4(mvMatrixUniform2_, mvMatrix_);
    chart_->uniformMatrix4(pMatrix2_, chart_->pMatrix());
    break;
  case Series3DType::Surface:
    if (surfaceMeshEnabled_) {
      chart_->useProgram(meshProgram_);
      chart_->uniformMatrix4(mesh_mvMatrixUniform_, mvMatrix_);
      chart_->uniformMatrix4(mesh_pMatrix_, chart_->pMatrix());
    }
    if (isoLineHeights_.size() > 0) {
      chart_->useProgram(isoLineProgram_);
      chart_->uniformMatrix4(isoLine_mvMatrixUniform_, mvMatrix_);
      chart_->uniformMatrix4(isoLine_pMatrix_, chart_->pMatrix());
    }
    if (chart_->isIntersectionLinesEnabled() || clippingLinesEnabled() ||
	!chart_->intersectionPlanes_.empty()) {
      chart_->useProgram(singleColorProgram_);
      chart_->uniformMatrix4(singleColor_mvMatrixUniform_, mvMatrix_);
      chart_->uniformMatrix4(singleColor_pMatrix_, chart_->pMatrix());
      chart_->useProgram(positionProgram_);
      chart_->uniformMatrix4(position_mvMatrixUniform_, mvMatrix_);
      chart_->uniformMatrix4(position_pMatrix_, chart_->pMatrix());
    }
    break;
  };
}

void WAbstractGridData::loadBinaryResource(FloatBuffer& data,
					   std::vector<WGLWidget::Buffer>& buffers)
{
  buffers.push_back(chart_->createBuffer());
  // binaryResources_.push_back(new WMemoryResource("application/octet", this));
  // binaryResources_.back()->setData(Utils::toCharPointer(data),
  // 				   data.size()*sizeof(float));
  // WGLWidget::ArrayBuffer buffer =
  //   chart_->createAndLoadArrayBuffer(binaryResources_.back()->url());

  chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, buffers.back());
  chart_->bufferDatafv(WGLWidget::ARRAY_BUFFER, data, WGLWidget::STATIC_DRAW,
		       true);
  // chart_->bufferData(WGLWidget::ARRAY_BUFFER,
  // 		     buffer,
  // 		     WGLWidget::STATIC_DRAW);
}

void WAbstractGridData::initializePointSeriesBuffers()
{
  int Nx = nbXPoints();
  int Ny = nbYPoints();

  int cnt = countSimpleData();
  std::vector<FloatBuffer> simplePtsArrays, simplePtsSizes;
  std::vector<FloatBuffer> coloredPtsArrays, coloredPtsSizes, coloredPtsColors;

  simplePtsArrays.push_back(Utils::createFloatBuffer(3*cnt));
  simplePtsSizes.push_back(Utils::createFloatBuffer(cnt));
  coloredPtsArrays.push_back(Utils::createFloatBuffer(3*(Nx*Ny-cnt)));
  coloredPtsSizes.push_back(Utils::createFloatBuffer(Nx*Ny-cnt));
  coloredPtsColors.push_back(Utils::createFloatBuffer(4*(Nx*Ny-cnt)));

  pointDataFromModel(simplePtsArrays[0], simplePtsSizes[0], coloredPtsArrays[0],
		     coloredPtsSizes[0], coloredPtsColors[0]);

  for (unsigned i = 0; i < simplePtsArrays.size(); i++) {
    if (simplePtsArrays[i].size() != 0) { // don't include empty buffers
      // pos of simple points
      loadBinaryResource(simplePtsArrays[i], vertexPosBuffers_);
      vertexPosBufferSizes_.push_back(simplePtsArrays[i].size());
      // sizes of simple points
      loadBinaryResource(simplePtsSizes[i], vertexSizeBuffers_);
    }

    if (coloredPtsArrays[i].size() != 0) {
      // pos of colored points
      loadBinaryResource(coloredPtsArrays[i], vertexPosBuffers2_);
      vertexPosBuffer2Sizes_.push_back(coloredPtsArrays[i].size());
      // size of colored points
      loadBinaryResource(coloredPtsSizes[i], vertexSizeBuffers2_);
      // color of colored points
      loadBinaryResource(coloredPtsColors[i], vertexColorBuffers2_);
    }
  }

}

std::vector<WSurfaceSelection> WAbstractGridData::pickSurface(int x, int y) const
{
  WVector3 re;
  WVector3 rd;
  chart_->createRay(x, y, re, rd);

  WMatrix4x4 invTransform = chart_->cameraMatrix() * mvMatrix_;
#ifndef WT_TARGET_JAVA
  invTransform =
#endif
    invTransform.inverted();
  WVector4 camera;
  camera = invTransform * camera;
  WVector3 camera3 = WVector3(camera);

  int Nx = nbXPoints();
  int Ny = nbYPoints();

  std::vector<FloatBuffer> simplePtsArrays;
  int nbXaxisBuffers;
  int nbYaxisBuffers;

  nbXaxisBuffers = Nx/(SURFACE_SIDE_LIMIT-1);
  nbYaxisBuffers = Ny/(SURFACE_SIDE_LIMIT-1);
  if (Nx % (SURFACE_SIDE_LIMIT-1) != 0) {
    nbXaxisBuffers++;
  }
  if (Ny % (SURFACE_SIDE_LIMIT-1) != 0) {
    nbYaxisBuffers++;
  }

  for (int i=0; i < nbXaxisBuffers-1; i++) {
    for (int j=0; j < nbYaxisBuffers-1; j++) {
      simplePtsArrays.push_back(Utils::createFloatBuffer(3*SURFACE_SIDE_LIMIT*SURFACE_SIDE_LIMIT));
    };
    simplePtsArrays.push_back(Utils::createFloatBuffer(3*SURFACE_SIDE_LIMIT*(Ny - (nbYaxisBuffers-1)*(SURFACE_SIDE_LIMIT-1))));
  }
  for (int j=0; j < nbYaxisBuffers-1; j++) {
    simplePtsArrays.push_back(Utils::createFloatBuffer(3*(Nx - (nbXaxisBuffers-1)*(SURFACE_SIDE_LIMIT-1))*SURFACE_SIDE_LIMIT));
  }
  simplePtsArrays.push_back(Utils::createFloatBuffer(3*(Nx - (nbXaxisBuffers-1)*(SURFACE_SIDE_LIMIT-1))*(Ny - (nbYaxisBuffers-1)*(SURFACE_SIDE_LIMIT-1))));

  surfaceDataFromModel(simplePtsArrays);

  std::vector<WSurfaceSelection> result;

  for (unsigned i = 0; i < simplePtsArrays.size(); i++) {
    int Nx_patch = SURFACE_SIDE_LIMIT, Ny_patch = SURFACE_SIDE_LIMIT;
    if ( (i+1) % nbYaxisBuffers == 0) {
      Ny_patch = Ny - (nbYaxisBuffers - 1)*(SURFACE_SIDE_LIMIT-1);
    }
    if ((int)i >= (nbXaxisBuffers-1)*nbYaxisBuffers) {
      Nx_patch = Nx - (nbXaxisBuffers - 1)*(SURFACE_SIDE_LIMIT-1);
    }
    IntBuffer vertexIndices = Utils::createIntBuffer((Nx_patch-1)*(Ny_patch+1)*2);
    generateVertexIndices(vertexIndices, Nx_patch, Ny_patch);

    for (size_t j = 0; j < vertexIndices.size() - 2; ++j) {
      if (vertexIndices[j] == vertexIndices[j+1] || vertexIndices[j+1] == vertexIndices[j+2]
	  || vertexIndices[j] == vertexIndices[j+2]) {
	// Not a triangle, skip it.
	continue;
      }
      WVector3 point;
      double distance = rayTriangleIntersect(re, rd, camera3,
				   WVector3(simplePtsArrays[i][vertexIndices[j] * 3],
					    simplePtsArrays[i][vertexIndices[j] * 3 + 1],
					    simplePtsArrays[i][vertexIndices[j] * 3 + 2]),
				   WVector3(simplePtsArrays[i][vertexIndices[j + 1] * 3],
					    simplePtsArrays[i][vertexIndices[j + 1] * 3 + 1],
					    simplePtsArrays[i][vertexIndices[j + 1] * 3 + 2]),
				   WVector3(simplePtsArrays[i][vertexIndices[j + 2] * 3],
					    simplePtsArrays[i][vertexIndices[j + 2] * 3 + 1],
					    simplePtsArrays[i][vertexIndices[j + 2] * 3 + 2]),
				   point);
      if (distance != std::numeric_limits<double>::infinity()) {
	double resX = point.x() * (chart_->axis(Axis::X3D).maximum() -
				   chart_->axis(Axis::X3D).minimum())
	  + chart_->axis(Axis::X3D).minimum();
	double resY = point.y() * (chart_->axis(Axis::Y3D).maximum() -
				   chart_->axis(Axis::Y3D).minimum())
	  + chart_->axis(Axis::Y3D).minimum();
	double resZ = point.z() * (chart_->axis(Axis::Z3D).maximum() - 
				   chart_->axis(Axis::Z3D).minimum())
	  + chart_->axis(Axis::Z3D).minimum();
	result.push_back(WSurfaceSelection(distance, resX, resY, resZ));
      }
    }
  }
  return result;
}

void WAbstractGridData::drawIsoLines() const
{
  chart_->useProgram(isoLineProgram_);
  chart_->depthFunc(WGLWidget::LEQUAL);
  chart_->uniformMatrix4(isoLine_cMatrix_, chart_->jsMatrix());
  chart_->lineWidth(1.0);
  chart_->activeTexture(WGLWidget::TEXTURE0);
  chart_->bindTexture(WGLWidget::TEXTURE_2D, isoLineColorMapTexture_);
  chart_->uniform1i(isoLine_TexSampler_,0);
  for (size_t i = 0; i < isoLineHeights_.size(); ++i) {
    if (isoLineBufferSizes_[i] == 0)
      continue;
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, isoLineBuffers_[i]);
    chart_->vertexAttribPointer(isoLineVertexPosAttr_,
			  3,
			  WGLWidget::FLOAT,
			  false,
			  0,
			  0);
    chart_->enableVertexAttribArray(isoLineVertexPosAttr_);
    chart_->drawArrays(WGLWidget::LINES, 0, isoLineBufferSizes_[i] / 3);
    chart_->disableVertexAttribArray(isoLineVertexPosAttr_);
  }
}

void WAbstractGridData::linesForIsoLevel(double z, std::vector<float> &result) const
{
  int Nx = nbXPoints();
  int Ny = nbYPoints();

  double minZ = chart_->axis(Axis::Z3D).minimum();
  double maxZ = chart_->axis(Axis::Z3D).maximum();
  double scaledZ = (z - minZ) / (maxZ - minZ);

  std::vector<FloatBuffer> simplePtsArrays;
  int nbXaxisBuffers;
  int nbYaxisBuffers;

  nbXaxisBuffers = Nx/(SURFACE_SIDE_LIMIT-1);
  nbYaxisBuffers = Ny/(SURFACE_SIDE_LIMIT-1);
  if (Nx % (SURFACE_SIDE_LIMIT-1) != 0) {
    nbXaxisBuffers++;
  }
  if (Ny % (SURFACE_SIDE_LIMIT-1) != 0) {
    nbYaxisBuffers++;
  }

  for (int i=0; i < nbXaxisBuffers-1; i++) {
    for (int j=0; j < nbYaxisBuffers-1; j++) {
      simplePtsArrays.push_back(Utils::createFloatBuffer(3*SURFACE_SIDE_LIMIT*SURFACE_SIDE_LIMIT));
    };
    simplePtsArrays.push_back(Utils::createFloatBuffer(3*SURFACE_SIDE_LIMIT*(Ny - (nbYaxisBuffers-1)*(SURFACE_SIDE_LIMIT-1))));
  }
  for (int j=0; j < nbYaxisBuffers-1; j++) {
    simplePtsArrays.push_back(Utils::createFloatBuffer(3*(Nx - (nbXaxisBuffers-1)*(SURFACE_SIDE_LIMIT-1))*SURFACE_SIDE_LIMIT));
  }
  simplePtsArrays.push_back(Utils::createFloatBuffer(3*(Nx - (nbXaxisBuffers-1)*(SURFACE_SIDE_LIMIT-1))*(Ny - (nbYaxisBuffers-1)*(SURFACE_SIDE_LIMIT-1))));

  surfaceDataFromModel(simplePtsArrays);

  for (unsigned i = 0; i < simplePtsArrays.size(); i++) {
    int Nx_patch = SURFACE_SIDE_LIMIT, Ny_patch = SURFACE_SIDE_LIMIT;
    if ( (i+1) % nbYaxisBuffers == 0) {
      Ny_patch = Ny - (nbYaxisBuffers - 1)*(SURFACE_SIDE_LIMIT-1);
    }
    if ((int)i >= (nbXaxisBuffers-1)*nbYaxisBuffers) {
      Nx_patch = Nx - (nbXaxisBuffers - 1)*(SURFACE_SIDE_LIMIT-1);
    }
    IntBuffer vertexIndices = Utils::createIntBuffer((Nx_patch-1)*(Ny_patch+1)*2);
    generateVertexIndices(vertexIndices, Nx_patch, Ny_patch);

    for (size_t j = 0; j < vertexIndices.size() - 2; ++j) {
      if (vertexIndices[j] == vertexIndices[j+1] || vertexIndices[j+1] == vertexIndices[j+2]
	  || vertexIndices[j] == vertexIndices[j+2]) {
	// Not a triangle, skip it.
	continue;
      }
      WVector3 a = WVector3(simplePtsArrays[i][vertexIndices[j] * 3],
			    simplePtsArrays[i][vertexIndices[j] * 3 + 1],
			    simplePtsArrays[i][vertexIndices[j] * 3 + 2]);
      WVector3 b = WVector3(simplePtsArrays[i][vertexIndices[j + 1] * 3],
			    simplePtsArrays[i][vertexIndices[j + 1] * 3 + 1],
			    simplePtsArrays[i][vertexIndices[j + 1] * 3 + 2]);
      WVector3 c = WVector3(simplePtsArrays[i][vertexIndices[j + 2] * 3],
			    simplePtsArrays[i][vertexIndices[j + 2] * 3 + 1],
			    simplePtsArrays[i][vertexIndices[j + 2] * 3 + 2]);
      std::vector<WVector3> intersections;
      if ((a.z() >= scaledZ && b.z() < scaledZ) || 
	  (a.z() < scaledZ && b.z() >= scaledZ)) {
	double factor = (scaledZ - a.z()) / (b.z() - a.z());
	WVector3 d = b - a;
	WVector3 e = d * factor;
	WVector3 f = a + e;
	intersections.push_back(f);
      }
      if ((b.z() >= scaledZ && c.z() < scaledZ) ||
	  (b.z() < scaledZ && c.z() >= scaledZ)) {
	double factor = (scaledZ - b.z()) / (c.z() - b.z());
	WVector3 d = c - b;
	WVector3 e = d * factor;
	WVector3 f = b + e;
	intersections.push_back(f);
      }
      if (intersections.size() < 2 &&
	  ((c.z() >= scaledZ && a.z() < scaledZ) ||
	   (c.z() < scaledZ && a.z() >= scaledZ))) {
	double factor = (scaledZ - c.z()) / (a.z() - c.z());
	WVector3 d = a - c;
	WVector3 e = d * factor;
	WVector3 f = c + e;
	intersections.push_back(f);
      }
      if (intersections.size() == 2) {
	result.push_back((float)intersections[0].x());
	result.push_back((float)intersections[0].y());
	result.push_back((float)intersections[0].z());
	result.push_back((float)intersections[1].x());
	result.push_back((float)intersections[1].y());
	result.push_back((float)intersections[1].z());
      }
    }
  }
}

WGLWidget::Texture WAbstractGridData::isoLineColorMapTexture()
{
  std::unique_ptr<WPaintDevice> cpd;
  if (isoLineColorMap_ == nullptr) {
    return colorTexture();
  } else {
    cpd = chart_->createPaintDevice(WLength(1), WLength(1024));
    WPainter painter(cpd.get());
    isoLineColorMap_->createStrip(&painter);
    painter.end();
  }

  WGLWidget::Texture tex = chart_->createTexture();
  chart_->bindTexture(WGLWidget::TEXTURE_2D, tex);
  chart_->pixelStorei(WGLWidget::UNPACK_FLIP_Y_WEBGL, 1);
  chart_->texImage2D(WGLWidget::TEXTURE_2D, 0,
		     WGLWidget::RGBA, WGLWidget::RGBA,
		     WGLWidget::UNSIGNED_BYTE, cpd.get());

  return tex;
}

WBarSelection WAbstractGridData::pickBar(int x, int y) const
{
  WVector3 re;
  WVector3 rd;
  chart_->createRay(x, y, re, rd);

  double closestDistance = std::numeric_limits<double>::infinity();
  size_t closestI = 0;
  size_t closestJ = 0;

  WMatrix4x4 invTransform = chart_->cameraMatrix() * mvMatrix_;
#ifndef WT_TARGET_JAVA
  invTransform =
#endif
    invTransform.inverted();
  WVector4 camera;
  camera = invTransform * camera;
  WVector3 camera3 = WVector3(camera);

  int Nx = nbXPoints();
  int Ny = nbYPoints();

  int cnt = Nx*Ny;
  std::vector<FloatBuffer> simplePtsArrays;
  std::vector<FloatBuffer> barVertexArrays;

  int nbSimpleBarBuffers = cnt/BAR_BUFFER_LIMIT + 1;

  int PT_INFO_SIZE = 4;
  int PTS_PER_BAR = 8;
  for (int i = 0; i < nbSimpleBarBuffers - 1; ++i) {
    simplePtsArrays.push_back(Utils::createFloatBuffer(PT_INFO_SIZE*BAR_BUFFER_LIMIT));
    barVertexArrays.push_back(Utils::createFloatBuffer(PTS_PER_BAR*3*BAR_BUFFER_LIMIT));
  }
  simplePtsArrays.push_back(Utils::createFloatBuffer(PT_INFO_SIZE*(cnt - BAR_BUFFER_LIMIT*(nbSimpleBarBuffers-1))));
  barVertexArrays.push_back(Utils::createFloatBuffer(PTS_PER_BAR*3*(cnt - BAR_BUFFER_LIMIT*(nbSimpleBarBuffers-1))));

  barDataFromModel(simplePtsArrays);
  for (size_t i = 0; i < simplePtsArrays.size(); ++i) {
    barSeriesVertexData(simplePtsArrays[i], barVertexArrays[i]);
  }

  for (size_t i = 0; i < simplePtsArrays.size(); ++i) {
    IntBuffer vertexIndices =
      Utils::createIntBuffer(12*3*(simplePtsArrays[i].size()/PT_INFO_SIZE));
    generateVertexIndices(vertexIndices, 0, 0, simplePtsArrays[i].size()/PT_INFO_SIZE);

    for (size_t j = 0; j < vertexIndices.size(); j += 3) {
      WVector3 point;
      double distance = rayTriangleIntersect(re, rd, WVector3(camera),
				   WVector3(barVertexArrays[i][vertexIndices[j] * 3],
					    barVertexArrays[i][vertexIndices[j] * 3 + 1],
					    barVertexArrays[i][vertexIndices[j] * 3 + 2]),
				   WVector3(barVertexArrays[i][vertexIndices[j + 1] * 3],
					    barVertexArrays[i][vertexIndices[j + 1] * 3 + 1],
					    barVertexArrays[i][vertexIndices[j + 1] * 3 + 2]),
				   WVector3(barVertexArrays[i][vertexIndices[j + 2] * 3],
					    barVertexArrays[i][vertexIndices[j + 2] * 3 + 1],
					    barVertexArrays[i][vertexIndices[j + 2] * 3 + 2]),
				   point);
      if (distance < closestDistance) {
	closestDistance = distance;
	closestI = i;
	closestJ = j;
      }
    }
  }

  if (closestDistance != std::numeric_limits<double>::infinity()) {
    size_t tmp = BAR_BUFFER_LIMIT * closestI + closestJ / (12*3);
    return WBarSelection(
	closestDistance,
	model_->index(tmp / Ny + 1, tmp % Ny + 1)
    );
  } else {
    return WBarSelection(
	std::numeric_limits<double>::infinity(),
	WModelIndex()
    );
  }
}

void WAbstractGridData::setIsoLevels(const std::vector<double> &isoLevels)
{
  isoLineHeights_ = isoLevels;
  if (chart_)
    chart_->updateChart(WFlags<ChartUpdates>(ChartUpdates::GLContext) | ChartUpdates::GLTextures);
}

const std::vector<double> &WAbstractGridData::isoLevels() const
{
  return isoLineHeights_;
}

void WAbstractGridData
::setIsoColorMap(const std::shared_ptr<WAbstractColorMap>& colormap)
{
  isoLineColorMap_ = colormap;
  if (chart_)
    chart_->updateChart(WFlags<ChartUpdates>(ChartUpdates::GLContext) | ChartUpdates::GLTextures);
}

double WAbstractGridData::rayTriangleIntersect(const WVector3& re, const WVector3& rd, const WVector3 &camera, const WVector3& v0, const WVector3& v1, const WVector3& v2, WVector3 &point) const
{
  // Ray-triangle intersection
  //
  // Uses formula 6 from:
  // Möller, Tomas, and Ben Trumbore. "Fast, minimum storage ray/triangle intersection."
  // ACM SIGGRAPH 2005 Courses. ACM, 2005.
  // URL: http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
  WVector3 e1 = v1 - v0;
  WVector3 e2 = v2 - v0;
  WVector3 tr = re - v0;
  WVector3 P = rd.cross(e2);
  WVector3 Q = tr.cross(e1);
  double m = P.dot(e1);
  double t = Q.dot(e2) / m;
  if (t < 0) {
    return std::numeric_limits<double>::infinity();
  }
  double gamma = Q.dot(rd) / m;
  if (gamma < 0 || gamma > 1) {
    return std::numeric_limits<double>::infinity();
  }
  double beta = P.dot(tr) / m;
  if (beta < 0 || beta > 1 - gamma) {
    return std::numeric_limits<double>::infinity();
  }
#ifndef WT_TARGET_JAVA
  point = WVector3(re.x() + rd.x() * t, re.y() + rd.y() * t, re.z() + rd.z() * t);
#else
  point.setElement(0, re.x() + rd.x() * t);
  point.setElement(1, re.y() + rd.y() * t);
  point.setElement(2, re.z() + rd.z() * t);
#endif
  double distance = WVector3(point - camera).length();
  return distance;
}

void WAbstractGridData::initializeSurfaceSeriesBuffers()
{
  int Nx = nbXPoints();
  int Ny = nbYPoints();

  std::vector<FloatBuffer> simplePtsArrays;
  int nbXaxisBuffers;
  int nbYaxisBuffers;

  nbXaxisBuffers = Nx/(SURFACE_SIDE_LIMIT-1);
  nbYaxisBuffers = Ny/(SURFACE_SIDE_LIMIT-1);
  if (Nx % (SURFACE_SIDE_LIMIT-1) != 0) {
    nbXaxisBuffers++;
  }
  if (Ny % (SURFACE_SIDE_LIMIT-1) != 0) {
    nbYaxisBuffers++;
  }

  for (int i=0; i < nbXaxisBuffers-1; i++) {
    for (int j=0; j < nbYaxisBuffers-1; j++) {
      simplePtsArrays.push_back(Utils::createFloatBuffer(3*SURFACE_SIDE_LIMIT*SURFACE_SIDE_LIMIT));
    }
    simplePtsArrays.push_back(Utils::createFloatBuffer(3*SURFACE_SIDE_LIMIT*(Ny - (nbYaxisBuffers-1)*(SURFACE_SIDE_LIMIT-1))));
  }
  for (int j=0; j < nbYaxisBuffers-1; j++) {
    simplePtsArrays.push_back(Utils::createFloatBuffer(3*(Nx - (nbXaxisBuffers-1)*(SURFACE_SIDE_LIMIT-1))*SURFACE_SIDE_LIMIT));
  }
  simplePtsArrays.push_back(Utils::createFloatBuffer(3*(Nx - (nbXaxisBuffers-1)*(SURFACE_SIDE_LIMIT-1))*(Ny - (nbYaxisBuffers-1)*(SURFACE_SIDE_LIMIT-1))));

  surfaceDataFromModel(simplePtsArrays);

  for (unsigned i = 0; i < simplePtsArrays.size(); i++) {
    loadBinaryResource(simplePtsArrays[i], vertexPosBuffers_);
    vertexPosBufferSizes_.push_back(simplePtsArrays[i].size());
  }

  for (size_t i = 0; i < isoLineHeights_.size(); ++i) {
    std::vector<float> lines;
    linesForIsoLevel(isoLineHeights_[i], lines);
#ifdef WT_TARGET_JAVA
    FloatBuffer buff = Utils::createFloatBuffer(lines.size());
    for (size_t j = 0; j < lines.size(); ++j) {
      buff.push_back(lines[j]);
    }
    loadBinaryResource(buff, isoLineBuffers_);
#else
    loadBinaryResource(lines, isoLineBuffers_);
#endif
    isoLineBufferSizes_.push_back(lines.size());
  }

  for (unsigned i = 0; i < simplePtsArrays.size(); i++) {
    // initialize vertex-index-buffer
    indexBuffers_.push_back(chart_->createBuffer());
    int Nx_patch = SURFACE_SIDE_LIMIT, Ny_patch = SURFACE_SIDE_LIMIT;
    if ( (i+1) % nbYaxisBuffers == 0) {
      Ny_patch = Ny - (nbYaxisBuffers - 1)*(SURFACE_SIDE_LIMIT-1);
    }
    if ((int)i >= (nbXaxisBuffers-1)*nbYaxisBuffers) {
      Nx_patch = Nx - (nbXaxisBuffers - 1)*(SURFACE_SIDE_LIMIT-1);
    }
    IntBuffer vertexIndices = Utils::createIntBuffer((Nx_patch-1)*(Ny_patch+1)*2);
    generateVertexIndices(vertexIndices, Nx_patch, Ny_patch);

    chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, indexBuffers_[i]);
    chart_->bufferDataiv(WGLWidget::ELEMENT_ARRAY_BUFFER,
			 vertexIndices,
			 WGLWidget::STATIC_DRAW,
			 WGLWidget::UNSIGNED_SHORT);
    indexBufferSizes_.push_back(vertexIndices.size());

    // initialize mesh-index-buffer
    overlayLinesBuffers_.push_back(chart_->createBuffer());
    IntBuffer lineIndices = Utils::createIntBuffer(2*Nx_patch*Ny_patch);
    generateMeshIndices(lineIndices, Nx_patch, Ny_patch);

    chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, overlayLinesBuffers_[i]);
    chart_->bufferDataiv(WGLWidget::ELEMENT_ARRAY_BUFFER,
			 lineIndices,
			 WGLWidget::STATIC_DRAW,
			 WGLWidget::UNSIGNED_SHORT);
    lineBufferSizes_.push_back(lineIndices.size());
  }
}

void WAbstractGridData::initializeBarSeriesBuffers()
{
  int Nx = nbXPoints();
  int Ny = nbYPoints();

  int cnt = countSimpleData();
  std::vector<FloatBuffer> simplePtsArrays;
  std::vector<FloatBuffer> coloredPtsArrays;
  std::vector<FloatBuffer> coloredPtsColors;
  std::vector<FloatBuffer> barVertexArrays;
  std::vector<FloatBuffer> coloredBarVertexArrays;

  int nbSimpleBarBuffers = cnt/BAR_BUFFER_LIMIT + 1;
  int nbColoredBarBuffers = (Nx*Ny - cnt)/BAR_BUFFER_LIMIT + 1;

  // construct buffers
  // Structure: PtsArrays-buffer contains (x, y, z_0, z_1), ... where z_0 is the value to stack onto, and z_1 is the heigth of the bar itself
  //            barVertexArrays contain de vertex data for the actual bars
  int PT_INFO_SIZE = 4;
  int PTS_PER_BAR = 8;
  for (int i=0; i < nbSimpleBarBuffers-1; i++) {
    simplePtsArrays.push_back(Utils::createFloatBuffer(PT_INFO_SIZE*BAR_BUFFER_LIMIT));
    barVertexArrays.push_back(Utils::createFloatBuffer(PTS_PER_BAR*3*BAR_BUFFER_LIMIT));
  }
  simplePtsArrays.push_back(Utils::createFloatBuffer(PT_INFO_SIZE*(cnt - BAR_BUFFER_LIMIT*(nbSimpleBarBuffers-1))));
  barVertexArrays.push_back(Utils::createFloatBuffer(PTS_PER_BAR*3*(cnt - BAR_BUFFER_LIMIT*(nbSimpleBarBuffers-1))));
  for (int i=0; i < nbColoredBarBuffers-1; i++) {
    coloredPtsArrays.push_back(Utils::createFloatBuffer(PT_INFO_SIZE*BAR_BUFFER_LIMIT));
    coloredPtsColors.push_back(Utils::createFloatBuffer(PTS_PER_BAR*PT_INFO_SIZE*BAR_BUFFER_LIMIT));
    coloredBarVertexArrays.push_back(Utils::createFloatBuffer(PTS_PER_BAR*3*BAR_BUFFER_LIMIT));
  }
  coloredPtsArrays.push_back(Utils::createFloatBuffer(PT_INFO_SIZE*(Nx*Ny - cnt - BAR_BUFFER_LIMIT*(nbColoredBarBuffers-1))));
  coloredPtsColors.push_back(Utils::createFloatBuffer(PTS_PER_BAR*PT_INFO_SIZE*(Nx*Ny - cnt - BAR_BUFFER_LIMIT*(nbColoredBarBuffers-1))));
  coloredBarVertexArrays.push_back(Utils::createFloatBuffer(PTS_PER_BAR*3*(Nx*Ny - cnt - BAR_BUFFER_LIMIT*(nbColoredBarBuffers-1))));

  // fill buffers with data
  barDataFromModel(simplePtsArrays, coloredPtsArrays, coloredPtsColors);
  for (unsigned i = 0; i < simplePtsArrays.size(); i++) {
    barSeriesVertexData(simplePtsArrays[i], barVertexArrays[i]);
  }
  for (unsigned i = 0; i < coloredPtsArrays.size(); i++) {
    barSeriesVertexData(coloredPtsArrays[i], coloredBarVertexArrays[i]);
  }

  for (unsigned i = 0; i < barVertexArrays.size(); i++) {
    loadBinaryResource(barVertexArrays[i], vertexPosBuffers_);
    vertexPosBufferSizes_.push_back(barVertexArrays[i].size());
  }
  for (unsigned i = 0; i < coloredBarVertexArrays.size(); i++) {
    loadBinaryResource(coloredBarVertexArrays[i], vertexPosBuffers2_);
    vertexPosBuffer2Sizes_.push_back(coloredBarVertexArrays[i].size());
    loadBinaryResource(coloredPtsColors[i], vertexColorBuffers2_);
  }

  for (unsigned i = 0; i < simplePtsArrays.size(); i++) {
    // initialize vertex-index-buffer
    indexBuffers_.push_back(chart_->createBuffer());
    IntBuffer vertexIndices =
      Utils::createIntBuffer(12*3*(simplePtsArrays[i].size()/PT_INFO_SIZE));
    generateVertexIndices(vertexIndices, 0, 0, simplePtsArrays[i].size()/PT_INFO_SIZE);

    chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, indexBuffers_[i]);
    chart_->bufferDataiv(WGLWidget::ELEMENT_ARRAY_BUFFER,
			 vertexIndices,
			 WGLWidget::STATIC_DRAW,
			 WGLWidget::UNSIGNED_SHORT);
    indexBufferSizes_.push_back(vertexIndices.size());

    // initialize mesh-index-buffer
    overlayLinesBuffers_.push_back(chart_->createBuffer());

    IntBuffer lineIndices = Utils::createIntBuffer(24*(simplePtsArrays[i].size()/PT_INFO_SIZE));
    generateMeshIndices(lineIndices, 0, 0, simplePtsArrays[i].size()/PT_INFO_SIZE);
    chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, overlayLinesBuffers_[i]);
    chart_->bufferDataiv(WGLWidget::ELEMENT_ARRAY_BUFFER,
			 lineIndices,
			 WGLWidget::STATIC_DRAW,
			 WGLWidget::UNSIGNED_SHORT);
    lineBufferSizes_.push_back(lineIndices.size());

    // initialize texture-coordinate buffer
    FloatBuffer texCoordArray = Utils::createFloatBuffer(PTS_PER_BAR*2*(simplePtsArrays[i].size()/PT_INFO_SIZE));
    generateTextureCoords(texCoordArray, simplePtsArrays[i], simplePtsArrays[i].size()/PT_INFO_SIZE);
    colormapTexBuffers_.push_back(chart_->createBuffer());
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, colormapTexBuffers_[i]);
    chart_->bufferDatafv(WGLWidget::ARRAY_BUFFER,
			 texCoordArray,
			 WGLWidget::STATIC_DRAW);
  }
  for (unsigned i = 0; i < coloredPtsArrays.size(); i++) {
    // initialize vertex-index-buffer
    indexBuffers2_.push_back(chart_->createBuffer());
    IntBuffer vertexIndices =
      Utils::createIntBuffer(12*3*(coloredPtsArrays[i].size()/PT_INFO_SIZE));
    generateVertexIndices(vertexIndices, 0, 0, coloredPtsArrays[i].size()/PT_INFO_SIZE);

    chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, indexBuffers2_[i]);
    chart_->bufferDataiv(WGLWidget::ELEMENT_ARRAY_BUFFER,
			 vertexIndices,
			 WGLWidget::STATIC_DRAW,
			 WGLWidget::UNSIGNED_SHORT);
    indexBufferSizes2_.push_back(vertexIndices.size());

    // initialize mesh-index-buffer
    overlayLinesBuffers2_.push_back(chart_->createBuffer());
    IntBuffer lineIndices =
      Utils::createIntBuffer(24*(coloredPtsArrays[i].size()/PT_INFO_SIZE));
    generateMeshIndices(lineIndices, 0, 0,
			coloredPtsArrays[i].size()/PT_INFO_SIZE);
    chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER,
		       overlayLinesBuffers2_[i]);
    chart_->bufferDataiv(WGLWidget::ELEMENT_ARRAY_BUFFER,
			 lineIndices,
			 WGLWidget::STATIC_DRAW,
			 WGLWidget::UNSIGNED_SHORT);
    lineBufferSizes2_.push_back(lineIndices.size());
  }
}

void WAbstractGridData::barSeriesVertexData(FloatBuffer& verticesIN,
				    FloatBuffer& verticesOUT) const
{
  float x, y, z0, z;
  for (unsigned i = 0; i < verticesIN.size()/4; i++) {
    int index = i*4;

    x = verticesIN[index];
    y = verticesIN[index + 1];
    z0 = verticesIN[index + 2];
    z = verticesIN[index + 2] + verticesIN[index + 3];

    float barWidthX_ = (float)(WAbstractGridData::barWidthX_/nbXPoints());
    float barWidthY_ = (float)(WAbstractGridData::barWidthY_/nbYPoints());
    // points of the front plane
    push(verticesOUT, x - barWidthX_/2, y + barWidthY_/2, z0);
    push(verticesOUT, x - barWidthX_/2, y + barWidthY_/2, z);
    push(verticesOUT, x + barWidthX_/2, y + barWidthY_/2, z);
    push(verticesOUT, x + barWidthX_/2, y + barWidthY_/2, z0);
    // points of the back plane
    push(verticesOUT, x - barWidthX_/2, y - barWidthY_/2, z0);
    push(verticesOUT, x - barWidthX_/2, y - barWidthY_/2, z);
    push(verticesOUT, x + barWidthX_/2, y - barWidthY_/2, z);
    push(verticesOUT, x + barWidthX_/2, y - barWidthY_/2, z0);
  }

}

void WAbstractGridData::generateVertexIndices(IntBuffer& indicesOUT,
				      int Nx, int Ny, int size) const
{
  bool forward = true;

  switch (seriesType_) {
  case Series3DType::Point:
    break;
  case Series3DType::Surface:
    for (int i=0; i < Nx-1; i++) {
      if (forward) { // make pass in forward direction
	for (int j=0; j < Ny; j++) {
	  indicesOUT.push_back( i     * Ny + j );
	  indicesOUT.push_back( (i+1) * Ny + j );
	}
	indicesOUT.push_back( (i+1) * Ny + (Ny-1) );
	forward = false;
      } else { // make pass in backward direction
	for (int j=Ny-1; j >= 0; j--) {
	  indicesOUT.push_back( i     * Ny + j );
	  indicesOUT.push_back( (i+1) * Ny + j );
	}
	indicesOUT.push_back( (i+1) * Ny );
	forward = true;
      }
    }
    break;
  case Series3DType::Bar:
    for (int i=0; i < size; i++) {
      int index = i*8;
      //front
      push(indicesOUT, index + 0, index + 1, index + 2);
      push(indicesOUT, index + 0, index + 2, index + 3);
      //left
      push(indicesOUT, index + 4, index + 5, index + 1);
      push(indicesOUT, index + 4, index + 1, index + 0);
      //right
      push(indicesOUT, index + 3, index + 2, index + 6);
      push(indicesOUT, index + 3, index + 6, index + 7);
      //back
      push(indicesOUT, index + 7, index + 6, index + 5);
      push(indicesOUT, index + 7, index + 5, index + 4);
      //bottom
      push(indicesOUT, index + 4, index + 0, index + 3);
      push(indicesOUT, index + 4, index + 3, index + 7);
      //top
      push(indicesOUT, index + 1, index + 5, index + 6);
      push(indicesOUT, index + 1, index + 6, index + 2);
    }
    break;
  };
}

void WAbstractGridData::generateMeshIndices(IntBuffer& indicesOUT,
				    int Nx, int Ny, int size)
{
  bool forward = true;

  switch (seriesType_) {
  case Series3DType::Point:
    break;
  case Series3DType::Surface:
    for (int i=0; i < Nx; i++) { // make a pass in y-axis direction
      if (forward) { // make pass in forward direction
	for (int j=0; j < Ny; j++) {
	  indicesOUT.push_back(i * Ny + j);
	}
	forward = false;
      } else { // make pass in backward direction
	for (int j=Ny-1; j >= 0; j--) {
	  indicesOUT.push_back(i * Ny + j);
	}
	forward = true;
      }
    }
    if (forward == true) {
      forward = false;
      for (int i=0; i < Ny; i++) { // make a pass in x-axis direction
	if (forward) { // make pass in forward direction
	  for (int j=0; j < Nx; j++) {
	    indicesOUT.push_back(j*Ny + i);
	  }
	  forward = false;
	} else { // make pass in backward direction
	  for (int j=Nx-1; j >= 0; j--) {
	    indicesOUT.push_back(j*Ny + i);
	  }
	  forward = true;
	}
      }
    } else {
      forward = false;
      for (int i=Ny-1; i >= 0; i--) { // make a pass in x-axis direction
	if (forward) { // make pass in forward direction
	  for (int j=0; j < Nx; j++) {
	    indicesOUT.push_back(j*Ny + i);
	  }
	  forward = false;
	} else { // make pass in backward direction
	  for (int j=Nx-1; j >= 0; j--) {
	    indicesOUT.push_back(j*Ny + i);
	  }
	  forward = true;
	}
      }
    }
    break;
  case Series3DType::Bar:
    for (int i=0; i < size; i++) {
      int index = i*8;
      indicesOUT.push_back(index+0);indicesOUT.push_back(index+1);
      indicesOUT.push_back(index+1);indicesOUT.push_back(index+2);
      indicesOUT.push_back(index+2);indicesOUT.push_back(index+3);
      indicesOUT.push_back(index+3);indicesOUT.push_back(index+0);

      indicesOUT.push_back(index+4);indicesOUT.push_back(index+5);
      indicesOUT.push_back(index+5);indicesOUT.push_back(index+6);
      indicesOUT.push_back(index+6);indicesOUT.push_back(index+7);
      indicesOUT.push_back(index+7);indicesOUT.push_back(index+4);

      indicesOUT.push_back(index+0);indicesOUT.push_back(index+4);
      indicesOUT.push_back(index+1);indicesOUT.push_back(index+5);
      indicesOUT.push_back(index+2);indicesOUT.push_back(index+6);
      indicesOUT.push_back(index+3);indicesOUT.push_back(index+7);
    }
#if 0
    for (int i=0; i<Nx; i++) {
      for (int j=0; j<Ny; j++) {
	int index = (i*Ny + j)*8;
	indicesOUT.push_back(index+0);indicesOUT.push_back(index+1);
	indicesOUT.push_back(index+1);indicesOUT.push_back(index+2);
	indicesOUT.push_back(index+2);indicesOUT.push_back(index+3);
	indicesOUT.push_back(index+3);indicesOUT.push_back(index+0);

	indicesOUT.push_back(index+4);indicesOUT.push_back(index+5);
	indicesOUT.push_back(index+5);indicesOUT.push_back(index+6);
	indicesOUT.push_back(index+6);indicesOUT.push_back(index+7);
	indicesOUT.push_back(index+7);indicesOUT.push_back(index+4);

	indicesOUT.push_back(index+0);indicesOUT.push_back(index+4);
	indicesOUT.push_back(index+1);indicesOUT.push_back(index+5);
	indicesOUT.push_back(index+2);indicesOUT.push_back(index+6);
	indicesOUT.push_back(index+3);indicesOUT.push_back(index+7);
      }
    }
#endif
    break;
  };
}

void WAbstractGridData::generateTextureCoords(FloatBuffer& coordsOUT,
					      const FloatBuffer& dataArray,
					      int size)
{
  switch (seriesType_) {
  case Series3DType::Point:
  case Series3DType::Surface:
    break;
  case Series3DType::Bar:
    if (colormap_ == nullptr) {
      for (int i=0; i < size; i++) {
	for (int k=0; k<16; k++) {
	  coordsOUT.push_back(0.0f);
	}
      }
    } else {
      float min = (float)chart_->toPlotCubeCoords(colormap_->minimum(), 
						  Axis::Z3D);
      float max = (float)chart_->toPlotCubeCoords(colormap_->maximum(),
						  Axis::Z3D);
      for (int i=0; i < size; i++) {
	float zNorm = (dataArray[i*4 + 3] - min)/(max - min);
	for (int k=0; k<8; k++) {
	  coordsOUT.push_back(0.0f);
	  coordsOUT.push_back(zNorm);
	}
      }
    }
    break;
  };
}

void WAbstractGridData::setClippingMin(Axis axis, float v)
{
  minPtChanged_ = true;
  if (jsMinPt_.initialized())
    minPt_ = jsMinPt_.value();
  minPt_[axisToIndex(axis)] = v;
  if (chart_)
    chart_->updateChart(WFlags<ChartUpdates>(ChartUpdates::GLContext) | ChartUpdates::GLTextures);
}

float WAbstractGridData::clippingMin(Axis axis) const
{
  std::size_t idx = axisToIndex(axis);
  return jsMinPt_.initialized() ? jsMinPt_.value()[idx] : minPt_[idx];
}

JSlot &WAbstractGridData::changeClippingMin(Axis axis)
{
  if (axis == Axis::X3D) {
    return changeClippingMinX_;
  } else if (axis == Axis::Y3D) {
    return changeClippingMinY_;
  } else if (axis == Axis::Z3D) {
    return changeClippingMinZ_;
  } else {
    throw WException("Invalid axis for 3D chart");
  }
}

void WAbstractGridData::setClippingMax(Axis axis, float v)
{
  maxPtChanged_ = true;
  if (jsMaxPt_.initialized())
    maxPt_ = jsMaxPt_.value();
  maxPt_[axisToIndex(axis)] = v;
  if (chart_)
    chart_->updateChart(WFlags<ChartUpdates>(ChartUpdates::GLContext) | ChartUpdates::GLTextures);
}

float WAbstractGridData::clippingMax(Axis axis) const
{
  std::size_t idx = axisToIndex(axis);
  return jsMaxPt_.initialized() ? jsMaxPt_.value()[idx] : maxPt_[idx];
}

JSlot &WAbstractGridData::changeClippingMax(Axis axis)
{
  if (axis == Axis::X3D) {
    return changeClippingMaxX_;
  } else if (axis == Axis::Y3D) {
    return changeClippingMaxY_;
  } else if (axis == Axis::Z3D) {
    return changeClippingMaxZ_;
  } else {
    throw WException("Invalid axis for 3D chart");
  }
}

void WAbstractGridData::initShaders()
{
  switch (seriesType_) {
  case Series3DType::Bar:
    // shaders for the bar series data
    fragShader_ = chart_->createShader(WGLWidget::FRAGMENT_SHADER);
    chart_->shaderSource(fragShader_, barFragShaderSrc);
    chart_->compileShader(fragShader_);
    vertShader_ =
      chart_->createShader(WGLWidget::VERTEX_SHADER);
    chart_->shaderSource(vertShader_, barVertexShaderSrc);
    chart_->compileShader(vertShader_);

    seriesProgram_ = chart_->createProgram();
    chart_->attachShader(seriesProgram_, vertShader_);
    chart_->attachShader(seriesProgram_, fragShader_);
    chart_->linkProgram(seriesProgram_);

    vertexPosAttr_ = chart_->getAttribLocation(seriesProgram_,
						  "aVertexPosition");
    barTexCoordAttr_ = chart_->getAttribLocation(seriesProgram_,
  						 "aTextureCoord");
    mvMatrixUniform_ = chart_->getUniformLocation(seriesProgram_, "uMVMatrix");
    pMatrix_ = chart_->getUniformLocation(seriesProgram_, "uPMatrix");
    cMatrix_ = chart_->getUniformLocation(seriesProgram_, "uCMatrix");
    TexSampler_ = chart_->getUniformLocation(seriesProgram_, "uSampler");

    // shaders for colored bar series data
    colFragShader_ = chart_->createShader(WGLWidget::FRAGMENT_SHADER);
    chart_->shaderSource(colFragShader_, colBarFragShaderSrc);
    chart_->compileShader(colFragShader_);
    colVertShader_ = chart_->createShader(WGLWidget::VERTEX_SHADER);
    chart_->shaderSource(colVertShader_, colBarVertexShaderSrc);
    chart_->compileShader(colVertShader_);

    colSeriesProgram_ = chart_->createProgram();
    chart_->attachShader(colSeriesProgram_, colFragShader_);
    chart_->attachShader(colSeriesProgram_, colVertShader_);
    chart_->linkProgram(colSeriesProgram_);

    vertexPosAttr2_ = chart_->getAttribLocation(colSeriesProgram_,
					       "aVertexPosition");
    vertexColAttr2_ = chart_->getAttribLocation(colSeriesProgram_, "aVertexColor");
    mvMatrixUniform2_ = chart_->getUniformLocation(colSeriesProgram_,
						  "uMVMatrix");
    pMatrix2_ = chart_->getUniformLocation(colSeriesProgram_, "uPMatrix");
    cMatrix2_ = chart_->getUniformLocation(colSeriesProgram_, "uCMatrix");
    break;
  case Series3DType::Point:
    // shaders for the point series data
    fragShader_ =
      chart_->createShader(WGLWidget::FRAGMENT_SHADER);
    chart_->shaderSource(fragShader_, ptFragShaderSrc);
    chart_->compileShader(fragShader_);
    vertShader_ =
      chart_->createShader(WGLWidget::VERTEX_SHADER);
    chart_->shaderSource(vertShader_, ptVertexShaderSrc);
    chart_->compileShader(vertShader_);

    seriesProgram_ = chart_->createProgram();
    chart_->attachShader(seriesProgram_, vertShader_);
    chart_->attachShader(seriesProgram_, fragShader_);
    chart_->linkProgram(seriesProgram_);

    vertexPosAttr_ = chart_->getAttribLocation(seriesProgram_,
					       "aVertexPosition");
      vertexSizeAttr_ = chart_->getAttribLocation(seriesProgram_, "aPointSize");
    mvMatrixUniform_ = chart_->getUniformLocation(seriesProgram_, "uMVMatrix");
    pMatrix_ = chart_->getUniformLocation(seriesProgram_, "uPMatrix");
    cMatrix_ = chart_->getUniformLocation(seriesProgram_, "uCMatrix");
    TexSampler_ = chart_->getUniformLocation(seriesProgram_, "uSampler");
    pointSpriteUniform_ = chart_->getUniformLocation(seriesProgram_, "uPointSprite");
    vpHeightUniform_ = chart_->getUniformLocation(seriesProgram_, "uVPHeight");
    offset_ = chart_->getUniformLocation(seriesProgram_, "uOffset");
    scaleFactor_ = chart_->getUniformLocation(seriesProgram_,
						 "uScaleFactor");
    // shader for points with color
    colFragShader_ =
      chart_->createShader(WGLWidget::FRAGMENT_SHADER);
    chart_->shaderSource(colFragShader_, colPtFragShaderSrc);
    chart_->compileShader(colFragShader_);
    colVertShader_ =
      chart_->createShader(WGLWidget::VERTEX_SHADER);
    chart_->shaderSource(colVertShader_, colPtVertexShaderSrc);
    chart_->compileShader(colVertShader_);

    colSeriesProgram_ = chart_->createProgram();
    chart_->attachShader(colSeriesProgram_, colVertShader_);
    chart_->attachShader(colSeriesProgram_, colFragShader_);
    chart_->linkProgram(colSeriesProgram_);

    vertexPosAttr2_ = chart_->getAttribLocation(colSeriesProgram_,
					       "aVertexPosition");
    vertexSizeAttr2_ = chart_->getAttribLocation(colSeriesProgram_,
					       "aPointSize");
    vertexColAttr2_ = chart_->getAttribLocation(colSeriesProgram_, "aColor");
    mvMatrixUniform2_ = chart_->getUniformLocation(colSeriesProgram_,
						  "uMVMatrix");
    pMatrix2_ = chart_->getUniformLocation(colSeriesProgram_, "uPMatrix");
    cMatrix2_ = chart_->getUniformLocation(colSeriesProgram_, "uCMatrix");
    pointSpriteUniform2_ = chart_->getUniformLocation(colSeriesProgram_, "uPointSprite");
    vpHeightUniform2_ = chart_->getUniformLocation(colSeriesProgram_, "uVPHeight");
    break;
  case Series3DType::Surface:
    // shaders for the surface series data
    fragShader_ =
      chart_->createShader(WGLWidget::FRAGMENT_SHADER);
    chart_->shaderSource(fragShader_, surfFragShaderSrc);
    chart_->compileShader(fragShader_);
    vertShader_ =
      chart_->createShader(WGLWidget::VERTEX_SHADER);
    chart_->shaderSource(vertShader_, surfVertexShaderSrc);
    chart_->compileShader(vertShader_);

    seriesProgram_ = chart_->createProgram();
    chart_->attachShader(seriesProgram_, vertShader_);
    chart_->attachShader(seriesProgram_, fragShader_);
    chart_->linkProgram(seriesProgram_);

    vertexPosAttr_ = chart_->getAttribLocation(seriesProgram_,
						   "aVertexPosition");
    mvMatrixUniform_ = chart_->getUniformLocation(seriesProgram_, "uMVMatrix");
    pMatrix_ = chart_->getUniformLocation(seriesProgram_, "uPMatrix");
    cMatrix_ = chart_->getUniformLocation(seriesProgram_, "uCMatrix");
    TexSampler_ = chart_->getUniformLocation(seriesProgram_, "uSampler");
    offset_ = chart_->getUniformLocation(seriesProgram_, "uOffset");
    scaleFactor_ = chart_->getUniformLocation(seriesProgram_,
						   "uScaleFactor");
    minPtUniform_ = chart_->getUniformLocation(seriesProgram_, "uMinPt");
    maxPtUniform_ = chart_->getUniformLocation(seriesProgram_, "uMaxPt");
    dataMinPtUniform_ = chart_->getUniformLocation(seriesProgram_, "uDataMinPt");
    dataMaxPtUniform_ = chart_->getUniformLocation(seriesProgram_, "uDataMaxPt");

    if (chart_->isIntersectionLinesEnabled() || clippingLinesEnabled() ||
	!chart_->intersectionPlanes_.empty()) {
      singleColorFragShader_ =
	chart_->createShader(WGLWidget::FRAGMENT_SHADER);
      chart_->shaderSource(singleColorFragShader_, surfFragSingleColorShaderSrc);
      chart_->compileShader(singleColorFragShader_);

      singleColorProgram_ = chart_->createProgram();
      chart_->attachShader(singleColorProgram_, vertShader_);
      chart_->attachShader(singleColorProgram_, singleColorFragShader_);
      chart_->linkProgram(singleColorProgram_);

      chart_->useProgram(singleColorProgram_);
      singleColor_vertexPosAttr_ = chart_->getAttribLocation(singleColorProgram_, "aVertexPosition");
      singleColor_mvMatrixUniform_ = chart_->getUniformLocation(singleColorProgram_, "uMVMatrix");
      singleColor_pMatrix_ = chart_->getUniformLocation(singleColorProgram_, "uPMatrix");
      singleColor_cMatrix_ = chart_->getUniformLocation(singleColorProgram_, "uCMatrix");
      singleColorUniform_ = chart_->getUniformLocation(singleColorProgram_, "uColor");
      singleColor_minPtUniform_ = chart_->getUniformLocation(singleColorProgram_, "uMinPt");
      singleColor_maxPtUniform_ = chart_->getUniformLocation(singleColorProgram_, "uMaxPt");
      singleColor_dataMinPtUniform_ = chart_->getUniformLocation(singleColorProgram_, "uDataMinPt");
      singleColor_dataMaxPtUniform_ = chart_->getUniformLocation(singleColorProgram_, "uDataMaxPt");
      singleColor_marginUniform_ = chart_->getUniformLocation(singleColorProgram_, "uMargin");

      positionFragShader_ =
	chart_->createShader(WGLWidget::FRAGMENT_SHADER);
      chart_->shaderSource(positionFragShader_, surfFragPosShaderSrc);
      chart_->compileShader(positionFragShader_);

      positionProgram_ = chart_->createProgram();
      chart_->attachShader(positionProgram_, vertShader_);
      chart_->attachShader(positionProgram_, positionFragShader_);
      chart_->linkProgram(positionProgram_);

      chart_->useProgram(positionProgram_);
      position_vertexPosAttr_ = chart_->getAttribLocation(positionProgram_, "aVertexPosition");
      position_mvMatrixUniform_ = chart_->getUniformLocation(positionProgram_, "uMVMatrix");
      position_pMatrix_ = chart_->getUniformLocation(positionProgram_, "uPMatrix");
      position_cMatrix_ = chart_->getUniformLocation(positionProgram_, "uCMatrix");
      position_marginUniform_ = chart_->getUniformLocation(positionProgram_, "uMargin");
      position_minPtUniform_ = chart_->getUniformLocation(positionProgram_, "uMinPt");
      position_maxPtUniform_ = chart_->getUniformLocation(positionProgram_, "uMaxPt");
      position_dataMinPtUniform_ = chart_->getUniformLocation(positionProgram_, "uDataMinPt");
      position_dataMaxPtUniform_ = chart_->getUniformLocation(positionProgram_, "uDataMaxPt");
    }
    break;
  };

  if (seriesType_ == Series3DType::Bar ||
      (seriesType_ == Series3DType::Surface && surfaceMeshEnabled_)) {
    // shaders for the overlay lines
    meshFragShader_ =
      chart_->createShader(WGLWidget::FRAGMENT_SHADER);
    chart_->shaderSource(meshFragShader_, meshFragShaderSrc);
    chart_->compileShader(meshFragShader_);
    meshVertShader_ =
      chart_->createShader(WGLWidget::VERTEX_SHADER);
    chart_->shaderSource(meshVertShader_, meshVertexShaderSrc);
    chart_->compileShader(meshVertShader_);

    meshProgram_ = chart_->createProgram();
    chart_->attachShader(meshProgram_, meshVertShader_);
    chart_->attachShader(meshProgram_, meshFragShader_);
    chart_->linkProgram(meshProgram_);

    meshVertexPosAttr_ = chart_->getAttribLocation(meshProgram_,
						   "aVertexPosition");
    mesh_mvMatrixUniform_ = chart_->getUniformLocation(meshProgram_, "uMVMatrix");
    mesh_pMatrix_ = chart_->getUniformLocation(meshProgram_, "uPMatrix");
    mesh_cMatrix_ = chart_->getUniformLocation(meshProgram_, "uCMatrix");
    mesh_colorUniform_ = chart_->getUniformLocation(meshProgram_, "uColor");
    mesh_minPtUniform_ = chart_->getUniformLocation(meshProgram_, "uMinPt");
    mesh_maxPtUniform_ = chart_->getUniformLocation(meshProgram_, "uMaxPt");
    mesh_dataMinPtUniform_ = chart_->getUniformLocation(meshProgram_, "uDataMinPt");
    mesh_dataMaxPtUniform_ = chart_->getUniformLocation(meshProgram_, "uDataMaxPt");
  }

  if (seriesType_ == Series3DType::Surface && isoLineHeights_.size() > 0) {
    isoLineFragShader_ =
      chart_->createShader(WGLWidget::FRAGMENT_SHADER);
    chart_->shaderSource(isoLineFragShader_, isoLineFragShaderSrc);
    chart_->compileShader(isoLineFragShader_);

    isoLineVertexShader_ =
      chart_->createShader(WGLWidget::VERTEX_SHADER);
    chart_->shaderSource(isoLineVertexShader_, isoLineVertexShaderSrc);
    chart_->compileShader(isoLineVertexShader_);

    isoLineProgram_ = chart_->createProgram();
    chart_->attachShader(isoLineProgram_, isoLineVertexShader_);
    chart_->attachShader(isoLineProgram_, isoLineFragShader_);
    chart_->linkProgram(isoLineProgram_);

    isoLineVertexPosAttr_ = chart_->getAttribLocation(isoLineProgram_, "aVertexPosition");
    isoLine_mvMatrixUniform_ = chart_->getUniformLocation(isoLineProgram_, "uMVMatrix");
    isoLine_pMatrix_ = chart_->getUniformLocation(isoLineProgram_, "uPMatrix");
    isoLine_cMatrix_ = chart_->getUniformLocation(isoLineProgram_, "uCMatrix");
    isoLine_TexSampler_ = chart_->getUniformLocation(isoLineProgram_, "uSampler");
    isoLine_offset_ = chart_->getUniformLocation(isoLineProgram_, "uOffset");
    isoLine_scaleFactor_ = chart_->getUniformLocation(isoLineProgram_, "uScaleFactor");
  }
}

void WAbstractGridData::paintGL() const
{
  if (hidden_)
    return;

  loadPointSpriteTexture(pointSpriteTexture_);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_MAG_FILTER, WGLWidget::NEAREST);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_MIN_FILTER, WGLWidget::NEAREST);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_WRAP_S,WGLWidget::CLAMP_TO_EDGE);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_WRAP_T,WGLWidget::CLAMP_TO_EDGE);

  // compatible chart-type check
  switch (seriesType_) {
  case Series3DType::Point:
    if (chart_->type() != ChartType::Scatter)
      return;
    break;
  case Series3DType::Surface:
    if (chart_->type() != ChartType::Scatter)
      return;
    break;
  case Series3DType::Bar:
    if (chart_->type() != ChartType::Category)
      return;
    break;
  }

  chart_->disable(WGLWidget::CULL_FACE);
  chart_->enable(WGLWidget::DEPTH_TEST);

  for (unsigned i = 0; i < vertexPosBuffers_.size(); i++) {

  chart_->useProgram(seriesProgram_);
  chart_->uniformMatrix4(cMatrix_, chart_->jsMatrix());
  chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexPosBuffers_[i]);
  chart_->vertexAttribPointer(vertexPosAttr_,
			      3,
			      WGLWidget::FLOAT,
			      false,
			      0,
			      0);
  chart_->enableVertexAttribArray(vertexPosAttr_);

  if ( seriesType_ == Series3DType::Bar ) {
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, colormapTexBuffers_[i]);
    chart_->vertexAttribPointer(barTexCoordAttr_,
				2,
				WGLWidget::FLOAT,
				false,
				0,
				0);
    chart_->enableVertexAttribArray(barTexCoordAttr_);
  }

  double xMin = chart_->axis(Axis::X3D).minimum();
  double xMax = chart_->axis(Axis::X3D).maximum();
  double yMin = chart_->axis(Axis::Y3D).minimum();
  double yMax = chart_->axis(Axis::Y3D).maximum();
  double zMin = chart_->axis(Axis::Z3D).minimum();
  double zMax = chart_->axis(Axis::Z3D).maximum();

  chart_->activeTexture(WGLWidget::TEXTURE0);
  chart_->bindTexture(WGLWidget::TEXTURE_2D, colormapTexture_);
  chart_->uniform1i(TexSampler_,0);
  if (!minPtUniform_.isNull()) {
    chart_->uniform3fv(minPtUniform_, jsMinPt_);
    chart_->uniform3fv(maxPtUniform_, jsMaxPt_);
    chart_->uniform3f(dataMinPtUniform_, xMin, yMin, zMin);
    chart_->uniform3f(dataMaxPtUniform_, xMax, yMax, zMax);
  }

  if (seriesType_ == Series3DType::Bar) {
    chart_->enable(WGLWidget::POLYGON_OFFSET_FILL);
    float unitOffset = (float) (std::pow(5,meshPen_.width().value()) < 10000 ? std::pow(5,meshPen_.width().value()) : 10000);
    chart_->polygonOffset(1, unitOffset);
    chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, indexBuffers_[i]);
    chart_->drawElements(WGLWidget::TRIANGLES, indexBufferSizes_[i],
			 WGLWidget::UNSIGNED_SHORT, 0);
    chart_->disableVertexAttribArray(vertexPosAttr_);
    chart_->disableVertexAttribArray(barTexCoordAttr_);
  } else if (seriesType_ == Series3DType::Surface) {
    chart_->enable(WGLWidget::POLYGON_OFFSET_FILL);
    float unitOffset = (float) (std::pow(5,meshPen_.width().value()) < 10000 ? std::pow(5,meshPen_.width().value()) : 10000);
    chart_->polygonOffset(1, unitOffset);
    chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, indexBuffers_[i]);
    chart_->drawElements(WGLWidget::TRIANGLE_STRIP, indexBufferSizes_[i],
			 WGLWidget::UNSIGNED_SHORT, 0);
    chart_->disable(WGLWidget::POLYGON_OFFSET_FILL);
    chart_->disableVertexAttribArray(vertexPosAttr_);
  } else {
    // draw simple points
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexSizeBuffers_[i]);
    chart_->vertexAttribPointer(vertexSizeAttr_,
				1,
				WGLWidget::FLOAT,
				false,
				0,
				0);
    chart_->enableVertexAttribArray(vertexSizeAttr_);
    chart_->activeTexture(WGLWidget::TEXTURE1);
    chart_->bindTexture(WGLWidget::TEXTURE_2D, pointSpriteTexture_);
    chart_->uniform1i(pointSpriteUniform_,1);

    chart_->uniform1f(vpHeightUniform_,chart_->height().value());

    chart_->drawArrays(WGLWidget::POINTS,
		       0,
		       vertexPosBufferSizes_[i]/3);
    chart_->disableVertexAttribArray(vertexPosAttr_);
    chart_->disableVertexAttribArray(vertexSizeAttr_);
  }

  // draw mesh
  if ( seriesType_ == Series3DType::Bar ||
       (seriesType_ == Series3DType::Surface && surfaceMeshEnabled_) ) {
    chart_->useProgram(meshProgram_);
    chart_->depthFunc(WGLWidget::LEQUAL);
    chart_->uniformMatrix4(mesh_cMatrix_, chart_->jsMatrix());
    chart_->uniform3fv(mesh_minPtUniform_, jsMinPt_);
    chart_->uniform3fv(mesh_maxPtUniform_, jsMaxPt_);
    chart_->uniform3f(mesh_dataMinPtUniform_, xMin, yMin, zMin);
    chart_->uniform3f(mesh_dataMaxPtUniform_, xMax, yMax, zMax);
    chart_->uniform4f(mesh_colorUniform_,
		      (float)meshPen_.color().red(),
		      (float)meshPen_.color().green(),
		      (float)meshPen_.color().blue(),
		      (float)meshPen_.color().alpha());
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexPosBuffers_[i]);
    chart_->vertexAttribPointer(meshVertexPosAttr_,
				3,
				WGLWidget::FLOAT,
				false,
				0,
				0);
    chart_->enableVertexAttribArray(meshVertexPosAttr_);
    chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, overlayLinesBuffers_[i]);
    if (seriesType_ == Series3DType::Surface) {
      chart_->lineWidth(meshPen_.width().value() == 0 ?
			1.0 :
			meshPen_.width().value());
      chart_->drawElements(WGLWidget::LINE_STRIP, lineBufferSizes_[i],
			   WGLWidget::UNSIGNED_SHORT, 0);
    } else if (seriesType_ == Series3DType::Bar) {
      chart_->lineWidth(meshPen_.width().value() == 0 ?
			1.0 :
			meshPen_.width().value());
      chart_->drawElements(WGLWidget::LINES, lineBufferSizes_[i],
			   WGLWidget::UNSIGNED_SHORT, 0);
    }
    chart_->disableVertexAttribArray(meshVertexPosAttr_);
  }
  }
  for (unsigned i = 0; i < vertexPosBuffers2_.size(); i++) {
    // draw colored points
    chart_->useProgram(colSeriesProgram_);
    chart_->uniformMatrix4(cMatrix2_, chart_->jsMatrix());
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexPosBuffers2_[i]);
    chart_->vertexAttribPointer(vertexPosAttr2_,
				3,
				WGLWidget::FLOAT,
				false,
				0,
				0);
    chart_->enableVertexAttribArray(vertexPosAttr2_);
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexColorBuffers2_[i]);
    chart_->vertexAttribPointer(vertexColAttr2_,
				4,
				WGLWidget::FLOAT,
				false,
				0,
				0);
    chart_->enableVertexAttribArray(vertexColAttr2_);
    if (seriesType_ == Series3DType::Point) {
      chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexSizeBuffers2_[i]);
      chart_->vertexAttribPointer(vertexSizeAttr2_,
				  1,
				  WGLWidget::FLOAT,
				  false,
				  0,
				  0);
      chart_->enableVertexAttribArray(vertexSizeAttr2_);
      chart_->activeTexture(WGLWidget::TEXTURE0);
      chart_->bindTexture(WGLWidget::TEXTURE_2D, pointSpriteTexture_);
      chart_->uniform1i(pointSpriteUniform2_,0);

      chart_->uniform1f(vpHeightUniform2_,chart_->height().value());

      chart_->drawArrays(WGLWidget::POINTS,
			 0,
			 vertexPosBuffer2Sizes_[i]/3);
      chart_->disableVertexAttribArray(vertexSizeAttr2_);
    } else if (seriesType_ == Series3DType::Bar) {
      chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, indexBuffers2_[i]);
      chart_->drawElements(WGLWidget::TRIANGLES, indexBufferSizes2_[i],
			   WGLWidget::UNSIGNED_SHORT, 0);
    }
    chart_->disableVertexAttribArray(vertexPosAttr2_);
    chart_->disableVertexAttribArray(vertexColAttr2_);

    if (seriesType_ == Series3DType::Bar) {
      chart_->useProgram(meshProgram_);
      chart_->depthFunc(WGLWidget::LEQUAL);
      chart_->enable(WGLWidget::POLYGON_OFFSET_FILL);
      chart_->polygonOffset(1, 0.001);
      chart_->uniformMatrix4(mesh_cMatrix_, chart_->jsMatrix());
      chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexPosBuffers2_[i]);
      chart_->vertexAttribPointer(meshVertexPosAttr_,
				  3,
				  WGLWidget::FLOAT,
				  false,
				  0,
				  0);
      chart_->enableVertexAttribArray(meshVertexPosAttr_);
      chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, overlayLinesBuffers2_[i]);
      chart_->drawElements(WGLWidget::LINES, lineBufferSizes2_[i],
			   WGLWidget::UNSIGNED_SHORT, 0);
      chart_->disableVertexAttribArray(meshVertexPosAttr_);
    }
  }

  if (seriesType_ == Series3DType::Surface && isoLineHeights_.size() > 0) {
    drawIsoLines();
  }

  chart_->enable(WGLWidget::CULL_FACE);
  chart_->disable(WGLWidget::DEPTH_TEST);
}

void WAbstractGridData::paintGLIndex(unsigned index) const
{
  paintGLIndex(index, 0.0, 0.0, 0.0);
}

void WAbstractGridData::paintGLIndex(unsigned index, double marginX, double marginY, double marginZ) const
{
  if (hidden_)
    return;

  if (seriesType_ != Series3DType::Surface ||
      chart_->type() != ChartType::Scatter)
    return;

  double xMin = chart_->axis(Axis::X3D).minimum();
  double xMax = chart_->axis(Axis::X3D).maximum();
  double yMin = chart_->axis(Axis::Y3D).minimum();
  double yMax = chart_->axis(Axis::Y3D).maximum();
  double zMin = chart_->axis(Axis::Z3D).minimum();
  double zMax = chart_->axis(Axis::Z3D).maximum();

  chart_->disable(WGLWidget::CULL_FACE);
  chart_->enable(WGLWidget::DEPTH_TEST);

  for (unsigned i = 0; i < vertexPosBuffers_.size(); i++) {
    chart_->useProgram(singleColorProgram_);
    chart_->uniformMatrix4(singleColor_cMatrix_, chart_->jsMatrix());
    chart_->uniform3fv(singleColor_minPtUniform_, jsMinPt_);
    chart_->uniform3fv(singleColor_maxPtUniform_, jsMaxPt_);
    chart_->uniform3f(singleColor_dataMinPtUniform_, xMin, yMin, zMin);
    chart_->uniform3f(singleColor_dataMaxPtUniform_, xMax, yMax, zMax);
    chart_->uniform3f(singleColor_marginUniform_, marginX, marginY, marginZ);
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexPosBuffers_[i]);
    chart_->vertexAttribPointer(singleColor_vertexPosAttr_,
				3,
				WGLWidget::FLOAT,
				false,
				0,
				0);
    chart_->enableVertexAttribArray(singleColor_vertexPosAttr_);

    // Encoding the index as a color value.
    //
    // Since white is the clear color of the index texture,
    // this allows for 16777215 meshes. That should be enough,
    // or is even a tad excessive.
    float r = ((index >> 16) & 0xff) / 255.0f;
    float g = ((index >> 8) & 0xff) / 255.0f;
    float b = (index & 0xff) / 255.0f;
    chart_->uniform3f(singleColorUniform_, r, g, b);

    chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, indexBuffers_[i]);
    chart_->drawElements(WGLWidget::TRIANGLE_STRIP, indexBufferSizes_[i],
        	   WGLWidget::UNSIGNED_SHORT, 0);
    chart_->disableVertexAttribArray(singleColor_vertexPosAttr_);
  }
  chart_->enable(WGLWidget::CULL_FACE);
  chart_->disable(WGLWidget::DEPTH_TEST);
}

void WAbstractGridData::paintGLPositions() const
{
  paintGLPositions(0.0, 0.0, 0.0);
}

void WAbstractGridData::paintGLPositions(double marginX, double marginY, double marginZ) const
{
  if (hidden_)
    return;

  if (seriesType_ != Series3DType::Surface || 
      chart_->type() != ChartType::Scatter)
    return;

  double xMin = chart_->axis(Axis::X3D).minimum();
  double xMax = chart_->axis(Axis::X3D).maximum();
  double yMin = chart_->axis(Axis::Y3D).minimum();
  double yMax = chart_->axis(Axis::Y3D).maximum();
  double zMin = chart_->axis(Axis::Z3D).minimum();
  double zMax = chart_->axis(Axis::Z3D).maximum();

  chart_->disable(WGLWidget::CULL_FACE);
  chart_->enable(WGLWidget::DEPTH_TEST);

  for (unsigned i = 0; i < vertexPosBuffers_.size(); i++) {
    chart_->useProgram(positionProgram_);
    chart_->uniformMatrix4(position_cMatrix_, chart_->jsMatrix());
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexPosBuffers_[i]);
    chart_->uniform3fv(position_minPtUniform_, jsMinPt_);
    chart_->uniform3fv(position_maxPtUniform_, jsMaxPt_);
    chart_->uniform3f(position_dataMinPtUniform_, xMin, yMin, zMin);
    chart_->uniform3f(position_dataMaxPtUniform_, xMax, yMax, zMax);
    chart_->uniform3f(position_marginUniform_, marginX, marginY, marginZ);
    chart_->vertexAttribPointer(position_vertexPosAttr_,
				3,
				WGLWidget::FLOAT,
				false,
				0,
				0);
    chart_->enableVertexAttribArray(position_vertexPosAttr_);

    chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, indexBuffers_[i]);
    chart_->drawElements(WGLWidget::TRIANGLE_STRIP, indexBufferSizes_[i],
        	   WGLWidget::UNSIGNED_SHORT, 0);
    chart_->disableVertexAttribArray(position_vertexPosAttr_);
  }
  chart_->enable(WGLWidget::CULL_FACE);
  chart_->disable(WGLWidget::DEPTH_TEST);
}

void WAbstractGridData::setChart(WCartesian3DChart *chart)
{
  WAbstractDataSeries3D::setChart(chart);
  jsMinPt_ = WGLWidget::JavaScriptVector(3);
  jsMaxPt_ = WGLWidget::JavaScriptVector(3);
  chart->addJavaScriptVector(jsMinPt_);
  chart->addJavaScriptVector(jsMaxPt_);
  minPtChanged_ = true;
  maxPtChanged_ = true;
  changeClippingMinX_.setJavaScript("function(o,e,pos) {"
      "var obj = " + chart_->jsRef() + ".wtObj;" +
      jsMinPt_.jsRef() + "[0] = pos;" +
      chart->repaintSlot().execJs() + " }", 1);
  changeClippingMaxX_.setJavaScript("function(o,e,pos) {"
      "var obj = " + chart_->jsRef() + ".wtObj;" +
      jsMaxPt_.jsRef() + "[0] = pos;" +
      chart->repaintSlot().execJs() + " }", 1);
  changeClippingMinY_.setJavaScript("function(o,e,pos) {"
      "var obj = " + chart_->jsRef() + ".wtObj;" +
      jsMinPt_.jsRef() + "[1] = pos;" +
      chart->repaintSlot().execJs() + " }", 1);
  changeClippingMaxY_.setJavaScript("function(o,e,pos) {"
      "var obj = " + chart_->jsRef() + ".wtObj;" +
      jsMaxPt_.jsRef() + "[1] = pos;" +
      chart->repaintSlot().execJs() + " }", 1);
  changeClippingMinZ_.setJavaScript("function(o,e,pos) {"
      "var obj = " + chart_->jsRef() + ".wtObj;" +
      jsMinPt_.jsRef() + "[2] = pos;" +
      chart->repaintSlot().execJs() + " }", 1);
  changeClippingMaxZ_.setJavaScript("function(o,e,pos) {"
      "var obj = " + chart_->jsRef() + ".wtObj;" +
      jsMaxPt_.jsRef() + "[2] = pos;" +
      chart->repaintSlot().execJs() + " }", 1);
}

std::vector<cpp17::any> WAbstractGridData::getGlObjects()
{
  std::vector<cpp17::any> res;
  std::vector<std::vector<WGLWidget::Buffer> *> buffers;
  buffers.push_back(&vertexPosBuffers_);
  buffers.push_back(&vertexPosBuffers2_);
  buffers.push_back(&vertexSizeBuffers_);
  buffers.push_back(&vertexSizeBuffers2_);
  buffers.push_back(&vertexColorBuffers2_);
  buffers.push_back(&indexBuffers_);
  buffers.push_back(&indexBuffers2_);
  buffers.push_back(&overlayLinesBuffers_);
  buffers.push_back(&overlayLinesBuffers2_);
  buffers.push_back(&colormapTexBuffers_);
  buffers.push_back(&isoLineBuffers_);
  for (std::size_t i = 0; i < buffers.size(); ++i) {
    for (std::size_t j = 0; j < buffers[i]->size(); ++j) {
      res.push_back((*buffers[i])[j]);
    }
  }
  res.push_back(fragShader_);
  res.push_back(colFragShader_);
  res.push_back(meshFragShader_);
  res.push_back(singleColorFragShader_);
  res.push_back(positionFragShader_);
  res.push_back(isoLineFragShader_);
  res.push_back(vertShader_);
  res.push_back(colVertShader_);
  res.push_back(meshVertShader_);
  res.push_back(isoLineVertexShader_);
  res.push_back(seriesProgram_);
  res.push_back(colSeriesProgram_);
  res.push_back(meshProgram_);
  res.push_back(singleColorProgram_);
  res.push_back(isoLineProgram_);
  res.push_back(colormapTexture_);
  res.push_back(isoLineColorMapTexture_);
  res.push_back(pointSpriteTexture_);
  return res;
}

void WAbstractGridData::initializeGL()
{
  chart_->initJavaScriptVector(jsMinPt_);
  chart_->initJavaScriptVector(jsMaxPt_);
}

void WAbstractGridData::resizeGL()
{
  if (!seriesProgram_.isNull()) {
    chart_->useProgram(seriesProgram_);
    chart_->uniformMatrix4(pMatrix_, chart_->pMatrix());
  }
  if (!colSeriesProgram_.isNull()) {
    chart_->useProgram(colSeriesProgram_);
    chart_->uniformMatrix4(pMatrix2_, chart_->pMatrix());
  }
  if (!singleColorProgram_.isNull()) {
    chart_->useProgram(singleColorProgram_);
    chart_->uniformMatrix4(singleColor_pMatrix_, chart_->pMatrix());
  }
  if (!positionProgram_.isNull()) {
    chart_->useProgram(positionProgram_);
    chart_->uniformMatrix4(position_pMatrix_, chart_->pMatrix());
  }
  if (!meshProgram_.isNull()) {
    chart_->useProgram(meshProgram_);
    chart_->uniformMatrix4(mesh_pMatrix_, chart_->pMatrix());
  }
  if (!isoLineProgram_.isNull()) {
    chart_->useProgram(isoLineProgram_);
    chart_->uniformMatrix4(isoLine_pMatrix_, chart_->pMatrix());
  }
}

void WAbstractGridData::deleteAllGLResources()
{
  if (seriesProgram_.isNull()) { // never been painted
    return;
  }
  if (!singleColorProgram_.isNull()) {
    chart_->detachShader(singleColorProgram_, singleColorFragShader_);
    chart_->detachShader(singleColorProgram_, vertShader_);
    chart_->deleteShader(singleColorFragShader_);
    chart_->deleteProgram(singleColorProgram_);
    singleColorProgram_.clear();
  }
  if (!positionProgram_.isNull()) {
    chart_->detachShader(positionProgram_, positionFragShader_);
    chart_->detachShader(positionProgram_, vertShader_);
    chart_->deleteShader(positionFragShader_);
    chart_->deleteProgram(positionProgram_);
    positionProgram_.clear();
  }
  if (!seriesProgram_.isNull()) {
    chart_->detachShader(seriesProgram_, fragShader_);
    chart_->detachShader(seriesProgram_, vertShader_);
  }
  chart_->deleteShader(fragShader_);
  chart_->deleteShader(vertShader_);
  chart_->deleteProgram(seriesProgram_);
  seriesProgram_.clear();
  if (!colSeriesProgram_.isNull()) {
    chart_->detachShader(colSeriesProgram_, colFragShader_);
    chart_->detachShader(colSeriesProgram_, colVertShader_);
    chart_->deleteShader(colFragShader_);
    chart_->deleteShader(colVertShader_);
    chart_->deleteProgram(colSeriesProgram_);
    colSeriesProgram_.clear();
  }
  if (!meshProgram_.isNull()) {
    chart_->detachShader(meshProgram_, meshFragShader_);
    chart_->detachShader(meshProgram_, meshVertShader_);
    chart_->deleteShader(meshFragShader_);
    chart_->deleteShader(meshVertShader_);
    chart_->deleteProgram(meshProgram_);
    meshProgram_.clear();
  }
  if (!isoLineProgram_.isNull()) {
    chart_->detachShader(isoLineProgram_, isoLineFragShader_);
    chart_->detachShader(isoLineProgram_, isoLineVertexShader_);
    chart_->deleteShader(isoLineFragShader_);
    chart_->deleteShader(isoLineVertexShader_);
    chart_->deleteProgram(isoLineProgram_);
    isoLineProgram_.clear();
  }

  for (unsigned i = 0; i < vertexPosBuffers_.size(); i++) {
    if (!vertexPosBuffers_[i].isNull()) {
      chart_->deleteBuffer(vertexPosBuffers_[i]);
      vertexPosBuffers_[i].clear();
    }
  }
  for (unsigned i = 0; i < vertexSizeBuffers_.size(); i++) {
    if (!vertexSizeBuffers_[i].isNull()) {
      chart_->deleteBuffer(vertexSizeBuffers_[i]);
      vertexSizeBuffers_[i].clear();
    }
  }
  for (unsigned i = 0; i < vertexPosBuffers2_.size(); i++) {
    if (!vertexPosBuffers2_[i].isNull()) {
      chart_->deleteBuffer(vertexPosBuffers2_[i]);
      vertexPosBuffers2_[i].clear();
    }
  }
  for (unsigned i = 0; i < vertexColorBuffers2_.size(); i++) {
    if (!vertexColorBuffers2_[i].isNull()) {
      chart_->deleteBuffer(vertexColorBuffers2_[i]);
      vertexColorBuffers2_[i].clear();
    }
  }
  for (unsigned i = 0; i < vertexSizeBuffers2_.size(); i++) {
    if (!vertexSizeBuffers2_[i].isNull()) {
      chart_->deleteBuffer(vertexSizeBuffers2_[i]);
      vertexSizeBuffers2_[i].clear();
    }
  }
  for (size_t i = 0; i < isoLineBuffers_.size(); ++i) {
    if (!isoLineBuffers_[i].isNull()) {
      chart_->deleteBuffer(isoLineBuffers_[i]);
      isoLineBuffers_[i].clear();
    }
  }
  for (unsigned i = 0; i < indexBuffers_.size(); i++) {
    if (!indexBuffers_[i].isNull()) {
      chart_->deleteBuffer(indexBuffers_[i]);
      indexBuffers_[i].clear();
    }
  }
  for (unsigned i = 0; i < indexBuffers2_.size(); i++) {
    if (!indexBuffers2_[i].isNull()) {
      chart_->deleteBuffer(indexBuffers2_[i]);
      indexBuffers2_[i].clear();
    }
  }
  for (unsigned i = 0; i < overlayLinesBuffers_.size(); i++) {
    if (!overlayLinesBuffers_[i].isNull()) {
      chart_->deleteBuffer(overlayLinesBuffers_[i]);
      overlayLinesBuffers_[i].clear();
    }
  }
  for (unsigned i = 0; i < overlayLinesBuffers2_.size(); i++) {
    if (!overlayLinesBuffers2_[i].isNull()) {
      chart_->deleteBuffer(overlayLinesBuffers2_[i]);
      overlayLinesBuffers2_[i].clear();
    }
  }
  for (unsigned i = 0; i < colormapTexBuffers_.size(); i++) {
    if (!colormapTexBuffers_[i].isNull()) {
      chart_->deleteBuffer(colormapTexBuffers_[i]);
      colormapTexBuffers_[i].clear();
    }
  }
  vertexPosBuffers_.clear();
  vertexPosBufferSizes_.clear();
  vertexSizeBuffers_.clear();
  vertexPosBuffers2_.clear();
  vertexPosBuffer2Sizes_.clear();
  vertexSizeBuffers2_.clear();
  vertexColorBuffers2_.clear();
  indexBuffers_.clear();
  indexBuffers2_.clear();
  indexBufferSizes_.clear();
  indexBufferSizes2_.clear();
  overlayLinesBuffers_.clear();
  overlayLinesBuffers2_.clear();
  lineBufferSizes_.clear();
  lineBufferSizes2_.clear();
  colormapTexBuffers_.clear();
  isoLineBuffers_.clear();
  isoLineBufferSizes_.clear();

  if (!colormapTexture_.isNull()) {
    chart_->deleteTexture(colormapTexture_);
    colormapTexture_.clear();
  }

  if (!isoLineColorMapTexture_.isNull()) {
    chart_->deleteTexture(isoLineColorMapTexture_);
    isoLineColorMapTexture_.clear();
  }

  if (!pointSpriteTexture_.isNull()) {
    chart_->deleteTexture(pointSpriteTexture_);
    pointSpriteTexture_.clear();
  }
}


  }
}
