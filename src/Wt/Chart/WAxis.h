// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CHART_WAXIS_H_
#define CHART_WAXIS_H_

#include <Wt/Chart/WChartGlobal.h>
#include <Wt/WAny.h>
#include <Wt/WFont.h>
#include <Wt/WPainterPath.h>
#include <Wt/WPen.h>
#include <Wt/WSignal.h>
#include <Wt/WString.h>
#include <Wt/WTransform.h>

#include <cfloat>

namespace Wt {

  namespace Chart {

class WAbstractChartImplementation;

/*! \brief Enumeration that indicates a chart axis.
 *
 * \sa WCartesianChart::axis(Axis)
 *
 * \ingroup charts
 */
enum class Axis {
  X = 0,             //!< X axis.
  Y = 1,             //!< First Y axis (== Y1).
  Y1 = Y,            //!< First Y axis (== Y).
  Y2 = 2,            //!< Second Y Axis.
  Ordinate = Y,      //!< Ordinate axis (== Y1 for a 2D plot).
  X3D = 0,          //!< X axis on 3D chart.
  Y3D = 3,          //!< Y axis on 3D chart.
  Z3D = 1 /* = Y */ //!< Z axis on 3D chart.
};

/*! \brief Enumeration that indicates a logical location for an axis.
 *
 * The location is dependent on the values of the other axis.
 *
 * \sa WAxis::setLocation(AxisValue)
 *
 * \ingroup charts
 */
enum class AxisValue {
  Minimum = 0x1,    //!< The minimum value.
  Maximum = 0x2,    //!< The maximum value.
  Zero = 0x4,       //!< The zero value (if displayed).
  Both = 0x8   //!< At both sides (Minimum and Maximum).
};

W_DECLARE_OPERATORS_FOR_FLAGS(AxisValue)

/*! \brief Axis configuration.
 * 
 * This describes the configuration of an axis determining how
 * it should be drawn. To be passed into getLabelTicks().
 */
struct AxisConfig {
  AxisConfig()
    : side(AxisValue::Minimum), zoomLevel(1)
  { }

  /*! \brief The side the axis is drawn on, should be Minimum, Maximum, or Zero
   */
  AxisValue side;

  /*! \brief The zoom level.
   *
   * Different axis labels can be drawn depending on the zoom
   * level. These are requested by WCartesianChart in powers of 2 (1,
   * 2, 4, 8,...)
   */
  int zoomLevel; 
};

/*! \brief Enumeration that indicates which way the axis ticks point.
 *
 * \sa WAxis::setTickDirection(TickDirection)
 */
enum class TickDirection {
  Outwards, //!< Towards of the outside of the chart.
  Inwards   //!< Pointing inwards to the chart.
};

/*! \brief Enumeration that indicates a scale for an axis.
 *
 * The scale determines how values are mapped onto an axis.
 *
 * \sa WAxis::setScale(AxisScale scale)
 *
 * \ingroup charts
 */
enum class AxisScale {
  /*! A discrete scale is set as the scale for the X axis in a \link
   * Chart::ChartType::Category ChartType::Category\endlink, and is only
   * applicable there. It lists all values, evenly spaced, and
   * consecutively in the order of the model. The categories are
   * converted to numbers using their ordinal (first category = 0,
   * second = 1, ...).
   */
  Discrete = 0,

  /*! A linear scale is the default scale for all axes, except for the
   * X scale in a AxisScale::Discrete. It maps values in a linear
   * fashion on the axis.
   */
  Linear = 1,

  /*! A logarithmic scale is useful for plotting values with of a
   * large range, but only works for positive values.
   */
  Log = 2,

  /*! A date scale is a special linear scale, which is useful for the
   * X axis in a ChartType::Scatter, when the X series contain dates (of type
   * WDate). The dates are converted to numbers, as Julian Days.
   */
  Date = 3,

  /*! A datetime scale is a special linear scale, which is useful for
   * the X axis in a ChartType::Scatter, when the X series contain timedates
   * (of type WDateTime). The dates are converted to numbers, as the
   * number of seconds since the Unix Epoch (midnight Coordinated
   * Universal Time (UTC) of January 1, 1970).
   */
  DateTime = 4
};

/*! \brief Represents a Date time unit
 */
enum class DateTimeUnit { Seconds, Minutes, Hours, Days, Months, Years };

/*! \brief Enumeration for a tick type */
enum class TickLength { Zero, Short, Long };

/*! \class WAxis Wt/Chart/WAxis.h Wt/Chart/WAxis.h
 *  \brief Class which represents an axis of a cartesian chart.
 *
 * A cartesian chart has two or three axes: an X axis (\link
 * Chart::Axis::X Axis::X\endlink), a Y axis (\link Chart::Axis::Y
 * Axis::Y\endlink) and optionally a second Y axis (\link Chart::Axis::Y2
 * Axis::Y2\endlink). Each of the up to three axes in a cartesian
 * chart has a unique id() that identifies which of these three axes
 * it is in the enclosing chart().
 *
 * Use setVisible() to change the visibility of an axis,
 * setGridLinesEnabled() to show grid lines for an axis. The pen
 * styles for rendering the axis or grid lines may be changed using
 * setPen() and setGridLinesPen(). A margin between the axis and the
 * main plot area may be configured using setMargin().
 *
 * By default, the axis will automatically adjust its range so that
 * all data will be visible. You may manually specify a range using
 * setMinimum(), setMaximum or setRange(). The interval between labels
 * is by default automatically adjusted depending on the axis length
 * and the range, but may be manually specified using
 * setLabelInterval().
 *
 * The axis has support for being "broken", to support displaying data
 * with a few outliers which would otherwise swamp the chart. This is
 * not done automatically, but instead you need to use setBreak() to
 * specify the value range that needs to be omitted from the axis. The
 * omission is rendered in the axis and in bars that cross the break.
 *
 * The labels are shown using a "%.4g" format string for numbers, and
 * a suitable format for \link Chart::AxisScale::Date AxisScale::Date\endlink or
 * \link Chart::AxisScale::DateTime AxisScale::DateTime\endlink scales, based on
 * heuristics. The format may be customized using
 * setLabelFormat(). The angle of the label text may be changed using
 * setLabelAngle(). By default, all labels are printed horizontally.
 * 
 * \sa WCartesianChart
 *
 * \ingroup charts
 */
class WT_API WAxis
{
public:
  /*! \brief Constant which indicates automatic minimum calculation.
   *
   * \sa setMinimum()
   */
  static const double AUTO_MINIMUM;

  /*! \brief Constant which indicates automatic maximum calculation
   *
   * \sa setMaximum()
   */
  static const double AUTO_MAXIMUM;

  /*! \brief Constructor
   */
  WAxis();

  /*! \brief Destructor
   */
  virtual ~WAxis();

  /*! \brief Returns the axis id.
   *
   * \sa chart(), WCartesianChart::axis()
   */
  Axis id() const { return axis_; }

  /*! \brief Sets whether this axis is visible.
   *
   * Changes whether the axis is displayed, including ticks and
   * labels. The rendering of the grid lines is controlled separately
   * by setGridLinesEnabled().
   *
   * The default value is true for the X axis and first Y axis, but false
   * for the second Y axis.
   *
   * \sa setGridLinesEnabled().
   */
  void setVisible(bool visible);

  /*! \brief Returns whether this axis is visible.
   *
   * \sa setVisible()
   */
  bool isVisible() const { return visible_; }

  /*! \brief Sets the axis location.
   *
   * Configures the location of the axis, relative to values on the
   * other axis (i.e. Y values for the X axis, and X values for the
   * Y axis).
   *
   * The default value is Chart::AxisValue::Minimum.
   *
   * \sa location()
   */
  void setLocation(AxisValue value);

  /*! \brief Returns the axis location.
   *
   * \sa setLocation()
   */
  AxisValue location() const { return location_; }

  /*! \brief Sets the scale of the axis.
   *
   * For the X scale in a \link Chart::ChartType::Category
   * ChartType::Category\endlink, the scale should be left unchanged to
   * \link Chart::AxisScale::Discrete AxisScale::Discrete\endlink.
   *
   * For all other axes, the default value is \link Chart::AxisScale::Linear
   * AxisScale::Linear\endlink, but this may be changed to \link
   * Chart::AxisScale::Log AxisScale::Log\endlink or \link Chart::AxisScale::Date
   * AxisScale::Date\endlink. \link Chart::AxisScale::Date AxisScale::Date\endlink is
   * only useful for the X axis in a ChartType::Scatter which contains WDate
   * values.
   *
   * \sa scale()
   */
  void setScale(AxisScale scale);

  /*! \brief Returns the scale of the axis.
   *
   * \sa setScale()
   */
  AxisScale scale() const { return scale_; }

  /*! \brief Sets the minimum value displayed on the axis.
   *
   * By default, the minimum and maximum values are determined
   * automatically so that all the data can be displayed.
   *
   * The numerical value corresponding to a data point is 
   * defined by it's AxisScale type.
   *
   * \sa setMaximum(), setAutoLimits()
   */
  void setMinimum(double minimum);

  /*! \brief Returns the minimum value displayed on the axis.
   *
   * This returned the minimum value that was set using setMinimum(),
   * or otherwise the automatically calculated (and rounded) minimum.
   *
   * The numerical value corresponding to a data point is defined by
   * it's AxisScale type.
   *
   * \sa setMinimum(), setAutoLimits(), setRoundLimits()
   */
  double minimum() const;

  /*! \brief Sets the maximum value for the axis displayed on the axis.
   *
   * By default, the minimum and maximum values are determined
   * automatically so that all the data can be displayed.
   *
   * The numerical value corresponding to a data point is 
   * defined by it's AxisScale type.
   *
   * \sa setMinimum(), setAutoLimits()
   */
  void setMaximum(double maximum);

  /*! \brief Returns the maximum value displayed on the axis.
   *
   * This returned the maximum value that was set using setMaximum(),
   * or otherwise the automatically calculated (and rounded) maximum.
   *
   * The numerical value corresponding to a data point is defined by
   * it's AxisScale type.
   *
   * \sa setMaximum(), setAutoLimits(), setRoundLimits()
   */
  double maximum() const;

  /*! \brief Sets the axis range (minimum and maximum values) manually.
   *
   * Specifies both minimum and maximum value for the axis. This
   * automatically disables automatic range calculation.
   *
   * The numerical value corresponding to a data point is defined by
   * it's AxisScale type.
   *
   * \sa setMinimum(), setMaximum()
   */
  void setRange(double minimum, double maximum);

  /*! \brief Sets the axis resolution.
   *
   * Specifies the axis resolution, in case maximum-minimum <
   * resolution minimum and maximum are modified so the maximum -
   * minimum = resolution
   *
   * The default resolution is 0, which uses a built-in epsilon.
   *
   * \sa resolution()
   */
  void setResolution(double resolution);

  /*! \brief Returns the axis resolution.
   *
   * \sa setResolution()
   */
  double resolution() const { return resolution_; }

  /*! \brief Let the minimum and/or maximum be calculated from the
   *         data.
   *
   * Using this method, you can indicate that you want to have
   * automatic limits, rather than limits set manually using
   * setMinimum() or setMaximum().
   *
   * \p locations can be Chart::AxisValue::Minimum and/or Chart::AxisValue::Maximum.
   *
   * The default value is Chart::AxisValue::Minimum | Chart::AxisValue::Maximum.
   */
  void setAutoLimits(WFlags<AxisValue> locations);

  /*! \brief Returns the limits that are calculated automatically.
   *
   * This returns the limits (Chart::AxisValue::Minimum and/or
   * Chart::AxisValue::Maximum) that are calculated automatically from the
   * data, rather than being specified manually using setMinimum()
   * and/or setMaximum().
   *
   * \sa setAutoLimits()
   */
  WFlags<AxisValue> autoLimits() const;

  /*! \brief Specifies whether limits should be rounded.
   *
   * When enabling rounding, this has the effect of rounding down the
   * minimum value, or rounding up the maximum value, to the nearest
   * label interval.
   *
   * By default, rounding is enabled for an auto-calculated limited,
   * and disabled for a manually specifed limit.
   *
   * \sa setAutoLimits()
   */
  void setRoundLimits(WFlags<AxisValue> locations);

  /*! \brief Returns whether limits should be rounded.
   *
   * \sa setRoundLimits()
   */
  WFlags<AxisValue> roundLimits() const { return roundLimits_; }

  /*! \brief Specifies a range that needs to be omitted from the axis.
   *
   * This is useful to display data with a few outliers which would
   * otherwise swamp the chart. This is not done automatically, but
   * instead you need to use setBreak() to specify the value range
   * that needs to be omitted from the axis. The omission is rendered
   * in the axis and in SeriesType::Bar that cross the break.
   *
   * \note This feature is incompatible with the interactive features
   *       of WCartesianChart.
   */
  void setBreak(double minimum, double maximum);

  /*! \brief Sets the label interval.
   *
   * Specifies the interval for displaying labels (and ticks) on the axis.
   * The default value is 0.0, and indicates that the interval should be
   * computed automatically.
   *
   * The unit for the label interval is in logical units (i.e. the same as
   * minimum or maximum).
   *
   * \sa setLabelFormat()
   */
  void setLabelInterval(double labelInterval);

  /*! \brief Returns the label interval.
   *
   * \sa setLabelInterval()
   */
  double labelInterval() const { return labelInterval_; }

  /*! \brief Sets a point to be included as one of the labels (if possible).
   *
   * The given point will be included as one of the labels, by adjusting
   * the minimum value on the axis, if that minimum is auto-computed. This
   * is only applicable to a Linear scale axis.
   *
   * The default value is 0.0.
   *
   * \sa setRoundLimits()
   */
  void setLabelBasePoint(double point);

  /*! \brief Returns the base point for labels.
   *
   * \sa setLabelBasePoint()
   */
  double labelBasePoint() const { return labelBasePoint_; }

  /*! \brief Sets the label format.
   *
   * Sets a format string which is used to format values, both for the
   * axis labels as well as data series values
   * (see WDataSeries::setLabelsEnabled()).
   *
   * For an axis with a \link Chart::AxisScale::Linear AxisScale::Linear\endlink
   * or \link Chart::AxisScale::Log AxisScale::Log\endlink scale, the format string
   * must be a format string that is accepted by snprintf() and which
   * formats one double. If the format string is an empty string,
   * then WLocale::toString() is used.
   *
   * For an axis with a \link Chart::AxisScale::Date AxisScale::Date\endlink
   * scale, the format string must be a format string accepted by
   * WDate::toString(), to format a date. If the format string is an
   * empty string, a suitable format is chosen based on heuristics.
   *
   * For an axis with a \link Chart::AxisScale::DateTime
   * AxisScale::DateTime\endlink scale, the format string must be a format
   * string accepted by WDateTime::toString(), to format a date. If
   * the format string is an empty string, a suitable format is chosen
   * based on heuristics.
   *
   * The default value is "%.4g" for a numeric axis, and a suitable
   * format for date(time) scales based on a heuristic taking into
   * account the current axis range.
   *
   * \sa labelFormat()
   */
  void setLabelFormat(const WString& format);

  /*! \brief Returns the label format string.
   *
   * \sa setLabelFormat()
   */
  WString labelFormat() const;

  /*! \brief Sets the label angle.
   *
   * Sets the angle used for displaying the labels (in degrees). A 0 angle
   * corresponds to horizontal text.
   *
   * The default value is 0.0.
   *
   * \sa labelAngle()
   */
  void setLabelAngle(double angle);

  /*! \brief Returns the label angle.
   *
   * \sa setLabelAngle()
   */
  double labelAngle() const { return labelAngle_; }

  /*! \brief Sets the title orientation
   *
   * Sets the orientation used for displaying the title.
   *
   * The default value is Orientation::Horizontal
   *
   * \sa titleOrientation()
   */
  void setTitleOrientation(const Orientation& orientation);

  /*! \brief Returns the title orientation.
   *
   * \sa setTitleOrientation()
   */
  const Orientation& titleOrientation() const { return titleOrientation_; }

  /*! \brief Sets whether gridlines are displayed for this axis.
   *
   * When <i>enabled</i>, gird lines are drawn for each tick on this axis,
   * using the gridLinesPen().
   *
   * Unlike all other visual aspects of an axis, rendering of the
   * gridlines is not controlled by setDisplayEnabled().
   *
   * \sa setGridLinesPen(), isGridLinesEnabled()
   */
  void setGridLinesEnabled(bool enabled);

  /*! \brief Returns whether gridlines are displayed for this axis.
   *
   * \sa setGridLinesEnabled()
   */
  bool isGridLinesEnabled() const { return gridLines_; }

  /*! \brief Changes the pen used for rendering the axis and ticks.
   *
   * The default value is a StandardColor::Black pen of 0 width.
   *
   * \sa setGridLinesPen().
   */
  void setPen(const WPen& pen);

  /*! \brief Returns the pen used for rendering the axis and ticks.
   *
   * \sa setPen()
   */
  const WPen& pen() const { return pen_; }

  /*! \brief Changes the pen used for rendering labels for this axis
   *
   * The default value is a StandardColor::Black pen of 0 width.
   *
   * \sa setGridLinesPen().
   * \sa setPen().
   * \sa WCartesianChart::setTextPen()
   */
  void setTextPen(const WPen& pen);

  /*! \brief Returns the pen used for rendering labels for this axis
   *
   * \sa setTextPen()
   */
  const WPen& textPen() const { return textPen_; }

  /*! \brief Changes the pen used for rendering the grid lines.
   *
   * The default value is a StandardColor::Gray pen of 0 width.
   *
   * \sa setPen(), gridLinesPen().
   */
  void setGridLinesPen(const WPen& pen);

  /*! \brief Returns the pen used for rendering the grid lines.
   *
   * \sa setGridLinesPen()
   */
  const WPen& gridLinesPen() const { return gridLinesPen_; }

  /*! \brief Sets the margin between the axis and the plot area.
   *
   * The margin is defined in pixels.
   *
   * The default value is 0.
   *
   * \sa margin()
   */
  void setMargin(int pixels);

  /*! \brief Returns the margin between the axis and the plot area.
   *
   * \sa setMargin()
   */
  int margin() const { return margin_; }

  /*! \brief Sets the axis title.
   *
   * The default title is empty.
   *
   * \sa title()
   */
  void setTitle(const WString& title);

  /*! \brief Returns the axis title.
   *
   * \sa setTitle()
   */
  const WString& title() const { return title_; }

  /*! \brief Sets the axis title font.
   *
   * The default title font is a 12 point Sans Serif font.
   *
   * \sa titleFont()
   */
  void setTitleFont(const WFont& titleFont);

  /*! \brief Returns the axis title font.
   *
   * \sa setTitleFont()
   */
  const WFont& titleFont() const { return titleFont_; }

  /*! \brief Sets the offset from the axis for the title label.
   */
  void setTitleOffset(double offset) { titleOffset_ = offset; }

  /*! \brief Returns the title offset.
   */
  double titleOffset() const { return titleOffset_; }

  /*! \brief Sets the axis label font.
   *
   * The default label font is a 10 point Sans Serif font.
   *
   * \sa labelFont()
   */
  void setLabelFont(const WFont& labelFont);

  /*! \brief Returns the axis label font.
   *
   * \sa setLabelFont()
   */
  const WFont& labelFont() const { return labelFont_; }

  /*! \brief Returns the label for a value.
   *
   * This returns the label text that corresponds to a given value.
   *
   * The default implementation uses the labelFormat() to properly represent
   * the value.
   */
  virtual WString label(double u) const;

  /*! \brief Set the range to zoom to on this axis.
   *
   * The minimum is the lowest value to be displayed, and
   * the maximum is the highest value to be displayed.
   *
   * If the difference between minimum and maximum is less
   * than minimumZoomRange(), the zoom range will be made
   * more narrow around the center of minimum and maximum.
   *
   * If the given minimum is larger than the given maximum, the
   * two values are swapped.
   *
   * Only applies to a WCartesianChart in interactive mode.
   *
   * \note This is only implemented for the X and first Y axis. It has no effect on the second Y axis.
   */
  void setZoomRange(double minimum, double maximum);

  /*! \brief Method::Get the zoom range minimum for this axis.
   *
   * Only applies to a WCartesianChart in interactive mode.
   *
   * \sa setZoomRange()
   */
  double zoomMinimum() const;

  /*! \brief Method::Get the zoom range maximum for this axis.
   *
   * Only applies to a WCartesianChart in interactive mode.
   *
   * \sa setZoomRange()
   */
  double zoomMaximum() const;

  /*! \brief A signal triggered when the zoom range is changed on the client side.
   *
   * Only applies to a WCartesianChart in interactive mode.
   *
   * \note If you want to use this signal, you must connect a signal listener
   *       before the chart is rendered.
   */
  Signal<double, double>& zoomRangeChanged() { return zoomRangeChanged_; }

  /*! \brief Sets the zoom level for this axis.
   *
   * Only applies to a WCartesianChart in interactive mode.
   * The zoom level should be >= 1 and smaller than maxZoom()
   *
   * \note This is only implemented for the X and first Y axis. It has no effect on the second Y axis.
   *
   * \deprecated Use setZoomRange() instead.
   */
  void setZoom(double zoom);

  /*! \brief Method::Get the zoom level for this axis.
   *
   * Only applies to a WCartesianChart in interactive mode.
   *
   * \see setZoom()
   *
   * \deprecated Use zoomMinimum() and zoomMaximum() instead.
   */
  double zoom() const;
  
  /*! \brief Sets the maximum zoom level for this axis.
   *
   * Only applies to a WCartesianChart in interactive mode.
   * The zoom level should be >= 1 (1 = no zoom).
   *
   * \note This is only implemented for the X and first Y axis. It has no effect on the second Y axis.
   *
   * \deprecated Use setMinimumZoomRange(double) instead
   */
  void setMaxZoom(double maxZoom);

  /*! \brief Method::Get the maximum zoom level for this axis.
   *
   * Only applies to a WCartesianChart in interactive mode.
   *
   * \see setMaxZoom()
   *
   * \deprecated Use minimumZoomRange() instead
   */
  double maxZoom() const;

  /*! \brief Sets the minimum zoom range for this axis.
   *
   * Only applies to a WCartesianChart in interactive mode.
   *
   * This range is the smallest difference there can be between
   * zoomMinimum() and zoomMaximum().
   *
   * \note This is only implemented for the X and first Y axis. It has no effect on the second Y axis.
   */
  void setMinimumZoomRange(double size);

  /*! \brief Method::Get the minimum zoom range for this axis.
   *
   * Only applies to a WCartesianChart in interactive mode.
   *
   * \sa setMinimumZoomRange()
   */
  double minimumZoomRange() const;

  /*! \brief Sets the value to pan to for this axis.
   *
   * This sets the leftmost (horizontal axis)
   * or bottom (vertical axis) value to be displayed on the chart.
   *
   * Note that if this would cause the chart to go out of bounds,
   * the panning of the chart will be automatically adjusted.
   *
   * Only applies to a WCartesianChart in interactive mode.
   *
   * \note This is only implemented for the X and first Y axis. It has no effect on the second Y axis.
   *
   * \note If the pan position has been changed on the client side, this may not reflect the actual pan position.
   *
   * \deprecated Use setZoomRange() instead.
   */
  void setPan(double pan);

  /*! \brief Method::Get the value to pan to for this axis, when pan is enabled on the chart.
   *
   * Only applies to a WCartesianChart in interactive mode.
   *
   * \sa setPan()
   *
   * \deprecated Use zoomMinimum() instead.
   */
  double pan() const;

  /*! \brief Sets the padding between the chart area and this axis.
   *
   * \sa padding()
   */
  void setPadding(int padding);

  /*! \brief Returns the padding between the chart area and this axis.
   * 
   * \sa setPadding()
   */
  int padding() const { return padding_; }

  /*! \brief Sets the direction that the axis ticks should point to.
   *
   * If set to TickDirection::Outwards, the axis ticks will point outside of
   * the chart, and the labels will be on the outside.
   *
   * If set to TickDirection::Inwards, the axis ticks will point inside of
   * the chart, and the labels will be on the inside. Also,
   * the padding() will be set to 25.
   *
   * \sa tickDirection()
   * \sa setPadding()
   */
  void setTickDirection(TickDirection direction);

  /*! \brief Gets the direction that the axis ticks point to.
   *
   * \sa setTickDirection()
   */
  TickDirection tickDirection() const { return tickDirection_; }

  /*! \brief Enables soft clipping of axis labels.
   *
   * This is set to \c false by for a 3D chart and to \c true for a 2D
   * chart.
   *
   * This setting determines how labels should be clipped in case not
   * the entire axis is visible due to clipping. "Hard" clipping is
   * done by the paint device and may truncate labels. "Soft" clipping
   * will determine if the corresponding tick is visible, and draw the
   * label (unclipped), preventing labels from being truncated. For a
   * 2D chart, this feature is only relevant when \link
   * WCartesianChart::setZoomEnabled zoom is enabled \endlink on a
   * WCartesianChart.
   * <table>
   * <tr style="vertical-align:top"><td>
   * \image html WAxis-partialLabelClipping-disabled.png "Soft clipping enabled (slower)."
   * This is the default for WCartesianChart. The tick for 0 is visible, and the 0 is shown
   * completely. The tick for 01/01/86 is not visible, so its label is not shown.
   * </td><td>
   * \image html WAxis-partialLabelClipping-enabled.png "Soft clipping disabled (faster)."
   * The tick of the 0 is visible, but the 0 is shown partially. Also, the tick of 01/01/86 is not
   * visible, but the label is partially shown.
   * </td></tr>
   * </table>
   */
  void setSoftLabelClipping(bool enabled);

  /*! \brief Returns whether soft label clipping is enabled
   *
   * \sa setSoftLabelCLipping()
   */
  bool softLabelClipping() const { return !partialLabelClipping_; }

  int segmentCount() const { return (int)segments_.size(); }

  double segmentMargin() const { return segmentMargin_; }

  bool prepareRender(Orientation orientation, double length) const;

  void render(WPainter& painter,
	      WFlags<AxisProperty> properties,
	      const WPointF& axisStart,
	      const WPointF& axisEnd,
	      double tickStart, double tickEnd, double labelPos,
	      WFlags<AlignmentFlag> labelFlags,
	      const WTransform &transform = WTransform(),
	      AxisValue side = AxisValue::Minimum) const {
    std::vector<WPen> pens;
    std::vector<WPen> textPens;
    render(painter, properties, axisStart, axisEnd, 
	   tickStart, tickEnd, labelPos, labelFlags, 
	   transform, side, pens, textPens);
  }

  void render(WPainter& painter,
	      WFlags<AxisProperty> properties,
	      const WPointF& axisStart,
	      const WPointF& axisEnd,
	      double tickStart, double tickEnd, double labelPos,
	      WFlags<AlignmentFlag> labelFlags,
	      const WTransform &transform,
	      AxisValue side,
	      std::vector<WPen> pens,
	      std::vector<WPen> textPens) const;

  std::vector<double> gridLinePositions(AxisConfig config) const;

  /*! \brief Set whether this axis should be inverted.
   *
   * When inverted, the axis will be drawn in the opposite direction, e.g.
   * if normally, the low values are on the left and high values on the right,
   * when inverted, the low values will be on the right and high values on the left.
   */
  void setInverted(bool inverted = true);

  /*! \brief Method::Get whether this axis is inverted.
   *
   * \sa setInverted()
   */
  bool inverted() const { return inverted_; }

#ifndef WT_TARGET_JAVA
  /*! \brief A label transform function.
   *
   * The label transform is a function from double to double.
   *
   * \sa WAxis::setLabelTransform()
   */
  typedef std::function<double (double)> LabelTransform;
#else
  /*! \brief A label transform function.
   *
   * The label transform is a function from double to double.
   *
   * \sa WAxis::setLabelTransform()
   */
  class LabelTransform {
  public:
    /*! \brief Apply the label transform.
     */
    virtual double apply(double d) const = 0;
  };
private:
  class IdentityLabelTransform : public LabelTransform {
  public:
    virtual double apply(double d) const {
      return d;
    }
  };
public:
#endif

  /*! \brief Set the transform function to apply to a given side.
   *
   * The label transform must be a function from double to double,
   * and will be applied on the double value of the model coordinate
   * of every axis tick.
   *
   * The label transform will not move the position of the axis
   * ticks, only change the labels displayed at the ticks.
   *
   * This can be useful in combination with a location() set to
   * AxisValue::Both, to show different labels on each side.
   *
   * If AxisScale::Date or AxisScale::DateTime are used, the double value will be
   * in seconds since the Epoch (00:00:00 UTC, January 1, 1970).
   *
   * Only AxisValue::Minimum, AxisValue::Zero and AxisValue::Maximum are accepted for
   * side. If you set a label transform for another side, the label
   * transform will not be used.
   *
   * The label transform will not be used if the scale() is AxisScale::Discrete.
   */
  void setLabelTransform(const LabelTransform& transform, AxisValue side);

  /*! \brief Method::Get the label transform configured for the given side.
   *
   * If no transform is configured for the given side, the identity
   * function is returned.
   *
   * \sa setLabelTransform()
   */
  LabelTransform labelTransform(AxisValue side) const;

  void setRenderMirror(bool enable) { renderingMirror_ = enable; }
  double calcTitleSize(WPaintDevice *device, Orientation orientation) const;
  double calcMaxTickLabelSize(WPaintDevice *device, Orientation orientation) const;

protected:
  /*! \brief Represents a label/tick on the axis.
   */
  struct WT_API TickLabel {
    /*! \brief Position on the axis. */
    double u;

    /*! \brief Tick length */
    TickLength tickLength;

    /*! \brief Label text */
    WString    label;

    /*! \brief Creates a label tick. */
    TickLabel(double v, TickLength length, const WString& l = WString());
  };

  /*! \brief Returns the label (and ticks) information for this axis.
   */
  virtual void getLabelTicks(std::vector<TickLabel>& ticks, int segment, AxisConfig config) const;

  /*! \brief Returns the Date format 
   */
  virtual WString autoDateFormat(const WDateTime& dt, DateTimeUnit unit, bool atTick) const;

private:

  WAbstractChartImplementation *chart_;
  Axis             axis_;
  bool             visible_;
  AxisValue        location_;
  AxisScale        scale_;
  double           resolution_;
  double           labelInterval_;
  double           labelBasePoint_;
  WString          labelFormat_;
  bool             defaultLabelFormat_;
  bool             gridLines_;
  WPen             pen_, gridLinesPen_;
  int              margin_;
  double           labelAngle_;
  WString          title_;
  WFont            titleFont_, labelFont_;
  WFlags<AxisValue> roundLimits_;
  double           segmentMargin_;
  double           titleOffset_;
  WPen             textPen_;
  Orientation      titleOrientation_;
  double	   maxZoom_;
  double	   minimumZoomRange_;
  double           zoomMin_;
  double           zoomMax_;
  bool 		   zoomRangeDirty_;
  int              padding_;
  TickDirection    tickDirection_;
  bool             partialLabelClipping_;
  bool		   inverted_;
  std::map<AxisValue, LabelTransform > labelTransforms_;

  // for 3D charts, don't call update when the labelangle is tempor. changed
  bool             renderingMirror_;

  Signal<double, double> zoomRangeChanged_;

  struct Segment {
    double minimum, maximum;
    mutable double renderMinimum, renderMaximum, renderLength, renderStart;
    mutable DateTimeUnit dateTimeRenderUnit;
    mutable int dateTimeRenderInterval;

    Segment();
    Segment(const Segment &other); // Explicit copy constructor for JWt
  };

  std::vector<Segment> segments_;

  mutable double renderInterval_;
  mutable double fullRenderLength_; // Render length including padding

  void init(WAbstractChartImplementation* chart, Axis axis);
  void update();

  template <typename T>
  bool set(T& m, const T& v);

  void computeRange(const Segment& segment) const;

  double getValue(const cpp17::any& v) const;
  static double getDateValue(const cpp17::any& value);

  double calcAutoNumLabels(Orientation orientation, const Segment& s) const;
  WString defaultDateTimeFormat(const Segment& s) const;

  double mapFromDevice(double d) const;
  double mapToDevice(const cpp17::any &value) const;
  double mapToDevice(const cpp17::any& value, int segment) const;
  double mapToDevice(double value) const;
  double mapToDevice(double value, int segment) const;
  // Check whether the given point is within the limits of this axis
  bool isOnAxis(double d) const;

  // The smallest value that is shown on the chart, including padding
  // (smaller than minimum())
  double drawnMinimum() const;

  // The largest value that is shown on the chart, including padding
  // (larger than maximum())
  double drawnMaximum() const;

  long long getDateNumber(const Wt::WDateTime& dt) const;
  void setZoomRangeFromClient(double minimum, double maximum);

  bool hasLabelTransformOnSide(AxisValue side) const;

  void renderLabels(WPainter &painter,
		   const std::vector<WString> &labels,
		   const WPainterPath &path,
		   WFlags<AlignmentFlag> flags,
		   double angle, int margin,
		   const WTransform &transform,
		   const WPen& pen) const;

  friend class WCartesianChart;
  friend class WCartesian3DChart;
  friend class WChart2DRenderer;
  friend class WAxisSliderWidget;
  friend class LineSeriesRenderer;
};

template <typename T>
bool WAxis::set(T& m, const T& v)
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

#endif // CHART_WAXIS_H_
