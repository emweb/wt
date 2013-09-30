/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPen"

namespace Wt {

WPen::WPen()
  : penStyle_(SolidLine),
    penCapStyle_(SquareCap),
    penJoinStyle_(BevelJoin),
    width_(0),
    color_(black)
{ }

WPen::WPen(PenStyle style)
  : penStyle_(style),
    penCapStyle_(SquareCap),
    penJoinStyle_(BevelJoin),
    width_(0),
    color_(black)
{ }

WPen::WPen(const WColor& color)
  : penStyle_(SolidLine),
    penCapStyle_(SquareCap),
    penJoinStyle_(BevelJoin),
    width_(0),
    color_(color)
{ }

WPen::WPen(GlobalColor color)
  : penStyle_(SolidLine),
    penCapStyle_(SquareCap),
    penJoinStyle_(BevelJoin),
    width_(0),
    color_(color)
{ }

WPen::WPen(const WGradient& gradient)
  : penStyle_(SolidLine),
    penCapStyle_(SquareCap),
    penJoinStyle_(BevelJoin),
    width_(0),
    color_(black),
    gradient_(gradient)
{ }

#ifdef WT_TARGET_JAVA
WPen WPen::clone() const
{
  WPen result;
  result.penStyle_ = penStyle_;
  result.penCapStyle_ = penCapStyle_;
  result.penJoinStyle_ = penJoinStyle_;
  result.width_ = width_;
  result.color_ = color_;
  return result;
}
#endif // WT_TARGET_JAVA

void WPen::setStyle(PenStyle style)
{
  penStyle_ = style;
}

void WPen::setCapStyle(PenCapStyle style)
{
  penCapStyle_ = style;
}

void WPen::setJoinStyle(PenJoinStyle style)
{
  penJoinStyle_ = style;
}

void WPen::setWidth(const WLength& width)
{
  width_ = width;
}

void WPen::setColor(const WColor& color)
{
  color_ = color;
  gradient_ = WGradient();
}

void WPen::setGradient(const WGradient& gradient)
{
  gradient_ = gradient;
}

bool WPen::operator==(const WPen& other) const
{
  return
       penStyle_ == other.penStyle_
    && penCapStyle_ == other.penCapStyle_
    && penJoinStyle_ == other.penJoinStyle_
    && width_ == other.width_
    && color_ == other.color_
    && gradient_ == other.gradient_;
}

bool WPen::operator!=(const WPen& other) const
{
  return !(*this == other);
}

}
