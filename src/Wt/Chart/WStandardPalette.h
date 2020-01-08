// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CHART_WSTANDARD_PALETTE_H_
#define CHART_WSTANDARD_PALETTE_H_

#include <Wt/WDllDefs.h>
#include <Wt/Chart/WChartPalette.h>

namespace Wt {

class WColor;

  namespace Chart {

/*! \brief Enumeration that indicates the palette flavour.
 */
enum class PaletteFlavour {
  Neutral = 0,     //!< Neutral palette
  Bold = 1,        //!< Bold palette
  Muted = 2,       //!< Muted palette
  GrayScale = 255  //!< Grayscale palette
};

/*! \class WStandardPalette Wt/Chart/WStandardPalette.h Wt/Chart/WStandardPalette.h
 *  \brief Standard styling for rendering series in charts.
 *
 * This class provides four standard palettes, each composed of eight
 * different colors (these are recycled at index 8).
 *
 * The three colored palettes are a variation on those defined at
 * http://www.modernlifeisrubbish.co.uk/article/web-2.0-colour-palette.
 *
 * The following table lists the background color, and font color of the
 * different palettes:
 *
 * <table style="margin-left:auto; margin-right:auto;">
 *   <tr>
 *     <td style="width: 10ex; text-align:center"><b>Neutral</b></td>
 *     <td style="width: 10ex; text-align:center"><b>Bold</b></td>
 *     <td style="width: 10ex; text-align:center"><b>Muted</b></td>
 *     <td style="width: 10ex; text-align:center"><b>GrayScale</b></td>
 *   </tr>
 *   <tr>
 *     <td style="background-color:#c3d9ff">Gmail blue</td>
 *     <td style="background-color:#ff1a00">Mozilla red</td>
 *     <td style="background-color:#b02b2c; color:white">Ruby on Rails red</td>
 *     <td style="background-color:rgb(255,255,255);">Gray #1</td>
 *   </tr>
 *   <tr>
 *     <td style="background-color:#eeeeee">Shiny silver</td>
 *     <td style="background-color:#4096ee">Flock blue</td>
 *     <td style="background-color:#3f4c6b; color:white">Mozilla blue</td>
 *     <td style="background-color:rgb(223,223,223);">Gray #2</td>
 *   </tr>
 *   <tr>
 *     <td style="background-color:#ffff88">Interactive action yellow</td>
 *     <td style="background-color:#ff7400; color:white">RSS orange</td>
 *     <td style="background-color:#d15600; color:white">Etsy vermillion</td>
 *     <td style="background-color:rgb(191,191,191);">Gray #3</td>
 *   </tr>
 *   <tr>
 *     <td style="background-color:#cdeb8b">Qoop mint</td>
 *     <td style="background-color:#008c00; color:white">Techcrunch green</td>
 *     <td style="background-color:#356aa0; color:white">Digg blue</td>
 *     <td style="background-color:rgb(159,159,159);">Gray #4</td>
 *   </tr>
 *   <tr>
 *     <td style="background-color:#356aa0; color:white">Digg blue</td>
 *     <td style="background-color:#ff0084">Flickr pink</td>
 *     <td style="background-color:#c79810; color:white">43 Things gold</td>
 *     <td style="background-color:rgb(127,127,127); color:white">Gray #5</td>
 *   </tr>
 *   <tr>
 *     <td style="background-color:#36393d; color:white">Shadows grey</td>
 *     <td style="background-color:#006e2e; color:white">Newsvine green</td>
 *     <td style="background-color:#73880a; color:white">Writely olive</td>
 *     <td style="background-color:rgb(95,95,95); color:white">Gray #6</td>
 *   </tr>
 *   <tr>
 *     <td style="background-color:#f9f7ed">Magnolia Mag.nolia</td>
 *     <td style="background-color:#f9f7ed">Magnolia Mag.nolia</td>
 *     <td style="background-color:#d01f3c; color:white">Last.fm crimson</td>
 *     <td style="background-color:rgb(63,63,63); color:white">Gray #7</td>
 *   </tr>
 *   <tr>
 *     <td style="background-color:#ff7400; color:white">RSS orange</td>
 *     <td style="background-color:#cc0000; color:white">Rollyo red</td>
 *     <td style="background-color:#6bba70">Basecamp green</td>
 *     <td style="background-color:rgb(31,31,31); color:white">Gray #8</td>
 *   </tr>
 *  </table>
 *
 * The border pen is in all cases a gray pen of 0 width, while the stroke
 * pen is a line of width 2 in the background color.
 *
 * \ingroup charts
 */
class WT_API WStandardPalette : public WChartPalette
{
public:
  /*! \brief Typedef for enum Wt::Chart::PaletteFlavour */
  typedef PaletteFlavour Flavour;

  /*! \brief Creates a standard palette of a particular flavour.
   */
  WStandardPalette(PaletteFlavour flavour);

  virtual WBrush brush(int index) const override;
  virtual WPen   borderPen(int index) const override;
  virtual WPen   strokePen(int index) const override;
  virtual WColor fontColor(int index) const override;

  /*! \brief Returns the color for the given index.
   */
  virtual WColor color(int index) const;

private:
  PaletteFlavour flavour_;
};

  }
}

#endif // CHART_WSTANDARD_PALETTE_H_
