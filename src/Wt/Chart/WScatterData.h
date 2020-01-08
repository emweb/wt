// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CHART_SCATTERDATA_H
#define CHART_SCATTERDATA_H

#include <Wt/WModelIndex.h>
#include <Wt/Chart/WAbstractDataSeries3D.h>
#include <Wt/Chart/WCartesian3DChart.h>
#include <Wt/Chart/WSelection.h>

namespace Wt {
class WMemoryResource;

  namespace Chart {

/*! \class WScatterData
 *  \brief Class representing a collection of points for on a 3D chart.
 *
 * General information can be found at WAbstractDataSeries3D. The model should 
 * be structured as a table where every row represents a point. In the simplest 
 * case, there are three columns representing the x-, y- and z-values. By 
 * default, this is column 0 for X, column 1 for Y and column 2 for Z. It is 
 * also possible to provide an additional column containing information on the 
 * color for each point. The same is possible for the size. Color-information 
 * in the model should be present as a WColor.
 *
 * If these extra columns are not included, the ItemDataRole::MarkerBrushColor and 
 * ItemDataRole::MarkerScaleFactor can still be used to style individual points. These 
 * dataroles should be set on the values in the column containing the z-values.
 *
 * The figure below shows an upward spiral of points, with droplines enabled 
 * and a pointsize of 5 pixels.
 *
 * \image html spiral.png "An example of WScatterData"
 *
 * \ingroup charts
 */
class WT_API WScatterData : public WAbstractDataSeries3D {
public:
  /*! \brief Constructor
   */
  WScatterData(std::shared_ptr<WAbstractItemModel> model);

  /*! \brief Enables or disables droplines for all points.
   *
   * Enabling droplines will cause a line to be drawn from every point to the 
   * the ground-plane of the chart's plotcube. By default the droplines are 
   * disabled.
   *
   * \sa setDroplinesPen()
   */
  void setDroplinesEnabled(bool enable = true);

  /*! \brief Returns whether droplines are enabled.
   *
   * \sa setDroplinesEnabled(), setDroplinesPen()
   */
  bool droplinesEnabled() const { return droplinesEnabled_; }

  /*! \brief Sets the pen that is used to draw droplines.
   *
   * The default pen is a default constructed WPen. 
   *
   * Note: only the width and color of the pen are used.
   *
   * \sa setDroplinesEnabled()
   */
  void setDroplinesPen(const WPen &pen);

  /*! \brief Returns the pen that is used to draw droplines.
   *
   * \sa setDroplinesEnabled(), setDroplinesPen()
   */
  WPen droplinesPen() const { return droplinesPen_; }

  /*! \brief Sets the column-index from the model that is used for the 
   * x-coordinate of all points.
   *
   * The default X column index is 0.
   */
  void setXSeriesColumn(int columnNumber) { XSeriesColumn_ = columnNumber; }

  /*! \brief Returns the column-index from the model that is used for the 
   * x-coordinate of all points.
   *
   * \sa setXSeriesColumn()
   */
  int XSeriesColumn() const { return XSeriesColumn_; }

  /*! \brief Sets the column-index from the model that is used for the 
   * y-coordinate of all points.
   *
   * The default X column index is 1.
   */
  void setYSeriesColumn(int columnNumber) { YSeriesColumn_ = columnNumber; }

  /*! \brief Returns the column-index from the model that is used for the 
   * y-coordinate of all points.
   *
   * \sa setYSeriesColumn()
   */
  int YSeriesColumn() const { return YSeriesColumn_; }

  /*! \brief Sets the column-index from the model that is used for the 
   * z-coordinate of all points.
   *
   * The default Z column index is 2.
   *
   * Note that this column is also used to check for a ItemDataRole::MarkerBrushColor 
   * and a ItemDataRole::MarkerScaleFactor is no color-column or size-column are set.
   *
   * \sa setColorColumn(), setSizeColumn()
   */
  void setZSeriesColumn(int columnNumber) { ZSeriesColumn_ = columnNumber; }

  /*! \brief Returns the column-index from the model that is used for the 
   * z-coordinate of all points.
   *
   * \sa setZSeriesColumn()
   */
  int ZSeriesColumn() const { return ZSeriesColumn_; }

  /*! \brief Configure a column in the model to be used for the color of the 
   * points.
   *
   * By default, the color-column is set to -1. This means there is no column 
   * which specifies color-values. Also, the basic mechanism of using the 
   * ItemDataRole::MarkerBrushColor (if present) is then active. The Z-seriescolumn is 
   * checked for the presence of this Role.
   *
   * /sa setZSeriesColumn()
   */
  void setColorColumn(int columnNumber, ItemDataRole role = ItemDataRole::Display);

  /*! \brief Configure a column in the model to be used for the size of the 
   * points.
   *
   * By default, the size-column is set to -1. This means there is no column 
   * which specifies size-values. Also, the basic mechanism of using the 
   * ItemDataRole::MarkerScaleFactor (if present) is then active. The Z-seriescolumn is 
   * checked for the presence of this Role.
   *
   * /sa setZSeriesColumn()
   */
  void setSizeColumn(int columnNumber, ItemDataRole role = ItemDataRole::Display);

  /*! \brief Pick points on this WScatterData using a single pixel.
   *
   * x,y are the screen coordinates of the pixel from the top left of
   * the chart, and radius is the radius in pixels around that pixel.
   * All points around the ray projected through the pixel within the
   * given radius will be returned.
   */
  std::vector<WPointSelection> pickPoints(int x, int y, int radius) const;

  /*! \brief Pick points on this WScatterData inside of a rectangle.
   *
   * The screen coordinates (x1, y1) and (x2, y2) from the top left of
   * the chart define a rectangle within which the points should be selected.
   */
  std::vector<WPointSelection> pickPoints(int x1, int y1, int x2, int y2) const;

  virtual double minimum(Axis axis) const override;

  virtual double maximum(Axis axis) const override;

  virtual std::vector<cpp17::any> getGlObjects() override;

  virtual void initializeGL() override {}
  virtual void paintGL() const override;
  virtual void updateGL() override;
  virtual void resizeGL() override;
  virtual void deleteAllGLResources() override;

private:
  int countSimpleData() const;
  void dataFromModel(FloatBuffer& simplePtsArray,
		     FloatBuffer& simplePtsSize,
		     FloatBuffer& coloredPtsArray,
		     FloatBuffer& coloredPtsSize,
		     FloatBuffer& coloredPtsColor);
  void dropLineVertices(FloatBuffer& dataPoints,
			FloatBuffer& verticesOUT);
  void initShaders();
  void findXRange() const;
  void findYRange() const;
  void findZRange() const;

  int XSeriesColumn_;
  int YSeriesColumn_;
  int ZSeriesColumn_;

  int colorColumn_;
  ItemDataRole asColorRole_;
  ItemDataRole asSizeRole_;
  int sizeColumn_;

  bool droplinesEnabled_;
  WPen droplinesPen_;

  mutable double xMin_, xMax_, yMin_, yMax_;

  mutable bool xRangeCached_;
  mutable bool yRangeCached_;

  WGLWidget::Buffer vertexPosBuffer_, vertexSizeBuffer_;
  WGLWidget::Buffer vertexPosBuffer2_, vertexSizeBuffer2_, vertexColorBuffer2_;
  WGLWidget::Buffer lineVertBuffer_;

  int vertexBufferSize_, vertexBuffer2Size_, lineVertBufferSize_;

  WGLWidget::Texture colormapTexture_;
  WGLWidget::Texture pointSpriteTexture_;

  WGLWidget::Shader vertexShader_;
  WGLWidget::Shader colVertexShader_;
  WGLWidget::Shader linesVertShader_;
  WGLWidget::Shader fragmentShader_;
  WGLWidget::Shader colFragmentShader_;
  WGLWidget::Shader linesFragShader_;

  WGLWidget::Program shaderProgram_;
  WGLWidget::Program colShaderProgram_;
  WGLWidget::Program linesProgram_;

  WGLWidget::AttribLocation posAttr_;
  WGLWidget::AttribLocation posAttr2_;
  WGLWidget::AttribLocation posAttrLines_;
  WGLWidget::AttribLocation sizeAttr_;
  WGLWidget::AttribLocation sizeAttr2_;
  WGLWidget::AttribLocation colorAttr2_;
  WGLWidget::UniformLocation mvMatrixUniform_;
  WGLWidget::UniformLocation mvMatrixUniform2_;
  WGLWidget::UniformLocation mvMatrixUniform3_;
  WGLWidget::UniformLocation pMatrixUniform_;
  WGLWidget::UniformLocation pMatrixUniform2_;
  WGLWidget::UniformLocation pMatrixUniform3_;
  WGLWidget::UniformLocation cMatrixUniform_;
  WGLWidget::UniformLocation cMatrixUniform2_;
  WGLWidget::UniformLocation cMatrixUniform3_;
  WGLWidget::UniformLocation lineColorUniform_;
  WGLWidget::UniformLocation samplerUniform_;
  WGLWidget::UniformLocation pointSpriteUniform_;
  WGLWidget::UniformLocation pointSpriteUniform2_;
  WGLWidget::UniformLocation vpHeightUniform_;
  WGLWidget::UniformLocation vpHeightUniform2_;
  WGLWidget::UniformLocation offsetUniform_;
  WGLWidget::UniformLocation scaleFactorUniform_;
};
    
  }
}

#endif
