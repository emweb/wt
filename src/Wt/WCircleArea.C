/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WCircleArea.h"
#include "Wt/WPoint.h"
#include "Wt/WPointF.h"

#include "DomElement.h"
#include "WebUtils.h"

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
  x_ = point.x();
  y_ = point.y();

  repaint();
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
  coords << static_cast<int>(x_) << ','
         << static_cast<int>(y_) << ','
         << static_cast<int>(r_);
  element.setAttribute("coords", coords.str());

  return WAbstractArea::updateDom(element, all);
}

std::string WCircleArea::updateAreaCoordsJS()
{
  std::stringstream coords;
  char buf[30];

  coords << "[" << jsRef() << ",[";
  coords << Utils::round_js_str(x_, 2, buf) << ',';
  coords << Utils::round_js_str(y_, 2, buf) << ',';
  coords << Utils::round_js_str(r_, 2, buf) << "]]";

  return coords.str();
}

}
