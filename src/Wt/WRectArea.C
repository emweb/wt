/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WRectArea.h"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

WRectArea::WRectArea()
  : x_(0), y_(0), width_(0), height_(0)
{ }

WRectArea::WRectArea(int x, int y, int width, int height)
  : x_(x), y_(y), width_(width), height_(height)
{ }

WRectArea::WRectArea(double x, double y, double width, double height)
  : x_(x),
    y_(y),
    width_(width),
    height_(height)
{ }

WRectArea::WRectArea(const WRectF& rect)
  : x_(rect.x()),
    y_(rect.y()),
    width_(rect.width()),
    height_(rect.height())
{ }

void WRectArea::setX(int x)
{
  x_ = x;
  
  repaint();
}

void WRectArea::setY(int y)
{
  y_ = y;
  
  repaint();
}

void WRectArea::setWidth(int width)
{
  width_ = width;

  repaint();
}

void WRectArea::setHeight(int height)
{
  height_ = height;

  repaint();
}

bool WRectArea::updateDom(DomElement& element, bool all)
{
  element.setAttribute("shape", "rect");

  std::stringstream coords;

  int x = static_cast<int>(x_);
  int y = static_cast<int>(y_);
  int width = static_cast<int>(width_);
  int height = static_cast<int>(height_);
  if (x == 0 && y == 0 && width == 0 && height == 0)
    coords << "0%,0%,100%,100%";
  else
    coords << x << ',' << y << ',' << (x + width) << ',' << (y + height);
  element.setAttribute("coords", coords.str());

  return WAbstractArea::updateDom(element, all);
}

std::string WRectArea::updateAreaCoordsJS()
{
  std::stringstream coords;
  char buf[30];

  coords << "[" << jsRef() << ",[";
  coords << Utils::round_js_str(x_, 2, buf) << ',';
  coords << Utils::round_js_str(y_, 2, buf) << ',';
  coords << Utils::round_js_str((x_ + width_), 2, buf) << ',';
  coords << Utils::round_js_str((y_ + height_), 2, buf) << "]]";

  return coords.str();
}

}
