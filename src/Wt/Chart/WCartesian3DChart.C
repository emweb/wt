/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Chart/WCartesian3DChart.h"

#include "Wt/Chart/WChart3DImplementation.h"
#include "Wt/Chart/WAbstractColorMap.h"
#include "Wt/Chart/WAbstractGridData.h"
#include "Wt/WCanvasPaintDevice.h"
#include "Wt/WException.h"
#include "Wt/WFont.h"
#include "Wt/WLength.h"
#include "Wt/WPaintedWidget.h"
#include "Wt/WPainter.h"
#include "Wt/WPaintDevice.h"
#include "Wt/WPainterPath.h"
#include "Wt/WPen.h"
#include "Wt/WRasterImage.h"
#include "Wt/WRectF.h"
#include "Wt/WVector3.h"
#include "Wt/WVector4.h"
#include "Wt/Chart/WStandardPalette.h"
#include "WebUtils.h"

#include <string>
#include <fstream>
#include <cmath>

#include "cubeData"
#include "verticalFlapData"
#include "horizontalFlapData"
#include "cubeShaders"

using namespace Wt;

namespace {
static const double TICKLENGTH = 5.0;
static const double TITLEOFFSET = 30;
static const double ANGLE1 = 15;
static const double ANGLE2 = 80;
}

namespace Wt {
  namespace Chart {

WCartesian3DChart::WCartesian3DChart()
  : isViewSet_(false),
    xAxis_(std::make_unique<WAxis>()),
    yAxis_(std::make_unique<WAxis>()),
    zAxis_(std::make_unique<WAxis>()),
    chartType_(ChartType::Scatter),
    background_(StandardColor::White),
    chartPalette_(std::make_shared<WStandardPalette>(PaletteFlavour::Muted)),
    interface_(new WChart3DImplementation(this)),
    axisRenderWidth_(1024),
    axisRenderHeight_(256),
    gridRenderWidth_(512),
    textureScaling_(0),
    seriesCounter_(0),
    currentTopOffset_(0),
    currentBottomOffset_(0),
    currentLeftOffset_(0),
    currentRightOffset_(0),
    intersectionLinesEnabled_(false)
{
  XYGridEnabled_[0] = false; XYGridEnabled_[1] = false;
  XZGridEnabled_[0] = false; XZGridEnabled_[1] = false;
  YZGridEnabled_[0] = false; YZGridEnabled_[1] = false;

  xAxis_->init(interface_.get(), Axis::X3D);
  yAxis_->init(interface_.get(), Axis::Y3D);
  zAxis_->init(interface_.get(), Axis::Z3D);

  titleFont_.setFamily(FontFamily::SansSerif);
  titleFont_.setSize(WLength(15, LengthUnit::Point));

  addJavaScriptMatrix4(jsMatrix_);
}

WCartesian3DChart::WCartesian3DChart(ChartType type)
  : isViewSet_(false),
    xAxis_(std::make_unique<WAxis>()),
    yAxis_(std::make_unique<WAxis>()),
    zAxis_(std::make_unique<WAxis>()),
    chartType_(type),
    background_(StandardColor::White),
    chartPalette_(std::make_shared<WStandardPalette>(PaletteFlavour::Muted)),
    interface_(new WChart3DImplementation(this)),
    axisRenderWidth_(1024),
    axisRenderHeight_(256),
    gridRenderWidth_(512),
    textureScaling_(0),
    seriesCounter_(0),
    currentTopOffset_(0),
    currentBottomOffset_(0),
    currentLeftOffset_(0),
    currentRightOffset_(0),
    intersectionLinesEnabled_(false)
{
  XYGridEnabled_[0] = false; XYGridEnabled_[1] = false;
  XZGridEnabled_[0] = false; XZGridEnabled_[1] = false;
  YZGridEnabled_[0] = false; YZGridEnabled_[1] = false;

  xAxis_->init(interface_.get(), Axis::X3D);
  yAxis_->init(interface_.get(), Axis::Y3D);
  zAxis_->init(interface_.get(), Axis::Z3D);

  titleFont_.setFamily(FontFamily::SansSerif);
  titleFont_.setSize(WLength(15, LengthUnit::Point));

  addJavaScriptMatrix4(jsMatrix_);
}

WCartesian3DChart::~WCartesian3DChart()
{ }

void WCartesian3DChart
::addDataSeries(std::unique_ptr<WAbstractDataSeries3D> dataseries_)
{
  auto dataseries = dataseries_.get();
  // add it
  dataSeriesVector_.push_back(std::move(dataseries_));
  dataseries->setChart(this);

  if (dataseries->title().empty())
    dataseries->setDefaultTitle(++seriesCounter_);

  updateChart(ChartUpdates::GLContext | ChartUpdates::GLTextures);
}

std::unique_ptr<WAbstractDataSeries3D> WCartesian3DChart
::removeDataSeries(WAbstractDataSeries3D * dataseries)
{
  auto result = Wt::Utils::take(dataSeriesVector_, dataseries);

  std::vector<cpp17::any> glObjects = dataseries->getGlObjects();
  objectsToDelete.reserve(glObjects.size());
  for (std::size_t i = 0; i < glObjects.size(); ++i) {
    objectsToDelete.push_back(glObjects[i]);
  }

  updateChart(ChartUpdates::GLContext | ChartUpdates::GLTextures);

  return result;
}

std::vector<WAbstractDataSeries3D *> WCartesian3DChart::dataSeries() const
{
  std::vector<WAbstractDataSeries3D *> result;
  result.reserve(dataSeriesVector_.size());

  for (const auto& d : dataSeriesVector_)
    result.push_back(d.get());

  return result;
}

WAxis& WCartesian3DChart::axis(Axis axis)
{
  if (axis == Axis::X3D) {
    return *xAxis_;
  } else if (axis == Axis::Y3D) {
    return *yAxis_;
  } else if (axis == Axis::Z3D) {
    return *zAxis_;
  } else {
    throw WException("WCartesian3DChart: don't know this type of axis");
  }
}

void WCartesian3DChart::setAxis(std::unique_ptr<WAxis> waxis, Axis axis)
{
  if (axis == Axis::X3D) {
    xAxis_ = std::move(waxis);
    xAxis_->init(interface_.get(), Axis::X3D);
  } else if (axis == Axis::Y3D) {
    yAxis_ = std::move(waxis);
    yAxis_->init(interface_.get(), Axis::Y3D);
  } else if (axis == Axis::Z3D) {
    zAxis_ = std::move(waxis);
    zAxis_->init(interface_.get(), Axis::Z3D);
  } else {
    throw WException("WCartesian3DChart: don't know this type of axis");
  }
}

void WCartesian3DChart::setGridEnabled(Plane plane, Axis axis, bool enabled)
{
  switch (plane) {
  case Plane::XY:
    if (axis == Axis::X3D)
      XYGridEnabled_[0] = enabled;
    if (axis == Axis::Y3D)
      XYGridEnabled_[1] = enabled;
    break;
  case Plane::XZ:
    if (axis == Axis::X3D)
      XZGridEnabled_[0] = enabled;
    if (axis == Axis::Z3D)
      XZGridEnabled_[1] = enabled;
    break;
  case Plane::YZ:
    if (axis == Axis::Y3D)
      YZGridEnabled_[0] = enabled;
    if (axis == Axis::Z3D)
      YZGridEnabled_[1] = enabled;
    break;
  }

  updateChart(ChartUpdates::GLContext | ChartUpdates::GLTextures);
}

void WCartesian3DChart::setGridLinesPen(const WPen &pen)
{
  gridLinesPen_ = pen;

  updateChart(ChartUpdates::GLContext | ChartUpdates::GLTextures);
}

void WCartesian3DChart::setCubeLinesPen(const WPen &pen)
{
  cubeLinesPen_ = pen;

  updateChart(ChartUpdates::GLContext);
}

void WCartesian3DChart::setIntersectionLinesEnabled(bool enabled)
{
  intersectionLinesEnabled_ = enabled;
  updateChart(ChartUpdates::GLContext | ChartUpdates::GLTextures);
}

void WCartesian3DChart::setIntersectionLinesColor(WColor color)
{
  intersectionLinesColor_ = color;
  repaintGL(GLClientSideRenderer::PAINT_GL);
}

void WCartesian3DChart::setIntersectionPlanes(const std::vector<WCartesian3DChart::IntersectionPlane> &intersectionPlanes)
{
  intersectionPlanes_ = intersectionPlanes;
  updateChart(ChartUpdates::GLContext | ChartUpdates::GLTextures);
}

const std::vector<WCartesian3DChart::IntersectionPlane> &WCartesian3DChart::intersectionPlanes() const
{
  return intersectionPlanes_;
}

void WCartesian3DChart::setTitle(const WString &title)
{
  title_ = title;
  updateChart(ChartUpdates::GLTextures);
}

void WCartesian3DChart::setTitleFont(const WFont &titleFont)
{
  titleFont_ = titleFont;
  updateChart(ChartUpdates::GLTextures);
}

void WCartesian3DChart::setBackground(const WColor &background)
{
  background_ = background;

  updateChart(ChartUpdates::GLContext | ChartUpdates::GLTextures);
}

void WCartesian3DChart
::setPalette(const std::shared_ptr<WChartPalette>& palette)
{
  chartPalette_ = palette;

  updateChart(ChartUpdates::GLContext | ChartUpdates::GLTextures);
}

void WCartesian3DChart::setType(ChartType type)
{
  chartType_ = type;

  xAxis_->init(interface_.get(), Axis::X3D);
  yAxis_->init(interface_.get(), Axis::Y3D);
  zAxis_->init(interface_.get(), Axis::Z3D);
}

void WCartesian3DChart::setLegendEnabled(bool enabled)
{
  legend_.setLegendEnabled(enabled);
  updateChart(ChartUpdates::GLTextures);
}

void WCartesian3DChart::setLegendLocation(Side side, AlignmentFlag alignment)
{
  legend_.setLegendLocation(LegendLocation::Outside, side, alignment);
  updateChart(ChartUpdates::GLTextures);
}

void WCartesian3DChart::setLegendStyle(const WFont &font, const WPen &border,
				       const WBrush &background)
{
  legend_.setLegendStyle(font, border, background);
  updateChart(ChartUpdates::GLTextures);
}

void WCartesian3DChart::setLegendColumns(int columns,
					 const WLength &columnWidth)
{
  legend_.setLegendColumns(columns);
  legend_.setLegendColumnWidth(columnWidth);
  updateChart(ChartUpdates::GLTextures);
}

void WCartesian3DChart::initLayout()
{
  textureScaling_ = 1;
  int widgetWidth = (int)(width().value());
  while ( widgetWidth > 0 && widgetWidth < 1024 ) {
    textureScaling_ *= 2;
    widgetWidth *= 2;
  }

  int axisOffset = (int)(axisRenderWidth_/textureScaling_/1.6*0.3);
  int axisWidth = axisRenderWidth_/textureScaling_;

  xAxis_->prepareRender(Orientation::Horizontal, axisWidth-2*axisOffset);
  yAxis_->prepareRender(Orientation::Horizontal, axisWidth-2*axisOffset);
  zAxis_->prepareRender(Orientation::Vertical, axisWidth-2*axisOffset);
}

WMatrix4x4 WCartesian3DChart::cameraMatrix() const
{
  return jsMatrix_.value();
}

void WCartesian3DChart::setCameraMatrix(const WMatrix4x4& matrix)
{
  worldTransform_ = matrix;

  updateChart(ChartUpdates::CameraMatrix);
}

void WCartesian3DChart::createRay(double x, double y, WVector3 &eye, WVector3 &direction) const {
  WMatrix4x4 transform = pMatrix_ * cameraMatrix();
  WMatrix4x4 invTransform = transform;
#ifndef WT_TARGET_JAVA
  invTransform =
#endif
    invTransform.inverted();
  WVector4 near_(
      x / width().value() * 2 - 1,
      y / height().value() * (-2) + 1,
      -1.0,
      1.0
    );
   WVector4 far_(
      near_.x(),
      near_.y(),
      1.0,
      1.0
    );
  near_ = invTransform * near_;
  far_ =  invTransform * far_;
  near_ = near_ / near_.w();
  far_ = far_ / far_.w();
  WVector4 ray = far_ - near_;
  ray.normalize();
#ifndef WT_TARGET_JAVA
  direction = WVector3(ray.x(), ray.z(), ray.y());
  eye = WVector3(near_.x(), near_.z(), near_.y());
#else
  direction.setElement(0, ray.x());
  direction.setElement(1, ray.z());
  direction.setElement(2, ray.y());
  eye.setElement(0, near_.x());
  eye.setElement(1, near_.z());
  eye.setElement(2, near_.y());
#endif
}

void WCartesian3DChart::initializeGL()
{
  // code for client-side interaction (rotation through mouse movement)
  if (!isViewSet_) {
    worldTransform_.lookAt(
			  0.5, 0.5, 5, // camera position
			  0.5, 0.5, 0.5,      // looking at
			  0, 1, 0);        // 'up' vector

    // set up a angle-view at the scene
    worldTransform_.translate(0.5, 0.5, 0.5);
    worldTransform_.rotate(45.0, 0.0, 1.0, 0.0);
    worldTransform_.rotate(20.0, 1.0, 0.0, 1.0);
    worldTransform_.rotate(5.0, 0.0, 1.0, 0.0);
    worldTransform_.scale(1.8);
    worldTransform_.translate(-0.5, -0.5, -0.5);

    isViewSet_ = true;
  }

  if (!restoringContext()) {
    initJavaScriptMatrix4(jsMatrix_);

    setClientSideLookAtHandler(jsMatrix_, // the name of the JS matrix
	0.5, 0.5, 0.5,                       // the center point
	0, 1, 0,                          // the up direction
	0.005, 0.005);                    // 'speed' factors
  }

  // perspective matrix
  // pMatrix_.perspective(45.0, 1.0, 0.1, 100);
  double ratio = height().value()/width().value();
  pMatrix_.ortho(-2, 2, -2*ratio, 2*ratio, -100, 100);

  disable(DEPTH_TEST);
  cullFace(BACK);
  enable(CULL_FACE);

  int w = (int)width().value();
  int h = (int)height().value();
  viewport(0, 0, w < 0 ? 0 : w, h < 0 ? 0 : h);


  // Do some things related to the textures that overlay the entire scene
  // (eg. title, legend, colormaps ...)
  init2DShaders();

  // vertex data
  const float vertexPos[12] = {
    -1, 1, 0,
    1, 1, 0,
    -1, -1, 0,
    1, -1, 0
  };
  const float texCo[] = {
    0, 1,
    1, 1,
    0, 0,
    1, 0
  };

  // create buffers
  overlayPosBuffer_ = createBuffer();
  bindBuffer(ARRAY_BUFFER, overlayPosBuffer_);
#ifndef WT_TARGET_JAVA
  int size = sizeof(vertexPos)/sizeof(*vertexPos);
#else
  int size = Utils::sizeofFunction(vertexPos);
#endif
  FloatBuffer vertexPosBuf = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    vertexPosBuf.push_back(vertexPos[i]);
  }
  bufferDatafv(ARRAY_BUFFER, vertexPosBuf, STATIC_DRAW);

  overlayTexCoBuffer_ = createBuffer();
  bindBuffer(ARRAY_BUFFER, overlayTexCoBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(texCo)/sizeof(*texCo);
#else
  size = Utils::sizeofFunction(texCo);
#endif
  FloatBuffer texCoBuf = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    texCoBuf.push_back(texCo[i]);
  }
  bufferDatafv(ARRAY_BUFFER, texCoBuf, STATIC_DRAW);

  clippingPlaneVertBuffer_ = createBuffer();
  bindBuffer(ARRAY_BUFFER, clippingPlaneVertBuffer_);
  FloatBuffer clippingPlaneBuf = Utils::createFloatBuffer(8);
  clippingPlaneBuf.push_back(-1.0f);
  clippingPlaneBuf.push_back(-1.0f);
  clippingPlaneBuf.push_back(-1.0f);
  clippingPlaneBuf.push_back(2.0f);
  clippingPlaneBuf.push_back(2.0f);
  clippingPlaneBuf.push_back(-1.0f);
  clippingPlaneBuf.push_back(2.0f);
  clippingPlaneBuf.push_back(2.0f);
  bufferDatafv(ARRAY_BUFFER, clippingPlaneBuf, STATIC_DRAW);

  for (unsigned i = 0; i < dataSeriesVector_.size(); i++) {
    dataSeriesVector_[i]->initializeGL();
  }

  if (restoringContext()) {
    updateChart(ChartUpdates::GLContext | ChartUpdates::GLTextures);
  } else {
    updateChart(ChartUpdates::GLContext | ChartUpdates::GLTextures | 
		ChartUpdates::CameraMatrix);
  }
}

void WCartesian3DChart::initializePlotCube()
{
  // Cube buffers
  cubeBuffer_ = createBuffer();
  bindBuffer(ARRAY_BUFFER, cubeBuffer_);
#ifndef WT_TARGET_JAVA
  int size = sizeof(cubeData)/sizeof(*cubeData);
#else
  int size = Utils::sizeofFunction(cubeData);
#endif
  cubeData_ = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    cubeData_.push_back(cubeData[i]);
  }
  bufferDatafv(ARRAY_BUFFER, cubeData_, STATIC_DRAW);
  cubeNormalsBuffer_ = createBuffer();
  bindBuffer(ARRAY_BUFFER, cubeNormalsBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(cubePlaneNormals)/sizeof(*cubePlaneNormals);
#else
  size = Utils::sizeofFunction(cubePlaneNormals);
#endif
  cubeNormalsData_ = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    cubeNormalsData_.push_back(cubePlaneNormals[i]);
  }
  bufferDatafv(ARRAY_BUFFER, cubeNormalsData_, STATIC_DRAW);
  cubeIndicesBuffer_ = createBuffer();
  bindBuffer(ELEMENT_ARRAY_BUFFER, cubeIndicesBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(cubeIndices)/sizeof(*cubeIndices);
#else
  size = Utils::sizeofFunction(cubeIndices);
#endif
  cubeIndices_ = Utils::createIntBuffer(size);
  for (int i=0; i < size; i++) {
    cubeIndices_.push_back(cubeIndices[i]);
  }
  bufferDataiv(ELEMENT_ARRAY_BUFFER, cubeIndices_, STATIC_DRAW, UNSIGNED_SHORT);
  // cube line buffers
  cubeLineNormalsBuffer_ = createBuffer();
  bindBuffer(ARRAY_BUFFER, cubeLineNormalsBuffer_);
  #ifndef WT_TARGET_JAVA
  size = sizeof(cubeLineNormals)/sizeof(*cubeLineNormals);
#else
  size = Utils::sizeofFunction(cubeLineNormals);
#endif
  FloatBuffer cubeLineNormalsArray = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    cubeLineNormalsArray.push_back(cubeLineNormals[i]);
  }
  bufferDatafv(ARRAY_BUFFER, cubeLineNormalsArray, STATIC_DRAW);
  cubeLineIndicesBuffer_ = createBuffer();
  bindBuffer(ELEMENT_ARRAY_BUFFER, cubeLineIndicesBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(cubeLineIndices)/sizeof(*cubeLineIndices);
#else
  size = Utils::sizeofFunction(cubeLineIndices);
#endif
  cubeLineIndices_ = Utils::createIntBuffer(size);
  for (int i=0; i < size; i++) {
    cubeLineIndices_.push_back(cubeLineIndices[i]);
  }
  bufferDataiv(ELEMENT_ARRAY_BUFFER, cubeLineIndices_, STATIC_DRAW, UNSIGNED_SHORT);

  // Axis slab buffers (horizontal and vertical separated)
  axisBuffer_ = createBuffer(); // vertices
  bindBuffer(ARRAY_BUFFER, axisBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(axisSlabData)/sizeof(*axisSlabData);
#else
  size = Utils::sizeofFunction(axisSlabData);
#endif
  axisSlabData_ = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    axisSlabData_.push_back(axisSlabData[i]);
  }
  bufferDatafv(ARRAY_BUFFER, axisSlabData_, STATIC_DRAW);
  axisIndicesBuffer_ = createBuffer(); // indices
  bindBuffer(ELEMENT_ARRAY_BUFFER, axisIndicesBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(axisSlabIndices)/sizeof(*axisSlabIndices);
#else
  size = Utils::sizeofFunction(axisSlabIndices);
#endif
  axisSlabIndices_ = Utils::createIntBuffer(size);
  for (int i=0; i < size; i++) {
    axisSlabIndices_.push_back(axisSlabIndices[i]);
  }
  bufferDataiv(ELEMENT_ARRAY_BUFFER, axisSlabIndices_, STATIC_DRAW, UNSIGNED_SHORT);
  axisInPlaneBuffer_ = createBuffer(); // in-plane booleans
  bindBuffer(ARRAY_BUFFER, axisInPlaneBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(axisInPlaneBools)/sizeof(*axisInPlaneBools);
#else
  size = Utils::sizeofFunction(axisInPlaneBools);
#endif
  axisInPlaneBools_ = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    axisInPlaneBools_.push_back(axisInPlaneBools[i]);
  }
  bufferDatafv(ARRAY_BUFFER, axisInPlaneBools_, STATIC_DRAW);
  axisPlaneNormalBuffer_ = createBuffer(); // plane normals
  bindBuffer(ARRAY_BUFFER, axisPlaneNormalBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(axisPlaneNormal)/sizeof(*axisPlaneNormal);
#else
  size = Utils::sizeofFunction(axisPlaneNormal);
#endif
  axisPlaneNormal_ = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    axisPlaneNormal_.push_back(axisPlaneNormal[i]);
  }
  bufferDatafv(ARRAY_BUFFER, axisPlaneNormal_, STATIC_DRAW);
  axisOutOfPlaneNormalBuffer_ = createBuffer(); // out-of-plane normals
  bindBuffer(ARRAY_BUFFER, axisOutOfPlaneNormalBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(axisOutOfPlaneNormal)/sizeof(*axisOutOfPlaneNormal);
#else
  size = Utils::sizeofFunction(axisOutOfPlaneNormal);
#endif
  axisOutOfPlaneNormal_ = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    axisOutOfPlaneNormal_.push_back(axisOutOfPlaneNormal[i]);
  }
  bufferDatafv(ARRAY_BUFFER, axisOutOfPlaneNormal_, STATIC_DRAW);

  // vertical axis flaps
  axisVertBuffer_ = createBuffer(); // vertices
  bindBuffer(ARRAY_BUFFER, axisVertBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(axisSlabDataVertical)/sizeof(*axisSlabDataVertical);
#else
  size = Utils::sizeofFunction(axisSlabDataVertical);
#endif
  axisSlabDataVert_ = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    axisSlabDataVert_.push_back(axisSlabDataVertical[i]);
  }
  bufferDatafv(ARRAY_BUFFER, axisSlabDataVert_, STATIC_DRAW);
  axisIndicesVertBuffer_ = createBuffer(); // indices
  bindBuffer(ELEMENT_ARRAY_BUFFER, axisIndicesVertBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(axisSlabIndicesVertical)/sizeof(*axisSlabIndicesVertical);
#else
  size = Utils::sizeofFunction(axisSlabIndicesVertical);
#endif
  axisSlabIndicesVert_ = Utils::createIntBuffer(size);
  for (int i=0; i < size; i++) {
    axisSlabIndicesVert_.push_back(axisSlabIndicesVertical[i]);
  }
  bufferDataiv(ELEMENT_ARRAY_BUFFER, axisSlabIndicesVert_, STATIC_DRAW, UNSIGNED_SHORT);
  axisInPlaneVertBuffer_ = createBuffer(); // in-plane booleans
  bindBuffer(ARRAY_BUFFER, axisInPlaneVertBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(axisInPlaneBoolsVertical)/sizeof(*axisInPlaneBoolsVertical);
#else
  size = Utils::sizeofFunction(axisInPlaneBoolsVertical);
#endif
  axisInPlaneBoolsVert_ = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    axisInPlaneBoolsVert_.push_back(axisInPlaneBoolsVertical[i]);
  }
  bufferDatafv(ARRAY_BUFFER, axisInPlaneBoolsVert_, STATIC_DRAW);
  axisPlaneNormalVertBuffer_ = createBuffer(); // plane normals
  bindBuffer(ARRAY_BUFFER, axisPlaneNormalVertBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(axisPlaneNormalVertical)/sizeof(*axisPlaneNormalVertical);
#else
  size = Utils::sizeofFunction(axisPlaneNormalVertical);
#endif
  axisPlaneNormalVert_ = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    axisPlaneNormalVert_.push_back(axisPlaneNormalVertical[i]);
  }
  bufferDatafv(ARRAY_BUFFER, axisPlaneNormalVert_, STATIC_DRAW);
  axisOutOfPlaneNormalVertBuffer_ = createBuffer(); // out-of-plane normals
  bindBuffer(ARRAY_BUFFER, axisOutOfPlaneNormalVertBuffer_);
#ifndef WT_TARGET_JAVA
  size = sizeof(axisOutOfPlaneNormalVertical)/sizeof(*axisOutOfPlaneNormalVertical);
#else
  size = Utils::sizeofFunction(axisOutOfPlaneNormalVertical);
#endif
  axisOutOfPlaneNormalVert_ = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    axisOutOfPlaneNormalVert_.push_back(axisOutOfPlaneNormalVertical[i]);
  }
  bufferDatafv(ARRAY_BUFFER, axisOutOfPlaneNormalVert_, STATIC_DRAW);


  // initialize shaders
  fragmentShader_ = createShader(FRAGMENT_SHADER);
  shaderSource(fragmentShader_, cubeFragmentShaderSrc);
  compileShader(fragmentShader_);
  vertexShader_ = createShader(VERTEX_SHADER);
  shaderSource(vertexShader_, cubeVertexShaderSrc);
  compileShader(vertexShader_);
  fragmentShader2_ = createShader(FRAGMENT_SHADER);
  shaderSource(fragmentShader2_, axisFragmentShaderSrc);
  compileShader(fragmentShader2_);
  vertexShader2_ = createShader(VERTEX_SHADER);
  shaderSource(vertexShader2_, axisVertexShaderSrc);
  compileShader(vertexShader2_);
  cubeLineFragShader_ = createShader(FRAGMENT_SHADER);
  shaderSource(cubeLineFragShader_, cubeLineFragmentShaderSrc);;
  compileShader(cubeLineFragShader_);
  cubeLineVertShader_ = createShader(VERTEX_SHADER);
  shaderSource(cubeLineVertShader_, cubeLineVertexShaderSrc);
  compileShader(cubeLineVertShader_);

  cubeProgram_ = createProgram();
  cubeLineProgram_ = createProgram();
  axisProgram_ = createProgram();
  attachShader(cubeProgram_, vertexShader_);
  attachShader(cubeProgram_, fragmentShader_);
  attachShader(cubeLineProgram_, cubeLineVertShader_);
  attachShader(cubeLineProgram_, cubeLineFragShader_);
  attachShader(axisProgram_, vertexShader2_);
  attachShader(axisProgram_, fragmentShader2_);
  linkProgram(cubeProgram_);
  linkProgram(cubeLineProgram_);
  linkProgram(axisProgram_);

  // get attributes and uniform variables from shaders
  cube_vertexPositionAttribute_ =
    getAttribLocation(cubeProgram_, "aVertexPosition");
  cube_planeNormalAttribute_ =
    getAttribLocation(cubeProgram_, "aPlaneNormal");
  cube_textureCoordAttribute_ =
    getAttribLocation(cubeProgram_, "aTextureCo");

  cubeLine_vertexPositionAttribute_ =
    getAttribLocation(cubeLineProgram_, "aVertexPosition");
  cubeLine_normalAttribute_ =
    getAttribLocation(cubeLineProgram_, "aNormal");

  axis_vertexPositionAttribute_ =
    getAttribLocation(axisProgram_, "aVertexPosition");
  axis_textureCoordAttribute_ =
    getAttribLocation(axisProgram_, "aTextureCo");
  axis_inPlaneAttribute_ =
    getAttribLocation(axisProgram_, "aInPlane");
  axis_planeNormalAttribute_ =
    getAttribLocation(axisProgram_, "aPlaneNormal");
  axis_outOfPlaneNormalAttribute_ =
    getAttribLocation(axisProgram_, "aOutOfPlaneNormal");

  cube_pMatrixUniform_ = getUniformLocation(cubeProgram_, "uPMatrix");
  cube_mvMatrixUniform_ = getUniformLocation(cubeProgram_, "uMVMatrix");
  cube_cMatrixUniform_ = getUniformLocation(cubeProgram_, "uCMatrix");
  cube_texSampler1Uniform_ = getUniformLocation(cubeProgram_, "uSampler1");
  cube_texSampler2Uniform_ = getUniformLocation(cubeProgram_, "uSampler2");
  cube_texSampler3Uniform_ = getUniformLocation(cubeProgram_, "uSampler3");

  cubeLine_pMatrixUniform_ = getUniformLocation(cubeLineProgram_, "uPMatrix");
  cubeLine_mvMatrixUniform_ = getUniformLocation(cubeLineProgram_, "uMVMatrix");
  cubeLine_cMatrixUniform_ = getUniformLocation(cubeLineProgram_, "uCMatrix");
  cubeLine_nMatrixUniform_ = getUniformLocation(cubeLineProgram_, "uNMatrix");
  cubeLine_colorUniform_ = getUniformLocation(cubeLineProgram_, "uColor");

  axis_pMatrixUniform_ = getUniformLocation(axisProgram_, "uPMatrix");
  axis_mvMatrixUniform_ = getUniformLocation(axisProgram_, "uMVMatrix");
  axis_cMatrixUniform_ = getUniformLocation(axisProgram_, "uCMatrix");
  axis_nMatrixUniform_  = getUniformLocation(axisProgram_, "uNMatrix");
  axis_normalAngleTextureUniform_ = getUniformLocation(axisProgram_,
						       "uNormalAngleTexture");
  axis_texSamplerUniform_ = getUniformLocation(axisProgram_, "uSampler");

  // load textures
  loadCubeTextures();

  // fill all texture related buffers
  axisTexCoordsHoriz_ = createBuffer();
  bindBuffer(ARRAY_BUFFER, axisTexCoordsHoriz_);
#ifndef WT_TARGET_JAVA
  size = sizeof(axisTexCo)/sizeof(*axisTexCo);
#else
  size = Utils::sizeofFunction(axisTexCo);
#endif
  axisTexCo_ = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    axisTexCo_.push_back(axisTexCo[i]);
  }
  bufferDatafv(ARRAY_BUFFER, axisTexCo_, STATIC_DRAW);

  axisTexCoordsVert_ = createBuffer();
  bindBuffer(ARRAY_BUFFER, axisTexCoordsVert_);
#ifndef WT_TARGET_JAVA
  size = sizeof(axisTexCoVertical)/sizeof(*axisTexCoVertical);
#else
  size = Utils::sizeofFunction(axisTexCoVertical);
#endif
  axisTexCoVert_ = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    axisTexCoVert_.push_back(axisTexCoVertical[i]);
  }
  bufferDatafv(ARRAY_BUFFER, axisTexCoVert_, STATIC_DRAW);

  cubeTexCoords_ = createBuffer();
  bindBuffer(ARRAY_BUFFER, cubeTexCoords_);
#ifndef WT_TARGET_JAVA
  size = sizeof(cubeTexCo)/sizeof(*cubeTexCo);
#else
  size = Utils::sizeofFunction(cubeTexCo);
#endif
  cubeTexCo_ = Utils::createFloatBuffer(size);
  for (int i=0; i < size; i++) {
    cubeTexCo_.push_back(cubeTexCo[i]);
  }
  bufferDatafv(ARRAY_BUFFER, cubeTexCo_, STATIC_DRAW);

  // set projection matrix uniforms
  useProgram(cubeProgram_);
  uniformMatrix4(cube_pMatrixUniform_, pMatrix_);
  useProgram(cubeLineProgram_);
  uniformMatrix4(cubeLine_pMatrixUniform_, pMatrix_);
  useProgram(axisProgram_);
  uniformMatrix4(axis_pMatrixUniform_, pMatrix_);
}

void WCartesian3DChart::initializeIntersectionLinesProgram()
{
  intersectionLinesFragmentShader_ = createShader(FRAGMENT_SHADER);
  shaderSource(intersectionLinesFragmentShader_, intersectionLinesFragmentShaderSrc);
  compileShader(intersectionLinesFragmentShader_);

  intersectionLinesProgram_ = createProgram();
  attachShader(intersectionLinesProgram_, vertexShader2D_);
  attachShader(intersectionLinesProgram_, intersectionLinesFragmentShader_);
  linkProgram(intersectionLinesProgram_);

  useProgram(intersectionLinesProgram_);
  intersectionLines_vertexPositionAttribute_ =
    getAttribLocation(intersectionLinesProgram_, "aVertexPosition");
  intersectionLines_vertexTextureCoAttribute_ =
    getAttribLocation(intersectionLinesProgram_, "aTextureCo");
  intersectionLines_viewportWidthUniform_ =
    getUniformLocation(intersectionLinesProgram_, "uVPwidth");
  intersectionLines_viewportHeightUniform_ =
    getUniformLocation(intersectionLinesProgram_, "uVPheight");
  intersectionLines_cameraUniform_ =
    getUniformLocation(intersectionLinesProgram_, "uCamera");
  intersectionLines_colorUniform_ =
    getUniformLocation(intersectionLinesProgram_, "uColor");
  intersectionLines_positionSamplerUniform_ =
    getUniformLocation(intersectionLinesProgram_, "uPositionSampler");
  intersectionLines_meshIndexSamplerUniform_ =
    getUniformLocation(intersectionLinesProgram_, "uMeshIndexSampler");
}

void WCartesian3DChart::initializeClippingPlaneProgram()
{
  clippingPlaneFragShader_ = createShader(FRAGMENT_SHADER);
  shaderSource(clippingPlaneFragShader_, clippingPlaneFragShaderSrc);
  compileShader(clippingPlaneFragShader_);

  clippingPlaneVertexShader_ = createShader(VERTEX_SHADER);
  shaderSource(clippingPlaneVertexShader_, clippingPlaneVertexShaderSrc);
  compileShader(clippingPlaneVertexShader_);

  clippingPlaneProgram_ = createProgram();
  attachShader(clippingPlaneProgram_, clippingPlaneVertexShader_);
  attachShader(clippingPlaneProgram_, clippingPlaneFragShader_);
  linkProgram(clippingPlaneProgram_);

  useProgram(clippingPlaneProgram_);
  clippingPlane_vertexPositionAttribute_ =
    getAttribLocation(clippingPlaneProgram_, "aVertexPosition");
  clippingPlane_mvMatrixUniform_ =
    getUniformLocation(clippingPlaneProgram_, "uMVMatrix");
  clippingPlane_pMatrixUniform_ =
    getUniformLocation(clippingPlaneProgram_, "uPMatrix");
  clippingPlane_cMatrixUniform_ =
    getUniformLocation(clippingPlaneProgram_, "uCMatrix");
  clippingPlane_clipPtUniform_ =
    getUniformLocation(clippingPlaneProgram_, "uClipPt");
  clippingPlane_dataMinPtUniform_ =
    getUniformLocation(clippingPlaneProgram_, "uDataMinPt");
  clippingPlane_dataMaxPtUniform_ =
    getUniformLocation(clippingPlaneProgram_, "uDataMaxPt");
  clippingPlane_clippingAxis_ =
    getUniformLocation(clippingPlaneProgram_, "uClippingAxis");
  clippingPlane_drawPositionUniform_ =
    getUniformLocation(clippingPlaneProgram_, "uDrawPosition");
}

void WCartesian3DChart::loadCubeTextures()
{
  initLayout();

  // Two horizontal and one vertical axis texture
  std::unique_ptr<WPaintDevice> cpdHoriz0 
    = createPaintDevice(WLength(1024), WLength(8*256));
  std::unique_ptr<WPaintDevice> cpdHoriz1
    = createPaintDevice(WLength(1024), WLength(8*256));
  std::unique_ptr<WPaintDevice> cpdVert
    = createPaintDevice(WLength(2*256), WLength(1024));

  paintHorizAxisTextures(cpdHoriz0.get());
  paintHorizAxisTextures(cpdHoriz1.get(), true);
  paintVertAxisTextures(cpdVert.get());

  if (horizAxisTexture_.isNull())
    horizAxisTexture_ = createTexture();
  bindTexture(TEXTURE_2D, horizAxisTexture_);
  pixelStorei(UNPACK_FLIP_Y_WEBGL, 1);
  texImage2D(TEXTURE_2D, 0, RGBA, RGBA, UNSIGNED_BYTE, cpdHoriz0.get());
  texParameteri(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR_MIPMAP_LINEAR);
  generateMipmap(TEXTURE_2D);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_S,CLAMP_TO_EDGE);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_T,CLAMP_TO_EDGE);

  if (horizAxisTexture2_.isNull())
    horizAxisTexture2_ = createTexture();
  bindTexture(TEXTURE_2D, horizAxisTexture2_);
  pixelStorei(UNPACK_FLIP_Y_WEBGL, 1);
  texImage2D(TEXTURE_2D, 0, RGBA, RGBA, UNSIGNED_BYTE, cpdHoriz1.get());
  texParameteri(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR_MIPMAP_LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_S,CLAMP_TO_EDGE);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_T,CLAMP_TO_EDGE);
  generateMipmap(TEXTURE_2D);

  if (vertAxisTexture_.isNull())
    vertAxisTexture_ = createTexture();
  bindTexture(TEXTURE_2D, vertAxisTexture_);
  pixelStorei(UNPACK_FLIP_Y_WEBGL, 1);
  texImage2D(TEXTURE_2D, 0, RGBA, RGBA, UNSIGNED_BYTE, cpdVert.get());
  texParameteri(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR_MIPMAP_LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_S,CLAMP_TO_EDGE);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_T,CLAMP_TO_EDGE);
  generateMipmap(TEXTURE_2D);

  // All gridline textures
  std::unique_ptr<WPaintDevice> pd1 = createPaintDevice(512, 512);
  paintGridLines(pd1.get(), Plane::XY);
  cubeTextureXY_ = createTexture();
  bindTexture(TEXTURE_2D, cubeTextureXY_);
  pixelStorei(UNPACK_FLIP_Y_WEBGL, 1);
  texImage2D(TEXTURE_2D, 0, RGBA, RGBA, UNSIGNED_BYTE, pd1.get());
  texParameteri(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR_MIPMAP_LINEAR);
  generateMipmap(TEXTURE_2D);
  std::unique_ptr<WPaintDevice> pd2 = createPaintDevice(512, 512);
  paintGridLines(pd2.get(), Plane::XZ);
  cubeTextureXZ_ = createTexture();
  bindTexture(TEXTURE_2D, cubeTextureXZ_);
  pixelStorei(UNPACK_FLIP_Y_WEBGL, 1);
  texImage2D(TEXTURE_2D, 0, RGBA, RGBA, UNSIGNED_BYTE, pd2.get());
  texParameteri(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR_MIPMAP_LINEAR);
  generateMipmap(TEXTURE_2D);
  std::unique_ptr<WPaintDevice> pd3 = createPaintDevice(512, 512);
  paintGridLines(pd3.get(), Plane::YZ);
  cubeTextureYZ_ = createTexture();
  bindTexture(TEXTURE_2D, cubeTextureYZ_);
  pixelStorei(UNPACK_FLIP_Y_WEBGL, 1);
  texImage2D(TEXTURE_2D, 0, RGBA, RGBA, UNSIGNED_BYTE, pd3.get());
  texParameteri(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR_MIPMAP_LINEAR);
  generateMipmap(TEXTURE_2D);
}

void WCartesian3DChart::init2DShaders()
{
  fragmentShader2D_ = createShader(FRAGMENT_SHADER);
  shaderSource(fragmentShader2D_, fragmentShaderSrc2D);
  compileShader(fragmentShader2D_);
  vertexShader2D_ = createShader(VERTEX_SHADER);
  shaderSource(vertexShader2D_, vertexShaderSrc2D);
  compileShader(vertexShader2D_);

  textureProgram_ = createProgram();
  attachShader(textureProgram_, vertexShader2D_);
  attachShader(textureProgram_, fragmentShader2D_);
  linkProgram(textureProgram_);

  texture_vertexPositionAttribute_ =
    getAttribLocation(textureProgram_, "aVertexPosition");
  texture_vertexTextureCoAttribute_ =
    getAttribLocation(textureProgram_, "aTextureCo");
  texture_texSamplerUniform_ =
    getUniformLocation(textureProgram_, "uSampler");
}

void WCartesian3DChart::initTitle()
{
  if (title_.empty())
    return;

  float pixelHeight = (float)(titleFont_.sizeLength().toPixels() * 1.5);

  // paint texture
  std::unique_ptr<WPaintDevice> titlePaintDev
    = createPaintDevice(width(), height());

  WPainter painter(titlePaintDev.get());
  painter.setFont(titleFont_);
  painter.drawText(WRectF(0, 0, width().value(), pixelHeight),
		   WFlags<AlignmentFlag>(AlignmentFlag::Center) | AlignmentFlag::Middle, title_);
  painter.end();

  titleTexture_ = createTexture();
  bindTexture(TEXTURE_2D, titleTexture_);
  pixelStorei(UNPACK_FLIP_Y_WEBGL, 1);
  texImage2D(TEXTURE_2D, 0, RGBA, RGBA, UNSIGNED_BYTE, titlePaintDev.get());
  texParameteri(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_S, CLAMP_TO_EDGE);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_T, CLAMP_TO_EDGE);

  currentTopOffset_ += pixelHeight;
}

void WCartesian3DChart::initColorMaps()
{
  // paint the colormaps
  std::unique_ptr<WPaintDevice> colorMapPaintDev
    = createPaintDevice(width(), height());

  WPainter painter(colorMapPaintDev.get());
  painter.translate(0, currentTopOffset_);
  const int WIDTH = 100;
  int space = (int)(height().value())
    - currentTopOffset_ - currentBottomOffset_;
  const int VERT_MARGIN = (int)(space * 0.1);
  const int LEFT_OFFSET = 4;
  for (unsigned i = 0; i<dataSeriesVector_.size(); i++) {
    if (dataSeriesVector_[i]->colorMap() == nullptr ||
	dataSeriesVector_[i]->isHidden())
      continue;

    painter.save();
    if (dataSeriesVector_[i]->colorMapVisible()) {
      switch (dataSeriesVector_[i]->colorMapSide()) {
      case Side::Left:
	painter.translate(currentLeftOffset_, 0);
	dataSeriesVector_[i]->colorMap()->paintLegend
	  (&painter, WRectF(LEFT_OFFSET, VERT_MARGIN,
			    WIDTH, space - 2*VERT_MARGIN));
	currentLeftOffset_ += WIDTH;
	break;
      case Side::Right:
	painter.translate(width().value() - currentRightOffset_ - WIDTH, 0);
	dataSeriesVector_[i]->colorMap()->paintLegend
	  (&painter, WRectF(LEFT_OFFSET, VERT_MARGIN,
			    WIDTH, space - 2*VERT_MARGIN));
	currentRightOffset_ += WIDTH;
	break;
      default:
	throw WException("WCartesian3DChart: colormaps can only be put "
			 "left or right of the chart");
      }
    }
    painter.restore();
  }
  painter.end();

  colorMapTexture_ = createTexture();
  bindTexture(TEXTURE_2D, colorMapTexture_);
  pixelStorei(UNPACK_FLIP_Y_WEBGL, 1);
  texImage2D(TEXTURE_2D, 0, RGBA, RGBA, UNSIGNED_BYTE, colorMapPaintDev.get());
  texParameteri(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_S, CLAMP_TO_EDGE);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_T, CLAMP_TO_EDGE);
}

void WCartesian3DChart::initLegend()
{
  if (!legend_.isLegendEnabled())
    return;

  // paint the legend
  int MARGIN = 4;
  int legendWidth = legend_.width();
  int legendHeight = legend_.height(dataSeriesVector_);

  std::unique_ptr<WPaintDevice> legendPaintDev
    = createPaintDevice(width().value(), height().value());
  WPainter painter(legendPaintDev.get());

  // do some fancy moves based on Side and Alignment
  int space = 0;
  if (legend_.legendSide() == Side::Left) {
    space = (int)(height().value()) - currentTopOffset_ - currentBottomOffset_;
    painter.translate(currentLeftOffset_ + MARGIN, 0);
    currentLeftOffset_ += legendWidth + MARGIN;
  } else if (legend_.legendSide() == Side::Right) {
    space = (int)(height().value()) - currentTopOffset_ - currentBottomOffset_;
    painter.translate(width().value() 
		      - currentRightOffset_ - legendWidth - MARGIN, 0);
    currentRightOffset_ += legendWidth + MARGIN;
  } else if (legend_.legendSide() == Side::Top) {
    space = (int)(width().value()) - currentLeftOffset_ - currentRightOffset_;
    painter.translate(0, currentTopOffset_ + MARGIN);
    currentTopOffset_ += legendHeight + MARGIN;
  } else if (legend_.legendSide() == Side::Bottom) {
    space = (int)(width().value()) - currentLeftOffset_ - currentRightOffset_;
    painter.translate(0, height().value() 
		      - currentBottomOffset_ - legendHeight - MARGIN);
    currentBottomOffset_ += legendHeight + MARGIN;
  }

  if (legend_.legendSide() == Side::Left || legend_.legendSide() == Side::Right) {
    painter.translate(0, currentTopOffset_);
    switch (legend_.legendAlignment()) {
    case AlignmentFlag::Top:
      painter.translate(0, MARGIN);
      currentTopOffset_ += MARGIN;
      break;
    case AlignmentFlag::Middle:
      painter.translate(0, (space-legendHeight)/2);
      break;
    case AlignmentFlag::Bottom:
      painter.translate(0, space-legendHeight - MARGIN);
      currentBottomOffset_ += MARGIN;
      break;
    default:
      break; // legend-side does not match legend-alignment
    }
  }
  if (legend_.legendSide() == Side::Top || 
      legend_.legendSide() == Side::Bottom) {
    painter.translate(currentLeftOffset_, 0);
    space = (int)(width().value()) - currentLeftOffset_ - currentRightOffset_;
    switch (legend_.legendAlignment()) {
    case AlignmentFlag::Left:
      painter.translate(MARGIN, 0);
      currentLeftOffset_ += MARGIN;
      break;
    case AlignmentFlag::Center:
      painter.translate((space-legendWidth)/2, 0);
      break;
    case AlignmentFlag::Right:
      painter.translate(space-legendWidth - MARGIN, 0);
      currentRightOffset_ += MARGIN;
      break;
    default:
      break; // legend-side does not match legend-alignment
    }
  }

  legend_.renderLegend(&painter, dataSeriesVector_);
  painter.end();

  legendTexture_ = createTexture();
  bindTexture(TEXTURE_2D, legendTexture_);
  pixelStorei(UNPACK_FLIP_Y_WEBGL, 1);
  texImage2D(TEXTURE_2D, 0, RGBA, RGBA, UNSIGNED_BYTE, legendPaintDev.get());
  texParameteri(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_S, CLAMP_TO_EDGE);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_T, CLAMP_TO_EDGE);
}

void WCartesian3DChart::paintGL()
{
  using Wt::operator|;

  clearColor(background_.red()/255.0, background_.green()/255.0,
	     background_.blue()/255.0, background_.alpha()/255.0);
  clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
  enable(BLEND);
  blendFunc(SRC_ALPHA,ONE_MINUS_SRC_ALPHA);
  enable(CULL_FACE);

  WMatrix4x4 mvMatrix;

  // Draw the cube
  useProgram(cubeProgram_);
  //uniformMatrix4(cube_pMatrixUniform_, pMatrix_);
  uniformMatrix4(cube_mvMatrixUniform_, mvMatrix);
  uniformMatrix4(cube_cMatrixUniform_, jsMatrix_);

  bindBuffer(ARRAY_BUFFER, cubeBuffer_);
  vertexAttribPointer(cube_vertexPositionAttribute_,
  		      3,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(cube_vertexPositionAttribute_);
  bindBuffer(ARRAY_BUFFER, cubeNormalsBuffer_);
  vertexAttribPointer(cube_planeNormalAttribute_,
  		      3,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(cube_planeNormalAttribute_);
  bindBuffer(ARRAY_BUFFER, cubeTexCoords_);
  vertexAttribPointer(cube_textureCoordAttribute_,
		      2,
		      FLOAT,
		      false,
		      0,
		      0);
  enableVertexAttribArray(cube_textureCoordAttribute_);
  activeTexture(TEXTURE0);
  bindTexture(TEXTURE_2D, cubeTextureXY_);
  uniform1i(cube_texSampler1Uniform_, 0);
  activeTexture(TEXTURE1);
  bindTexture(TEXTURE_2D, cubeTextureXZ_);
  uniform1i(cube_texSampler2Uniform_, 1);
  activeTexture(TEXTURE2);
  bindTexture(TEXTURE_2D, cubeTextureYZ_);
  uniform1i(cube_texSampler3Uniform_, 2);
  enable(POLYGON_OFFSET_FILL);
  float unitOffset = (float) (std::pow(5,gridLinesPen_.width().value()) < 10000 ? std::pow(5,gridLinesPen_.width().value()) : 10000);
  polygonOffset(1, unitOffset);
  bindBuffer(ELEMENT_ARRAY_BUFFER, cubeIndicesBuffer_);
  drawElements(TRIANGLES, cubeIndices_.size(), UNSIGNED_SHORT, 0);
  disableVertexAttribArray(cube_vertexPositionAttribute_);
  disableVertexAttribArray(cube_textureCoordAttribute_);
  disable(POLYGON_OFFSET_FILL);

  // Draw the cube-lines
  useProgram(cubeLineProgram_);
  //  uniformMatrix4(cubeLine_pMatrixUniform_, pMatrix_);
  uniformMatrix4(cubeLine_mvMatrixUniform_, mvMatrix);
  uniformMatrix4(cubeLine_cMatrixUniform_, jsMatrix_);
#ifndef WT_TARGET_JAVA
  uniformMatrix4(cubeLine_nMatrixUniform_,
  		 (jsMatrix_ * mvMatrix)); //.inverted().transposed()
#else
  uniformMatrix4(cubeLine_nMatrixUniform_,
		 jsMatrix_.multiply(mvMatrix));
#endif
  uniform4f(cubeLine_colorUniform_,
	    (float)cubeLinesPen_.color().red(),
	    (float)cubeLinesPen_.color().green(),
	    (float)cubeLinesPen_.color().blue(),
	    (float)cubeLinesPen_.color().alpha());
  bindBuffer(ARRAY_BUFFER, cubeBuffer_);
  vertexAttribPointer(cubeLine_vertexPositionAttribute_,
  		      3,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(cubeLine_vertexPositionAttribute_);
  bindBuffer(ARRAY_BUFFER, cubeLineNormalsBuffer_);
  vertexAttribPointer(cubeLine_normalAttribute_,
  		      3,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(cubeLine_normalAttribute_);
  lineWidth(cubeLinesPen_.width().value() == 0 ?
	    1.0 :
	    cubeLinesPen_.width().value());
  bindBuffer(ELEMENT_ARRAY_BUFFER, cubeLineIndicesBuffer_);
  drawElements(LINES, cubeLineIndices_.size(), UNSIGNED_SHORT, 0);
  disableVertexAttribArray(cubeLine_vertexPositionAttribute_);
  disableVertexAttribArray(cubeLine_normalAttribute_);

  // Draw the axis-slabs
  useProgram(axisProgram_);
  //  uniformMatrix4(axis_pMatrixUniform_, pMatrix_);
  uniformMatrix4(axis_mvMatrixUniform_, mvMatrix);
  uniformMatrix4(axis_cMatrixUniform_, jsMatrix_);
#ifndef WT_TARGET_JAVA
  uniformMatrix4(axis_nMatrixUniform_,
		 (jsMatrix_ * mvMatrix)); // .inverted().transposed()
#else
  uniformMatrix4(axis_nMatrixUniform_,
		 jsMatrix_.multiply(mvMatrix));
#endif

  // horizontal axes
  bindBuffer(ARRAY_BUFFER, axisBuffer_);
  vertexAttribPointer(axis_vertexPositionAttribute_,
  		      3,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(axis_vertexPositionAttribute_);
  bindBuffer(ARRAY_BUFFER, axisTexCoordsHoriz_);
  vertexAttribPointer(axis_textureCoordAttribute_,
  		      2,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(axis_textureCoordAttribute_);
  bindBuffer(ARRAY_BUFFER, axisInPlaneBuffer_);
  vertexAttribPointer(axis_inPlaneAttribute_,
  		      1,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(axis_inPlaneAttribute_);
  bindBuffer(ARRAY_BUFFER, axisPlaneNormalBuffer_);
  vertexAttribPointer(axis_planeNormalAttribute_,
  		      3,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(axis_planeNormalAttribute_);
  bindBuffer(ARRAY_BUFFER, axisOutOfPlaneNormalBuffer_);
  vertexAttribPointer(axis_outOfPlaneNormalAttribute_,
  		      3,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(axis_outOfPlaneNormalAttribute_);

  // first drawing (with lable-angles in one direction)
  uniform1i(axis_normalAngleTextureUniform_, 1);
  activeTexture(TEXTURE1);
  bindTexture(TEXTURE_2D, horizAxisTexture_);
  uniform1i(axis_texSamplerUniform_, 1);
  bindBuffer(ELEMENT_ARRAY_BUFFER, axisIndicesBuffer_);
  drawElements(TRIANGLES, axisSlabIndices_.size(), UNSIGNED_SHORT, 0);
  // second drawing (with lable-angles in the other direction)
  uniform1i(axis_normalAngleTextureUniform_, 0);
  activeTexture(TEXTURE1);
  bindTexture(TEXTURE_2D, horizAxisTexture2_);
  uniform1i(axis_texSamplerUniform_, 1);
  drawElements(TRIANGLES, axisSlabIndices_.size(), UNSIGNED_SHORT, 0);

  disableVertexAttribArray(axis_vertexPositionAttribute_);
  disableVertexAttribArray(axis_textureCoordAttribute_);
  disableVertexAttribArray(axis_inPlaneAttribute_);
  disableVertexAttribArray(axis_planeNormalAttribute_);
  disableVertexAttribArray(axis_outOfPlaneNormalAttribute_);

  // vertical axes
  bindBuffer(ARRAY_BUFFER, axisVertBuffer_);
  vertexAttribPointer(axis_vertexPositionAttribute_,
  		      3,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(axis_vertexPositionAttribute_);
  bindBuffer(ARRAY_BUFFER, axisTexCoordsVert_);
  vertexAttribPointer(axis_textureCoordAttribute_,
  		      2,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(axis_textureCoordAttribute_);
  bindBuffer(ARRAY_BUFFER, axisInPlaneVertBuffer_);
  vertexAttribPointer(axis_inPlaneAttribute_,
  		      1,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(axis_inPlaneAttribute_);
  bindBuffer(ARRAY_BUFFER, axisPlaneNormalVertBuffer_);
  vertexAttribPointer(axis_planeNormalAttribute_,
  		      3,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(axis_planeNormalAttribute_);
  bindBuffer(ARRAY_BUFFER, axisOutOfPlaneNormalVertBuffer_);
  vertexAttribPointer(axis_outOfPlaneNormalAttribute_,
  		      3,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(axis_outOfPlaneNormalAttribute_);
  activeTexture(TEXTURE1);
  bindTexture(TEXTURE_2D, vertAxisTexture_);
  uniform1i(axis_texSamplerUniform_, 1);

  bindBuffer(ELEMENT_ARRAY_BUFFER, axisIndicesVertBuffer_);
  drawElements(TRIANGLES, axisSlabIndicesVert_.size(), UNSIGNED_SHORT, 0);
  uniform1i(axis_normalAngleTextureUniform_, 1);
  drawElements(TRIANGLES, axisSlabIndicesVert_.size(), UNSIGNED_SHORT, 0);
  disableVertexAttribArray(axis_vertexPositionAttribute_);
  disableVertexAttribArray(axis_textureCoordAttribute_);
  disableVertexAttribArray(axis_inPlaneAttribute_);
  disableVertexAttribArray(axis_planeNormalAttribute_);
  disableVertexAttribArray(axis_outOfPlaneNormalAttribute_);

  disable(BLEND);
  // Paint all grid data with clipping lines
  for (unsigned i = 0; i < dataSeriesVector_.size(); i++) {
    WAbstractGridData *gridData
      = dynamic_cast<WAbstractGridData*>(dataSeriesVector_[i].get());
    if (gridData &&
	gridData->type() == Series3DType::Surface &&
	gridData->clippingLinesEnabled()) {
      gridData->paintGL();
      renderClippingLines(gridData);
      enable(BLEND);
      disable(CULL_FACE);
      disable(DEPTH_TEST);
      depthMask(false);
      if (!intersectionLinesTexture_.isNull())
	paintPeripheralTexture(overlayPosBuffer_, overlayTexCoBuffer_,
			       intersectionLinesTexture_);
      depthMask(true);
      disable(BLEND);
      enable(CULL_FACE);
      enable(DEPTH_TEST);
    }
  }
  // Paint all grid data without clipping lines
  for (unsigned i = 0; i < dataSeriesVector_.size(); i++) {
    WAbstractGridData *gridData
      = dynamic_cast<WAbstractGridData*>(dataSeriesVector_[i].get());
    if (gridData &&
	gridData->type() == Series3DType::Surface &&
	!gridData->clippingLinesEnabled()) {
      gridData->paintGL();
    }
  }
  // Paint all other data, if intersection lines are not enabled
  if (!intersectionLinesEnabled_ && intersectionPlanes_.empty()) {
    for (unsigned i = 0; i < dataSeriesVector_.size(); i++) {
      WAbstractGridData *gridData
	= dynamic_cast<WAbstractGridData*>(dataSeriesVector_[i].get());
      if (!(gridData && gridData->type() == Series3DType::Surface)) {
	dataSeriesVector_[i]->paintGL();
      }
    }
  }

  if (intersectionLinesEnabled_ || !intersectionPlanes_.empty()) {
    bindFramebuffer(FRAMEBUFFER, intersectionLinesFramebuffer_);
    clearColor(0.0, 0.0, 0.0, 0.0);
    clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
    bindFramebuffer(FRAMEBUFFER, Framebuffer());

    if (intersectionLinesEnabled_)
      renderIntersectionLines();
    if (!intersectionPlanes_.empty())
      renderIntersectionLinesWithInvisiblePlanes();

    enable(BLEND);
    disable(CULL_FACE);
    disable(DEPTH_TEST);

    depthMask(false);
    if (!intersectionLinesTexture_.isNull())
      paintPeripheralTexture(overlayPosBuffer_, overlayTexCoBuffer_,
			     intersectionLinesTexture_);
    depthMask(true);

    disable(BLEND);
    enable(CULL_FACE);
    enable(DEPTH_TEST);

    // Paint all other data
    for (unsigned i = 0; i < dataSeriesVector_.size(); i++) {
      WAbstractGridData *gridData
	= dynamic_cast<WAbstractGridData*>(dataSeriesVector_[i].get());
      if (!gridData || gridData->type() != Series3DType::Surface) {
	dataSeriesVector_[i]->paintGL();
      }
    }
  }

  enable(BLEND);
  disable(CULL_FACE);

  // draw peripheral textures
  if (!titleTexture_.isNull())
    paintPeripheralTexture(overlayPosBuffer_, overlayTexCoBuffer_,
			   titleTexture_);

  if (!legendTexture_.isNull())
    paintPeripheralTexture(overlayPosBuffer_, overlayTexCoBuffer_,
			   legendTexture_);

  if (!colorMapTexture_.isNull())
    paintPeripheralTexture(overlayPosBuffer_, overlayTexCoBuffer_,
			   colorMapTexture_);

  disable(BLEND);
}

void WCartesian3DChart::paintPeripheralTexture(const Buffer& pos,
					       const Buffer& texCo,
					       const Texture& texture)
{
  useProgram(textureProgram_);
  bindBuffer(ARRAY_BUFFER, pos);
  vertexAttribPointer(texture_vertexPositionAttribute_,
  		      3,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(texture_vertexPositionAttribute_);
  bindBuffer(ARRAY_BUFFER, texCo);
  vertexAttribPointer(texture_vertexTextureCoAttribute_,
  		      2,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(texture_vertexTextureCoAttribute_);
  activeTexture(TEXTURE0);
  bindTexture(TEXTURE_2D, texture);
  uniform1i(texture_texSamplerUniform_, 0);
  drawArrays(TRIANGLE_STRIP, 0, 4);
  disableVertexAttribArray(texture_vertexPositionAttribute_);
  disableVertexAttribArray(texture_vertexTextureCoAttribute_);
}

void WCartesian3DChart::updateGL()
{
  if (updates_.test(ChartUpdates::GLContext)) {
    deleteAllGLResources();
    for (unsigned i = 0; i < dataSeriesVector_.size(); i++) {
      dataSeriesVector_[i]->deleteAllGLResources();
    }

    clearColor(background_.red()/255.0, background_.green()/255.0,
	       background_.blue()/255.0, background_.alpha()/255.0);
    initializePlotCube();
    if (intersectionLinesEnabled_ || !intersectionPlanes_.empty()) {
      initializeIntersectionLinesProgram();
      initOffscreenBuffer();
    }
    for (unsigned i = 0; i < dataSeriesVector_.size(); i++) {
      WAbstractGridData *data
	= dynamic_cast<WAbstractGridData *>(dataSeriesVector_[i].get());
      if (data) {
	initializeClippingPlaneProgram();
	// NOTE: the ! is not an error, if intersectionLinesEnabled_ is true,
	//       the program was already initialized.
	if (!intersectionLinesEnabled_ && intersectionPlanes_.empty()) {
	  initializeIntersectionLinesProgram();
	  initOffscreenBuffer();
	}
	break;
      }
    }
    for (unsigned i = 0; i < dataSeriesVector_.size(); i++) {
      dataSeriesVector_[i]->updateGL();
    }

    repaintGL(GLClientSideRenderer::RESIZE_GL);
    repaintGL(GLClientSideRenderer::PAINT_GL);
  }

  if (updates_.test(ChartUpdates::CameraMatrix)) {
    setJavaScriptMatrix4(jsMatrix_, worldTransform_);

    repaintGL(GLClientSideRenderer::PAINT_GL);
  }

  if (updates_.test(ChartUpdates::GLTextures)) {
    deleteGLTextures();
    loadCubeTextures();

    currentTopOffset_ = 0; currentBottomOffset_ = 0;
    currentLeftOffset_ = 0; currentRightOffset_ = 0;
    initTitle();
    if (legend_.legendSide() == Side::Left) {
      initColorMaps();
      initLegend();
    } else if (legend_.legendSide() == Side::Right) {
      initLegend();
      initColorMaps();
    } else {
      initLegend();
      initColorMaps();
    }
    repaintGL(GLClientSideRenderer::PAINT_GL);
  }

  updates_ = None;
}

void WCartesian3DChart::resizeGL(int width, int height)
{
  viewport(0, 0, width, height);

  double ratio = ((double)height)/width;
  pMatrix_ = WMatrix4x4();
  pMatrix_.ortho(-2, 2, -2*ratio, 2*ratio, -100, 100);
  useProgram(cubeProgram_);
  uniformMatrix4(cube_pMatrixUniform_, pMatrix_);
  useProgram(cubeLineProgram_);
  uniformMatrix4(cubeLine_pMatrixUniform_, pMatrix_);
  useProgram(axisProgram_);
  uniformMatrix4(axis_pMatrixUniform_, pMatrix_);

  bool clippingLinesEnabled = false;
  for (std::size_t i = 0; i < dataSeriesVector_.size(); i++) {
    WAbstractGridData *data
      = dynamic_cast<WAbstractGridData *>(dataSeriesVector_[i].get());
    if (data && data->clippingLinesEnabled()) {
      clippingLinesEnabled = true;
      break;
    }
  }
  if (intersectionLinesEnabled_ || 
      clippingLinesEnabled || 
      !intersectionPlanes_.empty())
    resizeOffscreenBuffer();

  // + change the projection matrices for all dataseries
  for (unsigned i = 0; i < dataSeriesVector_.size(); i++) {
    dataSeriesVector_[i]->resizeGL();
  }
}

void WCartesian3DChart::initOffscreenBuffer()
{
  offscreenDepthbuffer_ = createRenderbuffer();

  intersectionLinesFramebuffer_ = createFramebuffer();
  intersectionLinesTexture_ = createTexture();

  bindTexture(TEXTURE_2D, intersectionLinesTexture_);
  texParameteri(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
  texParameteri(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_S, CLAMP_TO_EDGE);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_T, CLAMP_TO_EDGE);

  meshIndexFramebuffer_ = createFramebuffer();
  meshIndexTexture_ = createTexture();

  bindTexture(TEXTURE_2D, meshIndexTexture_);
  texParameteri(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
  texParameteri(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_S, CLAMP_TO_EDGE);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_T, CLAMP_TO_EDGE);

  positionFramebuffer_ = createFramebuffer();
  positionTexture_ = createTexture();

  bindTexture(TEXTURE_2D, positionTexture_);
  texParameteri(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
  texParameteri(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_S, CLAMP_TO_EDGE);
  texParameteri(TEXTURE_2D, TEXTURE_WRAP_T, CLAMP_TO_EDGE);

  resizeOffscreenBuffer();
}

void WCartesian3DChart::resizeOffscreenBuffer()
{
  int w = (int) width().value();
  int h = (int) height().value();

  bindTexture(TEXTURE_2D, intersectionLinesTexture_);
  texImage2D(TEXTURE_2D, 0, RGBA, w, h, 0, RGBA);

  bindTexture(TEXTURE_2D, meshIndexTexture_);
  texImage2D(TEXTURE_2D, 0, RGB, w, h, 0, RGB);

  bindTexture(TEXTURE_2D, positionTexture_);
  texImage2D(TEXTURE_2D, 0, RGB, w, h, 0, RGB);

  bindRenderbuffer(RENDERBUFFER, offscreenDepthbuffer_);
  renderbufferStorage(RENDERBUFFER, DEPTH_COMPONENT16, w, h);

  bindFramebuffer(FRAMEBUFFER, intersectionLinesFramebuffer_);
  framebufferTexture2D(FRAMEBUFFER, COLOR_ATTACHMENT0, 
		       TEXTURE_2D, intersectionLinesTexture_, 0);
  framebufferRenderbuffer(FRAMEBUFFER, DEPTH_ATTACHMENT, 
			  RENDERBUFFER, offscreenDepthbuffer_);

  bindFramebuffer(FRAMEBUFFER, meshIndexFramebuffer_);
  framebufferTexture2D(FRAMEBUFFER, COLOR_ATTACHMENT0, 
		       TEXTURE_2D, meshIndexTexture_, 0);
  framebufferRenderbuffer(FRAMEBUFFER, DEPTH_ATTACHMENT, 
			  RENDERBUFFER, offscreenDepthbuffer_);

  bindFramebuffer(FRAMEBUFFER, positionFramebuffer_);
  framebufferTexture2D(FRAMEBUFFER, COLOR_ATTACHMENT0, 
		       TEXTURE_2D, positionTexture_, 0);
  framebufferRenderbuffer(FRAMEBUFFER, DEPTH_ATTACHMENT, 
			  RENDERBUFFER, offscreenDepthbuffer_);

  bindRenderbuffer(RENDERBUFFER, Renderbuffer());
  bindFramebuffer(FRAMEBUFFER, Framebuffer());
}

void WCartesian3DChart::deleteOffscreenBuffer()
{
  if (!offscreenDepthbuffer_.isNull()) {
    deleteRenderbuffer(offscreenDepthbuffer_);
    offscreenDepthbuffer_.clear();
  }
  if (!intersectionLinesFramebuffer_.isNull()) {
    deleteFramebuffer(intersectionLinesFramebuffer_);
    intersectionLinesFramebuffer_.clear();
  }
  if (!positionFramebuffer_.isNull()) {
    deleteFramebuffer(positionFramebuffer_);
    positionFramebuffer_.clear();
  }
  if (!meshIndexFramebuffer_.isNull()) {
    deleteFramebuffer(meshIndexFramebuffer_);
    meshIndexFramebuffer_.clear();
  }
  if (!intersectionLinesTexture_.isNull()) {
    deleteTexture(intersectionLinesTexture_);
    intersectionLinesTexture_.clear();
  }
  if (!meshIndexTexture_.isNull()) {
    deleteTexture(meshIndexTexture_);
    meshIndexTexture_.clear();
  }
  if (!positionTexture_.isNull()) {
    deleteTexture(positionTexture_);
    positionTexture_.clear();
  }
}

void WCartesian3DChart::renderIntersectionLines()
{
  using Wt::operator|;

  bindFramebuffer(FRAMEBUFFER, meshIndexFramebuffer_);

  clearColor(1.0, 1.0, 1.0, 1.0);
  clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

  for (unsigned i = 0; i < dataSeriesVector_.size(); i++) {
    WAbstractGridData *gridData 
      = dynamic_cast<WAbstractGridData*>(dataSeriesVector_[i].get());
    if (gridData && gridData->type() == Series3DType::Surface) {
      gridData->paintGLIndex(i);
    }
  }

  bindFramebuffer(FRAMEBUFFER, positionFramebuffer_);

  clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

  for (unsigned i = 0; i < dataSeriesVector_.size(); i++) {
    WAbstractGridData *gridData
      = dynamic_cast<WAbstractGridData*>(dataSeriesVector_[i].get());
    if (gridData && gridData->type() == Series3DType::Surface) {
      gridData->paintGLPositions();
    }
  }

  bindFramebuffer(FRAMEBUFFER, intersectionLinesFramebuffer_);

  disable(CULL_FACE);
  disable(DEPTH_TEST);
  enable(BLEND);

  useProgram(intersectionLinesProgram_);
  bindBuffer(ARRAY_BUFFER, overlayPosBuffer_);
  vertexAttribPointer(intersectionLines_vertexPositionAttribute_,
  		      3,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(intersectionLines_vertexPositionAttribute_);
  bindBuffer(ARRAY_BUFFER, overlayTexCoBuffer_);
  vertexAttribPointer(intersectionLines_vertexTextureCoAttribute_,
  		      2,
  		      FLOAT,
  		      false,
  		      0,
  		      0);
  enableVertexAttribArray(intersectionLines_vertexTextureCoAttribute_);

  uniformMatrix4(intersectionLines_cameraUniform_, jsMatrix_);
  uniform1f(intersectionLines_viewportWidthUniform_, width().value());
  uniform1f(intersectionLines_viewportHeightUniform_, height().value());
  uniform4f(intersectionLines_colorUniform_,
	    intersectionLinesColor_.red() / 255.0,
	    intersectionLinesColor_.green() / 255.0,
	    intersectionLinesColor_.blue() / 255.0,
	    intersectionLinesColor_.alpha() / 255.0);

  activeTexture(TEXTURE0);
  bindTexture(TEXTURE_2D, positionTexture_);
  uniform1i(intersectionLines_positionSamplerUniform_, 0);
  activeTexture(TEXTURE1);
  bindTexture(TEXTURE_2D, meshIndexTexture_);
  uniform1i(intersectionLines_meshIndexSamplerUniform_, 1);
  drawArrays(TRIANGLE_STRIP, 0, 4);
  disableVertexAttribArray(intersectionLines_vertexPositionAttribute_);
  disableVertexAttribArray(intersectionLines_vertexTextureCoAttribute_);

  bindFramebuffer(FRAMEBUFFER, Framebuffer());

  enable(CULL_FACE);
  enable(DEPTH_TEST);
  disable(BLEND);
}

void WCartesian3DChart::renderIntersectionLinesWithInvisiblePlanes()
{
  using Wt::operator|;

  for (std::size_t i = 0; i < intersectionPlanes_.size(); ++i) {
    bindFramebuffer(FRAMEBUFFER, meshIndexFramebuffer_);

    clearColor(1.0, 1.0, 1.0, 1.0);
    clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

    for (unsigned j = 0; j < dataSeriesVector_.size(); j++) {
      WAbstractGridData *gridData
	= dynamic_cast<WAbstractGridData*>(dataSeriesVector_[j].get());
      if (gridData && gridData->type() == Series3DType::Surface) {
	gridData->paintGLIndex(1);
      }
    }

    disable(CULL_FACE);
    enable(DEPTH_TEST);

    useProgram(clippingPlaneProgram_);

    double minX = xAxis_->minimum();
    double maxX = xAxis_->maximum();
    double minY = yAxis_->minimum();
    double maxY = yAxis_->maximum();
    double minZ = zAxis_->minimum();
    double maxZ = zAxis_->maximum();

    IntersectionPlane plane = intersectionPlanes_[i];
    if (plane.axis == Axis::X3D) {
      uniform1i(clippingPlane_clippingAxis_, 0);
    } else if (plane.axis == Axis::Y3D) {
      uniform1i(clippingPlane_clippingAxis_, 1);
    } else {
      uniform1i(clippingPlane_clippingAxis_, 2);
    }
    uniform3f(clippingPlane_clipPtUniform_, plane.position, 
	      plane.position, plane.position);
    uniform3f(clippingPlane_dataMinPtUniform_, minX, minY, minZ);
    uniform3f(clippingPlane_dataMaxPtUniform_, maxX, maxY, maxZ);
    uniform1i(clippingPlane_drawPositionUniform_, 0);

    WMatrix4x4 mvMatrix(1.0f, 0.0f, 0.0f, 0.0f,
		  0.0f, 0.0f, 1.0f, 0.0f,
		  0.0f, 1.0f, 0.0f, 0.0f,
		  0.0f, 0.0f, 0.0f, 1.0f);

    uniformMatrix4(clippingPlane_pMatrixUniform_, pMatrix_);
    uniformMatrix4(clippingPlane_mvMatrixUniform_, mvMatrix);
    uniformMatrix4(clippingPlane_cMatrixUniform_, jsMatrix_);

    bindBuffer(ARRAY_BUFFER, clippingPlaneVertBuffer_);
    vertexAttribPointer(clippingPlane_vertexPositionAttribute_,
		  2,
		  FLOAT,
		  false,
		  0,
		  0);
    enableVertexAttribArray(clippingPlane_vertexPositionAttribute_);

    drawArrays(TRIANGLE_STRIP, 0, 4);

    enable(CULL_FACE);
    disable(DEPTH_TEST);

    disableVertexAttribArray(clippingPlane_vertexPositionAttribute_);

    bindFramebuffer(FRAMEBUFFER, positionFramebuffer_);

    clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

    for (unsigned j = 0; j < dataSeriesVector_.size(); j++) {
      WAbstractGridData *gridData
	= dynamic_cast<WAbstractGridData*>(dataSeriesVector_[j].get());
      if (gridData && gridData->type() == Series3DType::Surface) {
	gridData->paintGLPositions();
      }
    }

    disable(CULL_FACE);
    enable(DEPTH_TEST);

    useProgram(clippingPlaneProgram_);

    uniform1i(clippingPlane_drawPositionUniform_, 1);

    uniformMatrix4(clippingPlane_pMatrixUniform_, pMatrix_);
    uniformMatrix4(clippingPlane_mvMatrixUniform_, mvMatrix);
    uniformMatrix4(clippingPlane_cMatrixUniform_, jsMatrix_);

    bindBuffer(ARRAY_BUFFER, clippingPlaneVertBuffer_);
    vertexAttribPointer(clippingPlane_vertexPositionAttribute_,
		  2,
		  FLOAT,
		  false,
		  0,
		  0);
    enableVertexAttribArray(clippingPlane_vertexPositionAttribute_);

    drawArrays(TRIANGLE_STRIP, 0, 4);

    enable(CULL_FACE);
    disable(DEPTH_TEST);

    disableVertexAttribArray(clippingPlane_vertexPositionAttribute_);

    bindFramebuffer(FRAMEBUFFER, intersectionLinesFramebuffer_);

    disable(CULL_FACE);
    disable(DEPTH_TEST);
    enable(BLEND);

    useProgram(intersectionLinesProgram_);
    bindBuffer(ARRAY_BUFFER, overlayPosBuffer_);
    vertexAttribPointer(intersectionLines_vertexPositionAttribute_,
			3,
			FLOAT,
			false,
			0,
			0);
    enableVertexAttribArray(intersectionLines_vertexPositionAttribute_);
    bindBuffer(ARRAY_BUFFER, overlayTexCoBuffer_);
    vertexAttribPointer(intersectionLines_vertexTextureCoAttribute_,
			2,
			FLOAT,
			false,
			0,
			0);
    enableVertexAttribArray(intersectionLines_vertexTextureCoAttribute_);

    uniformMatrix4(intersectionLines_cameraUniform_, jsMatrix_);
    uniform1f(intersectionLines_viewportWidthUniform_, width().value());
    uniform1f(intersectionLines_viewportHeightUniform_, height().value());
    uniform4f(intersectionLines_colorUniform_,
	plane.color.red() / 255.0,
	plane.color.green() / 255.0,
	plane.color.blue() / 255.0,
	plane.color.alpha() / 255.0);

    activeTexture(TEXTURE0);
    bindTexture(TEXTURE_2D, positionTexture_);
    uniform1i(intersectionLines_positionSamplerUniform_, 0);
    activeTexture(TEXTURE1);
    bindTexture(TEXTURE_2D, meshIndexTexture_);
    uniform1i(intersectionLines_meshIndexSamplerUniform_, 1);
    drawArrays(TRIANGLE_STRIP, 0, 4);
    disableVertexAttribArray(intersectionLines_vertexPositionAttribute_);
    disableVertexAttribArray(intersectionLines_vertexTextureCoAttribute_);

    bindFramebuffer(FRAMEBUFFER, Framebuffer());

    enable(CULL_FACE);
    enable(DEPTH_TEST);
    disable(BLEND);
  }
}

void WCartesian3DChart::renderClippingLines(WAbstractGridData *data)
{
  using Wt::operator|;

  bindFramebuffer(FRAMEBUFFER, intersectionLinesFramebuffer_);

  clearColor(0.0, 0.0, 0.0, 0.0);
  clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
  bindFramebuffer(FRAMEBUFFER, Framebuffer());

  for (int i = 0; i < 6; ++i) {
    bindFramebuffer(FRAMEBUFFER, meshIndexFramebuffer_);

    clearColor(1.0, 1.0, 1.0, 1.0);
    clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

    disable(CULL_FACE);
    enable(DEPTH_TEST);

    double minX = xAxis_->minimum();
    double maxX = xAxis_->maximum();
    double minY = yAxis_->minimum();
    double maxY = yAxis_->maximum();
    double minZ = zAxis_->minimum();
    double maxZ = zAxis_->maximum();

    useProgram(clippingPlaneProgram_);

    int clippingAxis = i / 2;
    uniform1i(clippingPlane_clippingAxis_, clippingAxis);
    if (i % 2 == 0) {
      uniform3fv(clippingPlane_clipPtUniform_, data->jsMaxPt_);
    } else {
      uniform3fv(clippingPlane_clipPtUniform_, data->jsMinPt_);
    }
    uniform3f(clippingPlane_dataMinPtUniform_, minX, minY, minZ);
    uniform3f(clippingPlane_dataMaxPtUniform_, maxX, maxY, maxZ);
    uniform1i(clippingPlane_drawPositionUniform_, 0);

    WMatrix4x4 mvMatrix(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

    uniformMatrix4(clippingPlane_pMatrixUniform_, pMatrix_);
    uniformMatrix4(clippingPlane_mvMatrixUniform_, mvMatrix);
    uniformMatrix4(clippingPlane_cMatrixUniform_, jsMatrix_);

    bindBuffer(ARRAY_BUFFER, clippingPlaneVertBuffer_);
    vertexAttribPointer(clippingPlane_vertexPositionAttribute_,
			2,
			FLOAT,
			false,
			0,
			0);
    enableVertexAttribArray(clippingPlane_vertexPositionAttribute_);

    drawArrays(TRIANGLE_STRIP, 0, 4);

    disableVertexAttribArray(clippingPlane_vertexPositionAttribute_);

    data->paintGLIndex(1,
	clippingAxis == 0 ? 0.01 : 0.0,
	clippingAxis == 1 ? 0.01 : 0.0,
	clippingAxis == 2 ? 0.01 : 0.0);
    // NOTE: This loop makes the number of draw calls quadratic in the number
    //       of surfaces. This will affect performance and may be undesirable
    //       when using more and larger surfaces.
    for (size_t j = 0; j < dataSeriesVector_.size(); ++j) {
      if (dataSeriesVector_[j].get() != data) {
	WAbstractGridData *gridData 
	  = dynamic_cast<WAbstractGridData*>(dataSeriesVector_[j].get());
	if (gridData &&
	    gridData->type() == Series3DType::Surface &&
	    gridData->clippingLinesEnabled()) {
	  gridData->paintGLIndex(0xffffff);
	}
      }
    }

    bindFramebuffer(FRAMEBUFFER, positionFramebuffer_);

    clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

    disable(WGLWidget::CULL_FACE);
    enable(WGLWidget::DEPTH_TEST);

    useProgram(clippingPlaneProgram_);

    uniform1i(clippingPlane_drawPositionUniform_, 1);

    bindBuffer(ARRAY_BUFFER, clippingPlaneVertBuffer_);
    vertexAttribPointer(clippingPlane_vertexPositionAttribute_,
			2,
			FLOAT,
			false,
			0,
			0);
    enableVertexAttribArray(clippingPlane_vertexPositionAttribute_);

    drawArrays(TRIANGLE_STRIP, 0, 4);

    disableVertexAttribArray(clippingPlane_vertexPositionAttribute_);

    data->paintGLPositions(clippingAxis == 0 ? 0.01 : 0.0,
	clippingAxis == 1 ? 0.01 : 0.0,
	clippingAxis == 2 ? 0.01 : 0.0);
    for (size_t j = 0; j < dataSeriesVector_.size(); ++j) {
      if (dataSeriesVector_[j].get() != data) {
	WAbstractGridData *gridData
	  = dynamic_cast<WAbstractGridData*>(dataSeriesVector_[j].get());
	if (gridData &&
	    gridData->type() == Series3DType::Surface &&
	    gridData->clippingLinesEnabled()) {
	  gridData->paintGLPositions();
	}
      }
    }

    bindFramebuffer(FRAMEBUFFER, intersectionLinesFramebuffer_);

    disable(CULL_FACE);
    disable(DEPTH_TEST);
    enable(BLEND);

    useProgram(intersectionLinesProgram_);
    bindBuffer(ARRAY_BUFFER, overlayPosBuffer_);
    vertexAttribPointer(intersectionLines_vertexPositionAttribute_,
			3,
			FLOAT,
			false,
			0,
			0);
    enableVertexAttribArray(intersectionLines_vertexPositionAttribute_);
    bindBuffer(ARRAY_BUFFER, overlayTexCoBuffer_);
    vertexAttribPointer(intersectionLines_vertexTextureCoAttribute_,
			2,
			FLOAT,
			false,
			0,
			0);
    enableVertexAttribArray(intersectionLines_vertexTextureCoAttribute_);

    uniformMatrix4(intersectionLines_cameraUniform_, jsMatrix_);
    uniform1f(intersectionLines_viewportWidthUniform_, width().value());
    uniform1f(intersectionLines_viewportHeightUniform_, height().value());
    uniform4f(intersectionLines_colorUniform_,
	data->clippingLinesColor().red() / 255.0,
	data->clippingLinesColor().green() / 255.0,
	data->clippingLinesColor().blue() / 255.0,
	data->clippingLinesColor().alpha() / 255.0);

    activeTexture(TEXTURE0);
    bindTexture(TEXTURE_2D, positionTexture_);
    uniform1i(intersectionLines_positionSamplerUniform_, 0);
    activeTexture(TEXTURE1);
    bindTexture(TEXTURE_2D, meshIndexTexture_);
    uniform1i(intersectionLines_meshIndexSamplerUniform_, 1);
    drawArrays(TRIANGLE_STRIP, 0, 4);
    disableVertexAttribArray(intersectionLines_vertexPositionAttribute_);
    disableVertexAttribArray(intersectionLines_vertexTextureCoAttribute_);

    bindFramebuffer(FRAMEBUFFER, Framebuffer());

    enable(CULL_FACE);
    enable(DEPTH_TEST);
    disable(BLEND);
  }
}

void WCartesian3DChart::deleteAllGLResources()
{
  if (cubeProgram_.isNull()) { // never been painted
    return;
  }
  if (!intersectionLinesProgram_.isNull()) {
    detachShader(intersectionLinesProgram_, intersectionLinesFragmentShader_);
    detachShader(intersectionLinesProgram_, vertexShader2D_);
    deleteShader(intersectionLinesFragmentShader_);
    deleteProgram(intersectionLinesProgram_);
    intersectionLinesProgram_.clear();
  }
  if (!clippingPlaneProgram_.isNull()) {
    detachShader(clippingPlaneProgram_, clippingPlaneFragShader_);
    detachShader(clippingPlaneProgram_, clippingPlaneVertexShader_);
    deleteShader(clippingPlaneFragShader_);
    deleteShader(clippingPlaneVertexShader_);
    deleteProgram(clippingPlaneProgram_);
    clippingPlaneProgram_.clear();
  }
  deleteBuffer(cubeBuffer_);
  deleteBuffer(cubeNormalsBuffer_);
  deleteBuffer(cubeIndicesBuffer_);
  deleteBuffer(cubeLineNormalsBuffer_);
  deleteBuffer(cubeLineIndicesBuffer_);
  deleteBuffer(axisBuffer_);
  deleteBuffer(axisIndicesBuffer_);
  deleteBuffer(axisInPlaneBuffer_);
  deleteBuffer(axisPlaneNormalBuffer_);
  deleteBuffer(axisOutOfPlaneNormalBuffer_);
  deleteBuffer(axisVertBuffer_);
  deleteBuffer(axisIndicesVertBuffer_);
  deleteBuffer(axisInPlaneVertBuffer_);
  deleteBuffer(axisPlaneNormalVertBuffer_);
  deleteBuffer(axisOutOfPlaneNormalVertBuffer_);

  deleteGLTextures();
  deleteBuffer(cubeTexCoords_);cubeTexCoords_.clear();
  deleteBuffer(axisTexCoordsHoriz_);axisTexCoordsHoriz_.clear();
  deleteBuffer(axisTexCoordsVert_);axisTexCoordsVert_.clear();

  if (!cubeProgram_.isNull()) {
    detachShader(cubeProgram_, fragmentShader_);
    detachShader(cubeProgram_, vertexShader_);
  }
  if (!axisProgram_.isNull()) {
    detachShader(axisProgram_, fragmentShader2_);
    detachShader(axisProgram_, vertexShader2_);
  }
  deleteShader(fragmentShader_);
  deleteShader(vertexShader_);
  deleteShader(fragmentShader2_);
  deleteShader(vertexShader2_);
  deleteShader(cubeLineFragShader_);
  deleteShader(cubeLineVertShader_);
  deleteProgram(cubeProgram_);cubeProgram_.clear();
  deleteProgram(axisProgram_);axisProgram_.clear();
  deleteProgram(cubeLineProgram_);cubeLineProgram_.clear();

  deleteOffscreenBuffer();

  clearBinaryResources();
}

void WCartesian3DChart::deleteGLTextures() {
  if (cubeProgram_.isNull()) { // never been painted
    return;
  }

  deleteTexture(horizAxisTexture_); horizAxisTexture_.clear();
  deleteTexture(horizAxisTexture2_); horizAxisTexture2_.clear();
  deleteTexture(vertAxisTexture_);vertAxisTexture_.clear();
  deleteTexture(cubeTextureXY_);cubeTextureXY_.clear();
  deleteTexture(cubeTextureXZ_);cubeTextureXZ_.clear();
  deleteTexture(cubeTextureYZ_);cubeTextureYZ_.clear();

  if (!titleTexture_.isNull())
    deleteTexture(titleTexture_);titleTexture_.clear();
  if (!legendTexture_.isNull())
    deleteTexture(legendTexture_);legendTexture_.clear();
  if (!colorMapTexture_.isNull())
    deleteTexture(colorMapTexture_);colorMapTexture_.clear();

  for (std::size_t i = 0; i < objectsToDelete.size(); ++i) {
    cpp17::any o = objectsToDelete[i];
    if (o.type() == typeid(WGLWidget::Buffer)) {
      WGLWidget::Buffer buf = cpp17::any_cast<WGLWidget::Buffer>(objectsToDelete[i]);
      if (!buf.isNull()) {
	deleteBuffer(buf);
      }
    } else if (o.type() == typeid(WGLWidget::Texture)) {
      WGLWidget::Texture tex = cpp17::any_cast<WGLWidget::Texture>(objectsToDelete[i]);
      if (!tex.isNull()) {
	deleteTexture(tex);
      }
    } else if (o.type() == typeid(WGLWidget::Shader)) {
      WGLWidget::Shader shader = cpp17::any_cast<WGLWidget::Shader>(objectsToDelete[i]);
      if (!shader.isNull()) {
	deleteShader(shader);
      }
    } else if (o.type() == typeid(WGLWidget::Program)) {
      WGLWidget::Program prog = cpp17::any_cast<WGLWidget::Program>(objectsToDelete[i]);
      if (!prog.isNull()) {
	deleteProgram(prog);
      }
    } else assert(false);
  }
  objectsToDelete.clear();
}

void WCartesian3DChart::updateChart(WFlags<ChartUpdates> flags)
{
  updates_ |= flags;

  repaintGL(GLClientSideRenderer::UPDATE_GL);
}

void WCartesian3DChart::resize(const WLength &width, const WLength &height)
{
  updateChart(ChartUpdates::GLTextures);
  WGLWidget::resize(width, height);
}

void WCartesian3DChart::paintHorizAxisTextures(WPaintDevice *paintDevice,
					       bool labelAngleMirrored)
{
  if (textureScaling_ == 0)
    throw WException("WCartesian3DChart: axes not initialized properly");

  double oldLabelAngleX = 0.0, oldLabelAngleY = 0.0;
  if (labelAngleMirrored) {
    xAxis_->setRenderMirror(true);
    yAxis_->setRenderMirror(true);
    oldLabelAngleX = xAxis_->labelAngle();
    xAxis_->setLabelAngle(-oldLabelAngleX);
    oldLabelAngleY = yAxis_->labelAngle();
    yAxis_->setLabelAngle(-oldLabelAngleY);
  }

  WPainter painter(paintDevice);
  painter.begin(paintDevice);

  // set device dimension on axis
  int axisOffset = (int)(axisRenderWidth_/textureScaling_/1.6*0.3);
  int axisWidth = axisRenderWidth_/textureScaling_;
  int axisHeight = axisRenderHeight_/textureScaling_;

  WPointF axisStart, axisEnd;
  double tickStart, tickEnd, labelPos;
  AlignmentFlag labelHFlag, labelVFlag;

  WPainterPath clippy = WPainterPath();

  // draw X-axis( LTR, labels underneath)
  clippy.addRect(WRectF(0,0,axisRenderWidth_,axisRenderHeight_));
  painter.setClipPath(clippy);
  painter.setClipping(true);
  painter.scale(textureScaling_, textureScaling_);
  axisStart = WPointF(axisOffset, 0.0);
  axisEnd = WPointF(axisWidth-axisOffset, 0.0);
  tickStart = 0.0; tickEnd = TICKLENGTH;
  labelPos = tickEnd;
  labelHFlag = AlignmentFlag::Center; labelVFlag = AlignmentFlag::Top;
  if (xAxis_->labelAngle() > ANGLE1) {
    labelHFlag = labelPos > 0 ? AlignmentFlag::Right : AlignmentFlag::Left;
    if (xAxis_->labelAngle() > ANGLE2)
      labelVFlag = AlignmentFlag::Middle;
  } else if (xAxis_->labelAngle() < -ANGLE1) {
    labelHFlag = labelPos > 0 ? AlignmentFlag::Left : AlignmentFlag::Right;
    if (xAxis_->labelAngle() < -ANGLE2)
      labelVFlag = AlignmentFlag::Middle;
  }

  xAxis_->render(painter,
		 AxisProperty::Line | AxisProperty::Labels,
		 axisStart,
		 axisEnd,
		 tickStart, tickEnd, labelPos,
		 WFlags<AlignmentFlag>(labelHFlag) | labelVFlag);

  // draw title
  double addOffset = xAxis_->titleOffset();
  WFont oldFont = painter.font();
  painter.setFont(xAxis_->titleFont());
  painter.drawText(WRectF(0, TITLEOFFSET+addOffset,
			  axisWidth, axisHeight-TITLEOFFSET-addOffset),
		   WFlags<AlignmentFlag>(AlignmentFlag::Center) | AlignmentFlag::Top, xAxis_->title());

  // draw X-axis( RTL, labels underneath)
  painter.scale(1.0/textureScaling_, 1.0/textureScaling_);
  painter.translate(0, axisRenderHeight_);
  painter.setClipPath(clippy);
  painter.scale(textureScaling_, textureScaling_);

  axisEnd = WPointF(axisOffset, 0.0);
  axisStart = WPointF(axisWidth-axisOffset, 0.0);
  tickStart = 0.0; tickEnd = TICKLENGTH;
  labelPos = tickEnd;

  xAxis_->render(painter,
		 AxisProperty::Line | AxisProperty::Labels,
		axisStart,
		axisEnd,
		tickStart, tickEnd, labelPos,
		WFlags<AlignmentFlag>(labelHFlag) | labelVFlag);

  // draw title
  painter.drawText(WRectF(0, TITLEOFFSET+addOffset,
			  axisWidth, axisHeight-TITLEOFFSET-addOffset),
		   WFlags<AlignmentFlag>(AlignmentFlag::Center) | AlignmentFlag::Top, xAxis_->title());

  // draw X-axis( LTR, labels above)
  painter.scale(1.0/textureScaling_, 1.0/textureScaling_);
  painter.translate(0, axisRenderHeight_);
  painter.setClipPath(clippy);
  painter.scale(textureScaling_, textureScaling_);

  axisStart = WPointF(axisOffset, axisHeight);
  axisEnd = WPointF(axisWidth-axisOffset, axisHeight);
  tickStart = -TICKLENGTH; tickEnd = 0.0;
  labelPos = tickEnd - 4;
  labelHFlag = AlignmentFlag::Center; labelVFlag = AlignmentFlag::Bottom;
  if (xAxis_->labelAngle() > ANGLE1) {
    labelHFlag = labelPos > 0 ? AlignmentFlag::Right : AlignmentFlag::Left;
    if (xAxis_->labelAngle() > ANGLE2)
      labelVFlag = AlignmentFlag::Middle;
  } else if (xAxis_->labelAngle() < -ANGLE1) {
    labelHFlag = labelPos > 0 ? AlignmentFlag::Left : AlignmentFlag::Right;
    if (xAxis_->labelAngle() < -ANGLE2)
      labelVFlag = AlignmentFlag::Middle;
  }

  xAxis_->render(painter,
		 AxisProperty::Line | AxisProperty::Labels,
		 axisStart,
		 axisEnd,
		 tickStart, tickEnd, labelPos,
		 WFlags<AlignmentFlag>(labelHFlag) | labelVFlag);

  // draw title
  painter.drawText(WRectF(0, 0, axisWidth, axisHeight-TITLEOFFSET-addOffset),
		   WFlags<AlignmentFlag>(AlignmentFlag::Center) | AlignmentFlag::Bottom, xAxis_->title());

  // draw X-axis( RTL, labels above)
  painter.scale(1.0/textureScaling_, 1.0/textureScaling_);
  painter.translate(0, axisRenderHeight_);
  painter.setClipPath(clippy);
  painter.scale(textureScaling_, textureScaling_);

  axisEnd = WPointF(axisOffset, axisHeight);
  axisStart = WPointF(axisWidth-axisOffset, axisHeight);
  tickStart = -TICKLENGTH; tickEnd = 0.0;
  labelPos = tickEnd - 4;

  xAxis_->render(painter,
		 AxisProperty::Line | AxisProperty::Labels,
		 axisStart,
		 axisEnd,
		 tickStart, tickEnd, labelPos,
		 WFlags<AlignmentFlag>(labelHFlag) | labelVFlag);

  // draw title
  painter.drawText(WRectF(0, 0, axisWidth, axisHeight-TITLEOFFSET-addOffset),
		   WFlags<AlignmentFlag>(AlignmentFlag::Center) | AlignmentFlag::Bottom, xAxis_->title());
  painter.setFont(oldFont);

  // draw Y-axis (LTR, label underneath)
  painter.scale(1.0/textureScaling_, 1.0/textureScaling_);
  painter.translate(0, axisRenderHeight_);
  painter.setClipPath(clippy);
  painter.scale(textureScaling_, textureScaling_);

  axisStart = WPointF(axisOffset, 0.0);
  axisEnd = WPointF(axisWidth-axisOffset, 0.0);
  tickStart = 0.0; tickEnd = TICKLENGTH;
  labelPos = tickEnd;
  labelHFlag = AlignmentFlag::Center; labelVFlag = AlignmentFlag::Top;
  if (yAxis_->labelAngle() > ANGLE1) {
    labelHFlag = labelPos > 0 ? AlignmentFlag::Right : AlignmentFlag::Left;
    if (yAxis_->labelAngle() > ANGLE2)
      labelVFlag = AlignmentFlag::Middle;
  } else if (yAxis_->labelAngle() < -ANGLE1) {
    labelHFlag = labelPos > 0 ? AlignmentFlag::Left : AlignmentFlag::Right;
    if (yAxis_->labelAngle() < -ANGLE2)
      labelVFlag = AlignmentFlag::Middle;
  }

  yAxis_->render(painter,
		 AxisProperty::Line | AxisProperty::Labels,
		 axisStart,
		 axisEnd,
		 tickStart, tickEnd, labelPos,
		 WFlags<AlignmentFlag>(labelHFlag) | labelVFlag);

  // draw title
  addOffset = yAxis_->titleOffset();
  painter.setFont(yAxis_->titleFont());
  painter.drawText(WRectF(0, TITLEOFFSET+addOffset,
			  axisWidth, axisHeight-TITLEOFFSET-addOffset),
		   WFlags<AlignmentFlag>(AlignmentFlag::Center) | AlignmentFlag::Top, yAxis_->title());

  // draw Y-axis (RTL, label underneath)
  painter.scale(1.0/textureScaling_, 1.0/textureScaling_);
  painter.translate(0, axisRenderHeight_);
  painter.setClipPath(clippy);
  painter.scale(textureScaling_, textureScaling_);

  axisEnd = WPointF(axisOffset, 0.0);
  axisStart = WPointF(axisWidth-axisOffset, 0.0);
  tickStart = 0.0; tickEnd = TICKLENGTH;
  labelPos = tickEnd;

  yAxis_->render(painter,
		 AxisProperty::Line | AxisProperty::Labels,
		 axisStart,
		 axisEnd,
		 tickStart, tickEnd, labelPos,
		 WFlags<AlignmentFlag>(labelHFlag) | labelVFlag);

  // draw title
  painter.drawText(WRectF(0, TITLEOFFSET+addOffset,
			  axisWidth, axisHeight-TITLEOFFSET-addOffset),
		   WFlags<AlignmentFlag>(AlignmentFlag::Center) | AlignmentFlag::Top, yAxis_->title());

  // draw Y-axis (LTR, labels above)
  painter.scale(1.0/textureScaling_, 1.0/textureScaling_);
  painter.translate(0, axisRenderHeight_);
  painter.setClipPath(clippy);
  painter.scale(textureScaling_, textureScaling_);

  axisStart = WPointF(axisOffset, axisHeight);
  axisEnd = WPointF(axisWidth-axisOffset, axisHeight);
  tickStart = -TICKLENGTH; tickEnd = 0.0;
  labelPos = tickEnd - 4;
  labelHFlag = AlignmentFlag::Center; labelVFlag = AlignmentFlag::Bottom;
  if (yAxis_->labelAngle() > ANGLE1) {
    labelHFlag = labelPos > 0 ? AlignmentFlag::Right : AlignmentFlag::Left;
    if (yAxis_->labelAngle() > ANGLE2)
      labelVFlag = AlignmentFlag::Middle;
  } else if (yAxis_->labelAngle() < -ANGLE1) {
    labelHFlag = labelPos > 0 ? AlignmentFlag::Left : AlignmentFlag::Right;
    if (yAxis_->labelAngle() < -ANGLE2)
      labelVFlag = AlignmentFlag::Middle;
  }

  yAxis_->render(painter,
		 AxisProperty::Line | AxisProperty::Labels,
		 axisStart,
		 axisEnd,
		 tickStart, tickEnd, labelPos,
		 WFlags<AlignmentFlag>(labelHFlag) | labelVFlag);

  // draw title
  painter.drawText(WRectF(0, 0, axisWidth, axisHeight-TITLEOFFSET-addOffset),
		   WFlags<AlignmentFlag>(AlignmentFlag::Center) | AlignmentFlag::Bottom, yAxis_->title());

  // draw Y-axis (RTL, labels above)
  painter.scale(1.0/textureScaling_, 1.0/textureScaling_);
  painter.translate(0, axisRenderHeight_);
  painter.setClipPath(clippy);
  painter.scale(textureScaling_, textureScaling_);

  axisEnd = WPointF(axisOffset, axisHeight);
  axisStart = WPointF(axisWidth-axisOffset, axisHeight);
  tickStart = -TICKLENGTH; tickEnd = 0.0;
  labelPos = tickEnd - 4;

  yAxis_->render(painter,
		 AxisProperty::Line | AxisProperty::Labels,
		 axisStart,
		 axisEnd,
		 tickStart, tickEnd, labelPos,
		 WFlags<AlignmentFlag>(labelHFlag) | labelVFlag);

  // draw title
  painter.drawText(WRectF(0, 0, axisWidth, axisHeight-TITLEOFFSET-addOffset),
		   WFlags<AlignmentFlag>(AlignmentFlag::Center) | AlignmentFlag::Bottom, yAxis_->title());
  painter.setFont(oldFont);

  if (labelAngleMirrored) {
    xAxis_->setLabelAngle(oldLabelAngleX);
    yAxis_->setLabelAngle(oldLabelAngleY);
    xAxis_->setRenderMirror(false);
    yAxis_->setRenderMirror(false);
  }
  painter.end();
}

void WCartesian3DChart::paintVertAxisTextures(WPaintDevice *paintDevice)
{
  // set device dimension on axis
  int axisOffset = (int)(axisRenderWidth_/textureScaling_/1.6*0.3);
  int axisWidth = axisRenderWidth_/textureScaling_;
  int axisHeight = axisRenderHeight_/textureScaling_;

  WPainter painter(paintDevice);

  // draw Z-axis (labels left)
  painter.scale(textureScaling_, textureScaling_);
  WPointF axisStart = WPointF(axisHeight, axisWidth-axisOffset);
  WPointF axisEnd = WPointF(axisHeight, axisOffset);
  double tickStart = -TICKLENGTH; double tickEnd = 0.0;
  double labelPos = tickEnd - 4;
  AlignmentFlag labelHFlag = AlignmentFlag::Right;
  AlignmentFlag labelVFlag = AlignmentFlag::Middle;
  if (zAxis_->labelAngle() > ANGLE1) {
    labelVFlag = labelPos < 0 ? AlignmentFlag::Bottom : AlignmentFlag::Top;
    if (zAxis_->labelAngle() > ANGLE2)
      labelHFlag = AlignmentFlag::Center;
  } else if (zAxis_->labelAngle() < -ANGLE1) {
    labelVFlag = labelPos < 0 ? AlignmentFlag::Top : AlignmentFlag::Bottom;
    if (zAxis_->labelAngle() < -ANGLE2)
      labelHFlag = AlignmentFlag::Center;
  }

  zAxis_->render(painter,
		AxisProperty::Line | AxisProperty::Labels,
		axisStart,
		axisEnd,
		tickStart, tickEnd, labelPos,
		WFlags<AlignmentFlag>(labelHFlag) | labelVFlag);

  // draw title
  double addOffset = zAxis_->titleOffset();
  painter.rotate(-90);
  WFont oldFont = zAxis_->titleFont();
  painter.setFont(zAxis_->titleFont());
  painter.drawText(WRectF(-axisWidth, 0,
			  axisWidth, axisHeight-TITLEOFFSET-addOffset),
		   WFlags<AlignmentFlag>(AlignmentFlag::Center) | AlignmentFlag::Bottom, zAxis_->title());
  painter.rotate(90);

  // draw Z-axis (labels right)
  axisStart = WPointF(axisHeight, axisWidth-axisOffset);
  axisEnd = WPointF(axisHeight, axisOffset);
  tickStart = 0.0; tickEnd = TICKLENGTH;
  labelPos = tickEnd;
  labelHFlag = AlignmentFlag::Left; labelVFlag = AlignmentFlag::Middle;
  if (zAxis_->labelAngle() > ANGLE1) {
    labelVFlag = labelPos < 0 ? AlignmentFlag::Bottom : AlignmentFlag::Top;
    if (zAxis_->labelAngle() > ANGLE2)
      labelHFlag = AlignmentFlag::Center;
  } else if (zAxis_->labelAngle() < -ANGLE1) {
    labelVFlag = labelPos < 0 ? AlignmentFlag::Top : AlignmentFlag::Bottom;
    if (zAxis_->labelAngle() < -ANGLE2)
      labelHFlag = AlignmentFlag::Center;
  }

  zAxis_->render(painter,
		 AxisProperty::Line | AxisProperty::Labels,
		 axisStart,
		 axisEnd,
		 tickStart, tickEnd, labelPos,
		 WFlags<AlignmentFlag>(labelHFlag) | labelVFlag);

  // draw title
  painter.rotate(-90);
  painter.drawText(WRectF(-axisWidth, axisHeight+TITLEOFFSET+addOffset,
			  axisWidth, axisHeight-TITLEOFFSET-addOffset),
			  WFlags<AlignmentFlag>(AlignmentFlag::Center) | AlignmentFlag::Top, zAxis_->title());
  painter.setFont(oldFont);
  painter.rotate(90);
  painter.end();
}

void WCartesian3DChart::paintGridLines(WPaintDevice *paintDevice, Plane plane)
{
  int axisOffset = (int)((double)axisRenderWidth_/(double)textureScaling_/1.6*0.3);
  int axisWidth = axisRenderWidth_/textureScaling_;
  // to go from pixels to plotcube-coordinates
  int renderLength = axisWidth-2*axisOffset;

  WPainter painter(paintDevice);
  painter.scale(textureScaling_, textureScaling_);
  if (gridLinesPen_.width().value() == 0)
    gridLinesPen_.setWidth(1);
  painter.setPen(gridLinesPen_);

  switch (plane) {
  case Plane::XY:
    if (XYGridEnabled_[0]) {
      std::vector<double> pos = xAxis_->gridLinePositions(AxisConfig());
      for (unsigned i = 0; i < pos.size(); i++) {
	if (pos[i] == 0 || pos[i] == gridRenderWidth_)
	  continue;
	int texPos = (int)(pos[i]/renderLength * 
			   (gridRenderWidth_/textureScaling_));
	painter.drawLine(texPos+0.5, 0.5, texPos+0.5, 
			 gridRenderWidth_/textureScaling_-0.5);
      }
    }
    if (XYGridEnabled_[1]) {
      std::vector<double> pos = yAxis_->gridLinePositions(AxisConfig());
      for (unsigned i = 0; i < pos.size(); i++) {
	if (pos[i] == 0 || pos[i] == gridRenderWidth_)
	  continue;
	int texPos = (int)(pos[i]/renderLength * 
			   (gridRenderWidth_/textureScaling_));
	painter.drawLine(0.5, texPos+0.5, 
			 gridRenderWidth_/textureScaling_-0.5, texPos+0.5);
      }
    }
    break;
  case Plane::XZ:
    if (XZGridEnabled_[0]) {
      std::vector<double> pos = xAxis_->gridLinePositions(AxisConfig());
      for (unsigned i = 0; i < pos.size(); i++) {
	if (pos[i] == 0 || pos[i] == gridRenderWidth_)
	  continue;
	int texPos = (int)(pos[i]/renderLength * (gridRenderWidth_/textureScaling_));
	painter.drawLine(texPos+0.5, 0.5, texPos+0.5, gridRenderWidth_/textureScaling_-0.5);
      }
    }
    if (XZGridEnabled_[1]) {
      std::vector<double> pos = zAxis_->gridLinePositions(AxisConfig());
      for (unsigned i = 0; i < pos.size(); i++) {
	if (pos[i] == 0 || pos[i] == gridRenderWidth_)
	  continue;
	int texPos = (int)(pos[i]/renderLength * (gridRenderWidth_/textureScaling_));
	painter.drawLine(0.5, texPos+0.5, gridRenderWidth_/textureScaling_-0.5, texPos+0.5);
      }
    }
    break;
  case Plane::YZ:
    if (YZGridEnabled_[0]) {
      std::vector<double> pos = yAxis_->gridLinePositions(AxisConfig());
      for (unsigned i = 0; i < pos.size(); i++) {
	if (pos[i] == 0 || pos[i] == gridRenderWidth_)
	  continue;
	int texPos = (int)(pos[i]/renderLength * (gridRenderWidth_/textureScaling_));
	painter.drawLine(texPos+0.5, 0.5, texPos+0.5, gridRenderWidth_/textureScaling_-0.5);
      }
    }
    if (YZGridEnabled_[1]) {
      std::vector<double> pos = zAxis_->gridLinePositions(AxisConfig());
      for (unsigned i = 0; i < pos.size(); i++) {
	if (pos[i] == 0 || pos[i] == gridRenderWidth_)
	  continue;
	int texPos = (int)(pos[i]/renderLength * (gridRenderWidth_/textureScaling_));
	painter.drawLine(0.5, texPos+0.5, gridRenderWidth_/textureScaling_-0.5, texPos+0.5);
      }
    }
    break;
  }
  painter.end();
}

double WCartesian3DChart::toPlotCubeCoords(double value, Axis axis)
{
  double min = 0.0, max = 1.0;

  if (axis == Axis::X3D) {
    min = xAxis_->minimum();
    max = xAxis_->maximum();
  } else if (axis == Axis::Y3D) {
    min = yAxis_->minimum();
    max = yAxis_->maximum();
  } else if (axis == Axis::Z3D) {
    min = zAxis_->minimum();
    max = zAxis_->maximum();
  } else {
    throw WException("WCartesian3DChart: don't know this type of axis");
  }

  return (value - min)/(max - min);
}


  }
}
