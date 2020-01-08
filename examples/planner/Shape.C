/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Shape.h"

#include <Wt/WPainterPath.h>

#include <cmath>

Circle::Circle(const WPointF& center, 
	       const ShapeColor& color, 
	       const double size)
  : Shape(center, color, size)
{
}



bool Circle::contains(const WPointF& point) const
{
  return distanceTo(center().x(), 
		    center().y(), 
		    point.x(), 
		    point.y()) 
    <= size();
}

WString Circle::shapeName() const
{
  return WString::tr("captcha.circle");
}
 
double Circle::distanceTo(const double x1, const double y1, 
			  const double x2, const double y2) const
{
  return std::sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

void Circle::paint(WPainter& painter) const
{
  WBrush b;
  b.setStyle(BrushStyle::Solid);
  b.setColor(color());
  
  WPainterPath pp;
  pp.addEllipse(center().x() - size(), 
		center().y() - size(), 
		size() * 2, size() * 2);
  
  painter.fillPath(pp, b);
}

Rectangle::Rectangle(const WPointF& center, 
		     const ShapeColor& color, 
		     const double size)
  : Shape(center, color, size)
{

}

bool Rectangle::contains(const WPointF& point) const
{
  return WRectF(center().x(), 
		center().y(), 
		size(), 
		size()).
    contains(point);
}

WString Rectangle::shapeName() const
{
  return WString::tr("captcha.rectangle");
}

void Rectangle::paint(WPainter& painter) const
{
  WBrush b;
  b.setStyle(BrushStyle::Solid);
  b.setColor(color());
  
  WPainterPath pp;
  pp.addRect(WRectF(center().x(), center().y(), size(), size()));
  
  painter.fillPath(pp, b);
}
