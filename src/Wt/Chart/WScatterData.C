// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bvba, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WScatterData"

#include "Wt/WAbstractItemModel"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WMemoryResource"
#include "Wt/Chart/WAbstractColorMap"
#include "WebUtils.h"

#include "gridDataShaders"

namespace Wt {
  namespace Chart {

WScatterData::WScatterData(WAbstractItemModel *model)
  : WAbstractDataSeries3D(model),
    XSeriesColumn_(0),
    YSeriesColumn_(1),
    ZSeriesColumn_(2),
    colorColumn_(-1),
    asColorRole_(Wt::MarkerPenColorRole),
    asSizeRole_(Wt::MarkerScaleFactorRole),
    sizeColumn_(-1),
    droplinesEnabled_(false),
    xRangeCached_(false),
    yRangeCached_(false)
{
}

#ifndef WT_TARGET_JAVA
void WScatterData::setColorColumn(int columnNumber, ItemDataRole role)
{
  colorColumn_ = columnNumber;
  asColorRole_ = role;
}

void WScatterData::setSizeColumn(int columnNumber, ItemDataRole role)
{
  sizeColumn_ = columnNumber;
  asSizeRole_ = role;
}
#else
void WScatterData::setColorColumn(int columnNumber, int role)
{
  colorColumn_ = columnNumber;
  asColorRole_ = role;
}

void WScatterData::setSizeColumn(int columnNumber, int role)
{
  sizeColumn_ = columnNumber;
  asSizeRole_ = role;
}
#endif

void WScatterData::setDroplinesEnabled(bool enabled)
{
  if (droplinesEnabled_ != enabled) {
    droplinesEnabled_ = enabled;
    if (chart_) {
      chart_->updateChart(WCartesian3DChart::GLContext);
    }
  }
}

void WScatterData::setDroplinesPen(const WPen &pen)
{
  droplinesPen_ = pen;

  if (chart_) {
    chart_->updateChart(WCartesian3DChart::GLContext);
  }
}

void WScatterData::updateGL()
{
  int N = model_->rowCount();

  int cnt = countSimpleData();
  FloatBuffer simplePtsArray = Utils::createFloatBuffer(3*cnt);
  FloatBuffer simplePtsSize = Utils::createFloatBuffer(cnt);
  FloatBuffer coloredPtsArray = Utils::createFloatBuffer(3*(N-cnt));
  FloatBuffer coloredPtsSize = Utils::createFloatBuffer(N-cnt);
  FloatBuffer coloredPtsColor = Utils::createFloatBuffer(4*(N-cnt));
  dataFromModel(simplePtsArray, simplePtsSize, coloredPtsArray, coloredPtsSize, coloredPtsColor);

  if (simplePtsArray.size() != 0) {
    // initialize vertex-buffer
    vertexPosBuffer_ = chart_->createBuffer();
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexPosBuffer_);
    chart_->bufferDatafv(WGLWidget::ARRAY_BUFFER,
			 simplePtsArray,
			 WGLWidget::STATIC_DRAW,
			 true);
    vertexBufferSize_ = simplePtsArray.size();
    
    // sizes of simple points
    vertexSizeBuffer_ = chart_->createBuffer();
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexSizeBuffer_);
    chart_->bufferDatafv(WGLWidget::ARRAY_BUFFER,
			 simplePtsSize,
			 WGLWidget::STATIC_DRAW,
			 true);
  }
  
  if (coloredPtsArray.size() != 0) {
    // pos of colored points
    vertexPosBuffer2_ = chart_->createBuffer();
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexPosBuffer2_);
    chart_->bufferDatafv(WGLWidget::ARRAY_BUFFER,
			 coloredPtsArray,
			 WGLWidget::STATIC_DRAW,
			 true);
    vertexBuffer2Size_ = coloredPtsArray.size();
    
    // size of colored points
    vertexSizeBuffer2_ = chart_->createBuffer();
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexSizeBuffer2_);
    chart_->bufferDatafv(WGLWidget::ARRAY_BUFFER,
			 coloredPtsSize,
			 WGLWidget::STATIC_DRAW,
			 true);
    
    // color of colored points
    vertexColorBuffer2_ = chart_->createBuffer();
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexColorBuffer2_);
    chart_->bufferDatafv(WGLWidget::ARRAY_BUFFER,
			 coloredPtsColor,
			 WGLWidget::STATIC_DRAW,
			 true);
  }

  if (droplinesEnabled_) {
    FloatBuffer dropLineVerts = Utils::createFloatBuffer(2*3*N);
    dropLineVertices(simplePtsArray, dropLineVerts);
    dropLineVertices(coloredPtsArray, dropLineVerts);
    lineVertBuffer_ = chart_->createBuffer();
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, lineVertBuffer_);
    chart_->bufferDatafv(WGLWidget::ARRAY_BUFFER,
			 dropLineVerts,
			 WGLWidget::STATIC_DRAW,
			 true);
    lineVertBufferSize_ = dropLineVerts.size();
  }

  // initialize texture
  colormapTexture_ = colorTexture();

  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_MAG_FILTER, WGLWidget::NEAREST);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_MIN_FILTER, WGLWidget::NEAREST);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_WRAP_S,WGLWidget::CLAMP_TO_EDGE);
  chart_->texParameteri(WGLWidget::TEXTURE_2D, WGLWidget::TEXTURE_WRAP_T,WGLWidget::CLAMP_TO_EDGE);

  initShaders();

  chart_->useProgram(shaderProgram_);
  chart_->uniformMatrix4(mvMatrixUniform_, mvMatrix_);
  chart_->uniformMatrix4(pMatrixUniform_, chart_->pMatrix());
  chart_->useProgram(colShaderProgram_);
  chart_->uniformMatrix4(mvMatrixUniform2_, mvMatrix_);
  chart_->uniformMatrix4(pMatrixUniform2_, chart_->pMatrix());
  chart_->useProgram(linesProgram_);
  chart_->uniformMatrix4(mvMatrixUniform3_, mvMatrix_);
  chart_->uniformMatrix4(pMatrixUniform3_, chart_->pMatrix());
  chart_->useProgram(shaderProgram_);
  float text_min, text_max;
  if (colormap_ != 0) {
    text_min = (float)chart_->toPlotCubeCoords(colormap_->minimum(), ZAxis_3D);
    text_max = (float)chart_->toPlotCubeCoords(colormap_->maximum(), ZAxis_3D);
    chart_->uniform1f(offsetUniform_, text_min);
    chart_->uniform1f(scaleFactorUniform_, 1.0/(text_max - text_min));
  } else {
    chart_->uniform1f(offsetUniform_, 0.0);
    chart_->uniform1f(scaleFactorUniform_, 1.0);
  }
}

void WScatterData::resizeGL()
{
  if (!shaderProgram_.isNull()) {
    chart_->useProgram(shaderProgram_);
    chart_->uniformMatrix4(pMatrixUniform_, chart_->pMatrix());
  }
  if (!colShaderProgram_.isNull()) {
    chart_->useProgram(colShaderProgram_);
    chart_->uniformMatrix4(pMatrixUniform2_, chart_->pMatrix());
  }
  if (!linesProgram_.isNull()) {
    chart_->useProgram(linesProgram_);
    chart_->uniformMatrix4(pMatrixUniform3_, chart_->pMatrix());
  }
}

void WScatterData::initShaders()
{
  // shader for simple points
  fragmentShader_ = chart_->createShader(WGLWidget::FRAGMENT_SHADER);
  chart_->shaderSource(fragmentShader_, ptFragShaderSrc);
  chart_->compileShader(fragmentShader_);
  vertexShader_ = chart_->createShader(WGLWidget::VERTEX_SHADER);
  chart_->shaderSource(vertexShader_, ptVertexShaderSrc);
  chart_->compileShader(vertexShader_);

  shaderProgram_ = chart_->createProgram();
  chart_->attachShader(shaderProgram_, vertexShader_);
  chart_->attachShader(shaderProgram_, fragmentShader_);
  chart_->linkProgram(shaderProgram_);

  posAttr_ =
    chart_->getAttribLocation(shaderProgram_, "aVertexPosition");
  if (!wApp->environment().agentIsIE())
    sizeAttr_ = chart_->getAttribLocation(shaderProgram_, "aPointSize");
  mvMatrixUniform_ = chart_->getUniformLocation(shaderProgram_, "uMVMatrix");
  pMatrixUniform_ = chart_->getUniformLocation(shaderProgram_, "uPMatrix");
  cMatrixUniform_ = chart_->getUniformLocation(shaderProgram_, "uCMatrix");
  samplerUniform_ = chart_->getUniformLocation(shaderProgram_, "uSampler");
  offsetUniform_ = chart_->getUniformLocation(shaderProgram_, "uOffset");
  scaleFactorUniform_ =
    chart_->getUniformLocation(shaderProgram_, "uScaleFactor");

  // shader for colored points
  colFragmentShader_ = chart_->createShader(WGLWidget::FRAGMENT_SHADER);
  chart_->shaderSource(colFragmentShader_, colPtFragShaderSrc);
  chart_->compileShader(colFragmentShader_);
  colVertexShader_ = chart_->createShader(WGLWidget::VERTEX_SHADER);
  chart_->shaderSource(colVertexShader_, colPtVertexShaderSrc);
  chart_->compileShader(colVertexShader_);

  colShaderProgram_ = chart_->createProgram();
  chart_->attachShader(colShaderProgram_, colVertexShader_);
  chart_->attachShader(colShaderProgram_, colFragmentShader_);
  chart_->linkProgram(colShaderProgram_);

  posAttr2_ = chart_->getAttribLocation(colShaderProgram_, "aVertexPosition");
  if (!wApp->environment().agentIsIE())
    sizeAttr2_ = chart_->getAttribLocation(colShaderProgram_, "aPointSize");
  colorAttr2_ = chart_->getAttribLocation(colShaderProgram_, "aColor");
  mvMatrixUniform2_ = chart_->
    getUniformLocation(colShaderProgram_, "uMVMatrix");
  pMatrixUniform2_ = chart_->getUniformLocation(colShaderProgram_, "uPMatrix");
  cMatrixUniform2_ = chart_->getUniformLocation(colShaderProgram_, "uCMatrix");

  // shader for droplines
  linesFragShader_ = chart_->createShader(WGLWidget::FRAGMENT_SHADER);
  chart_->shaderSource(linesFragShader_,meshFragShaderSrc);
  chart_->compileShader(linesFragShader_);

  linesVertShader_ = chart_->createShader(WGLWidget::VERTEX_SHADER);
  chart_->shaderSource(linesVertShader_, meshVertexShaderSrc);
  chart_->compileShader(linesVertShader_);

  linesProgram_ = chart_->createProgram();
  chart_->attachShader(linesProgram_, linesVertShader_);
  chart_->attachShader(linesProgram_, linesFragShader_);
  chart_->linkProgram(linesProgram_);

  posAttrLines_ = chart_->getAttribLocation(linesProgram_, "aVertexPosition");
  mvMatrixUniform3_ = chart_->
    getUniformLocation(linesProgram_, "uMVMatrix");
  pMatrixUniform3_ = chart_->getUniformLocation(linesProgram_, "uPMatrix");
  cMatrixUniform3_ = chart_->getUniformLocation(linesProgram_, "uCMatrix");
  lineColorUniform_ = chart_->getUniformLocation(linesProgram_, "uColor");
}

int WScatterData::countSimpleData() const
{
  int result;
#ifndef WT_TARGET_JAVA
  result = 0;
#else
  result = 0;
  int N = model_->rowCount();
  if (colorColumn_ != -1) {
    result = 0;
  } else {
    for (int i=0; i < N; i++) {
      if (model_->data(i,ZSeriesColumn_, MarkerBrushColorRole).empty()) {
	result++;
      }
    }
  }
#endif

  return result;
}

void WScatterData::dataFromModel(FloatBuffer& simplePtsArray,
				 FloatBuffer& simplePtsSize,
				 FloatBuffer& coloredPtsArray,
				 FloatBuffer& coloredPtsSize,
				 FloatBuffer& coloredPtsColor)
{
  int N = model_->rowCount();

  double xMin = chart_->axis(XAxis_3D).minimum();
  double xMax = chart_->axis(XAxis_3D).maximum();
  double yMin = chart_->axis(YAxis_3D).minimum();
  double yMax = chart_->axis(YAxis_3D).maximum();
  double zMin = chart_->axis(ZAxis_3D).minimum();
  double zMax = chart_->axis(ZAxis_3D).maximum();
  
  for (int i=0; i < N; i++) {
    if (colorColumn_ == -1 && model_->data(i,ZSeriesColumn_, MarkerBrushColorRole).empty()) {
      simplePtsArray.push_back((float)((Wt::asNumber(model_->data(i,XSeriesColumn_)) - xMin)/(xMax - xMin)));
      simplePtsArray.push_back((float)((Wt::asNumber(model_->data(i,YSeriesColumn_)) - yMin)/(yMax - yMin)));
      simplePtsArray.push_back((float)((Wt::asNumber(model_->data(i,ZSeriesColumn_)) - zMin)/(zMax - zMin)));
    } else if (colorColumn_ == -1) {
      coloredPtsArray.push_back((float)((Wt::asNumber(model_->data(i,XSeriesColumn_)) - xMin)/(xMax - xMin)));
      coloredPtsArray.push_back((float)((Wt::asNumber(model_->data(i,YSeriesColumn_)) - yMin)/(yMax - yMin)));
      coloredPtsArray.push_back((float)((Wt::asNumber(model_->data(i,ZSeriesColumn_)) - zMin)/(zMax - zMin)));
      WColor color = boost::any_cast<WColor>(model_->data(i,ZSeriesColumn_,MarkerBrushColorRole));
      coloredPtsColor.push_back((float)color.red());
      coloredPtsColor.push_back((float)color.green());
      coloredPtsColor.push_back((float)color.blue());
      coloredPtsColor.push_back((float)color.alpha());
    } else {
      coloredPtsArray.push_back((float)((Wt::asNumber(model_->data(i,XSeriesColumn_)) - xMin)/(xMax - xMin)));
      coloredPtsArray.push_back((float)((Wt::asNumber(model_->data(i,YSeriesColumn_)) - yMin)/(yMax - yMin)));
      coloredPtsArray.push_back((float)((Wt::asNumber(model_->data(i,ZSeriesColumn_)) - zMin)/(zMax - zMin)));
      WColor color = boost::any_cast<WColor>(model_->data(i,colorColumn_,asColorRole_));
      coloredPtsColor.push_back((float)color.red());
      coloredPtsColor.push_back((float)color.green());
      coloredPtsColor.push_back((float)color.blue());
      coloredPtsColor.push_back((float)color.alpha());
    }

    FloatBuffer& sizeArrayAlias = (colorColumn_ == -1 && model_->data(i,ZSeriesColumn_, MarkerBrushColorRole).empty()) ? simplePtsSize : coloredPtsSize;
    if (sizeColumn_ == -1 && model_->data(i,ZSeriesColumn_,MarkerScaleFactorRole).empty()) {
      sizeArrayAlias.push_back((float)pointSize_);
    } else if (sizeColumn_ == -1) {
      sizeArrayAlias.push_back((float)(Wt::asNumber(model_->data(i,ZSeriesColumn_,MarkerScaleFactorRole))));
    } else {
      sizeArrayAlias.push_back((float)(Wt::asNumber(model_->data(i,sizeColumn_, asSizeRole_))));
    }
  }
}

void WScatterData::dropLineVertices(FloatBuffer& dataPoints,
				    FloatBuffer& verticesOUT)
{
  int size = dataPoints.size();
  int index;
  for (int i=0; i < size/3; i++) {
    index = 3*i;
    verticesOUT.push_back(dataPoints[index]);
    verticesOUT.push_back(dataPoints[index + 1]);
    verticesOUT.push_back(dataPoints[index + 2]);
    verticesOUT.push_back(dataPoints[index]);
    verticesOUT.push_back(dataPoints[index + 1]);
    verticesOUT.push_back(0.0f);
  }
}

double WScatterData::minimum(Axis axis) const
{
  if (axis == XAxis_3D) {
    if (!xRangeCached_) {
      findXRange();
    }
    return xMin_;
  } else if (axis == YAxis_3D) {
    if (!yRangeCached_) {
      findYRange();
    }
    return yMin_;
  } else if (axis == ZAxis_3D) {
    if (!rangeCached_) {
      findZRange();
    }
    return zMin_;
  } else {
    throw WException("WScatterData: unknown Axis-type");
  }
}

double WScatterData::maximum(Axis axis) const
{
  if (axis == XAxis_3D) {
    if (!xRangeCached_) {
      findXRange();
    }
    return xMax_;
  } else if (axis == YAxis_3D) {
    if (!yRangeCached_) {
      findYRange();
    }
    return yMax_;
  } else if (axis == ZAxis_3D) {
    if (!rangeCached_) {
      findZRange();
    }
    return zMax_;
  } else {
    throw WException("WScatterData: unknown Axis-type");
  }
}

void WScatterData::findXRange() const
{
  int N = model_->rowCount();

  double minSoFar = std::numeric_limits<double>::max();
  double maxSoFar = -std::numeric_limits<double>::max();
  double xVal;
  for (int i = 0; i < N; i++) {
    xVal = Wt::asNumber(model_->data(i, XSeriesColumn_));
    if (xVal < minSoFar) {
      minSoFar = xVal;
    }
    if (xVal > maxSoFar) {
      maxSoFar = xVal;
    }
  }

  xMin_ = minSoFar; xMax_ = maxSoFar;
  xRangeCached_ = true;
}

void WScatterData::findYRange() const
{
  int N = model_->rowCount();

  double minSoFar = std::numeric_limits<double>::max();
  double maxSoFar = -std::numeric_limits<double>::max();
  double yVal;
  for (int i = 0; i < N; i++) {
    yVal = Wt::asNumber(model_->data(i, YSeriesColumn_));
    if (yVal < minSoFar) {
      minSoFar = yVal;
    }
    if (yVal > maxSoFar) {
      maxSoFar = yVal;
    }
  }

  yMin_ = minSoFar; yMax_ = maxSoFar;
  yRangeCached_ = true;
}

void WScatterData::findZRange() const
{
  int N = model_->rowCount();

  double minSoFar = std::numeric_limits<double>::max();
  double maxSoFar = -std::numeric_limits<double>::max();
  double zVal;
  for (int i = 0; i < N; i++) {
    zVal = Wt::asNumber(model_->data(i, ZSeriesColumn_));
    if (zVal < minSoFar) {
      minSoFar = zVal;
    }
    if (zVal > maxSoFar) {
      maxSoFar = zVal;
    }
  }

  zMin_ = minSoFar; zMax_ = maxSoFar;
  rangeCached_ = true;
}

void WScatterData::paintGL() const
{
  if (hidden_)
    return;
  
  chart_->disable(WGLWidget::CULL_FACE);
  chart_->enable(WGLWidget::DEPTH_TEST);

  if (!vertexPosBuffer_.isNull()) {
    // draw simple points
    chart_->useProgram(shaderProgram_);
    chart_->uniformMatrix4(cMatrixUniform_, chart_->jsMatrix());

    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexPosBuffer_);
    chart_->vertexAttribPointer(posAttr_,
				3,
				WGLWidget::FLOAT,
				false,
				0,
				0);
    chart_->enableVertexAttribArray(posAttr_);
    if (!wApp->environment().agentIsIE()) {
      chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexSizeBuffer_);
      chart_->vertexAttribPointer(sizeAttr_,
				  1,
				  WGLWidget::FLOAT,
				  false,
				  0,
				  0);
      chart_->enableVertexAttribArray(sizeAttr_);
    }

    chart_->activeTexture(WGLWidget::TEXTURE0);
    chart_->bindTexture(WGLWidget::TEXTURE_2D, colormapTexture_);
    chart_->uniform1i(samplerUniform_,0);

    chart_->drawArrays(WGLWidget::POINTS,
		       0,
		       vertexBufferSize_/3);
    chart_->disableVertexAttribArray(posAttr_);
    if (!wApp->environment().agentIsIE())
      chart_->disableVertexAttribArray(sizeAttr_);
  }

  if (!vertexPosBuffer2_.isNull()) {
    // draw colored points
    chart_->useProgram(colShaderProgram_);
    chart_->uniformMatrix4(cMatrixUniform2_, chart_->jsMatrix());

    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexPosBuffer2_);
    chart_->vertexAttribPointer(posAttr2_,
				3,
				WGLWidget::FLOAT,
				false,
				0,
				0);
    chart_->enableVertexAttribArray(posAttr2_);
    if (!wApp->environment().agentIsIE()) {
      chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexSizeBuffer2_);
      chart_->vertexAttribPointer(sizeAttr2_,
				  1,
				  WGLWidget::FLOAT,
				  false,
				  0,
				  0);
      chart_->enableVertexAttribArray(sizeAttr2_);
    }
    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, vertexColorBuffer2_);
    chart_->vertexAttribPointer(colorAttr2_,
				4,
				WGLWidget::FLOAT,
				false,
				0,
				0);
    chart_->enableVertexAttribArray(colorAttr2_);

    chart_->drawArrays(WGLWidget::POINTS,
		       0,
		       vertexBuffer2Size_/3);
    chart_->disableVertexAttribArray(posAttr2_);
    if (!wApp->environment().agentIsIE())
      chart_->disableVertexAttribArray(sizeAttr2_);
    chart_->disableVertexAttribArray(colorAttr2_);
  }

  if (droplinesEnabled_) {
    chart_->useProgram(linesProgram_);
    chart_->uniformMatrix4(cMatrixUniform3_, chart_->jsMatrix());
    chart_->uniform4f(lineColorUniform_,
		      (float)droplinesPen_.color().red(),
		      (float)droplinesPen_.color().green(),
		      (float)droplinesPen_.color().blue(),
		      (float)droplinesPen_.color().alpha());

    chart_->bindBuffer(WGLWidget::ARRAY_BUFFER, lineVertBuffer_);
    chart_->vertexAttribPointer(posAttrLines_,
				3,
				WGLWidget::FLOAT,
				false,
				0,
				0);
    chart_->enableVertexAttribArray(posAttrLines_);
    chart_->lineWidth(droplinesPen_.width().value() == 0 ? 
		      1.0 : 
		      droplinesPen_.width().value());
    chart_->drawArrays(WGLWidget::LINES,
		       0,
		       lineVertBufferSize_/3);
    chart_->disableVertexAttribArray(posAttrLines_);
  }
  
  chart_->enable(WGLWidget::CULL_FACE);
  chart_->disable(WGLWidget::DEPTH_TEST);
}

void WScatterData::deleteAllGLResources()
{
  if (!shaderProgram_.isNull()) {
    chart_->detachShader(shaderProgram_, vertexShader_);
    chart_->detachShader(shaderProgram_, fragmentShader_);
    chart_->deleteShader(vertexShader_);
    chart_->deleteShader(fragmentShader_);
    chart_->deleteProgram(shaderProgram_);
    shaderProgram_.clear();
  }
  if (!colShaderProgram_.isNull()) {
    chart_->detachShader(colShaderProgram_, colVertexShader_);
    chart_->detachShader(colShaderProgram_, colFragmentShader_);
    chart_->deleteShader(colVertexShader_);
    chart_->deleteShader(colFragmentShader_);
    chart_->deleteProgram(colShaderProgram_);
    colShaderProgram_.clear();
  }
  if (!linesProgram_.isNull()) {
    chart_->detachShader(linesProgram_, linesVertShader_);
    chart_->detachShader(linesProgram_, linesFragShader_);
    chart_->deleteShader(linesVertShader_);
    chart_->deleteShader(linesFragShader_);
    chart_->deleteProgram(linesProgram_);
    linesProgram_.clear();
  }

  if (!vertexPosBuffer_.isNull()) {
    chart_->deleteBuffer(vertexPosBuffer_);
    chart_->deleteBuffer(vertexSizeBuffer_);
    vertexPosBuffer_.clear();
  }
  if (!vertexPosBuffer2_.isNull()) {
    chart_->deleteBuffer(vertexPosBuffer2_);
    chart_->deleteBuffer(vertexSizeBuffer2_);
    chart_->deleteBuffer(vertexColorBuffer2_);
    vertexPosBuffer2_.clear();
  }
  if (!lineVertBuffer_.isNull()) {
    chart_->deleteBuffer(lineVertBuffer_);
    lineVertBuffer_.clear();
  }

  if (!colormapTexture_.isNull()) {
    chart_->deleteTexture(colormapTexture_);
    colormapTexture_.clear();
  }
}

  }
}
