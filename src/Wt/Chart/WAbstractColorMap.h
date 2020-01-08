// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CHART_WABSTRACTCOLORMAP_H
#define CHART_WABSTRACTCOLORMAP_H

#include <Wt/Chart/WChartGlobal.h>
#include <Wt/WColor.h>
#include <Wt/WFont.h>
#include <Wt/WObject.h>
#include <Wt/WRectF.h>
#include <limits>

namespace Wt {
  namespace Chart {

/*! \class WAbstractColorMap
 *  \brief Maps numerical values to colors
 *
 * The colormap has functionality to convert a numerical value to a WColor. For 
 * details on how, see the documentation of the implementations.
 *
 * A colormap has a certain numerical range. When a colormap is painted as a 
 * colored strip or as a legend, this range is what will be presented, even if 
 * the colormap has the ability to convert values outside this range.
 *
 * \ingroup charts
 */
class WT_API WAbstractColorMap : public WObject {
public:
  /*! \brief Constructor
   *
   * Constructor taking a minimum and maximum value, determining the active
   * range of the colormap. The default value for tickSpacing is 2 and the
   * default label format is 2 decimal points.
   */
  WAbstractColorMap(double min, double max)
    : min_(min), max_(max), tickSpacing_(2)
#ifndef WT_TARGET_JAVA
    , format_("%.2f")
#else
    , format_("0.00")
#endif
  { }

  virtual ~WAbstractColorMap() {}

  /*! \brief Converts a numerical value to a WColor
   */
  virtual WColor toColor(double value) const = 0;

  /*! \brief Paints the colormap as a colored strip.
   *
   * This paints the colormap from the minimum to the maximum value in the 
   * provided area (default = fill the entire paintdevice). This is no legend 
   * with ticks and labels, only the colors are painted.
   *
   * \sa paintLegend()
   */
  virtual void createStrip(WPainter *painter,
			   const WRectF& area = WRectF()) const = 0;

  /*! \brief Paints the colormap as a legend.
   *
   * The colormap is painted as a legend with ticks and value-labels. The 
   * parameter area can be used to specify a part of the paintdevice where the 
   * legend should be drawn. When drawing the legend, the tickspacing, 
   * labelformat and labelfont are taken into account.
   *
   * \sa setTickSpacing(), setFormatString(), setLabelFont()
   */
  virtual void paintLegend(WPainter *painter,
			   const WRectF& area = WRectF()) const = 0;

  /*! \brief Returns the minimum of the colormap range.
   */
  double minimum() const { return min_; }

  /*! \brief Returns the maximum of the colormap range.
   */
  double maximum() const { return max_; }

  /*! \brief Sets the tickspacing for the legend as a number of line-heights.
   *
   * The tickspacing must be specified as an integer number of line-heigths. 
   * For example, the default value of 2 will leave two line-heights between the
   * labels of the ticks.
   *
   * \sa paintLegend()
   */
  void setTickSpacing(int spacing) { tickSpacing_ = spacing; }

  /*! \brief Returns the tickspacing for the legend.
   *
   * \sa setTickSpacing()
   */
  int tickSpacing() const { return tickSpacing_; }

  /*! \brief Sets the format for the labels on the colormap-legend.
   *
   * The format string is interpreted by snprintf(). The default is a float
   * with two decimal places.
   *
   * \sa paintLegend()
   */
  void setFormatString(const WString& format) { format_ = format; }

  /*! \brief Returns the format string.
   *
   * \sa setFormatString()
   */
  const WString& formatString() const { return format_; }

  /*! \brief Sets the font to be used when drawing the labels in the legend.
   *
   * The default is a default constructed WFont.
   *
   * \sa paintLegend()
   */
  void setLabelFont(const WFont& font) { labelFont_ = font; }

  /*! \brief Returns the font to be used when drawing the labels in the legend.
   *
   * \sa setLabelFont
   */
  const WFont& labelFont() { return labelFont_; }

protected:
  double min_, max_;

  int tickSpacing_;
  WString format_;
  WFont labelFont_;
};

  }
}

#endif
