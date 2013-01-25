/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WRectArea"

#include "DomElement.h"

namespace Wt {

WRectArea::WRectArea()
  : x_(0), y_(0), width_(0), height_(0)
{ }

WRectArea::WRectArea(int x, int y, int width, int height)
  : x_(x), y_(y), width_(width), height_(height)
{ }

WRectArea::WRectArea(double x, double y, double width, double height)
  : x_(static_cast<int>(x)),
    y_(static_cast<int>(y)),
    width_(static_cast<int>(width)),
    height_(static_cast<int>(height))
{ }

WRectArea::WRectArea(const WRectF& rect)
  : x_(static_cast<int>(rect.x())),
    y_(static_cast<int>(rect.y())),
    width_(static_cast<int>(rect.width())),
    height_(static_cast<int>(rect.height()))
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

  if (x_ == 0 && y_ == 0 && width_ == 0 && height_ == 0)
    coords << "0%,0%,100%,100%";
  else
    coords << x_ << ',' << y_ << ',' << (x_ + width_) << ',' << (y_ + height_);
  element.setAttribute("coords", coords.str());

  return WAbstractArea::updateDom(element, all);
}

}
