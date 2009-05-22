/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBrush"

namespace Wt {

WBrush::WBrush()
  : style_(NoBrush),
    color_(black)
{ }

WBrush::WBrush(WBrushStyle style)
  : style_(style),
    color_(black)
{ }

WBrush::WBrush(const WColor& color)
  : style_(SolidPattern),
    color_(color)
{ }

WBrush::WBrush(GlobalColor color)
  : style_(SolidPattern),
    color_(color)
{ }

#ifdef WT_TARGET_JAVA
WBrush WBrush::clone() const
{
  WBrush result;
  result.color_ = color_;
  result.style_ = style_;
  return result;
}
#endif // WT_TARGET_JAVA

void WBrush::setColor(const WColor& color)
{
  color_ = color;
}

void WBrush::setStyle(WBrushStyle style)
{
  style_ = style;
}

bool WBrush::operator==(const WBrush& other) const
{
  return
       color_ == other.color_
    && style_ == other.style_;
}

bool WBrush::operator!=(const WBrush& other) const
{
  return !(*this == other);
}

}
