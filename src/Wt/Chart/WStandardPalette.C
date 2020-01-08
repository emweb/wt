// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/Chart/WStandardPalette.h>

#include <Wt/WBrush.h>
#include <Wt/WColor.h>
#include <Wt/WPen.h>

namespace {

  /*
   * From http://www.modernlifeisrubbish.co.uk/article/web-2.0-colour-palette
   */
  unsigned long standardColors[][8] = {
    {
      /* Neutrals */
      0xC3D9FF, // Gmail blue
      0xEEEEEE, // Shiny silver
      0xFFFF88, // Interactive action yellow
      0xCDEB8B, // Qoop mint
      0x356AA0, // Digg blue
      0x36393D, // Shadows grey
      0xF9F7ED, // Magnolia Mag.nolia
      0xFF7400  // RSS orange
    },

    {
      /* Bold */
      0xFF1A00, // Mozilla red
      0x4096EE, // Flock blue
      0xFF7400, // RSS orange
      0x008C00, // Techcrunch green
      0xFF0084, // Flickr pink
      0x006E2E, // Newsvine green
      0xF9F7ED, // Magnolia Mag.nolia
      0xCC0000, // Rollyo red
    },

    {
      /* Muted */
      0xB02B2C, // Ruby on Rails red
      0x3F4C6B, // Mozilla blue
      0xD15600, // Etsy vermillion
      0x356AA0, // Digg blue
      0xC79810, // 43 Things gold
      0x73880A, // Writely olive
      0xD01F3C, // Last.fm crimson
      0x6BBA70  // Basecamp green
    }
  };
}

namespace Wt {
  namespace Chart {

WStandardPalette::WStandardPalette(PaletteFlavour flavour)
  : flavour_(flavour)
{ }

WBrush WStandardPalette::brush(int index) const
{
  return WBrush(color(index));
}

WPen WStandardPalette::borderPen(int index) const
{
  WPen p(WColor(0x44, 0x44, 0x44));
  p.setCapStyle(PenCapStyle::Square);
  return p;
}

WPen WStandardPalette::strokePen(int index) const
{
  WPen p(color(index));
  p.setWidth(2);
  p.setJoinStyle(PenJoinStyle::Round);
  p.setCapStyle(PenCapStyle::Round);
  return p;
}

WColor WStandardPalette::fontColor(int index) const
{
  WColor c = color(index);
  if (c.red() + c.green() + c.blue() > 3*128) {
    return WColor(StandardColor::Black);
  } else
    return WColor(StandardColor::White);
}

WColor WStandardPalette::color(int index) const
{
  if (flavour_ != PaletteFlavour::GrayScale) {
    unsigned long rgb = standardColors[static_cast<unsigned int>(flavour_)]
      [index % 8];

    return WColor((rgb & 0xFF0000) >> 16,
		  (rgb & 0x00FF00) >> 8,
		  (rgb & 0x0000FF));
  } else {
    int v = 255 - (index % 8) * 32;

    return WColor(v, v, v);
  }
}

  }
}
