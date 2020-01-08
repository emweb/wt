/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WShadow.h"

namespace Wt {

WShadow::WShadow()
  : color_(StandardColor::Black),
    offsetX_(0),
    offsetY_(0),
    blur_(0)
{ }

WShadow::WShadow(double dx, double dy, const WColor& color, double blur)
  : color_(color),
    offsetX_(dx),
    offsetY_(dy),
    blur_(blur)
{ }

bool WShadow::none() const
{
  return offsetX_ == 0 && offsetY_ == 0 && blur_ == 0;
}

bool WShadow::operator==(const WShadow& other) const
{
  return color_ == other.color_
    && offsetX_ == other.offsetX_
    && offsetY_ == other.offsetY_
    && blur_ == other.blur_;
}

bool WShadow::operator!=(const WShadow& other) const
{
  return !(*this == other);
}

void WShadow::setOffsets(double dx, double dy)
{
  offsetX_ = dx;
  offsetY_ = dy;
}

void WShadow::setColor(const WColor& color)
{
  color_ = color;
}

void WShadow::setBlur(double blur)
{
  blur_ = blur;
}

}
