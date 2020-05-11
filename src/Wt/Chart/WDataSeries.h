// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CHART_WDATA_SERIES_H_
#define CHART_WDATA_SERIES_H_

#include <Wt/Chart/WAxis.h>
#include <Wt/WBrush.h>
#include <Wt/WObject.h>
#include <Wt/WPen.h>
#include <Wt/WShadow.h>
#include <Wt/Chart/WChartGlobal.h>
#include <Wt/WPainterPath.h>

namespace Wt {

  class WPointF;

  namespace Chart {

    class WAbstractChartModel;

/*! \brief Enumeration that indicates an aspect of the look.
 *
 * These flags are used to keep track of which aspects of the look
 * that are overridden from the values provided by the chart palette,
 * using one of the methods in this class.
 *
 * \sa setPen(), setBrush(), setMarkerPen(), setMarkerBrush(), setLabelColor()
 */
enum class CustomFlag {
  Pen = 0x1,         //!< A custom pen is set.
  Brush = 0x2,       //!< A custom brush is set.
  MarkerPen = 0x4,   //!< A custom marker pen is set.
  MarkerBrush = 0x8, //!< A custom marker brush is set.
  LabelColor = 0x10  //!< A custom label color is set.
};

/*! \class WDataSeries Wt/Chart/WDataSeries.h Wt/Chart/WDataSeries.h
 *  \brief A single data series in a cartesian chart.
 *
 * This class configures all aspects for rendering a single data series
 * in a cartesian chart. A data series renders Y data from a single
 * model column against the X series configured for the chart.
 *
 * \if cpp
 * The data column should contain data that can be converted to
 * a number, but should not necessarily be of a number type, see
 * also asNumber(const boost::any&).
 * \elseif java
 * The data column should contain data that can be converted to
 * a number, but should not necessarily be of a number type, see
 * also {javadoclink eu.webtoolkit.jwt.StringUtils#asNumber(Object)}.
 * \endif
 *
 * Multiple series of different types may be combined on a single chart.
 *
 * \image html ChartWDataSeries-1.png "Different styles of data series"
 * 
 * For a category chart, series may be stacked on top of each other.
 * This is controlled by setStacked() for a series, which if enabled,
 * will stack that series on top of the preceding data series. This
 * works regardless of whether they are of the same type, but
 * obviously works visually best if these series are of the same
 * type. When not stacked, bar series are rendered next to each other.
 * The margin between bars of different data series is controlled
 * using WCartesianChart::setBarMargin().
 *
 * The line and color type are by default based on the \link
 * WCartesianChart::palette() chart palette\endlink, but may be
 * overridden for a series using setPen(), setBrush(), etc...
 *
 * \sa WCartesianChart::addSeries()
 *
 * \ingroup charts
 */
class WT_API WDataSeries
#ifdef WT_TARGET_JAVA
  : public Wt::WObject
#endif // WT_TARGET_JAVA
{
public:
  /*! \brief Constructs a new data series.
   *
   * Creates a new data series which plots the Y values from the
   * model column <i>modelColumn</i>, with the indicated
   * <i>seriesType</i>. The Y values are mapped to the indicated
   * <i>axis</i>, which should correspond to one of the two Y axes.
   *
   * \sa WCartesianChart::addSeries()
   */
  WDataSeries(int modelColumn, SeriesType seriesType = SeriesType::Point,
	      Axis axis = Axis::Y1);

  /*! \brief Constructs a new data series.
   *
   * Creates a new data series which plots the Y values from the
   * model column <i>modelColumn</i>, with the indicated
   * <i>seriesType</i>. The Y values are mapped to the indicated
   * <i>yAxis</i>, which should correspond to one of the two Y axes.
   *
   * \sa WCartesianChart::addSeries()
   */
  WDataSeries(int modelColumn, SeriesType seriesType, int yAxis);

  /*! \brief Destructor.
   */
  ~WDataSeries();

  /*! \brief Sets the bar width.
   *
   * The bar width specifies the bar width (in axis dimensions).  For
   * category plots, which may have several bars for different series
   * next to each other, you will want to specify the same bar width
   * for each series.
   *
   * For scatter plots, you may want to set the bar width to a natural
   * size. E.g. if you are plotting weekly measurements, you could set
   * the width to correspond to a week (=7).
   *
   * The default value is 0.8 (which leaves a 20% margin between bars
   * for different categories in a category chart.
   *
   * \sa WCartesianChart::setBarMargin()
   */
  void setBarWidth(const double width);

  /*! \brief Returns the bar width.
   *
   * \sa setBarWidth() 
   */
  double barWidth() const;

  /*! \brief Sets the series type.
   *
   * The series type specifies how the data is plotted, i.e. using
   * mere point markers, lines, curves, or bars.
   */
  void setType(SeriesType t);

  /*! \brief Returns the series type.
   *
   * \sa setType()
   */
  SeriesType type() const { return type_; }

  /*! \brief Sets the model column.
   *
   * This specifies the model column from which the Y data is retrieved
   * that is plotted by this series.
   *
   * The data column should contain data that can be converted to
   * a number (but should not necessarily be of a number type).
   * \if java 
   * See also {javadoclink eu.webtoolkit.jwt.StringUtils#asNumber(Object)}.
   * \endif
   *
   * \if cpp
   * \sa Wt::asNumber()
   * \endif
   */
  void setModelColumn(int modelColumn);

  /*! \brief Returns the model column.
   *
   * \sa setModelColumn()
   */
  int modelColumn() const { return modelColumn_; }

  /*! \brief Sets the X series column.
   *
   * By default, the data series uses the X series column configured
   * for the chart. For a scatter plot, each series can have its own
   * matching X data, which is configured here. For other plots, this
   * setting is ignored.
   *
   * The default value is -1, which indicates that
   * WCartesianChart::XSeriesColumn() is to be used.
   *
   * \sa WCartesianChart::setXSeriesColumn()
   */
  void setXSeriesColumn(int modelColumn);

  /*! \brief Returns the X series column.
   *
   * \sa setXSeriesColumn()
   */
  int XSeriesColumn() const { return XSeriesColumn_; }

  /*! \brief Sets whether this series is stacked on top of the preceding series.
   *
   * For category charts, data from different series may be rendered
   * stacked on top of each other. The rendered value is the sum of the
   * value of this series plus the rendered value of the preceding
   * series. For line series, you probably will want to add filling
   * under the curve. A stacked bar series is rendered by a bar on top
   * of the preceding bar series.
   *
   * The default value is false.
   */
  void setStacked(bool stacked);

  /*! \brief Returns whether this series is stacked on top of the preceding
   *         series.
   *
   * \sa setStacked()
   */
  bool isStacked() const { return stacked_; }

  /*! \brief Binds this series to a chart axis.
   *
   * A data series may be bound to either the first or second Y axis.
   * Note that the second Y axis is by default not displayed.
   *
   * The default value is the first Y axis.
   *
   * \sa WAxis::setVisible()
   */
  void bindToAxis(Axis axis);

  /*! \brief Binds this series to a chart's X axis.
   *
   * Note that the second Y axis will not be displayed by default.
   *
   * The default value is the first X axis.
   *
   * \sa WAxis::setVisible()
   */
  void bindToXAxis(int xAxis);

  /*! \brief Binds this series to a chart's Y axis.
   *
   * Note that the second Y axis will not be displayed by default.
   *
   * The default value is the first Y axis.
   *
   * \sa WAxis::setVisible()
   */
  void bindToYAxis(int yAxis);

  /*! \brief Returns the Y axis used for this series.
   *
   * \sa bindToAxis()
   */
  Axis axis() const { return yAxis_ == 1 ? Axis::Y2 : Axis::Y1; }

  /*! \brief Returns the Y axis used for this series.
   *
   * \sa bindToXAxis()
   */
  int xAxis() const { return xAxis_; }

  /*! \brief Returns the Y axis used for this series.
   *
   * \sa bindToYAxis()
   */
  int yAxis() const { return yAxis_; }

  /*! \brief Sets which aspects of the look are overriden.
   *
   * Set which aspects of the look, that are by default based on the
   * chart palette, are overridden by custom settings.
   *
   * The default value is 0 (nothing overridden).
   */
  void setCustomFlags(WFlags<CustomFlag> customFlags);

  /*! \brief Returns which aspects of the look are overriden.
   *
   * \sa setCustomFlags()
   */
  WFlags<CustomFlag> customFlags() const { return customFlags_; }

  /*! \brief Overrides the pen used for drawing lines for this series.
   *
   * Overrides the pen that is used to draw this series. Calling this
   * method automatically adds CustomPen to the custom flags.
   *
   * The default value is a default WPen().
   *
   * \sa WChartPalette::strokePen(), WChartPalette::borderPen()
   */
  void setPen(const WPen& pen);

  /*! \brief Returns the pen used for drawing lines for this series.
   *
   * \sa setPen()
   */
  WPen pen() const;

  /*! \brief Overrides the brush used for filling areas for this series.
   *
   * Overrides the brush that is used to draw this series which is
   * otherwise provided by the chart palette. For a bar plot, this is the
   * brush used to fill the bars. For a line chart, this is the brush
   * used to fill the area under (or above) the line. Calling this
   * method automatically adds CustomBrush to the custom flags.
   *
   * \sa WChartPalette::brush()
   */
  void setBrush(const WBrush& brush);

  /*! \brief Returns the brush used for filling areas for this series.
   *
   * \sa setBrush()
   */
  WBrush brush() const;

  /*! \brief Sets a shadow used for stroking lines for this series.
   */
  void setShadow(const WShadow& shadow);

  /*! \brief Returns the shadow used for stroking lines for this series.
   *
   * \sa setShadow()
   */
  const WShadow& shadow() const;

  /*! \brief Sets the fill range for line or curve series.
   *
   * Line or curve series may be filled under or above the curve,
   * using the brush(). This setting specifies the range that is
   * filled. The default value for all but SeriesType::Bar is FillRangeType::None.
   *
   * Bar series may use FillRangeType::MinimumValue to configure the chart to
   * render its bars from the data point to the bottom of the chart or
   * FillRangeType::MaximumValue to render the bars from the data point to the
   * top of the chart. The default value for SeriesType::Bar is
   * FillRangeType::ZeroValue, which render bars from zero to the data value.
   */
  void setFillRange(FillRangeType fillRange);

  /*! \brief Returns the fill range (for line, curve and bar series).
   *
   * \sa setFillRange()
   */
  FillRangeType fillRange() const;

  /*! \brief Sets the data point marker.
   *
   * Specifies a marker that is displayed at the (X,Y) coordinate for each
   * series data point.
   *
   * The default value is a MarkerType::Circle for a SeriesType::Point, or MarkerType::None
   * otherwise.
   *
   * \sa setMarkerPen(), setMarkerBrush(), setCustomMarker()
   */
  void setMarker(MarkerType marker);

  /*! \brief Sets the custom marker.
   *
   * This will also changes the marker type to MarkerType::Custom.
   *
   * \sa setMarker()
   */
  void setCustomMarker(const WPainterPath& path);

  /*! \brief Returns the custom marker.
   *
   * \sa setCustomMarker()
   */
  WPainterPath customMarker() const { return customMarker_; };

  /*! \brief Returns the data point marker.
   *
   * \sa setMarker()
   */
  MarkerType marker() const { return marker_; }

  /*! \brief Sets the marker size.
   *
   * The default marker size is 6 pixels.
   */
  void setMarkerSize(double size);

  /*! \brief Returns the marker size.
   *
   * \sa setMarkerSize()
   */
  double markerSize() const { return markerSize_; }

  /*! \brief Sets the marker pen.
   *
   * Overrides the pen used for stroking the marker. By default the
   * marker pen is the same as pen(). Calling this method automatically adds
   * CustomMarkerPen to the custom flags.
   *
   * \sa setPen(), setMarkerBrush()
   */
  void setMarkerPen(const WPen& pen);

  /*! \brief Returns the marker pen.
   *
   * \sa setMarkerPen()
   */
  WPen markerPen() const;

  /*! \brief Sets the marker brush.
   *
   * Overrides the brush used for filling the marker. By default the
   * marker brush is the same as brush(). Calling this method
   * automatically adds CustomMarkerBrush to the custom flags.
   *
   * \sa setBrush(), setMarkerPen()
   */
  void setMarkerBrush(const WBrush& brush);

  /*! \brief Returns the marker brush.
   *
   * \sa setMarkerBrush()
   */
  WBrush markerBrush() const;

  /*! \brief Enables the entry for this series in the legend.
   *
   * When <i>enabled</i>, this series is added to the chart
   * legend.
   *
   * The default value is true.
   *
   * \sa WCartesianChart::setLegendEnabled().
   */
  void setLegendEnabled(bool enabled);

  /*! \brief Returns whether this series has an entry in the legend.
   *
   * \sa setLegendEnabled()
   */
  bool isLegendEnabled() const;

  /*! \brief Enables a label that is shown at the series data points.
   *
   * You may enable labels for the Axis::X, Axis::Y or both axes. The
   * label that is displayed is the corresponding value on that
   * axis. If both labels are enabled then they are combined in a
   * single text using the format: "<x-value>: <y-value>".
   *
   * The default values are false for both axes (no labels).
   *
   * \sa isLabelsEnabled()
   */
  void setLabelsEnabled(Axis axis, bool enabled = true);

  /*! \brief Returns whether labels are enabled for the given axis.
   *
   * \sa setLabelsEnabled()
   */
  bool isLabelsEnabled(Axis axis) const;

  /*! \brief Sets the label color.
   *
   * Specify the color used for the rendering labels at the data
   * points.
   *
   * \sa setLabelsEnabled()
   */
  void setLabelColor(const WColor& color);

  /*! \brief Returns the label color.
   *
   * \sa setLabelColor()
   */
  WColor labelColor() const;

  /*! \brief Hide/unhide this series.
   *
   * A hidden series will not be show in the chart and legend.
   */
  void setHidden(bool hidden);

  /*! \brief Return whether the series is hidden.
   *
   * \sa setHidden()
   */
  bool isHidden() const;

  /*! \brief Maps from device coordinates to model coordinates.
   *
   * Maps a position in the chart back to model coordinates, for data
   * in this data series.
   *
   * This uses WChart::mapFromDevice() passing the axis() to which this
   * series is bound.
   *
   * This method uses the axis dimensions that are based on the latest
   * chart rendering. If you have not yet rendered the chart, or wish
   * to already the mapping reflect model changes since the last
   * rendering, you should call WCartesianChart::initLayout() first.
   *
   * \sa mapToDevice()
   */ 
  WPointF mapFromDevice(const WPointF& deviceCoordinates) const;

  /*! \brief Maps from model values to device coordinates.
   *
   * Maps model values to device coordinates, for data in this data series.
   *
   * This uses WChart::mapToDevice() passing the axis() to which this
   * series is bound.
   *
   * This method uses the axis dimensions that are based on the latest
   * chart rendering. If you have not yet rendered the chart, or wish
   * to already the mapping reflect model changes since the last
   * rendering, you should call WCartesianChart::initLayout() first.
   *
   * \sa mapFromDevice()
   */ 
  WPointF mapToDevice(const cpp17::any& xValue, const cpp17::any& yValue,
		      int segment = 0) const;

  /*! \brief Set an offset to draw the data series at.
   *
   * The Y position of the data series will be drawn at an offset,
   * expressed in model coordinates. The axis labels won't follow
   * the same offset.
   *
   * The offset can be manipulated client side using a mouse or touch
   * drag if WCartesianChart::curveManipulationEnabled() is enabled.
   *
   * \note This is only supported for axes with linear scale.
   *
   * \sa setScale()
   * \sa WCartesianChart::setCurveManipulationEnabled()
   */
  void setOffset(double offset);

  /*! \brief Get the offset for this data series.
   *
   * \sa setOffset()
   */
  double offset() const { return offset_; }


  /*! \brief Set the scale to draw the data series at.
   *
   * The Y position of the data series will be scaled around the zero
   * position, and offset by offset().
   *
   * The scale can be manipulated client side using the scroll wheel
   * or a pinch motion if WCartesianChart::curveManipulationEnabled()
   * is enabled.
   *
   * \note This is only supported for axes with linear scale.
   *
   * \sa setOffset()
   * \sa WCartesianChart::setCurveManipulationEnabled()
   */
  void setScale(double scale);

  /*! \brief Get the scale for this data series.
   *
   * \sa setScale()
   */
  double scale() const { return scale_; }

  /*! \brief Set a model for this data series.
   *
   * If no model is set for this data series, the model of
   * the chart will be used.
   *
   * \note Individual models per data series are only supported for
   * ChartType::Scatter type charts.
   *
   * \sa WCartesianChart::setModel()
   */
  void setModel(const std::shared_ptr<WAbstractChartModel>& model);

  /*! \brief Get the model for this data series.
   *
   * This will return the model set for this data series,
   * if it is set.
   *
   * If no model is set for this data series, and the series
   * is associated with  a chart, the model of the chart is returned.
   *
   * If no model is set for this data series, and the series is
   * not associated with any data series, this will return null.
   *
   * \sa setModel()
   * \sa WCartesianChart::setModel()
   */
  std::shared_ptr<WAbstractChartModel> model() const;

  WCartesianChart *chart() { return chart_; }

private:
  WCartesianChart   *chart_;
  std::shared_ptr<WAbstractChartModel> model_;
  int                modelColumn_;
  int                XSeriesColumn_;
  bool               stacked_;
  SeriesType         type_;
  int                xAxis_;
  int                yAxis_;
  WFlags<CustomFlag> customFlags_;
  WPen               pen_, markerPen_;
  WBrush             brush_, markerBrush_;
  WColor             labelColor_;
  WShadow            shadow_;
  FillRangeType      fillRange_;
  MarkerType         marker_;
  double             markerSize_;
  bool               legend_;
  bool               xLabel_;
  bool               yLabel_;
  double             barWidth_;
  bool               hidden_;
  WPainterPath       customMarker_;
  double             offset_;
  double             scale_;
  mutable bool       offsetDirty_;
  mutable bool       scaleDirty_;

  // connections with the current model, used to disconnect from a model
  // when the model changes.
  std::vector<Wt::Signals::connection> modelConnections_;

  void modelReset();

  template <typename T>
  bool set(T& m, const T& v);

  void setChart(WCartesianChart *chart);
  void update();

  friend class WCartesianChart;
  friend class LineSeriesRenderer;
};

template <typename T>
bool WDataSeries::set(T& m, const T& v)
{
  if (m != v) {
    m = v;
    update();
    return true;
  } else
    return false;
}

  }
}

#endif // CHART_WDATA_SERIES_H_
