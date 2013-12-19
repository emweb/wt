/*
 * Copyright (C) 2013 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Chart/WAbstractGridData"

#include "Wt/Chart/WAbstractColorMap"
#include "Wt/WAbstractItemModel"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WException"
#include "Wt/WMemoryResource"
#include "Wt/WStandardItemModel"
#include "Wt/WStandardItem"
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

}

namespace Wt {
  namespace Chart {

const float WAbstractGridData::zeroBarCompensation = 0.001f;

WAbstractGridData::WAbstractGridData(WAbstractItemModel *model)
  : WAbstractDataSeries3D(model),
    seriesType_(PointSeries3D),
    surfaceMeshEnabled_(false),
    colorRoleEnabled_(false),
    barWidthX_(0.5f),
    barWidthY_(0.5f)
{}

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
      chart_->updateChart(WCartesian3DChart::GLContext);
  }
}

void WAbstractGridData::setSurfaceMeshEnabled(bool enabled)
{
  using Wt::operator|; // for CNOR

  if (enabled != surfaceMeshEnabled_) {
    surfaceMeshEnabled_ = enabled;
    if (seriesType_ == SurfaceSeries3D)
      if (chart_)
	chart_->updateChart(WCartesian3DChart::GLContext);
  }
}

void WAbstractGridData::setBarWidth(double xWidth, double yWidth)
{
  using Wt::operator|; // for CNOR

  if (xWidth != barWidthX_ || yWidth != barWidthY_) {
    barWidthX_ = xWidth;
    barWidthY_ = yWidth;
    if (chart_)
      chart_->updateChart(WCartesian3DChart::GLContext);
  }
}

void WAbstractGridData::setPen(const WPen &pen)
{
  using Wt::operator|; // for CNOR

  meshPen_ = pen;

  if (chart_)
    chart_->updateChart(WCartesian3DChart::GLContext);
}

float WAbstractGridData::stackAllValues(std::vector<WAbstractGridData*> dataseries,
				int i, int j) const
{
  float value = 0;
  for (unsigned k = 0; k<dataseries.size(); k++) {
    float modelVal = (float)Wt::asNumber(dataseries[k]->data(i, j));
    if (modelVal <= 0)
      modelVal = zeroBarCompensation;
    value += modelVal;
  }

  return value;
}

void WAbstractGridData::updateGL()
{
  // compatible chart-type check
  switch (seriesType_) {
  case PointSeries3D:
    if (chart_->type() != ScatterPlot)
      return;
    initializePointSeriesBuffers();
    break;
  case SurfaceSeries3D:
    if (chart_->type() != ScatterPlot)
      return;
    initializeSurfaceSeriesBuffers();
    break;
  case BarSeries3D:
    if (chart_->type() != CategoryChart)
      return;
    initializeBarSeriesBuffers();
    break;
  }

  // initialize texture (for colormap)
  colormapTexture_ = colorTexture();

  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_MAG_FILTER, WGLWidget::NEAREST);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_MIN_FILTER, WGLWidget::NEAREST);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_WRAP_S,WGLWidget::CLAMP_TO_EDGE);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_WRAP_T,WGLWidget::CLAMP_TO_EDGE);

  initShaders();

  // initialize colormap settings
  double min, max;
  switch (seriesType_) {
  case BarSeries3D:
    break; // coordinate-buffers made in initializeBarSeriesBuffers
  case PointSeries3D:
  case SurfaceSeries3D:
    chart_->useProgram(seriesProgram_);
    if (colormap_ != 0) {
      min = chart_->toPlotCubeCoords(colormap_->minimum(), ZAxis_3D);
      max = chart_->toPlotCubeCoords(colormap_->maximum(), ZAxis_3D);
      chart_->uniform1f(offset_, min);
      chart_->uniform1f(scaleFactor_, 1.0/(max-min));
    } else {
      chart_->uniform1f(offset_, 0.0);
      chart_->uniform1f(scaleFactor_, 1.0);
    }
    break;
  };

  // additional uniform initializations
  chart_->useProgram(seriesProgram_);
  chart_->uniformMatrix4(mvMatrixUniform_, mvMatrix_);
  chart_->uniformMatrix4(pMatrix_, chart_->pMatrix());
  switch (seriesType_) {
  case BarSeries3D:
    chart_->useProgram(meshProgram_);
    chart_->uniformMatrix4(mesh_mvMatrixUniform_, mvMatrix_);
    chart_->uniformMatrix4(mesh_pMatrix_, chart_->pMatrix());
  case PointSeries3D:
    chart_->useProgram(colSeriesProgram_);
    chart_->uniformMatrix4(mvMatrixUniform2_, mvMatrix_);
    chart_->uniformMatrix4(pMatrix2_, chart_->pMatrix());
    break;
  case SurfaceSeries3D:
    if (surfaceMeshEnabled_) {
      chart_->useProgram(meshProgram_);
      chart_->uniformMatrix4(mesh_mvMatrixUniform_, mvMatrix_);
      chart_->uniformMatrix4(mesh_pMatrix_, chart_->pMatrix());
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
				    FloatBuffer& verticesOUT)
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
				      int Nx, int Ny, int size)
{
  bool forward = true;

  switch (seriesType_) {
  case PointSeries3D:
    break;
  case SurfaceSeries3D:
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
  case BarSeries3D:
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
  case PointSeries3D:
    break;
  case SurfaceSeries3D:
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
  case BarSeries3D:
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
  case PointSeries3D:
  case SurfaceSeries3D:
    break;
  case BarSeries3D:
    if (colormap_ == 0) {
      for (int i=0; i < size; i++) {
	for (int k=0; k<16; k++) {
	  coordsOUT.push_back(0.0f);
	}
      }
    } else {
      float min = (float)chart_->
	toPlotCubeCoords(colormap_->minimum(), ZAxis_3D);
      float max = (float)chart_->
	toPlotCubeCoords(colormap_->maximum(), ZAxis_3D);
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

void WAbstractGridData::initShaders()
{
  switch (seriesType_) {
  case BarSeries3D:
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
  case PointSeries3D:
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
    if (!wApp->environment().agentIsIE())
      vertexSizeAttr_ = chart_->getAttribLocation(seriesProgram_, "aPointSize");
    mvMatrixUniform_ = chart_->getUniformLocation(seriesProgram_, "uMVMatrix");
    pMatrix_ = chart_->getUniformLocation(seriesProgram_, "uPMatrix");
    cMatrix_ = chart_->getUniformLocation(seriesProgram_, "uCMatrix");
    TexSampler_ = chart_->getUniformLocation(seriesProgram_, "uSampler");
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
    if (!wApp->environment().agentIsIE())
      vertexSizeAttr2_ = chart_->getAttribLocation(colSeriesProgram_,
						 "aPointSize");
    vertexColAttr2_ = chart_->getAttribLocation(colSeriesProgram_, "aColor");
    mvMatrixUniform2_ = chart_->getUniformLocation(colSeriesProgram_,
						  "uMVMatrix");
    pMatrix2_ = chart_->getUniformLocation(colSeriesProgram_, "uPMatrix");
    cMatrix2_ = chart_->getUniformLocation(colSeriesProgram_, "uCMatrix");
    break;
  case SurfaceSeries3D:
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
    break;
  };
  
  if (seriesType_ == BarSeries3D ||
      (seriesType_ == SurfaceSeries3D && surfaceMeshEnabled_)) {
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
  }
}

void WAbstractGridData::paintGL() const
{
  if (hidden_)
    return;

  // compatible chart-type check
  switch (seriesType_) {
  case PointSeries3D:
    if (chart_->type() != ScatterPlot)
      return;
    break;
  case SurfaceSeries3D:
    if (chart_->type() != ScatterPlot)
      return;
    break;
  case BarSeries3D:
    if (chart_->type() != CategoryChart)
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

  if ( seriesType_ == BarSeries3D ) {
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, colormapTexBuffers_[i]);
    chart_->vertexAttribPointer(barTexCoordAttr_,
				2,
				WGLWidget::FLOAT,
				false,
				0,
				0);
    chart_->enableVertexAttribArray(barTexCoordAttr_);
  }

  chart_->activeTexture(WGLWidget::TEXTURE0);
  chart_->bindTexture(WGLWidget::TEXTURE_2D, colormapTexture_);
  chart_->uniform1i(TexSampler_,0);

  if (seriesType_ == BarSeries3D) {
    chart_->enable(WGLWidget::POLYGON_OFFSET_FILL);
    float unitOffset = (float) (std::pow(5,meshPen_.width().value()) < 10000 ? std::pow(5,meshPen_.width().value()) : 10000);
    chart_->polygonOffset(1, unitOffset);
    chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, indexBuffers_[i]);
    chart_->drawElements(WGLWidget::TRIANGLES, indexBufferSizes_[i], 
			 WGLWidget::UNSIGNED_SHORT, 0);
    chart_->disableVertexAttribArray(vertexPosAttr_);
    chart_->disableVertexAttribArray(barTexCoordAttr_);
  } else if (seriesType_ == SurfaceSeries3D) {
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
    if (!wApp->environment().agentIsIE()) {
      chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexSizeBuffers_[i]);
      chart_->vertexAttribPointer(vertexSizeAttr_,
				  1,
				  WGLWidget::FLOAT,
				  false,
				  0,
				  0);
      chart_->enableVertexAttribArray(vertexSizeAttr_);
    }
    chart_->drawArrays(WGLWidget::POINTS,
		       0,
		       vertexPosBufferSizes_[i]/3);
    chart_->disableVertexAttribArray(vertexPosAttr_);
    if (!wApp->environment().agentIsIE())
      chart_->disableVertexAttribArray(vertexSizeAttr_);
  }
  
  // draw mesh
  if ( seriesType_ == BarSeries3D ||
       (seriesType_ == SurfaceSeries3D && surfaceMeshEnabled_) ) {
    chart_->useProgram(meshProgram_);
    chart_->depthFunc(WGLWidget::LEQUAL);
    chart_->uniformMatrix4(mesh_cMatrix_, chart_->jsMatrix());
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
    if (seriesType_ == SurfaceSeries3D) {
      chart_->lineWidth(meshPen_.width().value() == 0 ? 
			1.0 : 
			meshPen_.width().value());
      chart_->drawElements(WGLWidget::LINE_STRIP, lineBufferSizes_[i],
			   WGLWidget::UNSIGNED_SHORT, 0);
    } else if (seriesType_ == BarSeries3D) {
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
    if (seriesType_ == PointSeries3D) {
      if (!wApp->environment().agentIsIE()) {
	chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexSizeBuffers2_[i]);
	chart_->vertexAttribPointer(vertexSizeAttr2_,
				    1,
				    WGLWidget::FLOAT,
				    false,
				    0,
				    0);
	chart_->enableVertexAttribArray(vertexSizeAttr2_);
      }
      chart_->drawArrays(WGLWidget::POINTS,
			 0,
			 vertexPosBuffer2Sizes_[i]/3);
      if (!wApp->environment().agentIsIE())
	chart_->disableVertexAttribArray(vertexSizeAttr2_);
    } else if (seriesType_ == BarSeries3D) {
      chart_->bindBuffer(WGLWidget::ELEMENT_ARRAY_BUFFER, indexBuffers2_[i]);
      chart_->drawElements(WGLWidget::TRIANGLES, indexBufferSizes2_[i], 
			   WGLWidget::UNSIGNED_SHORT, 0);
    }
    chart_->disableVertexAttribArray(vertexPosAttr2_);
    chart_->disableVertexAttribArray(vertexColAttr2_);

    if (seriesType_ == BarSeries3D) {
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
  chart_->enable(WGLWidget::CULL_FACE);
  chart_->disable(WGLWidget::DEPTH_TEST);
}

void WAbstractGridData::initializeGL()
{
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
  if (!meshProgram_.isNull()) {
    chart_->useProgram(meshProgram_);
    chart_->uniformMatrix4(mesh_pMatrix_, chart_->pMatrix());
  }
}

void WAbstractGridData::deleteAllGLResources()
{
  if (seriesProgram_.isNull()) { // never been painted
    return;
  }
  chart_->detachShader(seriesProgram_, fragShader_);
  chart_->detachShader(seriesProgram_, vertShader_);
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

  for (unsigned i = 0; i < vertexPosBuffers_.size(); i++) {
    if (vertexPosBuffers_[i].jsRef() != "") {
      chart_->deleteBuffer(vertexPosBuffers_[i]);
      vertexPosBuffers_[i].clear();
    }
  }
  for (unsigned i = 0; i < vertexSizeBuffers_.size(); i++) {
    if (vertexSizeBuffers_[i].isNull()) {
      chart_->deleteBuffer(vertexSizeBuffers_[i]);
      vertexSizeBuffers_[i].clear();
    }
  }
  for (unsigned i = 0; i < vertexPosBuffers2_.size(); i++) {
    if (vertexPosBuffers2_[i].isNull()) {
      chart_->deleteBuffer(vertexPosBuffers2_[i]);
      vertexPosBuffers2_[i].clear();
      chart_->deleteBuffer(vertexColorBuffers2_[i]);
      vertexColorBuffers2_[i].clear();
    }
  }
  for (unsigned i = 0; i < vertexSizeBuffers2_.size(); i++) {
    if (vertexSizeBuffers2_[i].isNull()) {
      chart_->deleteBuffer(vertexSizeBuffers2_[i]);
      vertexSizeBuffers2_[i].clear();
    }
  }
  for (unsigned i = 0; i < indexBuffers_.size(); i++) {
    if (indexBuffers_[i].isNull()) {
      chart_->deleteBuffer(indexBuffers_[i]);
      indexBuffers_[i].clear();
    }
  }
  for (unsigned i = 0; i < overlayLinesBuffers_.size(); i++) {
    if (overlayLinesBuffers_[i].isNull()) {
      chart_->deleteBuffer(overlayLinesBuffers_[i]);
      overlayLinesBuffers_[i].clear();
    }
  }
  for (unsigned i = 0; i < colormapTexBuffers_.size(); i++) {
    if (colormapTexBuffers_[i].isNull()) {
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
  indexBufferSizes_.clear();
  overlayLinesBuffers_.clear();
  lineBufferSizes_.clear();
  colormapTexBuffers_.clear();
  
  if (colormapTexture_.isNull()) {
    chart_->deleteTexture(colormapTexture_);
    colormapTexture_.clear();
  }

}


  }
}
