// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CHART_WCARTESIAN_CHART_H_
#define CHART_WCARTESIAN_CHART_H_

#include <Wt/Chart/WAbstractChart.h>
#include <Wt/Chart/WAbstractChartModel.h>
#include <Wt/Chart/WAxis.h>
#include <Wt/Chart/WDataSeries.h>
#include <Wt/Chart/WLegend.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WContainerWidget.h>

namespace Wt {

class WAbstractItemModel;
class WPainter;
class WPainterPath;

  namespace Chart {

class WAxisSliderWidget;

/*! \class SeriesIterator Wt/Chart/WCartesianChart.h Wt/Chart/WCartesianChart.h
 *  \brief Abstract base class for iterating over series data in a chart.
 *
 * This class is specialized for rendering series data.
 *
 * \sa WCartesianChart::iterateSeries()
 *
 * \ingroup charts
 */
class WT_API SeriesIterator {
public:
  /*! \brief Destructor.
   */
  virtual ~SeriesIterator();

  /*! \brief Start handling a new segment.
   *
   * Because of a 'break' specified in an axis, axes may be divided in
   * one or two segments (in fact only the API limits this now to
   * two). The iterator will iterate all segments separately, but each time
   * with a different clipping region specified in the painter, corresponding
   * to that segment.
   *
   * The <i>currentSegmentArea</i> specifies the clipping area.
   */
  virtual void startSegment(int currentXSegment, int currentYSegment,
			    const WRectF& currentSegmentArea);

  /*! \brief End handling a particular segment.
   *
   * \sa startSegment()
   */
  virtual void endSegment();

  /*! \brief Start iterating a particular series.
   *
   * Returns whether the series values should be iterated.
   * The <i>groupWidth</i> is the width (in pixels) of a single bar
   * group. The chart contains <i>numBarGroups</i>, and the current
   * series is in the <i>currentBarGroup</i>'th group.
   */
  virtual bool startSeries(const WDataSeries& series, double groupWidth,
			   int numBarGroups, int currentBarGroup);

  /*! \brief End iterating a particular series.
   */
  virtual void endSeries();

  /*! \brief Process a value.
   *
   * Processes a value with model coordinates (<i>x</i>,
   * <i>y</i>). The y value may differ from the model's y value,
   * because of stacked series. The y value here corresponds to the
   * location on the chart, after stacking.
   *
   * The <i>stackY</i> argument is the y value from the previous
   * series (also after stacking). It will be 0, unless this series is
   * stacked.
   */
  virtual void newValue(const WDataSeries& series, double x, double y,
			double stackY, int xRow, int xColumn,
			int yRow, int yColumn);

  /*! \brief Returns the current X segment.
   */
  int currentXSegment() const { return currentXSegment_; }

  /*! \brief Returns the current Y segment.
   */
  int currentYSegment() const { return currentYSegment_; }

  static void setPenColor(WPen& pen, const WDataSeries& series,
		   int xRow, int xColumn,
		   int yRow, int yColumn,
		   ItemDataRole colorRole);

  static void setBrushColor(WBrush& brush, const WDataSeries& series,
		     int xRow, int xColumn,
		     int yRow, int yColumn,
		     ItemDataRole colorRole);

private:
  int currentXSegment_, currentYSegment_;
};

/*! \brief A curve label
 *
 * Curve labels can be added with WCartesianChart::addCurveLabel().
 * They are associated with a particular series, and are drawn at
 * the given point in model coordinates.
 * When the chart is transformed (zoom or pan)
 * or the associated series is manipulated, the curve label's position
 * will change, but not its size.
 *
 * \image html CurveLabel.png "A curve label"
 */
class WT_API CurveLabel {
public:
  /*! \brief Create a new curve label.
   *
   * Create a new curve label for given series,
   * at the given point with the given text.
   */
  CurveLabel(const WDataSeries &series, const WPointF &point, const WT_USTRING &label);

  /*! \brief Create a new curve label.
   *
   * Create a new curve label for given series,
   * at the given x, y coordinates and the given text.
   */
  CurveLabel(const WDataSeries &series, const cpp17::any &x, const cpp17::any &y, const WT_USTRING &label);

  /*! \brief Set the series this curve label is associated with.
   */
  void setSeries(const WDataSeries &series);

  /*! \brief Get the series this curve label is associated with.
   *
   * \sa setSeries()
   */
  const WDataSeries &series() const { return *series_; }

  /*! \brief Set the point in model coordinates this label is associated with.
   */
  void setPoint(const WPointF &point);

  /*! \brief Set the point in model coordinates this label is associated with.
   */
  void setPoint(const cpp17::any &x, const cpp17::any &y);

  /*! \brief Get the point in model coordinates this label is associated with.
   *
   * \note This uses asNumber(), which may not be the same conversion that
   *       WCartesianChart::mapToDevice() uses, depending on the scale of the
   *       axis. x() and y() will perform no conversion, so they may be safer to use.
   *
   * \sa setPoint()
   */
  const WPointF point() const;

  /*! \brief Get the x position for this label.
   *
   * \sa setPoint()
   */
  const cpp17::any &x() const { return x_; }

  /*! \brief Get the y position for this label.
   *
   * \sa setPoint()
   */
  const cpp17::any &y() const { return y_; }

  /*! \brief Set the label that should be drawn in the box.
   */
  void setLabel(const WT_USTRING &label);

  /*! \brief Get the label that should be drawn in the box.
   *
   * \sa setLabel()
   */
  const WT_USTRING &label() const { return label_; }

  /*! \brief Set the offset the text should be placed at.
   *
   * The offset is defined in pixels, with x values
   * going from left to right, and y values from top
   * to bottom.
   *
   * The default offset is (60, -20), which means the middle
   * of the label() is drawn 60 pixels to the right, and 20 pixels
   * above the point.
   */
  void setOffset(const WPointF &offset);

  /*! \brief Get the offset the text should be placed at.
   *
   * \sa setOffset()
   */
  const WPointF &offset() const { return offset_; }

  /*! \brief Set the width of the box in pixels.
   *
   * If the width is 0 (the default), server side font metrics
   * will be used to determine the size of the box.
   */
  void setWidth(int width);

  /*! \brief Get the width of the box in pixels.
   *
   * \sa setWidth()
   */
  int width() const { return width_; }

  /*! \brief Set the pen to use for the connecting line.
   *
   * This sets the pen to use for the line connecting the
   * \link point() point\endlink to the box with the
   * \link label() label\endlink at \link offset() offset\endlink
   * pixels from the point.
   */
  void setLinePen(const WPen &pen);

  /*! \brief Get the pen to use for the connecting line.
   *
   * \sa setLinePen()
   */
  const WPen &linePen() const { return linePen_; }

  /*! \brief Set the pen for the text in the box.
   */
  void setTextPen(const WPen &pen);

  /*! \brief Get the pen for the text in the box.
   *
   * \sa setTextPen()
   */
  const WPen &textPen() const { return textPen_; }

  /*! \brief Set the brush to use for the box around the text.
   *
   * This sets the brush used to fill the box with the
   * text defined in label().
   */
  void setBoxBrush(const WBrush &brush);

  /*! \brief Get the brush to use for the box around the text.
   *
   * \sa setBoxBrush()
   */
  const WBrush &boxBrush() const { return boxBrush_; }

  /*! \brief Set the brush used to fill the circle at point().
   */
  void setMarkerBrush(const WBrush &brush);

  /*! \brief Get the brush used to fill the circle at point().
   *
   * \sa setMarkerBrush()
   */
  const WBrush &markerBrush() const { return markerBrush_; }

  void render(WPainter &painter) const;

private:
  const WDataSeries *series_;
  cpp17::any x_, y_;
  WT_USTRING label_;
  WPointF offset_;
  int width_;
  WPen linePen_;
  WPen textPen_;
  WBrush boxBrush_;
  WBrush markerBrush_;

  static bool checkIntersectHorizontal(const WPointF &p1, const WPointF &p2, double minX, double maxX, double y);
  static bool checkIntersectVertical(const WPointF &p1, const WPointF &p2, double minY, double maxY, double x);
};

class WChart2DImplementation;
class WChartPalette;

/*! \brief Enumeration of mouse wheel actions for interactive charts.
 *
 * \sa WCartesianChart::WheelActions
 * \sa WCartesianChart::setWheelActions(WheelActions)
 *
 * \ingroup charts
 */
enum class InteractiveAction {
  ZoomX,        //!< Zoom x-axis
  ZoomY,        //!< Zoom y-axis
  ZoomXY,       //!< Zoom along both x and y-axes
  ZoomMatching, //!< Zoom y-axis on vertical scroll, x-axis on horizontal scroll
  PanX,         //!< Pan x-axis
  PanY,         //!< Pan y-axis
  PanMatching   //!< Pan y-axis on vertical scroll, x-axis on horizontal scroll
};

/*! \brief A map of mouse wheel actions for interactive charts, indexed by WFlags<KeyboardModifier>
 *
 * \sa WCartesianChart::setWheelActions(WheelActions)
 * \sa WCartesianChart::wheelActions()
 *
 * \ingroup charts
 */
typedef std::map<WFlags<KeyboardModifier>, InteractiveAction> WheelActions;

/*! \class WCartesianChart Wt/Chart/WCartesianChart.h Wt/Chart/WCartesianChart.h
 *  \brief A cartesian chart.
 *
 * A cartesian chart is a chart that uses X and Y axes. It can display
 * one or multiple data series, which each may be rendered using bars,
 * lines, areas, or points.
 *
 * To use a cartesian chart, the minimum you need to do is set a model
 * using setModel(), set the model column that holds the X data using
 * setXSeriesColumn(int modelColumn), and add one or more series using
 * addSeries(std::unique_ptr<WDataSeries>). Each series corresponds to
 * one data column that holds Y data.
 *
 * A cartesian chart is either a \link Chart::ChartType::Category
 * ChartType::Category\endlink or a \link Chart::ChartType::Scatter
 * ChartType::Scatter\endlink.
 *
 * In a <b>ChartType::Category</b>, the X series represent different
 * categories, which are listed consecutively in model row order. The
 * X axis scale is set to \link Chart::AxisScale::Discrete
 * AxisScale::Discrete\endlink.
 *
 * \image html ChartWCartesianChart-1.png "A category chart with bar series"
 *
 * Each series may be rendered differently, and this is configured in
 * the data series (see WDataSeries for more information).
 *
 * In a <b>ChartType::Scatter</b>, the X series data are interpreted as
 * numbers on a numerical scale. The scale for the X axis defaults to
 * a \link Chart::AxisScale::Linear AxisScale::Linear\endlink, but this may be
 * changed to a \link Chart::AxisScale::Date AxisScale::Date\endlink when the X
 * series contains dates (of type WDate) to create a time series
 * chart, or to a \link Chart::AxisScale::Log AxisScale::Log\endlink. A
 * ChartType::Scatter supports the same types of data series as a
 * ChartType::Category, but does not support stacking. In a scatter plot,
 * the X series do not need to be ordered in increasing values, and
 * may be set differently for each dataseries using
 * WDataSeries::setXSeriesColumn(int modelColumn).
 *
 * \image html ChartWCartesianChart-2.png "A time series scatter plot with line series"
 *
 * Missing data in a model series Y values is interpreted as a
 * <i>break</i>. For curve-like series, this breaks the curve (or
 * line).
 *
 * The cartesian chart has support for dual Y axes. Each data series may
 * be bound to one of the two Y axes. By default, only the first Y axis
 * is displayed. To show the second Y axis you will need to call:
 * 
 * \if cpp
 * \code
 * chart->axis(Axis::Y2).setVisible(true);
 * \endcode
 * \endif
 *
 * By default a chart has a horizontal X axis and a vertical Y axis,
 * which corresponds to a \link Wt::Orientation::Vertical Orientation::Vertical\endlink
 * orientation. The orientation may be changed to \link Wt::Orientation::Horizontal
 * Orientation::Horizontal\endlink using setOrientation().
 *
 * The styling of the series data are dictated by a palette which may
 * be set using setPalette(WChartPalette *), but may be overridden by
 * settings in each data series.
 *
 * <h3>CSS</h3>
 *
 * Styling through CSS is not applicable.
 *
 * \if java
 * <h3>Client-side interaction</h3>
 *
 * %WCartesianChart has several features that allow interaction with the
 * chart without server roundtrips. These features include zoom, pan,
 * crosshair and follow curve functionality.
 * 
 * \note Client side interaction is only available if the chart is drawn
 *       on an HTML canvas. This is the default rendering method on
 *       modern browsers, see WPaintedWidget::setPreferredMethod()
 *
 * \note Some features are currently not supported in interactive mode:
 *       - Axes set at AxisValue::Zero position will not always be drawn correctly.
 *	     They may be clipped off outside of the chart area, and when
 *         zooming, the axis ticks will change size.
 *       - WAxis::setBreak() is incompatible with interactive mode
 *
 * \endif
 *
 * \sa WDataSeries, WAxis
 * \sa WPieChart
 *
 * \ingroup charts modelview
 */
class WT_API WCartesianChart : public WAbstractChart
{

public:
  /*! \brief Creates a new cartesian chart.
   *
   * Creates a cartesian chart of type \link Wt::Chart::ChartType::Category
   * ChartType::Category\endlink.
   */
  WCartesianChart();

  /*! \brief Creates a new cartesian chart.
   *
   * Creates a cartesian chart of the indicated \p type.
   */
  WCartesianChart(ChartType type);

  ~WCartesianChart();

  /*! \brief Sets the chart type.
   *
   * The chart type determines how (x,y) data are interpreted. In a
   * \link Wt::Chart::ChartType::Category ChartType::Category\endlink, the X
   * values are categories, and these are plotted consecutively,
   * evenly spaced, and in row order. In a \link
   * Wt::Chart::ChartType::Scatter ChartType::Scatter\endlink, the X values are
   * interpreted numerically (as for Y values).
   *
   * The default chart type is a \link Wt::Chart::ChartType::Category
   * ChartType::Category\endlink.
   *
   * \sa type()
   * \sa WAxis::setScale(), axis(Axis)
   */
  void setType(ChartType type);

  /*! \brief Returns the chart type.
   *
   * \sa setType()
   */
  ChartType type() const { return type_; }

  /*! \brief Sets the chart orientation.
   *
   * Sets the chart orientation, which corresponds to the orientation of
   * the Y axis: a Wt::Orientation::Vertical orientation corresponds to the conventional
   * way of a horizontal X axis and vertical Y axis. A Wt::Orientation::Horizontal
   * orientation is the other way around.
   *
   * The default orientation is Wt::Orientation::Vertical.
   *
   * \sa orientation()
   */
  void setOrientation(Orientation orientation);

  /*! \brief Returns the chart orientation.
   *
   * \sa setOrientation()
   */  
  Orientation orientation() const { return orientation_; }

  /*! \brief Sets the the model column for the X series.
   *
   * Use this method to specify the default data for the X series.
   * For a \link Wt::Chart::ChartType::Scatter ChartType::Scatter\endlink this is
   * mandatory if an X series is not specified for every
   * \link Wt::Chart::WDataSeries WDataSeries\endlink. For a
   * \link Wt::Chart::ChartType::Category ChartType::Category\endlink, if not
   * specified, an increasing series of integer numbers will be
   * used (1, 2, ...).
   *
   * Scatterplot dataseries may each individually be given its own
   * X series data using WDataSeries::setXSeriesColumn(int modelColumn)
   *
   * The default value is -1 (not specified).
   *
   * The series column is reset to -1 when the model is set (or
   * changed). Thus you need to set a model before configuring the
   * series.
   *
   * \sa XSeriesColumn()
   */
  void setXSeriesColumn(int modelColumn);

  /*! \brief set the pen used to render the labels
   *
   * This method overwrites the pen for all axes
   *
   * \sa WAxis::setTextPen()
   */
  void setTextPen(const WPen& pen);

  /*! \brief Returns the model column for the X series.
   *
   * \sa setXSeriesColumn()
   */
  int XSeriesColumn() const { return XSeriesColumn_; }

  //later, activates a 3D plot
  //void setXYData(int modelColumnX, int modelColumnY);
  //bool is3D() const;

  /*! \brief Adds a data series.
   *
   * A single chart may display one or more data series. Each data series
   * displays data from a single model column in the chart. Series are
   * plotted in the order that they have been added to the chart.
   *
   * The series column is reset to -1 when the model is set (or
   * changed). Thus you need to set a model before configuring the
   * series.
   *
   * \sa removeSeries(), setSeries()
   */
  void addSeries(std::unique_ptr<WDataSeries> series);

  /*! \brief Removes a data series.
   *
   * This will disassociate the given series from any WAxisSliderWidgets.
   *
   * \sa addSeries(), setSeries()
   */
  std::unique_ptr<WDataSeries> removeSeries(WDataSeries *series);

  /*! \brief Sets all data series.
   *
   * Replaces the current list of series with the new list.
   *
   * \sa series(), addSeries(WDataSeries *), removeSeries(WDataSeries *)
   */
  void setSeries(std::vector<std::unique_ptr<WDataSeries> > series);

  /*! \brief Returns a data series corresponding to a data column.
   *
   * Returns a reference to the first data series that plots data
   * from \p modelColumn.
   */
  WDataSeries& series(int modelColumn);

  /*! \brief Returns a data series corresponding to a data column.
   *
   * Returns a const reference to the first data series that plots data
   * from \p modelColumn.
   */
  const WDataSeries& series(int modelColumn) const;

  /*! \brief Returns a list with the current data series.
   *
   * Returns the complete list of current data series.
   *
   * \sa setSeries(const std::vector<WDataSeries>&)
   */
  const std::vector<std::unique_ptr<WDataSeries> >& series() const { return series_; }

  /*! \brief Returns a chart axis.
   *
   * Returns a reference to the specified \p axis.
   */
  WAxis& axis(Axis axis);

  /*! \brief Accesses a chart axis.
   *
   * Returns a const reference to the specified \p axis.
   */
  const WAxis& axis(Axis axis) const;

  /*! \brief Sets an axis
   *
   * \sa axis(Axis axis)
   */
  void setAxis(std::unique_ptr<WAxis> waxis, Axis axis);

  /*! \brief Returns a vector of all X axes associated with this chart.
   *
   * This defaults to a vector of one axis.
   */
  std::vector<WAxis*> xAxes();

  /*! \brief Returns a vector of all X axes associated with this chart.
   *
   * This defaults to a vector of one axis.
   */
  std::vector<const WAxis*> xAxes() const;

  /*! \brief Returns a vector of all Y axes associated with this chart.
   *
   * This defaults to a vector of two axes: the Y1 and Y2 axes. Y1 will
   * be at index 0, and Y2 will be at index 1.
   */
  std::vector<WAxis*> yAxes();

  /*! \brief Returns a vector of all Y axes associated with this chart.
   *
   * This defaults to a vector of two axes: the Y1 and Y2 axes. Y1 will
   * be at index 0, and Y2 will be at index 1.
   */
  std::vector<const WAxis*> yAxes() const;

  /*! \brief Returns the number of X axes associated with this chart.
   */
  int xAxisCount() const;

  /*! \brief Returns the number of Y axes associated with this chart.
   */
  int yAxisCount() const;

  /*! \brief Retrieves the X axis at index i
   *
   * The following expression is always true:
   *
   * \if cpp
   * \code
   * &axis(X) == &xAxis(0)
   * \endcode
   * \endif
   *
   * \if java
   * \code
   * getAxis(Axis.XAxis) == getXAxis(0)
   * \endcode
   * \endif
   *
   * \note Precondition: 0 <= i < xAxisCount()
   */
  WAxis &xAxis(int i);

  /*! \brief Retrieves the X axis at index i
   *
   * The following expression is always true:
   *
   * \if cpp
   * \code
   * &axis(X) == &xAxis(0)
   * \endcode
   * \endif
   *
   * \if java
   * \code
   * getAxis(Axis.XAxis) == getXAxis(0)
   * \endcode
   * \endif
   *
   * \note Precondition: 0 <= i < xAxisCount()
   */
  const WAxis &xAxis(int i) const;

  /*! \brief Retrieves the Y axis at index i
   *
   * The following expressions are always true:
   *
   * \if cpp
   * \code
   * &axis(Y1) == &yAxis(0)
   * &axis(Y2) == &yAxis(1)
   * \endcode
   * \endif
   *
   * \if java
   * \code
   * axis(Y1) == yAxis(0)
   * axis(Y2) == yAxis(1)
   * \endcode
   * \endif
   *
   * \note Precondition: 0 <= i < yAxisCount()
   */
  WAxis &yAxis(int i);

  /*! \brief Retrieves the Y axis at index i
   *
   * The following expressions are always true:
   *
   * \if cpp
   * \code
   * &axis(Y1) == &yAxis(0)
   * &axis(Y2) == &yAxis(1)
   * \endcode
   * \endif
   *
   * \if java
   * \code
   * axis(Y1) == yAxis(0)
   * axis(Y2) == yAxis(1)
   * \endcode
   * \endif
   *
   * \note Precondition: 0 <= i < yAxisCount()
   */
  const WAxis &yAxis(int i) const;

  /*! \brief Adds a X axis to this chart.
   *
   * The first extra axis will have index 1, the next index 2,...
   *
   * Returns the index of the added axis.
   *
   * \note This transfers ownership of the given WAxis to this chart.
   *
   * \note Precondition: waxis is not null
   */
  int addXAxis(std::unique_ptr<WAxis> waxis);

  /*! \brief Adds a Y axis to this chart.
   *
   * The first extra axis will have index 2, the next index 3,...
   *
   * Returns the index of the added axis.
   *
   * \note This transfers ownership of the given WAxis to this chart.
   *
   * \note Precondition: waxis is not null
   */
  int addYAxis(std::unique_ptr<WAxis> waxis);

  /*! \brief Removes the X axis with the given id.
   *
   * The indices of the axes with an id higher than
   * xAxisId will be decremented.
   *
   * Any WDataSeries associated with the removed axis
   * are also removed.
   *
   * \note Precondition: 0 <= xAxisId < xAxisCount()
   */
  std::unique_ptr<WAxis> removeXAxis(int xAxisId);

  /*! \brief Removes the Y axis with the given id.
   *
   * The indices of the axes with an id higher than
   * yAxisId will be decremented.
   *
   * Any WDataSeries associated with the removed axis
   * are also removed.
   *
   * \note Precondition: 0 <= yAxisId < yAxisCount()
   */
  std::unique_ptr<WAxis> removeYAxis(int yAxisId);

  /*! \brief Clears all X axes.
   *
   * The effect is the same as repeatedly using removeYAxis()
   * until are axes are removed, i.e. any WDataSeries will
   * also be removed.
   */
  void clearXAxes();

  /*! \brief Clears all Y axes.
   *
   * The effect is the same as repeatedly using removeYAxis()
   * until are axes are removed, i.e. any WDataSeries will
   * also be removed.
   */
  void clearYAxes();

  /*! \brief Sets the margin between bars of different series.
   *
   * Use this method to change the margin that is set between bars of
   * different series. The margin is specified as a fraction of the
   * width. For example, a value of 0.1 adds a 10% margin between bars
   * of each series. Negative values are also allowed. For example, use
   * a margin of -1 to plot the bars of different series on top of
   * each other.
   *
   * The default value is 0.
   */
  void setBarMargin(double margin);

  /*! \brief Returns the margin between bars of different series.
   *
   * \sa setBarMargin(double)
   */
  double barMargin() const { return barMargin_; }

  /*! \brief Enables the legend.
   *
   * The location of the legend can be configured using
   * setLegendLocation(). Only series for which the legend is enabled
   * are included in this legend.
   *
   * The default value is \c false.
   *
   * \sa see WDataSeries::isLegendEnabled(), setLegendLocation()
   */
  void setLegendEnabled(bool enabled);

  /*! \brief Returns whether the legend is enabled.
   *
   * \sa setLegendEnabled()
   */
  bool isLegendEnabled() const { return legend_.isLegendEnabled(); }

  /*! \brief Configures the legend location.
   *
   * The legend can be renderd either inside or outside of the chart
   * area. When \p location is \link Wt::Chart::LegendLocation::Inside
   * Chart::LegendLocation::Inside\endlink, the legend will be rendered inside
   * the chart. When \p location is \link Wt::Chart::LegendLocation::Outside
   * Chart::LegendLocation::Outside\endlink, the legend is rendered outside the
   * chart, in the chart padding area.
   *
   * The provided \p side can either be Wt::Side::Left, Wt::Side::Right, Wt::Side::Top,
   * Wt::Side::Bottom and configures the side of the chart at which the
   * legend is displayed.
   *
   * The \p alignment specifies how the legend is aligned. This can be
   * a horizontal alignment flag (Wt::AlignmentFlag::Left, Wt::AlignmentFlag::Center, or
   * Wt::AlignmentFlag::Right), when the \p side is Side::Bottom or Side::Top, or a vertical
   * alignment flag (Wt::AlignmentFlag::Top, Wt::AlignmentFlag::Middle, or
   * Wt::AlignmentFlag::Bottom) when the \p side is Side::Left or Side::Right.
   *
   * The default location is \link Wt::Chart::LegendLocation::Outside
   * Chart::LegendLocation::Outside\endlink, Wt::Side::Right and
   * Wt::AlignmentFlag::Middle.
   *
   * To have more control over the legend, you could reimplement the
   * renderLegendItem() method to customize how one item in the legend
   * is rendered, or, alternatively you can disable the legend
   * generated by the chart itself, and reimplement the paint() method
   * in which you use the renderLegendItem() method repeatedly to
   * render a customized legend.
   *
   * \sa WDataSeries::setLegendEnabled()
   */
  void setLegendLocation(LegendLocation location, Side side,
			 AlignmentFlag alignment);

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
  void setLegendStyle(const WFont& font, const WPen& border,
		      const WBrush& background);

  /*! \brief Returns the legend location.
   *
   * \sa setLegendLocation()
   */
  LegendLocation legendLocation() const { return legend_.legendLocation(); }

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

  /*! \brief Returns the legend columns.
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

  /*! \brief Configures multiple legend columns.
   *
   * Multiple columns are typically useful when placing the legend at
   * the top or at the bottom of the chart.
   *
   * The default value is a single column, 100 pixels wide.
   *
   * When automatic chart layout is enabled, then the legend column width
   * is computed automatically, and this setting is ignored.
   *
   * \sa setAutoLayoutEnabled()
   */
  void setLegendColumns(int columns, const WLength& width);
  
  virtual void paint(WPainter& painter, const WRectF& rectangle = WRectF())
    const override;

  /*! \brief Draws the marker for a given data series.
   *
   * Draws the marker for the indicated \p series in the \p result.
   * This method is called while painting the chart, and you may
   * want to reimplement this method if you wish to provide a custom
   * marker for a particular data series.
   *
   * \sa setLegendEnabled()
   */
  virtual void drawMarker(const WDataSeries& series, WPainterPath& result)
    const;

  /*! \brief Renders the legend icon for a given data series.
   *
   * Renders the legend icon for the indicated \p series in the
   * \p painter at position \p pos.
   *
   * This method is called while rendering a legend item, and you may
   * want to reimplement this method if you wish to provide a custom
   * legend icon for a particular data series.
   *
   * \sa renderLegendItem()
   */
  virtual void renderLegendIcon(WPainter& painter, const WPointF& pos,
				const WDataSeries& series) const;

  /*! \brief Renders the legend item for a given data series.
   *
   * Renders the legend item for the indicated \p series in the
   * \p painter at position \p pos. The default implementation
   * draws the marker, and the series description to the right. The
   * series description is taken from the model's header data for that
   * series' data column.
   *
   * This method is called while painting the chart, and you may
   * want to reimplement this method if you wish to provide a custom
   * marker for a particular data series.
   *
   * \sa setLegendEnabled()
   */
  virtual void renderLegendItem(WPainter& painter, const WPointF& pos,
				const WDataSeries& series) const;

  /*! \brief Maps from device coordinates to model coordinates.
   *
   * Maps a position in the chart back to model coordinates.
   *
   * This uses the axis dimensions that are based on the latest chart
   * rendering. If you have not yet rendered the chart, or wish that
   * the mapping already reflects model changes since the last rendering,
   * you should call initLayout() first.
   *
   * If the chart is interactive, mapFromDevice will correctly take the current
   * zoom range into account.
   *
   * \sa mapToDevice()
   */
  WPointF mapFromDevice(const WPointF& point,
			Axis ordinateAxis = Axis::Ordinate) const;

  /*! \brief Maps from device coordinates to model coordinates.
   *
   * Maps a position in the chart back to model coordinates.
   *
   * This uses the axis dimensions that are based on the latest chart
   * rendering. If you have not yet rendered the chart, or wish that
   * the mapping already reflects model changes since the last rendering,
   * you should call initLayout() first.
   *
   * If the chart is interactive, mapFromDevice will correctly take the current
   * zoom range into account.
   *
   * \sa mapToDevice()
   */
  WPointF mapFromDevice(const WPointF& point,
                        int ordinateAxis) const;

  /*! \brief Maps from device coordinates to model coordinates, ignoring the current zoom range
   *
   * Maps a position in the chart back to model coordinates, as if the
   * chart was not zoomed in (nor panned).
   *
   * This uses the axis dimensions that are based on the latest chart
   * rendering. If you have not yet rendered the chart, or wish that
   * the mapping already reflects model changes since the last rendering,
   * you should call initLayout() first.
   *
   * This function will not take the current zoom range into
   * account. The mapping will be performed as if zoomRangeTransform()
   * is the identity transform.
   *
   * \sa mapToDeviceWithoutTransform()
   */
  WPointF mapFromDeviceWithoutTransform(const WPointF& point,
			Axis ordinateAxis = Axis::Ordinate) const;

  /*! \brief Maps from device coordinates to model coordinates.
   *
   * Maps a position in the chart back to model coordinates.
   *
   * This uses the axis dimensions that are based on the latest chart
   * rendering. If you have not yet rendered the chart, or wish that
   * the mapping already reflects model changes since the last rendering,
   * you should call initLayout() first.
   *
   * If the chart is interactive, mapFromDevice will correctly take the current
   * zoom range into account.
   *
   * \sa mapToDevice()
   */
  WPointF mapFromDevice(const WPointF &point,
                        const WAxis &xAxis,
                        const WAxis &yAxis) const;

  /*! \brief Maps from device coordinates to model coordinates, ignoring the current zoom range
   *
   * Maps a position in the chart back to model coordinates, as if the
   * chart was not zoomed in (nor panned).
   *
   * This uses the axis dimensions that are based on the latest chart
   * rendering. If you have not yet rendered the chart, or wish that
   * the mapping already reflects model changes since the last rendering,
   * you should call initLayout() first.
   *
   * This function will not take the current zoom range into
   * account. The mapping will be performed as if zoomRangeTransform()
   * is the identity transform.
   *
   * \sa mapToDeviceWithoutTransform()
   */
  WPointF mapFromDeviceWithoutTransform(const WPointF& point,
                        int ordinateAxis) const;

  /*! \brief Maps from device coordinates to model coordinates, ignoring the current zoom range
   *
   * Maps a position in the chart back to model coordinates, as if the
   * chart was not zoomed in (nor panned).
   *
   * This uses the axis dimensions that are based on the latest chart
   * rendering. If you have not yet rendered the chart, or wish that
   * the mapping already reflects model changes since the last rendering,
   * you should call initLayout() first.
   *
   * This function will not take the current zoom range into
   * account. The mapping will be performed as if zoomRangeTransform()
   * is the identity transform.
   *
   * \sa mapToDeviceWithoutTransform()
   */
  WPointF mapFromDeviceWithoutTransform(const WPointF& point,
                                        const WAxis &xAxis,
                                        const WAxis &yAxis) const;

  /*! \brief Maps model values onto chart coordinates.
   *
   * This returns the chart device coordinates for a (x,y) pair of model
   * values.
   *
   * This uses the axis dimensions that are based on the latest chart
   * rendering. If you have not yet rendered the chart, or wish that
   * the mapping already reflects model changes since the last rendering,
   * you should call initLayout() first.
   *
   * The \p xSegment and \p ySegment arguments are relevant only when
   * the corresponding axis is broken using WAxis::setBreak(). Then,
   * its possible values may be 0 (below the break) or 1 (above the
   * break).
   *
   * If the chart is interactive, mapToDevice will correctly take the current
   * zoom range into account.
   *
   * \sa mapFromDevice()
   */
  WPointF mapToDevice(const cpp17::any& xValue, const cpp17::any& yValue,
		      Axis axis = Axis::Ordinate, int xSegment = 0,
		      int ySegment = 0) const;

  /*! \brief Maps model values onto chart coordinates.
   *
   * This returns the chart device coordinates for a (x,y) pair of model
   * values.
   *
   * This uses the axis dimensions that are based on the latest chart
   * rendering. If you have not yet rendered the chart, or wish that
   * the mapping already reflects model changes since the last rendering,
   * you should call initLayout() first.
   *
   * The \p xSegment and \p ySegment arguments are relevant only when
   * the corresponding axis is broken using WAxis::setBreak(). Then,
   * its possible values may be 0 (below the break) or 1 (above the
   * break).
   *
   * If the chart is interactive, mapToDevice will correctly take the current
   * zoom range into account.
   *
   * \sa mapFromDevice()
   */
  WPointF mapToDevice(const cpp17::any& xValue, const cpp17::any& yValue,
                      int yAxis, int xSegment = 0,
                      int ySegment = 0) const;

  /*! \brief Maps model values onto chart coordinates.
   *
   * This returns the chart device coordinates for a (x,y) pair of model
   * values.
   *
   * This uses the axis dimensions that are based on the latest chart
   * rendering. If you have not yet rendered the chart, or wish that
   * the mapping already reflects model changes since the last rendering,
   * you should call initLayout() first.
   *
   * The \p xSegment and \p ySegment arguments are relevant only when
   * the corresponding axis is broken using WAxis::setBreak(). Then,
   * its possible values may be 0 (below the break) or 1 (above the
   * break).
   *
   * If the chart is interactive, mapToDevice will correctly take the current
   * zoom range into account.
   *
   * \sa mapFromDevice()
   */
  WPointF mapToDevice(const cpp17::any& xValue, const cpp17::any& yValue,
                      const WAxis &xAxis, const WAxis &yAxis,
                      int xSegment = 0, int ySegment = 0) const;

  /*! \brief Maps model values onto chart coordinates, ignoring the current zoom range
   *
   * This returns the chart device coordinates for a (x,y) pair of model
   * values.
   *
   * This uses the axis dimensions that are based on the latest chart
   * rendering. If you have not yet rendered the chart, or wish that
   * the mapping already reflects model changes since the last rendering,
   * you should call initLayout() first.
   *
   * The \p xSegment and \p ySegment arguments are relevant only when
   * the corresponding axis is broken using WAxis::setBreak(). Then,
   * its possible values may be 0 (below the break) or 1 (above the
   * break).
   *
   * This function will not take the current zoom range into
   * account.The mapping will be performed as if zoomRangeTransform()
   * is the identity transform.
   *
   * \sa mapFromDeviceWithoutTransform()
   */
  WPointF mapToDeviceWithoutTransform(const cpp17::any& xValue, const cpp17::any& yValue,
		      Axis axis = Axis::Ordinate, int xSegment = 0,
		      int ySegment = 0) const;

  /*! \brief Maps model values onto chart coordinates, ignoring the current zoom range
   *
   * This returns the chart device coordinates for a (x,y) pair of model
   * values.
   *
   * This uses the axis dimensions that are based on the latest chart
   * rendering. If you have not yet rendered the chart, or wish that
   * the mapping already reflects model changes since the last rendering,
   * you should call initLayout() first.
   *
   * The \p xSegment and \p ySegment arguments are relevant only when
   * the corresponding axis is broken using WAxis::setBreak(). Then,
   * its possible values may be 0 (below the break) or 1 (above the
   * break).
   *
   * This function will not take the current zoom range into
   * account.The mapping will be performed as if zoomRangeTransform()
   * is the identity transform.
   *
   * \sa mapFromDeviceWithoutTransform()
   */
  WPointF mapToDeviceWithoutTransform(const cpp17::any& xValue, const cpp17::any& yValue,
                      int yAxis, int xSegment = 0,
                      int ySegment = 0) const;

  /*! \brief Maps model values onto chart coordinates, ignoring the current zoom range
   *
   * This returns the chart device coordinates for a (x,y) pair of model
   * values.
   *
   * This uses the axis dimensions that are based on the latest chart
   * rendering. If you have not yet rendered the chart, or wish that
   * the mapping already reflects model changes since the last rendering,
   * you should call initLayout() first.
   *
   * The \p xSegment and \p ySegment arguments are relevant only when
   * the corresponding axis is broken using WAxis::setBreak(). Then,
   * its possible values may be 0 (below the break) or 1 (above the
   * break).
   *
   * This function will not take the current zoom range into
   * account.The mapping will be performed as if zoomRangeTransform()
   * is the identity transform.
   *
   * \sa mapFromDeviceWithoutTransform()
   */
  WPointF mapToDeviceWithoutTransform(const cpp17::any& xValue, const cpp17::any& yValue,
                                      const WAxis &xAxis, const WAxis &yAxis,
                                      int xSegment = 0, int ySegment = 0) const;

  /*! \brief Initializes the chart layout.
   *
   * The mapping between model and device coordinates is only
   * established after a rendering phase, or after calling initLayout
   * manually.
   *
   * You need a layout in order to use the mapFromDevice() and
   * mapToDevice() methods.
   *
   * Unless a specific chart rectangle is specified, the entire widget
   * area is assumed.
   */
  bool initLayout(const WRectF& rectangle = WRectF(),
		  WPaintDevice *device = nullptr) const;

  /*! \brief Creates a widget which renders the a legend item.
   * 
   * The legend item widget will contain a text and a WPaintedWidget 
   * which draws the series' symbol.
   */
  std::unique_ptr<WWidget> createLegendItemWidget(int index);

  /*! \brief Adds a data point area (used for displaying e.g. tooltips).
   *
   * You may want to specialize this is if you wish to modify (or
   * ignore) the area.
   *
   * \note Currently, an area is only created if the Wt::ItemDataRole::ToolTip data
   *       at the data point is not empty.
   */
  virtual void addDataPointArea(const WDataSeries& series,
				int xRow, int xColumn,
				std::unique_ptr<WAbstractArea> area);

  /*! \brief Sets the padding between the chart area and the axes.
   *
   * This calls WAxes::setPadding() on all axes.
   *
   * \sa axisPadding()
   */
  void setAxisPadding(int axisPadding);

  /*! \brief Returns the padding between the chart area and the axes.
   *
   * This number may not reflect the actual padding of the individual axes,
   * if another padding has been applied on the individual axes.
   *
   * \sa setAxisPadding()
   */
  int axisPadding() const { return axisPadding_; }

  /*! \brief Sets the pen of the border to be drawn around the chart area.
   *
   * \sa borderPen()
   */
  void setBorderPen(const WPen& pen);

  /*! \brief Returns the pen used to draw the border around the chart area.
   *
   * Defaults to PenStyle::None.
   *
   * \sa setBorderPen()
   */
  const WPen& borderPen() const { return borderPen_; }

  /*! \brief Add a curve label.
   *
   * \sa CurveLabel::CurveLabel()
   */
  void addCurveLabel(const CurveLabel &label);

  /*! \brief Configure all curve labels at once.
   *
   * \sa addCurveLabel()
   */
  void setCurveLabels(const std::vector<CurveLabel>& labels);

  /*! \brief Clear all curve labels
   *
   * \sa addCurveLabel()
   */
  void clearCurveLabels();

  /*! \brief Get all of the registered curve labels
   *
   * \sa setCurveLabels()
   */
  const std::vector<CurveLabel>& curveLabels() const { return curveLabels_; }

  /*! @name Client side interaction
   *
   * These methods allow to activate the client side interactive features of
   * a %WCartesianChart.
   * 
   * \note Client side interaction is only available if the chart is drawn
   *       on an HTML canvas. This is the default rendering method on
   *       modern browsers, see WPaintedWidget::setPreferredMethod()
   *
   * \note Some features are currently not supported in interactive mode:
   *       - Axes set at AxisValue::Zero position will not always be drawn correctly.
   *	     They may be clipped off outside of the chart area, and when
   *         zooming, the axis ticks will change size.
   *       - WAxis::setBreak() is incompatible with interactive mode
   */
  //!@{

  /*! \brief Returns whether this chart is interactive.
   *
   * Return true iff one of the interactive features is enabled, and
   * the chart is being rendered on an HTML canvas.
   */
  bool isInteractive() const;

  /*! \brief Enables zoom functionality.
   *
   * When using the mouse, press the ctrl key while scrolling to zoom
   * in/out a specific point on the chart. If you press shift+ctrl, it
   * will only zoom vertically. If you press alt+ctrl, it will only
   * zoom horizontally. To change these default mappings, use
   * setWheelActions().
   *
   * When using touch, you can use a pinch gesture to zoom in/out. If
   * the pinch gesture is vertical/horizontal, it will zoom only
   * vertically/horizontally, otherwise it will zoom both axes
   * equally.
   *
   * The default value is \c false.
   *
   * \sa zoomEnabled()
   * \sa setWheelActions(WheelActions)
   */
  void setZoomEnabled(bool zoom = true);

  /*! \brief Returns whether zoom is enabled.
   *
   * \sa setZoomEnabled()
   */
  bool zoomEnabled() const;

  /*! \brief Enables pan functionality.
   *
   * When using the mouse, you can click and drag to pan the chart (if
   * zoomed in), or use the scrollwheel.
   *
   * When using touch, you can drag to pan the chart. If the
   * rubberband effect is enabled, this is intertial (it will keep
   * scrolling after you let go) and there is an overscroll and bounce
   * back effect on the sides.
   *
   * The default value is \c false.
   *
   * \sa panEnabled()
   */
  void setPanEnabled(bool pan = true);
  
  /*! \brief Returns whether pan is enabled.
   *
   * \sa setPanEnabled()
   */
  bool panEnabled() const;

  /*! \brief Enables the crosshair functionality.
   *
   * When enabled, the crosshair will follow mouse movement,
   * and show in the top right corner the coordinate
   * (according to X axis and the first Y axis)
   * corresponding to this position.
   *
   * When using touch, the crosshair can be moved with a drag. If both
   * panning and the crosshair are enabled, the crosshair will be moved
   * when dragging close to the crosshair. Otherwise, the chart will pan.
   */
  void setCrosshairEnabled(bool crosshair = true);

  /*! \brief Returns whether the crosshair is enabled.
   *
   * \sa setCrosshairEnabled()
   */
  bool crosshairEnabled() const;

  /*! \brief Sets the crosshair color.
   *
   * The crosshair color is black by default.
   *
   * \sa setCrosshairEnabled()
   */
  void setCrosshairColor(const WColor &color);

  /*! \brief Returns the crosshair color.
   *
   * \sa setCrosshairEnabled(), setCrosshairColor()
   */
  const WColor& crosshairColor() const { return crosshairColor_; }

  /*! \brief Sets the X axis to use for the crosshair.
   *
   * Defaults to 0 (first X axis)
   */
  void setCrosshairXAxis(int xAxis);

  /*! \brief Returns the X axis to use for the crosshair.
   */
  int crosshairXAxis() const { return crosshairXAxis_; }

  /*! \brief Sets the Y axis to use for the crosshair.
   *
   * Defaults to 0 (first Y axis)
   */
  void setCrosshairYAxis(int yAxis);

  /*! \brief Returns the Y axis to use for the crosshair.
   */
  int crosshairYAxis() const { return crosshairYAxis_; }

  /*! \brief Enables the follow curve functionality for a data series.
   *
   * This enables follow curve functionality for the data series
   * corresponding to the given column.
   *
   * If the data series is of type SeriesType::Line or SeriesType::Curve, the
   * crosshair can only be moved in the x direction. The y position of
   * the crosshair will be determined by the value of the data
   * series. The crosshair will snap to the nearest point that is
   * defined in the data series.
   *
   * When using the mouse, the x position will change on mouseover.
   * When using touch, the x position can be moved with a drag. The
   * follow curve functionality has priority over the crosshair functionality.
   *
   * Use column index -1 or disableFollowCurve() to disable the follow
   * curve feature.
   *
   * \note The follow curve functionality requires that the X axis values
   *       of the data series are monotonically increasing or decreasing.
   *
   * \deprecated Use setFollowCurve(const WDataSeries*) instead
   */
  void setFollowCurve(int modelColumn);

  /*! \brief Enabled the follow curve funtionality for a data series.
   *
   * This enables follow curve functionality for the data series
   * corresponding to the given column.
   *
   * If the data series is of type SeriesType::Line or SeriesType::Curve, the
   * crosshair can only be moved in the x direction. The y position of
   * the crosshair will be determined by the value of the data
   * series. The crosshair will snap to the nearest point that is
   * defined in the data series.
   *
   * When using the mouse, the x position will change on mouseover.
   * When using touch, the x position can be moved with a drag. The
   * follow curve functionality has priority over the crosshair functionality.
   *
   * Set to null to disable the follow curve feature.
   *
   * \note The follow curve functionality requires that the X axis values
   *       of the data series are monotonically increasing or decreasing.
   */
  void setFollowCurve(const WDataSeries *series);

  /*! \brief Disable the follow curve functionality.
   *
   * \sa setFollowCurve()
   */
  void disableFollowCurve();

  /*! \brief Returns the curve that is to be followed.
   *
   * If follow curve functionality is not enabled, returns -1.
   *
   * \sa setFollowCurve()
   */
  const WDataSeries *followCurve() const;

  /*! \brief Enables/disables the inertial scrolling and rubberband effect.
   *
   * \sa setPanEnabled()
   */
  void setRubberBandEffectEnabled(bool rubberBand = true);

  /*! \brief Checks whether the rubberband effect is enabled.
   * 
   * \see setRubberBandEffectEnabled()
   */
  bool rubberBandEffectEnabled() const;

  /*! \brief Sets the mapping of mouse wheel actions for interactive charts.
   *
   * \sa wheelActions()
   */
  void setWheelActions(WheelActions wheelActions);

  /*! \brief Returns the current mouse wheel actions for interactive charts.
   *
   * \sa setWheelActions()
   */
  WheelActions wheelActions() const { return wheelActions_; }
  
  /*! \brief Enables or disables soft label clipping on all axes.
   *
   * \sa WAxis::setSoftLabelClipping()
   */
  void setSoftLabelClipping(bool enabled);

  /*! \brief Sets whether series selection is enabled.
   *
   * If series selection is enabled, series can be selected with a mouse
   * click or long press. If the selected series is a SeriesType::Line or SeriesType::Curve,
   * it can be manipulated if \link setCurveManipulationEnabled()
   * curve manipulation\endlink is enabled. The series that are not selected,
   * will be shown in a lighter color.
   */
  void setSeriesSelectionEnabled(bool enabled = true);

  /*! \brief Returns whether series selection is enabled.
   *
   * \sa setSeriesSelectionEnabled()
   */
  bool seriesSelectionEnabled() const { return seriesSelectionEnabled_; }

  /*! \brief A signal that notifies the selection of a new curve.
   *
   * This signal is emitted if a series is selected using a mouse
   * click or long press. The first argument is the selected
   * series. The second argument is the point that was selected, in model
   * coordinates.
   *
   * \sa setSeriesSelectionEnabled()
   */
  Signal<const WDataSeries *, WPointF>& seriesSelected() { return seriesSelected_; }

  /*! \brief Sets the series that is currently selected.
   *
   * The series with the given model column will be selected. The other
   * series will be shown in a lighter color. The series that is currently
   * selected is the one that can be manipulated if \link setCurveManipulationEnabled()
   * curve manipulation\endlink is enabled, and it is a SeriesType::Line or SeriesType::Curve.
   *
   * The selected series can be changed using a long touch or mouse click.
   *
   * If the argument provided is null or \link setSeriesSelectionEnabled() series
   * selection\endlink is not enabled, no series will be selected.
   *
   * \sa setCurveManipulationEnabled()
   * \sa setSeriesSelectionEnabled()
   */
  void setSelectedSeries(const WDataSeries *series);

  /*! \brief Get the currently selected curve.
   *
   * -1 means that no curve is currently selected.
   *
   * \sa setSelectedSeries()
   */
  const WDataSeries *selectedSeries() const { return selectedSeries_; }

  /*! \brief Enable curve manipulation.
   *
   * If curve manipulation is enabled, the \link WDataSeries::setScale() scale\endlink
   * and \link WDataSeries::setOffset() offset\endlink of the \link selectedSeries()
   * selected curve\endlink can be manipulated interactively using drag, scroll,
   * and pinch.
   *
   * \sa WDataSeries::setOffset()
   * \sa WDataSeries::setScale()
   * \sa WDataSeries::selectedSeries()
   */
  void setCurveManipulationEnabled(bool enabled = true);

  /*! \brief Returns whether curve manipulation is enabled.
   *
   * \sa setCurveManipulationEnabled()
   */
  bool curveManipulationEnabled() const { return curveManipulationEnabled_; }

  /*! \brief Enable on-demand loading
   *
   * By default, when on-demand loading is not enabled, the
   * entire chart area is loaded, regardless of whether it is
   * within the current zoom range of the X axis.
   *
   * When on-demand loading is enabled only the currently visible area +
   * some margin is loaded. As the visible area changes, different data
   * is loaded. This improves performance for charts with a lot of data
   * if not all of the data needs to be visible at the same time.
   *
   * This feature is especially useful in combination with WAxis::setMaximumZoomRange()
   * or WAxis::setMinZoom(), which makes it impossible for the user to view
   * all of the data at the same time, because that would incur too much overhead.
   *
   * \note On-demand loading requires that the X axis data for all data series
   *       is sorted in ascending order. This feature is optimized for equidistant
   *       X axis data, but that's not a requirement.
   *
   * \note If no minimum or maximum are set on the Y axis (or axes), then
   *       the chart will still have to scan all data of its data series to automatically
   *       determine the minimum and maximum Y axis values. If this performance hit is undesirable
   *       and the Y axis range is known or guaranteed to be within a certain range, make sure to
   *       \link WAxis::setRange() set a range\endlink on the Y axis (or axes).
   *
   * \sa onDemandLoadingEnabled()
   */
  void setOnDemandLoadingEnabled(bool enabled);

  /*! \brief Returns whether on-demand loading is enabled.
   *
   * \sa setOnDemandLoadingEnabled()
   */
  bool onDemandLoadingEnabled() const { return onDemandLoadingEnabled_; }

  /*! \brief Set the background brush for the unloaded area.
   *
   * \sa setOnDemandLoadingEnabled()
   * \sa loadingBackground()
   */
  void setLoadingBackground(const WBrush& brush);

  /*! \brief Returns the background brush for the unloaded area.
   *
   * \sa setOnDemandLoadingEnabled()
   * \sa setLoadingBackground()
   */
  const WBrush& loadingBackground() const { return loadingBackground_; }

  //!@}

  void iterateSeries(SeriesIterator *iterator,
		     WPainter *painter, bool reverseStacked = false, bool extremesOnly = false) const;

  // For use in WAxisSliderWidget
  void addAxisSliderWidget(WAxisSliderWidget *slider);
  void removeAxisSliderWidget(WAxisSliderWidget *slider);

private:
  struct AxisLocation {
    AxisLocation()
      : minOffset(0),
        maxOffset(0),
        initLoc(AxisValue::Minimum),
        finLoc(AxisValue::Minimum)
    { }

    int minOffset; // the number of pixels the axis should be shifted beyond
                   // the minimum due to this axis being an extra Y axis on
                   // the MinimumValue side
    int maxOffset; // the number of pixels the axis should be shifted beyond
                   // the maximum due to this axis being an extra Y axis on
                   // the MaximumValue side
    AxisValue initLoc; // where the axis has been placed after the
                       // ZeroValue out of bounds -> Minimum/Maximum transformation
    AxisValue finLoc; // where the axis has been placed after the
                      // Minimum/Maximum at the bound -> ZeroValue transformation
  };
  struct PenAssignment {
    WJavaScriptHandle<WPen> pen;
    WJavaScriptHandle<WPen> textPen;
    WJavaScriptHandle<WPen> gridPen;

    PenAssignment(const WJavaScriptHandle<WPen>& pen,
		  const WJavaScriptHandle<WPen>& textPen,
		  const WJavaScriptHandle<WPen>& gridPen)
      : pen(pen), textPen(textPen), gridPen(gridPen)
    { }
  };
  struct AxisStruct {
    AxisStruct() noexcept;
    AxisStruct(std::unique_ptr<WAxis> ax) noexcept;
    AxisStruct(const AxisStruct &) = delete;
    AxisStruct& operator=(const AxisStruct &) = delete;
    AxisStruct(AxisStruct&&) noexcept;
    AxisStruct& operator=(AxisStruct&&) noexcept;
    ~AxisStruct();

    std::unique_ptr<WAxis> axis;
    mutable int calculatedWidth;
    mutable AxisLocation location;
    mutable WTransform transform;
    WJavaScriptHandle<WTransform> transformHandle;
    std::unique_ptr<JSignal<>> transformChanged;
    std::vector<PenAssignment> pens;
  };

  std::unique_ptr<WChart2DImplementation> interface_;

  Orientation orientation_;
  int XSeriesColumn_;
  ChartType type_;
  std::vector<std::unique_ptr<WDataSeries> > series_;
  std::vector<AxisStruct> xAxes_;
  std::vector<AxisStruct> yAxes_;
  double barMargin_;
  WLegend legend_;
  int axisPadding_;
  WPen borderPen_;
  WPen textPen_;

  /* render state */
  mutable int width_, height_;
  mutable WRectF chartArea_;
  mutable bool hasDeferredToolTips_;

  bool jsDefined_;
  bool zoomEnabled_;
  bool panEnabled_;
  bool rubberBandEnabled_;
  bool crosshairEnabled_;
  WColor crosshairColor_;
  int crosshairXAxis_;
  int crosshairYAxis_;
  bool seriesSelectionEnabled_;
  const WDataSeries *selectedSeries_;
  const WDataSeries *followCurve_;
  bool curveManipulationEnabled_;
  bool onDemandLoadingEnabled_;
  WBrush loadingBackground_;
  bool cObjCreated_;

  Signal<const WDataSeries *, WPointF> seriesSelected_;
  JSignal<double, double> jsSeriesSelected_;

  typedef std::map<const WDataSeries *, WJavaScriptHandle<WPainterPath> > PainterPathMap;
  mutable PainterPathMap curvePaths_;
  std::vector<WJavaScriptHandle<WPainterPath> > freePainterPaths_;

  typedef std::map<const WDataSeries *, WJavaScriptHandle<WTransform> > TransformMap;
  mutable TransformMap curveTransforms_;
  std::vector<WJavaScriptHandle<WTransform> > freeTransforms_;

  std::vector<WJavaScriptHandle<WPen> > freePens_;

  std::vector<CurveLabel> curveLabels_;

  std::vector<WAxisSliderWidget *> axisSliderWidgets_;

  WheelActions wheelActions_;

  JSignal<double, double> loadTooltip_;
  struct BarTooltip {
    BarTooltip(const WDataSeries &series, int xRow, int xColumn, int yRow, int yColumn)
      : series(&series),
	xRow(xRow),
	xColumn(xColumn),
	yRow(yRow),
	yColumn(yColumn)
    { }

    double xs[4];
    double ys[4];
    const WDataSeries *series;
    int xRow, xColumn, yRow, yColumn;
  };
  mutable std::vector<BarTooltip> barTooltips_;

  void init();
  static std::string wheelActionsToJson(WheelActions wheelActions);

  static WColor lightenColor(const WColor &in);

  virtual void getDomChanges(std::vector<DomElement *>& result, WApplication *app) override;

protected:
  virtual void modelChanged() override;
  virtual void modelReset() override;

  /** @name Rendering logic
   */
  //!@{
  /*! \brief Paints the widget.
   *
   * This calls render() to paint on the paint device.
   */
  virtual void paintEvent(WPaintDevice *paintDevice) override;

  /*! \brief Renders the chart.
   *
   * Renders the chart within the given rectangle. To accomodate both
   * rendering of horizontal and vertically oriented charts, all rendering
   * logic assumes horizontal. This "chart coordinates" space is transformed
   * to painter coordinates using hv().
   *
   * \if cpp
   * The default implementation looks like:
   * \code
   * painter.save();
   * painter.translate(rectangle.topLeft());
   *
   * if (initLayout(rectangle, painter.device())) {
   *   renderBackground(painter);
   *   renderGrid(painter, axis(Axis::X));
   *   for (int i = 0; i < yAxisCount(); ++i)
   *     renderGrid(painter, yAxis(i));
   *   renderAxes(painter, AxisProperty::Line);
   *   renderSeries(painter);
   *   renderAxes(painter, AxisProperty::Labels);
   *   renderBorder(painter);
   *   renderCurveLabels(painter);
   *   renderLegend(painter);
   *   renderOther(painter);
   * }
   *
   * painter.restore();
   * \endcode
   * \endif
   */
  virtual void render(WPainter& painter, const WRectF& rectangle) const;

  /*! \brief Map (x, y) value pair to chart coordinates coordinates.
   *
   * The result needs further transformation using hv() to painter
   * coordinates.
   *
   * \if cpp
   *
   * The default implementation is:
   *
   * \code
   * return map(xValue, yValue, yAxis == Y1Axis ? 0 : 1, currentXSegment, currentYSegment);
   * \endcode
   *
   * \endif
   */
  virtual WPointF map(double xValue, double yValue, 
		      Axis yAxis = Axis::Ordinate,
		      int currentXSegment = 0, int currentYSegment = 0) const;

  /*! \brief Map (x, y) value pair to chart coordinates coordinates.
   *
   * The result needs further transformation using hv() to painter
   * coordinates.
   *
   * \if cpp
   *
   * The default implementation is:
   *
   * \code
   * return map(xValue, yValue, xAxis(0), this->yAxis(yAxis), currentXSegment, currentYSegment);
   * \endcode
   *
   * \endif
   */
  virtual WPointF map(double xValue, double yValue, int yAxis,
                      int currentXSegment = 0, int currentYSegment = 0)
    const;

  /*! \brief Map (x, y) value pair to chart coordinates coordinates.
   *
   * The result needs further transformation using hv() to painter
   * coordinates.
   */
  virtual WPointF map(double xValue, double yValue,
                      const WAxis &xAxis, const WAxis &yAxis,
                      int currentXSegment = 0, int currentYSegment = 0) const;

  /*! \brief Utility function for rendering text.
   *
   * This method renders text on the chart position <i>pos</i>, with a
   * particular alignment <i>flags</i>. These are both specified in
   * chart coordinates. The position is converted to painter
   * coordinates using hv(), and the alignment flags are changed
   * accordingly. The rotation, indicated by <i>angle</i> is specified
   * in painter coordinates and thus an angle of 0 always indicates
   * horizontal text, regardless of the chart orientation.
   */
  virtual void renderLabel(WPainter& painter, const WString& text,
			   const WPointF& pos,
			   WFlags<AlignmentFlag> flags, double angle,
			   int margin) const;

  /*! \brief Conversion between chart and painter coordinates.
   *
   * Converts from chart coordinates to painter coordinates, taking
   * into account the chart orientation.
   */
  WPointF hv(double x, double y) const;

  /*! \brief Conversion between chart and painter coordinates.
   *
   * Converts from chart coordinates to painter coordinates, taking
   * into account the chart orientation.
   */
  WPointF hv(const WPointF& f) const;

  /*! \brief Conversion between chart and painter coordinates.
   *
   * Converts from chart coordinates to painter coordinates, taking
   * into account the chart orientation.
   */
  WRectF hv(const WRectF& f) const;

  /*! \brief Returns the segment area for a combination of X and Y
   *         segments.
   *
   * This segment area is used for clipping when rendering in a
   * particular segment.
   */
  WRectF chartSegmentArea(const WAxis& yAxis, int xSegment, int ySegment) const;

  /*! \brief Returns the segment area for a combination of X and Y
   *         segments.
   *
   * This segment area is used for clipping when rendering in a
   * particular segment.
   */
  WRectF chartSegmentArea(const WAxis& xAxis, const WAxis& yAxis, int xSegment, int ySegment) const;

  /*! \brief Calculates the chart area.
   *
   * This calculates the chartArea(), which is the rectangle (in chart
   * coordinates) that bounds the actual chart (thus excluding axes,
   * labels, titles, legend, etc...).
   *
   * \sa plotAreaPadding()
   */
  virtual void calcChartArea() const;

  /*! \brief Prepares the axes for rendering.
   *
   * Computes axis properties such as the range (if not manually
   * specified), label interval (if not manually specified) and axis
   * locations. These properties are stored within the axes.
   *
   * \sa initLayout()
   */
  virtual bool prepareAxes(WPaintDevice *device) const;

  /*! \brief Renders the background.
   *
   * \sa render()
   */
  virtual void renderBackground(WPainter& painter) const;

  /*! \brief Renders one or more properties of the axes.
   *
   * This calls renderAxis() for each axis.
   *
   * \sa render()
   */
  virtual void renderAxes(WPainter& painter,
			  WFlags<AxisProperty> properties) const;

  /*! \brief Renders the border of the chart area.
   *
   * \sa render()
   * \sa setBorderPen()
   */
  virtual void renderBorder(WPainter& painter) const;

  /*! \brief Renders the curve labels.
   *
   * \sa render()
   * \sa addCurveLabel()
   */
  virtual void renderCurveLabels(WPainter& painter) const;

  /*! \brief Renders all series data, including value labels.
   *
   * \sa render()
   */
  virtual void renderSeries(WPainter& painter) const;

  /*! \brief Renders the (default) legend and chart titles.
   *
   * \sa render()
   */
  virtual void renderLegend(WPainter& painter) const;

  /*! \brief Renders properties of one axis.
   *
   * \sa renderAxes()
   */
  virtual void renderAxis(WPainter& painter, const WAxis& axis,
			  WFlags<AxisProperty> properties) const;

  /*! \brief Renders grid lines along the ticks of the given axis.
   *
   * \sa render()
   */
  virtual void renderGrid(WPainter& painter, const WAxis& axis) const;

  /*! \brief Renders other, user-defined things
   *
   * The default implementation sets the painter's
   * \link WPainter::setClipPath() clip path\endlink to
   * the chart area, but does not enable clipping.
   *
   * This method can be overridden to draw extra content onto
   * the chart.
   *
   * %Chart coordinates can be mapped to device coordinates with
   * mapToDeviceWithoutTransform(). If these need to move and scale
   * along with the zoom range, those points can be transformed with
   * zoomRangeTransform().
   *
   * This method is called last by default. If you want to render
   * other things at some other moment, you can override
   * render(WPainter&, const WRectF&).
   */
  virtual void renderOther(WPainter& painter) const;

  /*! \brief Calculates the total number of bar groups.
   */
  int calcNumBarGroups();

  //!@}

  virtual void render(WFlags<RenderFlag> flags) override ;
  virtual void setFormData(const FormData& formData) override ;

  /*! \brief Returns the current zoom range transform.
   *
   * This transform maps device coordinates from the fully zoomed out position
   * to the current zoom range.
   *
   * This transform is a \link WTransform::isJavaScriptBound()
   * JavaScript bound\endlink transform if this chart is
   * interactive. Otherwise, this transform is just the identity
   * transform.
   *
   * \sa setZoomEnabled()
   * \sa setPanEnabled
   * \sa WAxis::setZoomRange()
   */
  WTransform zoomRangeTransform(int yAxis = 0) const;

  /*! \brief Returns the current zoom range transform.
   *
   * This transform maps device coordinates from the fully zoomed out position
   * to the current zoom range.
   *
   * This transform is a \link WTransform::isJavaScriptBound()
   * JavaScript bound\endlink transform if this chart is
   * interactive. Otherwise, this transform is just the identity
   * transform.
   *
   * \sa setZoomEnabled()
   * \sa setPanEnabled
   * \sa WAxis::setZoomRange()
   */
  WTransform zoomRangeTransform(const WAxis &xAxis, const WAxis &yAxis) const;

private:
  int calcNumBarGroups() const;
  std::vector<const WAxis*> collectAxesAtLocation(Axis axis, AxisValue side) const;
  int seriesIndexOf(int modelColumn) const;
  int seriesIndexOf(const WDataSeries &series) const;
  bool hasInwardsXAxisOnMinimumSide() const;
  bool hasInwardsYAxisOnMaximumSide() const;
  void clearPens();
  void clearPensForAxis(Axis axis, int axisId);
  void createPensForAxis(Axis axis, int axisId);
  std::string cObjJsRef() const; // WCartesianChart JS object
  void assignJSHandlesForAllSeries();
  void freeJSHandlesForAllSeries();
  void assignJSPathsForSeries(const WDataSeries &series);
  void freeJSPathsForSeries(const WDataSeries &series);
  void freeAllJSPaths();
  void assignJSTransformsForSeries(const WDataSeries &series);
  void freeJSTransformsForSeries(const WDataSeries &series);
  void freeAllJSTransforms();
  void updateJSPens(WStringStream& js) const;
  void updateJSPensForAxis(WStringStream& js, Axis axis, int axisId) const;
  void updateJSConfig(const std::string &key, cpp17::any value);
  WPainterPath pathForSeries(const WDataSeries &series) const; // For use in WAxisSliderWidget
  WTransform zoomRangeTransform(const WTransform &xTransform, const WTransform &yTransform) const;
  WTransform calculateCurveTransform(const WDataSeries &series) const;
  WTransform curveTransform(const WDataSeries &series) const;
  void setZoomAndPan();
  void addAreaMask();
  void xTransformChanged(int xAxis);
  void yTransformChanged(int yAxis);
  void jsSeriesSelected(double x, double y);
  void loadTooltip(double x, double y);
  WPointF hv(double x, double y, double width) const;
  WPointF inverseHv(double x, double y, double width) const;
  WPointF inverseHv(const WPointF &p) const;
  WRectF insideChartArea() const;
  int calcAxisSize(const WAxis &axis, WPaintDevice *device) const;
  void defineJavaScript();
  void drawMarker(const WDataSeries &series,
                  MarkerType marker,
                  WPainterPath &result) const;

  bool axisSliderWidgetForSeries(WDataSeries *series) const;

  class IconWidget : public WPaintedWidget {
  public:
    IconWidget(WCartesianChart *chart, int index);
    
  protected:
    virtual void paintEvent(WPaintDevice *paintDevice) override;
    
  private:
    WCartesianChart* chart_;
    int index_;
  };

  friend class WDataSeries;
  friend class SeriesRenderer;
  friend class LineSeriesRenderer;
  friend class BarSeriesRenderer;
  friend class LabelRenderIterator;
  friend class MarkerRenderIterator;
  friend class MarkerMatchIterator;
  friend class WChart2DImplementation;
  friend class WAxisSliderWidget;
};

  }
}

#endif // CHART_WCARTESIAN_CHART_H_
