// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CHART_WABSTRACTGRIDDATA_H
#define CHART_WABSTRACTGRIDDATA_H

#include <Wt/Chart/WAbstractDataSeries3D.h>
#include <Wt/Chart/WCartesian3DChart.h>
#include <Wt/Chart/WSelection.h>
#include <Wt/WPen.h>

namespace Wt {
class WAbstractItemModel;
class WMemoryResource;
class WVector3;

  namespace Chart {
  class WCartesian3DChart;

/*! \brief Enumeration with the possible representations of a WAbstractGridData
 *
 * \ingroup charts
 */
enum class Series3DType {
  Point,   //!< Series rendered as points
  Surface, //!< Series rendered as a surface
  Bar      //!< Series rendered as bars
};

/*! \class WAbstractGridData
 *  \brief Class representing grid-based data for on a 3D chart.
 *
 * General information can be found at WAbstractDataSeries3D. Information on
 * how the model is structured is provided in the subclasses. GridData can be
 * represented in three ways. This is indicated by \ref Series3DType and can be
 * either Series3DType::Point, Series3DType::Surface or Series3DType::Bar. Note that points and
 * surfaces can only be added to charts of type \ref ChartType::Scatter, while bars
 * can only be added to charts of type \ref ChartType::Category.
 *
 * When the data is shown as a surface, a mesh can be added to the surface.
 * This draws lines over the surface at the positions of the x- and y-values.
 * For bar-series data, it is possible to adjust the width of the bars in both
 * directions.
 *
 * The three types of data-representation are illustrated below.
 *
 * \image html gridDataTypes.png "The three representation types of grid-based data"
 *
 * \ingroup charts
 */
class WT_API WAbstractGridData : public WAbstractDataSeries3D {
public:
  /*! \brief Constructor
   */
  WAbstractGridData(std::shared_ptr<WAbstractItemModel> model);

  virtual ~WAbstractGridData();

  virtual double minimum(Axis axis) const override = 0;

  virtual double maximum(Axis axis) const override = 0;

  /*! \brief Sets the type of representation that will be used for the data.
   *
   * All representations in \ref Series3DType are possible for the data. Note
   * that \ref Series3DType::Point and \ref Series3DType::Surface can only be used on a
   * chart that is configured as a \ref ChartType::Scatter and \ref Series3DType::Bar can
   * only be used on a chart that is configured to be a \ref ChartType::Category.
   *
   * The default value is Series3DType::Point.
   */
  void setType(Series3DType type);

  /*! \brief Returns the type of representation that will be used for the data.
   *
   * \sa setType()
   */
  virtual Series3DType type() const { return seriesType_; }

  /*! \brief Enables or disables a mesh for when a surface is drawn.
   *
   * The default value is false. This option only takes effect when the type of
   * this WGridData is \ref Series3DType::Surface. The mesh is drawn at the position
   * of the x-axis and y-axis values.
   */
  void setSurfaceMeshEnabled(bool enabled = true);

  /*! \brief Returns whether the surface-mesh is enabled for this dataseries.
   *
   * \sa setSurfaceMeshEnabled()
   */
  bool isSurfaceMeshEnabled() { return surfaceMeshEnabled_; }

  /*! \brief Sets the bar-width.
   *
   * This option only takes effect when the type of this WGridData is
   * Series3DType::Bar. The values provided should be between 0 and 1, where 1 lets
   * the bars each take up 1/(nb of x/y-values) of the axis.
   *
   * The default bar-width is 0.5 in both directions.
   */
  void setBarWidth(double xWidth, double yWidth);

  /*! \brief Returns the bar-width in the X-axis direction.
   *
   * \sa setBarWidth()
   */
  double barWidthX() { return barWidthX_; }

  /*! \brief Returns the bar-width in the Y-axis direction.
   *
   * \sa setBarWidth()
   */
  double barWidthY() { return barWidthY_; }

  /*! \brief Sets the WPen that is used for drawing the mesh.
   *
   * Used when drawing the mesh on a surface or the lines around bars. The
   * default is a default constructed WPen (black and one pixel wide).
   *
   * Note: only the width and color of this WPen are used.
   *
   * \sa setSurfaceMeshEnabled()
   */
  void setPen(const WPen &pen);

  /*! \brief Returns the pen that is used for drawing the mesh.
   *
   * \sa setPen()
   */
  WPen pen() const { return meshPen_; }

  /*! \brief Find all points on the surface that are projected to the given pixel.
   *
   * A ray is cast from the given pixel's x,y position (from the top left of the chart,
   * in screen coordinates) and every intersection with the surface is returned, along
   * with its distance from the look point. Note that the coordinates of the intersection
   * points are interpolated between the data points that make up the surface.
   */
  std::vector<WSurfaceSelection> pickSurface(int x, int y) const;

  /*! \brief Return the bar that is closest to the look point at the given pixel.
   *
   * A ray is cast from the given pixel's x,y position (from the top left of the chart,
   * in screen coordinates), and the closest bar on this WAbstractGridData
   * is returned, along with its distance from the look point.
   *
   * Note that if this WAbstractGridData is hidden, this method still returns the
   * closest bar as if it was visible. Also, if multiple bars are on the same bar chart,
   * the bar that is returned may be behind another data series. Use the distance
   * field of the returned WBarSelection to determine which data series is in front from
   * the given angle.
   *
   * If there is no bar at the given pixel, then a selection with an invalid WModelIndex
   * is returned. The distance is then set to positive infinity.
   */
  WBarSelection pickBar(int x, int y) const;

  /*! \brief Set isoline levels.
   *
   * Isolines are drawn on the top or ground plane of the chart.
   * Only applies if the type is Series3DType::Surface.
   *
   * The isoline levels are set in the coordinate system of the item model.
   */
  void setIsoLevels(const std::vector<double> &isoLevels);

  /*! \brief Get all of the isoline levels.
   *
   * \sa addIsoLevel()
   */
  const std::vector<double> &isoLevels() const;

  /*! \brief Set the color map for the isolines.
   *
   * When no color map is defined for the isolines, i.e. isoColorMap()
   * is set to NULL, the color map of this WAbstractGridData will be used.
   *
   * The isolines are only drawn if the type is Series3DType::Surface.
   *
   * \sa addIsoLevel(), setColorMap()
   */
  void setIsoColorMap(const std::shared_ptr<WAbstractColorMap>& colormap);

  /*! \brief Get the color map for the isolines.
   *
   * \sa setIsoColorMap()
   */
  std::shared_ptr<WAbstractColorMap> isoColorMap() const
    { return isoLineColorMap_; }

  /*! \brief Set the value below which the data series will be clipped on the given axis.
   *
   * This only affects data series whose type is Series3DType::Surface.
   */
  void setClippingMin(Axis axis, float v);

  /*! \brief Gets the value below which the data series will be clipped on the given axis.
   *
   * \sa setClippingMin()
   */
  float clippingMin(Axis axis) const;

  /*! \brief JSlot to change the value below which the data series will be clipped on the given axis.
   *
   * The JSlot takes one extra argument: the value to clip below.
   *
   * The jsRef() of this JSlot is only valid when this WAbstractGridData has been added to
   * a WCartesian3DChart. If this WAbstractGridData moves to another WCartesian3DChart, the
   * jsRef() of the JSlot changes.
   *
   * \sa setClippingMin()
   */
  JSlot &changeClippingMin(Axis axis);

  /*! \brief Set the value above which the data series will be clipped on the given axis.
   *
   * This only affects data series whose type is Series3DType::Surface.
   */
  void setClippingMax(Axis axis, float v);

  /*! \brief Gets the value above which the data series will be clipped on the given axis.
   *
   * \sa setClippingMax()
   */
  float clippingMax(Axis axis) const;

  /*! \brief JSlot to change the value above which the data series will be clipped on the given axis.
   *
   * The JSlot takes one extra argument: the value to clip below.
   *
   * The jsRef() of this JSlot is only valid when this WAbstractGridData has been added to
   * a WCartesian3DChart. If this WAbstractGridData moves to another WCartesian3DChart, the
   * jsRef() of the JSlot changes.
   *
   * \sa setClippingMax()
   */
  JSlot &changeClippingMax(Axis axis);

  /*! \brief Sets whether clipping lines should be drawn where a surface
   *         is clipped.
   *
   *  Clipping lines are disabled by default. Note that rendering will
   *  be significantly slower when enabled.
   */
  void setClippingLinesEnabled(bool clippingLinesEnabled);

  /*! \brief Returns whether clipping lines are enabled.
   *
   * \sa setClippingLinesEnabled()
   */
  bool clippingLinesEnabled() const { return clippingLinesEnabled_; }

  /*! \brief Sets the color of the clipping lines.
   *
   * \sa setClippingLinesEnabled()
   */
  void setClippingLinesColor(WColor color);

  /*! \brief Gets the color of the clipping lines.
   *
   * \sa setClippingLinesColor(), setClippingLinesEnabled()
   */
  WColor clippingLinesColor() const { return clippingLinesColor_; }

  static const int SURFACE_SIDE_LIMIT = 256; // = sqrt(2^16)
  static const int BAR_BUFFER_LIMIT = 8190; // = 2^16/8

  virtual int nbXPoints() const = 0;
  virtual int nbYPoints() const = 0;
  virtual WString axisLabel(int u, Axis axis) const = 0;
  virtual cpp17::any data(int i, int j) const = 0;

  virtual void setChart(WCartesian3DChart *chart) override;

  virtual std::vector<cpp17::any> getGlObjects() override;

  virtual void initializeGL() override;
  virtual void paintGL() const override;
  virtual void updateGL() override;
  virtual void resizeGL() override;
  virtual void deleteAllGLResources() override;

protected:
  virtual void pointDataFromModel(FloatBuffer& simplePtsArray,
				  FloatBuffer& simplePtsSize,
				  FloatBuffer& coloredPtsArray,
				  FloatBuffer& coloredPtsSize,
				  FloatBuffer& coloredPtsColor) const = 0;
  virtual void surfaceDataFromModel(std::vector<FloatBuffer>& simplePtsArrays) const = 0;
  virtual void barDataFromModel(std::vector<FloatBuffer>& simplePtsArrays) const = 0;
  virtual void barDataFromModel(std::vector<FloatBuffer>& simplePtsArrays,
				std::vector<FloatBuffer>& coloredPtsArrays,
				std::vector<FloatBuffer>& coloredPtsColors) const = 0;

  // data without color-role
  virtual int countSimpleData() const = 0;
  // adds up the value (i, j) for all dataseries, i and j should be specified
  // without any offsets for axes in the model
  float stackAllValues(std::vector<WAbstractGridData*> dataseries,
		       int i, int j) const;

  Series3DType seriesType_;

  bool surfaceMeshEnabled_;
  bool colorRoleEnabled_;

  double barWidthX_;
  double barWidthY_;

  WPen meshPen_;

  static const float zeroBarCompensation;

private:
  void initShaders();
  void initializePointSeriesBuffers();
  void initializeSurfaceSeriesBuffers();
  void initializeBarSeriesBuffers();
  void loadBinaryResource(FloatBuffer&,
			  std::vector<WGLWidget::Buffer>& buffers);
  void barSeriesVertexData(FloatBuffer& verticesIN,
			   FloatBuffer& verticesOUT) const;
  void generateVertexIndices(IntBuffer& indicesOUT,
			     int Nx, int Ny, int size = 0) const;
  void generateMeshIndices(IntBuffer& indicesOUT,
			   int Nx, int Ny, int size = 0);
  void generateTextureCoords(FloatBuffer& coordsOUT,
			     const FloatBuffer& dataArray,
			     int size);

  void paintGLIndex(unsigned index) const;
  void paintGLIndex(unsigned index, double marginX, double marginY, double marginZ) const;
  void paintGLPositions() const;
  void paintGLPositions(double marginX, double marginY, double marginZ) const;

  double rayTriangleIntersect(const WVector3 &re, const WVector3 &rd, const WVector3 &camera, const WVector3 &v0, const WVector3 &v1, const WVector3 &v2, WVector3 &point) const;

  void drawIsoLines() const;
  void linesForIsoLevel(double z, std::vector<float> &result) const;
  WGLWidget::Texture isoLineColorMapTexture();

  std::vector<int> vertexPosBufferSizes_, vertexPosBuffer2Sizes_;
  std::vector<int> indexBufferSizes_, lineBufferSizes_;
  std::vector<int> indexBufferSizes2_, lineBufferSizes2_;
  std::vector<int> isoLineBufferSizes_;

  std::vector<double> isoLineHeights_;
  std::shared_ptr<WAbstractColorMap> isoLineColorMap_;

  std::vector<WGLWidget::Buffer> vertexPosBuffers_;
  std::vector<WGLWidget::Buffer> vertexPosBuffers2_;
  std::vector<WGLWidget::Buffer> vertexSizeBuffers_;
  std::vector<WGLWidget::Buffer> vertexSizeBuffers2_;
  std::vector<WGLWidget::Buffer> vertexColorBuffers2_;
  std::vector<WGLWidget::Buffer> indexBuffers_;
  std::vector<WGLWidget::Buffer> indexBuffers2_;
  std::vector<WGLWidget::Buffer> overlayLinesBuffers_;
  std::vector<WGLWidget::Buffer> overlayLinesBuffers2_;
  std::vector<WGLWidget::Buffer> colormapTexBuffers_;
  std::vector<WGLWidget::Buffer> isoLineBuffers_;

  std::vector<WMemoryResource*> binaryResources_;

  // Shader program
  WGLWidget::Shader fragShader_;
  WGLWidget::Shader colFragShader_;
  WGLWidget::Shader meshFragShader_;
  WGLWidget::Shader singleColorFragShader_;
  WGLWidget::Shader positionFragShader_;
  WGLWidget::Shader isoLineFragShader_;

  WGLWidget::Shader vertShader_;
  WGLWidget::Shader colVertShader_;
  WGLWidget::Shader meshVertShader_;
  WGLWidget::Shader isoLineVertexShader_;

  WGLWidget::Program seriesProgram_;
  WGLWidget::Program colSeriesProgram_;
  WGLWidget::Program meshProgram_;
  WGLWidget::Program singleColorProgram_;
  WGLWidget::Program positionProgram_;
  WGLWidget::Program isoLineProgram_;

  // Shader attributes
  WGLWidget::AttribLocation vertexPosAttr_;
  WGLWidget::AttribLocation vertexPosAttr2_;
  WGLWidget::AttribLocation singleColor_vertexPosAttr_;
  WGLWidget::AttribLocation position_vertexPosAttr_;
  WGLWidget::AttribLocation meshVertexPosAttr_;
  WGLWidget::AttribLocation isoLineVertexPosAttr_;

  WGLWidget::AttribLocation vertexSizeAttr_;
  WGLWidget::AttribLocation vertexSizeAttr2_;
  WGLWidget::AttribLocation vertexColAttr2_;

  WGLWidget::AttribLocation barTexCoordAttr_;

  // Shader uniform variables
  WGLWidget::UniformLocation mvMatrixUniform_;
  WGLWidget::UniformLocation mvMatrixUniform2_;
  WGLWidget::UniformLocation singleColor_mvMatrixUniform_;
  WGLWidget::UniformLocation position_mvMatrixUniform_;
  WGLWidget::UniformLocation mesh_mvMatrixUniform_;
  WGLWidget::UniformLocation isoLine_mvMatrixUniform_;

  WGLWidget::UniformLocation pMatrix_;
  WGLWidget::UniformLocation pMatrix2_;
  WGLWidget::UniformLocation singleColor_pMatrix_;
  WGLWidget::UniformLocation position_pMatrix_;
  WGLWidget::UniformLocation mesh_pMatrix_;
  WGLWidget::UniformLocation isoLine_pMatrix_;

  WGLWidget::UniformLocation cMatrix_;
  WGLWidget::UniformLocation cMatrix2_;
  WGLWidget::UniformLocation singleColor_cMatrix_;
  WGLWidget::UniformLocation position_cMatrix_;
  WGLWidget::UniformLocation mesh_cMatrix_;
  WGLWidget::UniformLocation isoLine_cMatrix_;

  WGLWidget::UniformLocation TexSampler_;
  WGLWidget::UniformLocation isoLine_TexSampler_;
  WGLWidget::UniformLocation mesh_colorUniform_;

  WGLWidget::UniformLocation singleColorUniform_;

  WGLWidget::UniformLocation pointSpriteUniform_;
  WGLWidget::UniformLocation pointSpriteUniform2_;

  WGLWidget::UniformLocation vpHeightUniform_;
  WGLWidget::UniformLocation vpHeightUniform2_;

  WGLWidget::UniformLocation offset_;
  WGLWidget::UniformLocation isoLine_offset_;
  WGLWidget::UniformLocation scaleFactor_;
  WGLWidget::UniformLocation isoLine_scaleFactor_;

  WGLWidget::UniformLocation minPtUniform_;
  WGLWidget::UniformLocation mesh_minPtUniform_;
  WGLWidget::UniformLocation singleColor_minPtUniform_;
  WGLWidget::UniformLocation position_minPtUniform_;
  WGLWidget::UniformLocation maxPtUniform_;
  WGLWidget::UniformLocation mesh_maxPtUniform_;
  WGLWidget::UniformLocation singleColor_maxPtUniform_;
  WGLWidget::UniformLocation position_maxPtUniform_;

  WGLWidget::UniformLocation dataMinPtUniform_;
  WGLWidget::UniformLocation mesh_dataMinPtUniform_;
  WGLWidget::UniformLocation singleColor_dataMinPtUniform_;
  WGLWidget::UniformLocation position_dataMinPtUniform_;
  WGLWidget::UniformLocation dataMaxPtUniform_;
  WGLWidget::UniformLocation mesh_dataMaxPtUniform_;
  WGLWidget::UniformLocation singleColor_dataMaxPtUniform_;
  WGLWidget::UniformLocation position_dataMaxPtUniform_;

  WGLWidget::UniformLocation singleColor_marginUniform_;
  WGLWidget::UniformLocation position_marginUniform_;

  WGLWidget::Texture colormapTexture_;
  WGLWidget::Texture isoLineColorMapTexture_;
  WGLWidget::Texture pointSpriteTexture_;

  std::vector<float> minPt_;
  std::vector<float> maxPt_;
  WGLWidget::JavaScriptVector jsMinPt_;
  WGLWidget::JavaScriptVector jsMaxPt_;
  bool minPtChanged_;
  bool maxPtChanged_;

  JSlot changeClippingMinX_;
  JSlot changeClippingMaxX_;
  JSlot changeClippingMinY_;
  JSlot changeClippingMaxY_;
  JSlot changeClippingMinZ_;
  JSlot changeClippingMaxZ_;

  bool clippingLinesEnabled_;
  WColor clippingLinesColor_;

  friend class WCartesian3DChart;
};

  }
}

#endif
