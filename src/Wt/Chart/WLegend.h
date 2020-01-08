// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CHART_WLEGEND_H
#define CHART_WLEGEND_H

#include <Wt/Chart/WChartGlobal.h>
#include <Wt/WBrush.h>
#include <Wt/WFont.h>
#include <Wt/WLength.h>
#include <Wt/WPen.h>

namespace Wt {
  namespace Chart {

class WLegend {
public:
  WLegend();

  void setLegendEnabled(bool enabled);
  bool isLegendEnabled() const { return legendEnabled_; }
  void setLegendLocation(LegendLocation location, Side side, AlignmentFlag alignment);
  void 	setLegendStyle(const WFont &font, const WPen &border, const WBrush &background);
  LegendLocation legendLocation() const { return legendLocation_; }
  Side legendSide() const { return legendSide_; }
  AlignmentFlag legendAlignment() const { return legendAlignment_; }
  int legendColumns() const { return legendColumns_; }
  WLength legendColumnWidth() const { return legendColumnWidth_; }
  WFont legendFont() const { return legendFont_; }
  WPen legendBorder() const { return legendBorder_; }
  WBrush legendBackground() const { return legendBackground_; }
  void setLegendColumns(int columns);
  void setLegendColumnWidth(const WLength &columnWidth);

protected:
  bool legendEnabled_;
  LegendLocation legendLocation_;
  Side legendSide_;
  AlignmentFlag legendAlignment_;
  int legendColumns_;
  WLength legendColumnWidth_;
  WFont legendFont_;
  WPen legendBorder_;
  WBrush legendBackground_;
};

  }
}

#endif
