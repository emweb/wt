// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Chart/WLegend.h"

namespace Wt {
  namespace Chart {

WLegend::WLegend()
  : legendEnabled_(false),
    legendLocation_(LegendLocation::Outside),
    legendSide_(Side::Right),
    legendAlignment_(AlignmentFlag::Middle),
    legendColumns_(1),
    legendColumnWidth_(100),
    legendBorder_(PenStyle::None),
    legendBackground_(BrushStyle::None)
{
  legendFont_.setFamily(FontFamily::SansSerif, "Arial");
  legendFont_.setSize(WLength(10, LengthUnit::Point));
}
    
void WLegend::setLegendEnabled(bool enabled) {
  if (legendEnabled_ != enabled) {
    legendEnabled_ = enabled;
  }
}
    
void WLegend::setLegendLocation(LegendLocation location, 
				Side side, 
				AlignmentFlag alignment)
{
  legendLocation_ = location;
  legendSide_ = side;
  legendAlignment_ = alignment;
}
    
void WLegend::setLegendStyle(const WFont &font, 
			     const WPen &border, 
			     const WBrush &background)
{
  legendFont_ = font;
  legendBorder_ = border;
  legendBackground_ = background;  
}

void WLegend::setLegendColumns(int columns)
{
  legendColumns_ = columns;
}

void WLegend::setLegendColumnWidth(const WLength &columnWidth)
{
  legendColumnWidth_ = columnWidth;
}

  }
}
