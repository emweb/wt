// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "WStandardColorMap.h"

#include "Wt/WAny.h"
#include "Wt/WPainter.h"
#include "Wt/WPaintDevice.h"
#include "Wt/WPen.h"
#include "Wt/WException.h"
#include "Wt/WRasterImage.h"

#include <algorithm>
#include <iostream>

namespace Wt {
  namespace Chart {

WStandardColorMap::WStandardColorMap(double min, double max, bool continuous)
  : WAbstractColorMap(min,max),
    continuous_(continuous)
{
  double interval;
  if (continuous_) {
    interval = (max_ - min_)/4;
  } else {
    interval = (max_ - min_)/5;
  }
  colors_.push_back(Pair(min_, WColor(255,255,178)));
  colors_.push_back(Pair(min_+1*interval, WColor(254,204,92)));
  colors_.push_back(Pair(min_+2*interval, WColor(253,141,60)));
  colors_.push_back(Pair(min_+3*interval, WColor(240,59,32)));
  colors_.push_back(Pair(min_+4*interval, WColor(189,0,38)));
}

WStandardColorMap::WStandardColorMap(double min, double max,
				     const std::vector<WStandardColorMap::Pair>& colors,
				     bool continuous)
  : WAbstractColorMap(min,max),
    continuous_(continuous)
{
  // check if vector is sorted
  double prev = -std::numeric_limits<double>::max();
  for (unsigned i = 0; i < colors.size(); i++) {
    double val = colors[i].value();
    if (val < prev) {
      throw WException("WStandardColorMap: the provided vector is not sorted");
    }
    prev = val;
  }

  colors_ = colors;
}

void WStandardColorMap::discretise(int numberOfBands)
{
  if (!continuous_ || colors_.size() <= 1)
    return;

  double val0 = colors_[0].value();
  double interval = ( colors_[colors_.size()-1].value() - colors_[0].value() )
    / numberOfBands;

  std::vector<WStandardColorMap::Pair> newColors;
  for (int i=0; i<numberOfBands; i++) {
    // color taken corresponds to value in the middle of the band
    WStandardColorMap::Pair newCol(val0+i*interval,
				   toColor(val0+i*interval + interval/2));
    newColors.push_back(newCol);
  }

  colors_ = newColors;
  continuous_ = false;
}

WColor WStandardColorMap::toColor(double value) const
{
  if (colors_.size() == 0)
    return WColor();
  
  // value outside of the colormap
  if (value < colors_[0].value()) {
    return colors_[0].color();
  } else if (value >= colors_[colors_.size()-1].value()) {
    return colors_[colors_.size()-1].color();
  }

  // value inside of the colormap
  unsigned i = 0;
  for (; i < colors_.size(); i++) {
    if (value < colors_[i].value())
      break;
  }

  if (continuous_) {
    Pair mapVal1 = colors_[i-1];
    Pair mapVal2 = colors_[i];
    
    double factor = (value - mapVal1.value())/(mapVal2.value() - mapVal1.value());
    return interpolate(mapVal1.color(), mapVal2.color(), factor);
  } else {
    return colors_[i-1].color();
  }
}

void WStandardColorMap::createStrip(WPainter *painter, const WRectF& area) const
{
  painter->save();
  painter->setRenderHint(RenderHint::Antialiasing, false);

  int width, height;
  if (area.isNull()) {
    width = (int)painter->device()->width().value();
    height = (int)painter->device()->height().value();
  } else {
    painter->translate((int)area.x(), (int)area.y());
    width = (int)area.width();
    height = (int)area.height();
  }

  double valueInterval = (max_ - min_)/height;
  double offset = valueInterval/2;
  for (int i=0; i<height; i++) {
    WColor color = toColor(min_ + offset + i*valueInterval);
    
    painter->setBrush(WBrush(color));
    WPen linePen(color); linePen.setWidth(1);
    painter->setPen(linePen);
    
    painter->drawLine(0, height-(0.5+i),
		      width, height-(0.5+i));
  }

  painter->restore();
}

void WStandardColorMap::paintLegend(WPainter *painter,
				    const WRectF& area) const
{
  painter->save();
  WPainterPath clipPath;
  painter->setRenderHint(RenderHint::Antialiasing, false);
  painter->setFont(labelFont_);
  int height;
  if (area.isNull()) {
    height = (int)painter->device()->height().value();
  } else {
    clipPath.addRect(area);
    painter->setClipPath(clipPath);
    painter->setClipping(true);

    painter->translate(area.x(), area.y());
    height = (int)area.height();
  }

  int textHeight = (int)painter->font().sizeLength().toPixels();
  // draw the colormap with a box around it
  int stripWidth = 50;

  createStrip(painter, WRectF(0, (int)(textHeight/2+0.5), (int)stripWidth, (int)(height-textHeight)));

  painter->setPen(WPen());
  painter->setBrush(WBrush());
  painter->drawRect(WRectF(0.5, (int)(textHeight/2) + 0.5, stripWidth, height-textHeight));

  // draw the ticks + labels
  painter->translate(stripWidth, textHeight/2);
  if (continuous_) {
    int lineHeights = (int)(height/textHeight);
    int lhPerTick = 1 + tickSpacing_;
    int nbTicks = lineHeights % lhPerTick == 0 ? lineHeights/lhPerTick : lineHeights/lhPerTick + 1;
    int interval = (height-textHeight)/(nbTicks-1);
    int rest = (height-textHeight) % (nbTicks-1);
    int adjustedInterval = interval;
    
    double value = max_;
    double valDiff = (max_-min_)/(nbTicks-1);
    for (int i=0; i < nbTicks; i++) {
      painter->drawLine(0, 0.5, 4, 0.5);
      painter->drawText(10, -textHeight/2, 40, textHeight,
			WFlags<AlignmentFlag>(AlignmentFlag::Left) | AlignmentFlag::Middle, 
			Wt::asString(value, format_));
      value -= valDiff;

      if (rest > 0) {
	adjustedInterval = interval + 1;
	rest--;
      } else {
	adjustedInterval = interval;
      }
      painter->translate(0, adjustedInterval);
    }
  } else {
    // first paint tick for maximum value
    painter->drawLine(0, 0.5, 4, 0.5);
    painter->drawText(10, -textHeight/2, 100, textHeight,
		      WFlags<AlignmentFlag>(AlignmentFlag::Left) | AlignmentFlag::Middle,
		      Wt::asString(max_, format_));
    // paint the rest of the ticks
    int nbTicks = colors_.size();
    int prevDiff = 0;
    for (int i=nbTicks-1; i >= 0; i--) {
      double relPos = -(colors_[i].value()-max_)/(max_-min_);
      double diff = relPos*(height-textHeight);
      int roundedDiff = (int)(diff + 0.5);
      painter->translate(0, roundedDiff-prevDiff);
      painter->drawLine(0, 0.5, 4, 0.5);
      painter->drawText(10, -textHeight/2, 40, textHeight,
			WFlags<AlignmentFlag>(AlignmentFlag::Left) | AlignmentFlag::Middle,
			Wt::asString(colors_[i].value(), format_));
      prevDiff = roundedDiff;
    }
  }

  painter->restore();
}

WColor WStandardColorMap::interpolate(const WColor& color1,
				      const WColor& color2,
				      double factor) const
{
  return WColor((int)((1-factor)*color1.red() + factor*color2.red()),
		(int)((1-factor)*color1.green() + factor*color2.green()),
		(int)((1-factor)*color1.blue() + factor*color2.blue()));
}

  }
}
