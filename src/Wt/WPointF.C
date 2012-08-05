/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPointF"

namespace Wt {

WPointF::WPointF()
  : x_(0), y_(0)
{ }

bool WPointF::operator== (const WPointF& other) const
{
  return (x_ == other.x_) && (y_ == other.y_);
}

bool WPointF::operator!= (const WPointF& other) const
{
  return !(*this == other);
}

WPointF& WPointF::operator+= (const WPointF& other)
{
  x_ += other.x_;
  y_ += other.y_;

  return *this;
}

}
