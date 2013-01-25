/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WCircleArea"
#include "Wt/WPoint"
#include "Wt/WPointF"

#include "DomElement.h"

namespace Wt {

WCircleArea::WCircleArea()
  : x_(0), y_(0), r_(0)
{ }

WCircleArea::WCircleArea(int x, int y, int radius)
  : x_(x), y_(y), r_(radius)
{ }

void WCircleArea::setCenter(const WPoint& point)
{
  setCenter(point.x(), point.y());
}

void WCircleArea::setCenter(const WPointF& point)
{
  setCenter(static_cast<int>(point.x()),
	    static_cast<int>(point.y()));
}

void WCircleArea::setCenter(int x, int y)
{
  x_ = x;
  y_ = y;

  repaint();
}

void WCircleArea::setRadius(int radius)
{
  r_ = radius;

  repaint();
}

bool WCircleArea::updateDom(DomElement& element, bool all)
{
  element.setAttribute("shape", "circle");

  std::stringstream coords;
  coords << x_ << ',' << y_ << ',' << r_;
  element.setAttribute("coords", coords.str());

  return WAbstractArea::updateDom(element, all);
}

}
