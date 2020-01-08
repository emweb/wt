// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CHART_WPLOTDATA_H
#define CHART_WPLOTDATA_H

#include <Wt/Chart/WAxis.h>
#include <Wt/WColor.h>
#include <Wt/WObject.h>
#include <Wt/WMatrix4x4.h>
#include <Wt/WGLWidget.h>

namespace Wt {
  class WAbstractItemModel;

  namespace Chart {
    class WCartesian3DChart;
    class WAbstractColorMap;

/*! \class WAbstractDataSeries3D
 *  \brief Abstract base class for dataseries that can be drawn on a 
 * WCartesian3DChart
 *
 * <h3>General</h3>
 * 
 * The model that is provided at construction or with setModel() contains the 
 * data of a dataseries. Implementations of this class format the data for 
 * representation on the chart and perform all necessary drawing operations. 
 * Note that if a dataseries holds numerical data it should be added to a chart 
 * of type \ref ChartType::Scatter, if it holds categorical data it should be added to 
 * a chart of type \ref ChartType::Category.
 *
 * <h3>Color</h3>
 *
 * The color used to draw data on a chart can be specified in a number of ways. 
 * The priority of this is as follows (1 being the highest):
 *
 * <ol>
 * <li>ItemDataRole::MarkerBrushColor set on a value in the model</li>
 * <li>WAbstractColorMap set on the dataseries</li>
 * <li>WChartPalette present in the chart</li>
 * </ol>
 * 
 * A chart-palette will specify one color for the entire dataseries. Each new 
 * dataseries on a chart will receive another color.\n A colormap assigns
 * different colors to the data within one dataseries, based on the z-value of 
 * the data. WStandardColorMap provides an easy way to create a colormap that 
 * is either smooth or consists of a number of bands.
 *
 * <h3>Data-roles</h3>
 *
 * The roles on the model which are taken into account are:\n
 * <ul>
 * <li>ItemDataRole::MarkerBrushColor: this determines the color of a datapoint and overrides the default</li>
 * <li>ItemDataRole::MarkerScaleFactor: this determines the size of a datapoint and overrides the default</li>
 * </ul> 
 *
 * Some representations of the data ignore these roles. For example, when a 
 * surface is drawn, the roles are ignored.
 *
 * <h3>Implementing a new dataseries class</h3>
 * 
 * When the existing implementations of %WAbstractDataSeries3D don't meet your 
 * needs, you might want to make your own. When doing this there are some 
 * details of the chart that you should know. The chart is made so that when 
 * a property of the chart changes, which affect any of the GL resources, 
 * all GL resources are destroyed and re-initialized. This eliminates the need 
 * to determine which chart-setting affect which GL-resources, which can be 
 * a complicated problem.
 *
 * Therefore only unchanging GL resources are initialized in initializeGL(). 
 * The initializeGL function in the chart is implemented to immediately request 
 * a call to updateGL(), which then initializes the rest of the GL resources. 
 * Every call to updateGL in the chart, will first call deleteAllGLResources() 
 * on all dataseries and will then call updateGL() on all dataseries. So, when 
 * implementing a dataseries: initialize unchanging GL resources in 
 * initializeGL(), initialize the rest of your GL resources in updateGL() and 
 * make GL-delete calls to all resources initialized in updateGL() in the 
 * function deleteAllGLResources(). It is also best to check isNull() on each 
 * of your GL-resources when deleting them.
 *
 * \ingroup charts
 */
class WT_API WAbstractDataSeries3D : public WObject {
public:
  /*! \brief Constructor
   *
   * This constructor takes a WAbstractItemModel as an argument. The model 
   * contains the data of this dataseries. How the model should be structured 
   * is dependent on the implementation. Therefore this information is 
   * found in the documentation of classes deriving from this one.
   */
  WAbstractDataSeries3D(std::shared_ptr<WAbstractItemModel> model);

  virtual ~WAbstractDataSeries3D();

  /*! \brief Sets the title of this dataseries.
   *
   * When a dataseries that did not have a title set, is added to a 
   * WCartesian3DChart it automatically gets the default title 'dataset i', 
   * with i the count of how many dataseries have been added to the chart.
   *
   * \sa name()
   */
  void setTitle(const WString& name);

  /*! \brief Returns the title of this dataseries.
   *
   * \sa setName()
   */
  const WString& title() const { return name_; }

  /*! \brief Sets a model from which the dataseries gets its data.
   *
   * Every dataseries needs a model from which it gets the data. How the
   * data is structured is determined by the type of dataseries. Therefore
   * more info on how to construct a proper model is provided in classes that
   * derive from this one.
   *
   * \sa model(), WAbstractDataSeries3D()
   */
  void setModel(const std::shared_ptr<WAbstractItemModel>& model);

  /*! \brief Returns a pointer to the model used by this dataseries.
   *
   * \sa setModel()
   */
  std::shared_ptr<WAbstractItemModel> model() const { return model_; }

  /*! \brief Returns the computed minimum value of this dataseries along the 
   * given axis.
   *
   * \sa maximum()
   */
  virtual double minimum(Axis axis) const = 0;

  /*! \brief Returns the computed maximum value of this dataseries along the 
   * given axis.
   *
   * \sa minimum()
   */
  virtual double maximum(Axis axis) const = 0;

  /*! \brief Returns a const pointer to the WCartesian3DChart on which the 
   * dataseries is drawn.
   */
  const WCartesian3DChart * chart() const { return chart_; }

  /*! \brief Sets the pointsize for drawing this dataseries.
   *
   * The default pointsize is 2 pixels.
   *
   * <em><b>Note: </b></em>Setting the point-size is currently not supported in IE.
   */
  void setPointSize(double size);

  /*! \brief Returns the pointsize for drawing this dataseries.
   *
   * \sa setPointSize()
   */
  double pointSize() const { return pointSize_; }

  /*! \brief Set the point sprite used for drawing this dataseries.
   *
   * This should be a local (server side) path to an image, such as a PNG or GIF.
   * Only the alpha channel of this image is used: the sprite only
   * decides if a pixel in the point appears or not. If the alpha
   * value is below 0.5, the pixel is discarded.
   *
   * For best effect, the point sprite's width and height should be
   * the same as the pointSize(), and the chart's antialiasing should
   * be disabled.
   *
   * Defaults to the empty string, meaning that every pixel of the
   * point will be drawn, yielding a square.
   */
  void setPointSprite(const std::string &image);

  /*! \brief Returns the point sprite used for drawing this dataseries.
   *
   * \sa setPointSprite()
   */
  const std::string &pointSprite() const { return pointSprite_; }

  /*! \brief Sets the colormap for this dataseries.
   *
   * Ownership of the WAbstractColorMap is transferred to this class.
   *
   * By default there is no colormap set. When a colormap is set on a 
   * dataseries, the color of WCartesian3DChart::palette() is no longer used 
   * for this series. The colormap associates a color to the data based on the 
   * z-value of the data. If the colormap is set to 0, the value of the palette 
   * will be used again.
   *
   * \sa setColorMapVisible(), setColorMapSide()
   */
  void setColorMap(const std::shared_ptr<WAbstractColorMap>& colormap);

  /*! \brief Returns the colormap used by this dataseries.
   *
   * If this dataseries has no colormap set, 0 will be returned.
   *
   * \sa setColorMap(), setColorMapVisible(), setColorMapSide()
   */
  const WAbstractColorMap * colorMap() const { return colormap_.get(); }

  /*! \brief Sets whether the colormap that is used should be shown alongside
   * the chart in the form of a legend.
   *
   * The default value is false.
   *
   * \sa setColorMap(), setColorMapSide()
   */
  void setColorMapVisible(bool enabled = true);

  /*! \brief Returns whether the colormap is shown alongside the chart in 
   * the form of a legend.
   *
   * \sa setColorMap(), setColorMapVisible(), setColorMapSide()
   */
  bool colorMapVisible() const { return showColorMap_; }

  /*! \brief Sets whether the colormap is shown on the left or right.
   *
   * The default side is Side::Right.
   *
   * Note: only Side::Left and Side::Right are valid values for this function. 
   *
   * \sa setColorMap(), setColorMapVisible()
   */
  void setColorMapSide(Side side);

  /*! \brief Returns on which side the colormap is shown.
   *
   * \sa setColorMap, setColorMapVisible(), setColorMapSide()
   */
  Side colorMapSide() const { return colorMapSide_; }

  /*! \brief Sets whether this dataseries is included in the chart-legend.
   *
   * By default, dataseries are enabled in the legend.
   */
  void setLegendEnabled(bool enabled = true) { legendEnabled_ = enabled; }

  /*! \brief Returns whether this dataseries is included in the chart-legend.
   *
   * \sa setLegendEnabled()
   */
  bool isLegendEnabled() const { return legendEnabled_; }

  /*! \brief Sets if this dataseries is hidden.
   *
   * By default dataseries are visible.
   */
  void setHidden(bool enabled = true);

  /*! \brief Returns if this dataseries is hidden.
   *
   * \sa setHidden()
   */
  bool isHidden() const { return hidden_; }

  // internal api used when a dataseries is added to a chart:
  //   if the title is empty, set it to "dataset i"
  void setDefaultTitle(int i);

  // internal api: first check for presence of a colormap!
  //   if there is no colormap, this will return the color from the chartpalette
  WColor chartpaletteColor() const;

  // internal api: only for use in WCartesian3DChart::addDataSeries
  virtual void setChart(WCartesian3DChart *chart);

  // internal api: only for use in WCartesian3DChart::removeDataSeries
  virtual std::vector<cpp17::any> getGlObjects();

  /*! \brief Initialize GL resources
   *
   * This function is called by initializeGL() in the chart to which this 
   * dataseries was added.
   */
  virtual void initializeGL() = 0;

  /*! \brief Update the client-side painting function.
   *
   * This function is called by paintGL() in the chart to which this 
   * dataseries was added.
   */
  virtual void paintGL() const = 0;

  /*! \brief Update GL resources
   *
   * This function is called by updateGL() in the chart to which this 
   * dataseries was added. Before this function is called, 
   * deleteAllGLResources() is called.
   *
   * \sa deleteAllGLResources()
   */
  virtual void updateGL() = 0;

  /*! \brief Act on resize events
   *
   * This function is called by resizeGL() in the chart to which this 
   * dataseries was added.
   */
  virtual void resizeGL() = 0;

  /*! \brief Delete GL resources
   *
   * This function is called by updateGL() in the chart to which this 
   * dataseries was added.
   */
  virtual void deleteAllGLResources() = 0;

protected:
  WGLWidget::Texture colorTexture();
  WGLWidget::Texture pointSpriteTexture();
  void loadPointSpriteTexture(const WGLWidget::Texture &tex) const;

  WString name_;

  std::shared_ptr<WAbstractItemModel> model_;
  WCartesian3DChart *chart_;

  mutable double zMin_, zMax_;
  mutable bool rangeCached_;

  double pointSize_;

  std::string pointSprite_;

  std::shared_ptr<WAbstractColorMap> colormap_;
  bool showColorMap_;
  Side colorMapSide_;

  bool legendEnabled_;

  bool hidden_;

  WMatrix4x4 mvMatrix_;

private:
  std::vector<Wt::Signals::connection> connections_;
};
    
  }
}


#endif
