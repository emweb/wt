// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WLineF.h>
#include <Wt/WPointF.h>

namespace Wt {

WLineF::WLineF()
  : x1_(0), y1_(0), x2_(0), y2_(0)
{ }

WLineF::WLineF(const WPointF& p1, const WPointF& p2)
  : x1_(p1.x()), y1_(p1.y()), x2_(p2.x()), y2_(p2.y())
{ }

WLineF::WLineF(double x1, double y1, double x2, double y2)
  : x1_(x1), y1_(y1), x2_(x2), y2_(y2)
{ }

WPointF WLineF::p1() const
{
  return WPointF(x1_, y1_);
}

WPointF WLineF::p2() const
{
  return WPointF(x2_, y2_);
}

bool WLineF::operator==(const WLineF& other) const
{
  return (x1_ == other.x1_)
    && (y1_ == other.y1_)
    && (x2_ == other.x2_)
    && (y2_ == other.y2_);
}

bool WLineF::operator!=(const WLineF& other) const
{
  return !(*this == other);
}

}
