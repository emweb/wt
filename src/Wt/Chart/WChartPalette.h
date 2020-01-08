// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CHART_WCHART_PALETTE_H_
#define CHART_WCHART_PALETTE_H_

#include <Wt/WDllDefs.h>

namespace Wt {

class WBrush;
class WColor;
class WPen;

  namespace Chart {

/*! \class WChartPalette Wt/Chart/WChartPalette.h Wt/Chart/WChartPalette.h
 *  \brief Abstract base class for styling rendered data series in charts.
 *
 * This class provides an interface for a palette which sets strokes
 * and fill strokes for data in a \link WAbstractChart
 * chart\endlink. A palette is an ordered list of styles, which is
 * indexed by the chart to get a suitable style for a particular
 * series (in case of WCartesianChart) or data row (in case of
 * WPieChart). Each style is defined by a brush, two pen styles (one
 * for borders, and one for plain lines), and a font color that is
 * appropriate for drawing text within the brushed area.
 *
 * To use a custom palette, you should reimplement this class, and then
 * use WAbstractChart::setPalette() to use an instance of the palette.
 *
 * \ingroup charts
 */
class WT_API WChartPalette
{
public:
  /*! \brief Destructor.
   */
  virtual ~WChartPalette();

  /*! \brief Returns a brush from the palette.
   *
   * Returns the brush for the style with given <i>index</i>.
   */
  virtual WBrush brush(int index) const = 0;

  /*! \brief Returns a border pen from the palette.
   *
   * Returns the pen for stroking borders around an area filled using the
   * brush at the same <i>index</i>.
   *
   * \sa strokePen(), brush()
   */
  virtual WPen borderPen(int index) const = 0;

  /*! \brief Returns a stroke pen from the palette.
   *
   * Returns the pen for stroking lines for the style with given <i>index</i>.
   *
   * \sa strokePen()
   */
  virtual WPen strokePen(int index) const = 0;

  /*! \brief Returns a font color from the palette.
   *
   * Returns a font color suitable for rendering text in the area filled
   * with the brush at the same <i>index</i>.
   *
   * \sa brush()
   */
  virtual WColor fontColor(int index) const = 0;
};

  }
}

#endif // CHART_WCHART_PALETTE_H_
