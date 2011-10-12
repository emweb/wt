/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <algorithm>

#include "Wt/WRectF"
#include "Wt/WPointF"

namespace Wt {

WRectF::WRectF()
  : x_(0), y_(0), width_(0), height_(0)
{ }

WRectF::WRectF(double x, double y, double width, double height)
  : x_(x), y_(y), width_(width), height_(height)
{ }

WRectF::WRectF(const WPointF& topLeft, const WPointF& bottomRight)
  : x_(topLeft.x()),
    y_(topLeft.y()),
    width_(bottomRight.x() - topLeft.x()),
    height_(bottomRight.y() - topLeft.y())
{ }

#ifdef WT_TARGET_JAVA
WRectF& WRectF::operator=(const WRectF& rhs)
{
  x_ = rhs.x_;
  y_ = rhs.y_;
  width_ = rhs.width_;
  height_ = rhs.height_;

  return *this;
}
#endif // WT_TARGET_JAVA

bool WRectF::operator==(const WRectF& rhs) const
{
  return 
       x_ == rhs.x_
    && y_ == rhs.y_
    && width_ == rhs.width_
    && height_ == rhs.height_;
}

bool WRectF::operator!=(const WRectF& rhs) const
{
  return !(*this == rhs);
}

#ifndef WT_TARGET_JAVA
bool WRectF::isNull() const
{
  return x_ == 0 && y_ == 0 && width_ == 0 && height_ == 0;
}
#endif //WT_TARGET_JAVA

bool WRectF::isEmpty() const
{
  return width_ == 0 && height_ == 0;
}

void WRectF::setX(double x)
{
  width_ += (x_ - x);
  x_ = x;
}

void WRectF::setY(double y)
{
  height_ += (y_ - y);
  y_ = y;
}

WPointF WRectF::center() const
{
  return WPointF(x_ + width_/2, y_ + height_/2);
}

WPointF WRectF::topLeft() const
{
  return WPointF(x_, y_);
}

WPointF WRectF::topRight() const
{
  return WPointF(x_ + width_, y_);
}

WPointF WRectF::bottomLeft() const
{
  return WPointF(x_, y_ + height_);
}

WPointF WRectF::bottomRight() const
{
  return WPointF(x_ + width_, y_ + height_);
}

bool WRectF::contains(double x, double y) const
{
  return x >= x_ && x <= (x_ + width_) && y >= y_ && y <= (y_ + height_);
}

bool WRectF::contains(const WPointF& p) const
{
  return contains(p.x(), p.y());
}

bool WRectF::intersects(const WRectF& other) const
{
  if (isEmpty() || other.isEmpty())
    return false;
  else {
    WRectF r1 = normalized();
    WRectF r2 = other.normalized();

    bool intersectX = (r2.left() >= r1.left() && r2.left() <= r1.right())
      || (r2.right() >= r1.left() && r2.right() <= r1.right());

    bool intersectY = (r2.top() >= r1.top() && r2.top() <= r1.bottom())
      || (r2.bottom() >= r1.top() && r2.bottom() <= r1.bottom());

    return intersectX && intersectY;
  }
}

WRectF WRectF::united(const WRectF& other) const
{
  if (isEmpty())
    return other;
  else if (other.isEmpty())
    return *this;
  else {
    WRectF r1 = normalized();
    WRectF r2 = other.normalized();

    double l = std::min(r1.left(), r2.left());
    double r = std::max(r1.right(), r2.right());
    double t = std::min(r1.top(), r2.top());
    double b = std::max(r1.bottom(), r2.bottom());

    return WRectF(l, t, r - l, b - t);
  }
}

WRectF WRectF::normalized() const
{
  double x, y, w, h;

  if (width_ > 0) {
    x = x_;
    w = width_;
  } else {
    x = x_ + width_;
    w = -width_;
  }

  if (height_ > 0) {
    y = y_;
    h = height_;
  } else {
    y = y_ + height_;
    h = -height_;
  }

  return WRectF(x, y, w, h);
}

}
