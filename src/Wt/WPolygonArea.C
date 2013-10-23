/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WPolygonArea"
#include "Wt/WPointF"

#include "DomElement.h"

namespace Wt {

WPolygonArea::WPolygonArea()
{ }

WPolygonArea::WPolygonArea(const std::vector<WPoint>& points)
 : points_(points)
{ }

#ifndef WT_TARGET_JAVA
WPolygonArea::WPolygonArea(const std::vector<WPointF>& points)
{
  setPoints(points);
}
#endif // WT_TARGET_JAVA

void WPolygonArea::addPoint(int x, int y)
{
  points_.push_back(WPoint(x, y));
  
  repaint();
}

void WPolygonArea::addPoint(double x, double y)
{
  points_.push_back(WPoint(static_cast<int>(x), static_cast<int>(y)));
  
  repaint();
}

void WPolygonArea::addPoint(const WPoint& point)
{
  points_.push_back(point);
  
  repaint();
}

void WPolygonArea::addPoint(const WPointF& point)
{
  points_.push_back(WPoint(static_cast<int>(point.x()),
			   static_cast<int>(point.y())));
               
  repaint();
}

void WPolygonArea::setPoints(const std::vector<WPoint>& points)
{
  points_ = points;
  
  repaint();
}

#ifndef WT_TARGET_JAVA
void WPolygonArea::setPoints(const std::vector<WPointF>& points)
{
  points_.clear();
  for (unsigned i = 0; i < points.size(); ++i)
    addPoint(points[i]);
}
#endif // WT_TARGET_JAVA

bool WPolygonArea::updateDom(DomElement& element, bool all)
{
  element.setAttribute("shape", "poly");

  std::stringstream coords;
  for (unsigned i = 0; i < points_.size(); ++i) {
    if (i != 0)
      coords << ',';
    coords << points_[i].x() << ',' << points_[i].y();
  }
  element.setAttribute("coords", coords.str());

  return WAbstractArea::updateDom(element, all);
}

}
