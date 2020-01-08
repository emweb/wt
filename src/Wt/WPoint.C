/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPoint.h"

namespace Wt {

WPoint::WPoint()
  : x_(0), y_(0)
{ }

bool WPoint::operator== (const WPoint& other) const
{
  return (x_ == other.x_) && (y_ == other.y_);
}

bool WPoint::operator!= (const WPoint& other) const
{
  return !(*this == other);
}

WPoint& WPoint::operator+= (const WPoint& other)
{
  x_ += other.x_;
  y_ += other.y_;

  return *this;
}

}
