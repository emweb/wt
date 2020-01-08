// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SHAPE_H_
#define SHAPE_H_

#include <Wt/WPainter.h>
#include <Wt/WColor.h>
#include <Wt/WString.h>

using namespace Wt;

class ShapeColor : public WColor
{
public:
  ShapeColor(const WColor& color, const WString& colorName)
    : WColor(color),
      colorName_(colorName)
  { }

  WString colorName() {
    return colorName_;
  }

private:
  WString colorName_;
};

class Shape
{
public:
  Shape(const WPointF& center, const ShapeColor& color, const double size)
    : center_(center),
      color_(color),
      size_(size)
  {}

  virtual bool contains(const WPointF& point) const =0;
  virtual WString shapeName() const =0;
  virtual void paint(WPainter& painter) const =0;

  ShapeColor color() const {
    return color_;
  }

  double size() const {
    return size_;
  }

protected:
  WPointF center() const {
    return center_;
  }

private:
  WPointF center_;
  ShapeColor color_;
  double size_;
};

class Circle : public Shape
{
public:
  Circle(const WPointF& center,
	 const ShapeColor& color, 
	 const double size);
  
  virtual bool contains(const WPointF& point) const override;
  virtual WString shapeName() const override;
  virtual void paint(WPainter& painter) const override;
 
 private:
  double distanceTo(const double x1, const double y1, 
		    const double x2, const double y2) const;
};

class Rectangle : public Shape
{
public:
  Rectangle(const WPointF& center,
	    const ShapeColor& color, 
	    const double size);

  virtual bool contains(const WPointF& point) const override;
  virtual WString shapeName() const override;
  virtual void paint(WPainter& painter) const override;
};

#endif // SHAPE_H_
