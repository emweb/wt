// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CHART_WSTANDARDCOLORMAP_H
#define CHART_WSTANDARDCOLORMAP_H

#include "WAbstractColorMap.h"
#include <map>

namespace Wt {
  namespace Chart {

/*! \class WStandardColorMap
 *  \brief Standard colorMap
 * 
 * The %WStandardColorMap is defined by a list of value-to-color pairs. The 
 * %WStandardColorMap has two modes: a continuous mode, in which the colors 
 * are linearly interpolated in between the pair values, and a 
 * non-continuous mode, where the values are not interpolated, so that the 
 * colormap has a banded effect. In non-continuous mode, the color of a 
 * given point P is the color of the pair with the largest value smaller than 
 * P.
 *
 * Numerical values above the maximum value in the list map to the maximum 
 * value's color, all values below the minimum value in the list map to the 
 * minimum value's color. The range indicated by the minimum and maximum passed 
 * to the constructor determines which part of the colormap is drawn by 
 * createStrip() or paintLegend().
 *
 * The figure below illustrates the possible colormaps that can be constructed 
 * from the list {"0.0 - StandardColor::DarkRed", "1.0 - StandardColor::Red",
 * "2.0 - StandardColor::Gray"}. The discrete map 
 * (on the left) has the range [0, 3], the continuous map (on the right) has  
 * the range [0, 2]. The utility method discretise() is also applied to 
 * the continuous colormap to obtain a colormap with 5 bands in the same range.
 *
 * \image html standardcolormaps.png "Different uses of WStandardColorMap"
 */
class WT_API WStandardColorMap : public WAbstractColorMap {
public:
  /*! \class Pair
   * \brief Contains a pair of a numerical value and a WColor
   */
  class Pair {
  public:
    Pair() {}
    
    /*! \brief Constructs a %WStandardColorMap::Pair from a double and a WColor.
     */
    Pair(double val, WColor col)
      : value_(val), color_(col)
    {}

    void setValue(double value) { value_ = value; }
    double value() const { return value_; }

    void setColor(const WColor& color) { color_ = color; }
    const WColor& color() const { return color_; }

  private:
    double value_;
    WColor color_;
  };

  /*! \brief Construct a default colormap.
   *
   * The default colormap is a transition from yellow to red. The color-scheme 
   * was taken from <a href="http://www.personal.psu.edu/faculty/c/a/cab38/ColorBrewer/ColorBrewer.html">ColorBrewer</a>, which contains lots of useful info 
   * and example schemes you might want to use for your custom colormaps.
   */
  WStandardColorMap(double min, double max, bool continuous);

  /*! \brief Construct a custom colormap.
   *
   * This constructor allows you to pass a list of value-to-color pairs that 
   * define a colormap as described in the class description.
   */
  WStandardColorMap(double min, double max,
		    const std::vector<WStandardColorMap::Pair>& colors,
		    bool continuous);

  /*! \brief Utility method to discretise a continuous colormap in a number of 
   * equally sized bands.
   *
   * This method makes a new list of value-to-color pairs by discretising the 
   * linear interpolation of the previous one into numberOfBands equally 
   * sized colorbands. This method only has effect if the colormap is 
   * continuous.
   */
  void discretise(int numberOfBands);

  bool continuous() const { return continuous_; }

  const std::vector<WStandardColorMap::Pair>& colorValuePairs() const 
  { return colors_;}

  virtual WColor toColor(double value) const override;

  virtual void createStrip(WPainter *painter,
                           const WRectF& area = WRectF()) const override;

  virtual void paintLegend(WPainter *painter,
                           const WRectF& area = WRectF()) const override;

private:
  WColor interpolate(const WColor& color1,const WColor& color2,
		     double factor) const;

  bool continuous_;
  std::vector<WStandardColorMap::Pair> colors_;
};

  }
}

#endif
