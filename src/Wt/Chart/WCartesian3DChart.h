// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CHART_WCARTESIAN_3DCHART_H
#define CHART_WCARTESIAN_3DCHART_H

#include <Wt/Chart/WAbstractDataSeries3D.h>
#include <Wt/Chart/WAxis.h>
#include <Wt/Chart/WChartGlobal.h>
#include <Wt/Chart/WChartPalette.h>
#include <Wt/Chart/WLegend3D.h>
#include <Wt/WBrush.h>
#include <Wt/WFont.h>
#include <Wt/WGLWidget.h>
#include <Wt/WMatrix4x4.h>
#include <Wt/WPaintedWidget.h>

namespace Wt {

class WVector3;

  namespace Chart {

class WChart3DImplementation;
class WAbstractGridData;

/*! \enum Plane
 *  \brief Lists the three orthogonal planes in 3D
 *
 * \ingroup charts
 */
enum class Plane {
  XY, //!< X/Y Plane
  XZ, //!< X/Z Plane
  YZ  //!< Y/Z Plane
};

/*! \enum ChartUpdates
 *  \brief Lists the different ways that a chart can be updated
 *
 * \ingroup charts
 */
enum class ChartUpdates {
  CameraMatrix = 0x1, //!< Camera matrix
  GLContext = 0x2, //!< GL context
  GLTextures = 0x4 //!< GL textures
};

/*! \class WCartesian3DChart
 *  \brief A 3D Cartesian chart
 *
 * The chart consists of a plotcube, which is always open on the front, and 
 * adapts to the data which is shown on the chart. The plotcube has three axes 
 * of type WAxis. Each of these can be manually configured as in the 2D case. 
 * The chart can be either a \ref ChartType::Scatter or a \ref ChartType::Category. This influences 
 * how the data is positioned in relation to the x/y-axis. Gridlines can also 
 * be drawn on each of the plotcube-planes. The chart has a mouse-handler which 
 * allows rotation of the chart around the center of the plotcube. Zooming in 
 * and out is possible by scrolling.
 *
 * Data that can be shown on the chart derives from WAbstractDataSeries3D. 
 * Multiple dataseries can be added to the chart using addDataSeries(). The 
 * color of the dataseries is by default determined by the colors of the 
 * WChartPalette. This way a separate color is assigned to each new dataseries. 
 * All rendering logic of the data is contained in the dataseries-classes and 
 * further styling is often possible there. For example, a WAbstractColorMap 
 * can be added to a dataseries, which will assign a color to datapoints based 
 * on their z-value. More information on this is found in the documentation of 
 * WAbstractDataSeries3D.
 * 
 * It is possible to assign a title to the chart. A legend can also be shown 
 * that lists the titles of all dataseries (unless disabled in the dataseries 
 * itself). The legend position and style can be configured. In addition to 
 * title and legend, a colormap-legend is shown for every dataseries which has 
 * a colormap enabled and indicates that it should be displayed on the chart.
 *
 * \image html Chart3DCombo.png "A scatterplot on the left, a category-chart on the right."
 *
 * \ingroup charts
 */
class WT_API WCartesian3DChart : public WGLWidget {
public:
  /*! \class IntersectionPlane
   *  \brief An invisible intersection plane.
   *
   * Describes an invisible intersection plane, with
   * the axis it is perpendicular to, its position and
   * the color of the intersection.
   */
  struct IntersectionPlane {
    Axis axis;
    double position;
    WColor color;

    /*! \brief Constructor
     *
     * Create an intersection plane perpendicular to the given axis,
     * at the given position on that axis, and the color that the
     * intersection lines should have.
     *
     * \sa setIntersectionPlanes()
     */
    IntersectionPlane(Axis axis, double position, WColor col)
      : axis(axis), position(position), color(col)
    {}
  };

  /*! \brief Constructor
   *
   * Constructs a cartesian 3D chart, with the type set to ChartType::Scatter, a
   * transparent background, a PaletteFlavour::Muted palette and no gridlines.
   */
  WCartesian3DChart();

  /*! \brief Constructor
   *
   * Construct a cartesian 3D chart with the specified type, a transparent
   * background, a PaletteFlavour::Muted palette and no gridlines.
   */
  WCartesian3DChart(ChartType type);

  /*! \brief Destructor
   */
  ~WCartesian3DChart();

  /*! \brief Add a dataseries to the chart
   *
   * If the chart is of type ChartType::Scatter only numerical dataseries should
   * be added and if it is of type ChartType::Category only categorical dataseries 
   * should be added. If multiple categorical datasets are added, the 
   * axis-labels of the first dataseries will be used on the chart.
   * 
   * \sa removeDataSeries()
   */
  void addDataSeries(std::unique_ptr<WAbstractDataSeries3D> dataseries);
  
  /*! \brief Removes a dataseries from a chart.
   *
   * \sa addDataSeries()
   */
  std::unique_ptr<WAbstractDataSeries3D> removeDataSeries
    (WAbstractDataSeries3D * dataseries);

  /*! \brief Returns all dataseries that were added to this chart.
   *
   * \sa addDataSeries(), removeDataSeries()
   */
  std::vector<WAbstractDataSeries3D *> dataSeries() const;

  /*! \brief Returns the specified axis belonging to the chart.
   */
  WAxis& axis(Axis axis);

  /*! \brief Returns the specified axis belonging to the chart.
   */
  const WAxis& axis(Axis axis) const;

  /*! \brief Sets an axis
   *
   * \sa axis(Axis axis)
   */
  void setAxis(std::unique_ptr<WAxis> waxis, Axis axis);

  /*! \brief Enable/disable gridlines.
   *
   * Enables or disables gridlines in the given plane, along the given axis. 
   * All gridlines are by default disabled.
   */
  void setGridEnabled(Plane plane, Axis axis, bool enabled = true);

  /*! \brief Returns whether the gridlines are enabled.
   *
   * \sa setGridEnabled()
   */
  bool isGridEnabled(Plane plane, Axis axis);

  /*! \brief Set whether intersection lines are shown between surface charts
   *
   * This is disabled by default.
   */
  void setIntersectionLinesEnabled(bool enabled = true);

  /*! \brief Returns whether intersection lines are shown between surface charts
   *
   * \sa setIntersectionLinesEnabled()
   */
  bool isIntersectionLinesEnabled() const { return intersectionLinesEnabled_; }

  /*! \brief Sets the color of the intersection lines between surface charts.
   */
  void setIntersectionLinesColor(WColor color);

  /*! \brief Gets the color of the intersection lines between surface charts.
   *
   * \sa setIntersectionLinesColor()
   */
  const WColor &intersectionLinesColor() const {
    return intersectionLinesColor_;
  }

  /*! \brief Set the invisible planes with which intersections are drawn.
   *
   * This plane is perpendicular to the given axis, and the intersection
   * is shown in the given color.
   *
   * Note that render times will take increasingly longer as you add more
   * intersection planes.
   */
  void setIntersectionPlanes
    (const std::vector<IntersectionPlane> &intersectionPlanes);

  /*! \brief Get the invisible planes with which intersections are drawn.
   *
   * \sa setIntersectionPlanes()
   */
  const std::vector<IntersectionPlane> &intersectionPlanes() const;

  /*! \brief Sets the pen used for drawing the gridlines.
   *
   * The default pen for drawing gridlines is a StandardColor::Black pen of width 0.
   *
   * \sa gridLinesPen()
   */
  void setGridLinesPen(const WPen & pen);

  /*! \brief Returns the pen used for drawing the gridlines.
   *
   * \sa setGridLinesPen()
   */
  const WPen& gridLinesPen() const { return gridLinesPen_; }

  /*! \brief Sets the pen used to draw the edges of the plotcube.
   *
   * The default pen for drawing cubelines is a StandardColor::Black pen of width 0.
   *
   * Note: Only width and color of the pen are used, all other styling is 
   * ignored.
   */
  void setCubeLinesPen(const WPen & pen);

  /*! \brief Returns a reference to the pen used for drawing the edges of 
   * the plotcube.
   *
   * The width and color of the pen are used when drawing the edges of the
   * plotcube
   *
   * \sa setCubeLinesPen()
   */
  const WPen& cubeLinesPen() const { return cubeLinesPen_; }

  /*! \brief Sets the type of this chart.
   *
   * Sets the type of this chart to either ChartType::Scatter (for drawing numerical 
   * data) or to ChartType::Category (for drawing categorical data).
   */
  void setType(ChartType type);

  /*! \brief Returns the type of this chart
   *
   * \sa setType()
   */
  ChartType type() const { return chartType_; }
 
  /*! \brief Sets the palette for this chart.
   * 
   * Ownership of the WChartPalette is transferred to the chart.
   * 
   * The given palette determines which color subsequent dataseries will have.
   * If a dataseries has a colormap set, then the palette is not used for this
   * data.
   */
  void setPalette(const std::shared_ptr<WChartPalette>& palette);

  /*! \brief Returns the palette used for this chart.
   *
   * \sa setPalette()
   */
  std::shared_ptr<WChartPalette> palette() const { return chartPalette_; }

  /*! \brief Sets the background color for this chart.
   *
   * This sets the GL-clearcolor. The default is transparant, which will cause 
   * the background to have the color set in css.
   */
  void setBackground(const WColor &background);

  /*! \brief Returns the background color used for this chart.
   *
   * \sa setBackground()
   */
  const WColor& background() const { return background_; }
  
  /*! \brief Sets the title that is put on the chart.
   *
   * The title is always put at the top of the chart and in the center.
   *
   * \sa setTitleFont()
   */
  void setTitle(const WString &title);

  /*! \brief Returns the title that is put at the top of this chart.
   *
   * \sa setTitle(), setTitleFont()
   */
  const WString& title() const { return title_; }

  /*! \brief Sets the font that is used to draw the title.
   *
   * The default font is the default constructed WFont.
   *
   * \sa setTitle()
   */
  void setTitleFont(const WFont &titleFont);

  /*! \brief Returns the font used to draw the title.
   *
   * \sa setTitle(), setTitleFont()
   */
  const WFont& titleFont() const { return titleFont_; }

  /*! \brief Enables the legend.
   *
   * The location of the legend can be configured using
   * setLegendLocation(). Only series for which the legend is enabled
   * are included in this legend.
   *
   * The default value is \c false.
   *
   * \sa setLegendLocation()
   */
  void setLegendEnabled(bool enabled);

  /*! \brief Returns whether the legend is enabled.
   *
   * \sa setLegendEnabled()
   */
  bool isLegendEnabled() const { return legend_.isLegendEnabled(); }

  /*! \brief Configures the location of the legend
   *
   * The provided \p side can either be Wt::Side::Left, Wt::Side::Right, Wt::Side::Top,
   * Wt::Side::Bottom and configures the side of the chart at which the
   * legend is displayed.
   *
   * The \p alignment specifies how the legend is aligned. This can be
   * a horizontal alignment flag (Wt::AlignmentFlag::Left, Wt::AlignmentFlag::Center, or
   * Wt::AlignmentFlag::Right), when the \p side is Side::Bottom or Side::Top, or a vertical
   * alignment flag (Wt::AlignmentFlag::Top, Wt::AlignmentFlag::Middle, or Wt::AlignmentFlag::Bottom)
   * when the \p side is Side::Left or Side::Right.
   *
   * The default location is Wt::Side::Right and Wt::AlignmentFlag::Middle.
   *
   * \sa setLegendEnabled()
   */
  void setLegendLocation(Side side, AlignmentFlag alignment);

  /*! \brief Configures the legend decoration.
   *
   * This configures the font, border and background for the legend.
   *
   * The default font is a 10pt sans serif font (the same as the
   * default axis label font), the default \p border is \link
   * Wt::PenStyle::None PenStyle::None\endlink and the default \p background is \link
   * Wt::BrushStyle::None BrushStyle::None\endlink.
   *
   * \sa setLegendEnabled()
   */
  void setLegendStyle(const WFont &font, const WPen &border,
		      const WBrush &background);

  /*! \brief Returns the legend side.
   *
   * \sa setLegendLocation()
   */
  Side legendSide() const { return legend_.legendSide(); }

  /*! \brief Returns the legend alignment.
   *
   * \sa setLegendLocation()
   */
  AlignmentFlag legendAlignment() const { return legend_.legendAlignment(); }

  /*! \brief Returns the number of legend columns.
   *
   * \sa setLegendColumns()
   */
  int legendColumns() const { return legend_.legendColumns(); }

  /*! \brief Returns the legend column width.
   *
   * \sa setLegendColumns()
   */
  WLength legendColumnWidth() const { return legend_.legendColumnWidth(); }

  /*! \brief Returns the legend font.
   *
   * \sa setLegendStyle()
   */
  WFont legendFont() const { return legend_.legendFont(); }

  /*! \brief Returns the legend border pen.
   *
   * \sa setLegendStyle()
   */
  WPen legendBorder() const { return legend_.legendBorder(); }

  /*! \brief Returns the legend background brush.
   *
   * \sa setLegendStyle()
   */
  WBrush legendBackground() const { return legend_.legendBackground(); }

  /*! \brief Configures the number of columns and columnwidth of the legend.
   *
   * The default value is a single column, 100 pixels wide. 
   */
  void setLegendColumns(int columns, const WLength &columnWidth);

  /*! \brief Initializes the chart layout.
   *
   * This method must be called before any methods relating to the layout of
   * the chart are called (eg. calling minimum() or maximum() on one of the 
   * axes). The method is also automatically called when the chart is rendered.
   */
  void initLayout();

  /*! \brief Set the camera-matrix.
   *
   * The viewpoint can be set with the camera-matrix. The chart is defined
   * in the world coordinate system as a cube with axes from 0 to 1 in all
   * three directions. Therefore the center of the cube is positioned at
   * (0.5, 0.5, 0.5). The camera can be most easily position with the lookAt
   * method of WMatrix4x4. A common use-case when manipulating the matrix is to
   * translate the center to the origin and then rotate.
   * 
   * \if cpp
   * For example:
   * \code{.cpp}
   *   ...
   * 
   *   WMatrix4x4 camera;
   *   camera.lookAt(0.5, 0.5, z,     // camera position
   *                 0.5, 0.5, 0.5,   // center of the scene
   *                 0, 1, 0);        // up direction
   *   camera.translate(0.5, 0.5, 0.5);
   * 
   *   ... // some rotations
   * 
   *   camera.translate(-0.5, -0.5, -0.5);
   * \endcode
   * \endif
   */
  void setCameraMatrix(const WMatrix4x4& matrix);

  /*! \brief Get the current camera-matrix.
   *
   * The matrix represents the current view on the scene. It corresponds to
   * a coordinate system where the chart's axes run from 0 to 1 in all three
   * directions.
   *
   * \sa setCameraMatrix()
   */
  WMatrix4x4 cameraMatrix() const;

  /*! \brief Get the current camera matrix as a JavaScriptMatrix4x4.
   *
   * This JavaScriptMatrix4x4 can be used to implement a custom mouse
   * handler using setClientSideMouseHandler().
   *
   * \sa setCameraMatrix()
   */
  JavaScriptMatrix4x4 jsMatrix() const { return jsMatrix_; }

  // below = internal API
  WMatrix4x4 pMatrix() const { return pMatrix_; }
  double toPlotCubeCoords(double value, Axis axis);

protected:

  /*! \brief Initialize the WebGL state when the widget is first shown.
   *
   * Specialized for chart rendering.
   */
  void initializeGL() override;

  /*! \brief Update the client-side painting function.
   *
   * Specialized for chart rendering.
   */
  void paintGL() override;

  /*! \brief Update state set in initializeGL()
   *
   * Specialized for chart rendering.
   */
  void updateGL() override;

  /*! \brief Act on resize events.
   *
   * Specialized for chart rendering.
   */
  void resizeGL(int width, int height) override;

public:

  /*! \brief Update the chart.
   */
  void updateChart(WFlags<ChartUpdates> flags);

  void resize(const WLength &width, const WLength &height) override;

  void createRay(double x, double y, WVector3 &eye, WVector3 &direction) const;
private:
  void initializePlotCube();
  void deleteAllGLResources();
  void deleteGLTextures();

  void initializeIntersectionLinesProgram();
  void initializeClippingPlaneProgram();
  void initOffscreenBuffer();
  void resizeOffscreenBuffer();
  void deleteOffscreenBuffer();
  void renderIntersectionLines();
  void renderIntersectionLinesWithInvisiblePlanes();
  void renderClippingLines(WAbstractGridData *data);

  void paintHorizAxisTextures(WPaintDevice *paintDevice,
			      bool labelAngleMirrored = false);
  void paintVertAxisTextures(WPaintDevice *paintDevice);
  void paintGridLines(WPaintDevice *paintDevice, Plane plane);
  void loadCubeTextures();

  // Methods for all peripheral textures
  void init2DShaders();
  void initTitle();
  void initColorMaps();
  void initLegend();

  void paintPeripheralTexture(const Buffer& pos, const Buffer& texCo,
			      const Texture& texture);

  WMatrix4x4 worldTransform_;
  bool isViewSet_;
  std::vector<std::unique_ptr<WAbstractDataSeries3D> > dataSeriesVector_;
  std::unique_ptr<WAxis> xAxis_, yAxis_, zAxis_;
  ChartType chartType_;

  bool XYGridEnabled_[2], XZGridEnabled_[2], YZGridEnabled_[2];
  WPen cubeLinesPen_;
  WPen gridLinesPen_;
  
  WColor background_;
  std::shared_ptr<WChartPalette> chartPalette_;
  WString title_;
  WFont titleFont_;

  WLegend3D legend_;

  std::unique_ptr<WChart3DImplementation> interface_;
  int axisRenderWidth_, axisRenderHeight_;
  int gridRenderWidth_;
  int textureScaling_;
  unsigned int seriesCounter_; // for default naming

  // settings and options for all peripherals (legend, colormap, title)
  int currentTopOffset_, currentBottomOffset_;
  int currentLeftOffset_, currentRightOffset_;

  // Update flags
  WFlags<ChartUpdates> updates_;

  bool intersectionLinesEnabled_;
  WColor intersectionLinesColor_;

  std::vector<IntersectionPlane> intersectionPlanes_;

  // Shader programs
  Shader fragmentShader_;
  Shader vertexShader_;
  Shader fragmentShader2_;
  Shader vertexShader2_;
  Shader cubeLineFragShader_;
  Shader cubeLineVertShader_;
  Shader vertexShader2D_; // for putting 2D stuff in the 3D canvas
  Shader fragmentShader2D_;
  Shader intersectionLinesFragmentShader_;
  Shader clippingPlaneFragShader_;
  Shader clippingPlaneVertexShader_;
  Program cubeProgram_;
  Program cubeLineProgram_;
  Program axisProgram_;
  Program textureProgram_;
  Program intersectionLinesProgram_;
  Program clippingPlaneProgram_;

  // Cube SHADER variables
  // Shader attributes
  AttribLocation cube_vertexPositionAttribute_;
  AttribLocation cube_planeNormalAttribute_;
  AttribLocation cube_textureCoordAttribute_;
  AttribLocation cubeLine_vertexPositionAttribute_;
  AttribLocation cubeLine_normalAttribute_;
  // Shader uniform variables
  UniformLocation cube_pMatrixUniform_;
  UniformLocation cube_mvMatrixUniform_;
  UniformLocation cube_cMatrixUniform_;
  UniformLocation cube_texSampler1Uniform_;
  UniformLocation cube_texSampler2Uniform_;
  UniformLocation cube_texSampler3Uniform_;
  UniformLocation cubeLine_pMatrixUniform_;
  UniformLocation cubeLine_mvMatrixUniform_;
  UniformLocation cubeLine_cMatrixUniform_;
  UniformLocation cubeLine_nMatrixUniform_;
  UniformLocation cubeLine_colorUniform_;

  // AXIS SHADER variables
  // Shader attributes
  AttribLocation axis_vertexPositionAttribute_;
  AttribLocation axis_textureCoordAttribute_;
  AttribLocation axis_inPlaneAttribute_;
  AttribLocation axis_planeNormalAttribute_;
  AttribLocation axis_outOfPlaneNormalAttribute_;

  // Shader uniform variables
  UniformLocation axis_pMatrixUniform_;
  UniformLocation axis_mvMatrixUniform_;
  UniformLocation axis_cMatrixUniform_;
  UniformLocation axis_nMatrixUniform_;
  UniformLocation axis_normalAngleTextureUniform_;
  UniformLocation axis_texSamplerUniform_;

  // TEXTURE SHADER variables
  AttribLocation texture_vertexPositionAttribute_;
  AttribLocation texture_vertexTextureCoAttribute_;
  UniformLocation texture_texSamplerUniform_;

  // INTERSECTION LINES SHADER variables
  AttribLocation intersectionLines_vertexPositionAttribute_;
  AttribLocation intersectionLines_vertexTextureCoAttribute_;
  UniformLocation intersectionLines_cameraUniform_;
  UniformLocation intersectionLines_viewportWidthUniform_;
  UniformLocation intersectionLines_viewportHeightUniform_;
  UniformLocation intersectionLines_positionSamplerUniform_;
  UniformLocation intersectionLines_colorUniform_;
  UniformLocation intersectionLines_meshIndexSamplerUniform_;

  // CLIPPING PLANE SHADER variables
  AttribLocation clippingPlane_vertexPositionAttribute_;
  UniformLocation clippingPlane_mvMatrixUniform_;
  UniformLocation clippingPlane_pMatrixUniform_;
  UniformLocation clippingPlane_cMatrixUniform_;
  UniformLocation clippingPlane_clipPtUniform_;
  UniformLocation clippingPlane_dataMinPtUniform_;
  UniformLocation clippingPlane_dataMaxPtUniform_;
  UniformLocation clippingPlane_clippingAxis_;
  UniformLocation clippingPlane_drawPositionUniform_;

  WMatrix4x4 pMatrix_;
  // A client-side JavaScript matrix variable (for rotation)
  JavaScriptMatrix4x4 jsMatrix_;

  // Vertex buffers
  FloatBuffer cubeData_;
  FloatBuffer cubeNormalsData_;
  IntBuffer cubeIndices_;
  FloatBuffer cubeTexCo_;
  IntBuffer cubeLineIndices_;
  FloatBuffer axisSlabData_;
  IntBuffer axisSlabIndices_;
  FloatBuffer axisInPlaneBools_;
  FloatBuffer axisPlaneNormal_;
  FloatBuffer axisOutOfPlaneNormal_;
  FloatBuffer axisTexCo_;
  FloatBuffer axisSlabDataVert_;
  IntBuffer axisSlabIndicesVert_;
  FloatBuffer axisInPlaneBoolsVert_;
  FloatBuffer axisPlaneNormalVert_;
  FloatBuffer axisOutOfPlaneNormalVert_;
  FloatBuffer axisTexCoVert_;

  Buffer cubeBuffer_;
  Buffer cubeNormalsBuffer_;
  Buffer cubeIndicesBuffer_;
  Buffer cubeLineNormalsBuffer_;
  Buffer cubeLineIndicesBuffer_;
  Buffer cubeTexCoords_;
  Buffer axisBuffer_;
  Buffer axisIndicesBuffer_;
  Buffer axisInPlaneBuffer_;
  Buffer axisPlaneNormalBuffer_;
  Buffer axisOutOfPlaneNormalBuffer_;
  Buffer axisVertBuffer_;
  Buffer axisIndicesVertBuffer_;
  Buffer axisInPlaneVertBuffer_;
  Buffer axisPlaneNormalVertBuffer_;
  Buffer axisOutOfPlaneNormalVertBuffer_;
  Buffer axisTexCoordsHoriz_;
  Buffer axisTexCoordsVert_;
  Buffer overlayPosBuffer_;
  Buffer overlayTexCoBuffer_;
  Buffer clippingPlaneVertBuffer_;

  // Textures
  Texture horizAxisTexture_;
  Texture horizAxisTexture2_; // with a negative label-angle
  Texture vertAxisTexture_;
  Texture cubeTextureXY_;
  Texture cubeTextureXZ_;
  Texture cubeTextureYZ_;
  Texture titleTexture_;
  Texture legendTexture_;
  Texture colorMapTexture_;
  Texture meshIndexTexture_;
  Texture positionTexture_;
  Texture intersectionLinesTexture_;

  // Offscreen buffers
  Framebuffer meshIndexFramebuffer_;
  Framebuffer positionFramebuffer_;
  Framebuffer intersectionLinesFramebuffer_;
  Renderbuffer offscreenDepthbuffer_;

  std::vector<cpp17::any> objectsToDelete;

  friend class WAbstractGridData;
  friend class WGridData;
};

W_DECLARE_OPERATORS_FOR_FLAGS(ChartUpdates)

  }
}

#endif
