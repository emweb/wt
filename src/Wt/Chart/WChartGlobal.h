// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCHARTGLOBAL_H_
#define WCHARTGLOBAL_H_

#include <Wt/WGlobal.h>

namespace Wt {
  namespace Chart {
/*! \brief Enumeration that specifies the type of a chart series.
 *
 * \sa WDataSeries::setType(SeriesType)
 * \sa WCartesianChart
 *
 * \ingroup charts
 */
enum class SeriesType {
  Point,  //!< Series rendered solely as point markers.
  Line,   //!< Series rendered as points connected by straight lines.
  Curve,  //!< Series rendered as points connected by curves.
  Bar     //!< Series rendered as bars.
};

/*! \brief Enumeration that specifies a type of point marker.
 *
 * \sa WDataSeries::setMarker(MarkerType marker)
 * \sa WCartesianChart
 *
 * \ingroup charts
 */
enum class MarkerType {
  None,       //!< Do not draw point markers.
  Square,   //!< Mark points using a square.
  Circle,   //!< Mark points using a circle.
  Cross,    //!< Mark points using a cross (+).
  XCross,   //!< Mark points using a cross (x).
  Triangle, //!< Mark points using a triangle.
  Custom,    //!< Mark points using a custom marker.
  Star,     //!< Mark points using a star.
  InvertedTriangle, //!< Mark points using an inverted (upside-down) triangle.
  Asterisk, //!< Mark points using an asterisk (*).
  Diamond   //!< Mark points using a diamond.
};

/*! \brief Enumeration that specifies how an area should be filled.
 *
 * Data series of type SeriesType::Line or CurveSerie may be filled under or
 * above the line or curve. This enumeration specifies the other limit
 * of this fill.
 * Data series of type SeriesType::Bar can use this setting to configure the bottom
 * of the chart.
 *
 * \sa WDataSeries::setFillRange(FillRangeType range)
 * \sa WCartesianChart
 *
 * \ingroup charts
 */
enum class FillRangeType {
  None,           //!< Do not fill under the curve.
  MinimumValue, //!< Fill from the curve to the chart bottom (min)
  MaximumValue, //!< Fill from the curve to the chart top
  ZeroValue     //!< Fill from the curve to the zero Y value.
};

/*! \brief Enumeration type that indicates a chart type for a cartesian
 *         chart.
 *
 * \ingroup charts
 */
enum class ChartType {
  Category, //!< The X series are categories
  Scatter    //!< The X series must be interpreted as numerical data
};

/*! \brief Enumeration that specifies a property of the axes.
 *
 * \ingroup charts
 */
enum class AxisProperty {
  Labels = 0x1,
  Line = 0x4     //<! Axis line and ticks.
};

W_DECLARE_OPERATORS_FOR_FLAGS(AxisProperty)

/*! \brief Enumeration type that indicates a legend location.
 *
 * \ingroup charts
 */
enum class LegendLocation {
  Inside, //!< Inside the chart area
  Outside //!< Outside the chart area (in the padding area)
};

  }
}

#endif // WCHARTGLOBAL_H_
