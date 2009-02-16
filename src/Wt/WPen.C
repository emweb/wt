/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPen"

namespace Wt {

WPen::WPen()
  : style_(SolidLine | SquareCap | BevelJoin),
    width_(0),
    color_(black)
{ }

WPen::WPen(PenStyle style)
  : style_(style | SquareCap | BevelJoin),
    width_(0),
    color_(black)
{ }

WPen::WPen(const WColor& color)
  : style_(SolidLine | SquareCap | BevelJoin),
    width_(0),
    color_(color)
{ }

WPen::WPen(GlobalColor color)
  : style_(SolidLine | SquareCap | BevelJoin),
    width_(0),
    color_(color)
{ }

void WPen::setStyle(PenStyle style)
{
  style_ &= ~StyleMask_;
  style_ |= style;
}

void WPen::setCapStyle(PenCapStyle style)
{
  style_ &= ~CapStyleMask_;
  style_ |= style;
}

void WPen::setJoinStyle(PenJoinStyle style)
{
  style_ &= ~JoinStyleMask_;
  style_ |= style;
}

void WPen::setWidth(const WLength& width)
{
  width_ = width;
}

void WPen::setColor(const WColor& color)
{
  color_ = color;
}

bool WPen::operator==(const WPen& other) const
{
  return
       style_ == other.style_
    && width_ == other.width_
    && color_ == other.color_;
}

bool WPen::operator!=(const WPen& other) const
{
  return !(*this == other);
}

}
