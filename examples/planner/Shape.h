// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SHAPE_H_
#define SHAPE_H_

#include <Wt/WPainter>
#include <Wt/WColor>
#include <Wt/WString>

class ShapeColor : public Wt::WColor
{
public:
  ShapeColor(const Wt::WColor& color, const Wt::WString& colorName)
    : WColor(color),
      colorName_(colorName)
  { }

  Wt::WString colorName() {
    return colorName_;
  }

private:
  Wt::WString colorName_;
};

class Shape
{
public:
  Shape(const Wt::WPointF& center, const ShapeColor& color, const double size) 
    : center_(center),
      color_(color),
      size_(size)
  {}

  virtual ~Shape();

  virtual bool contains(const Wt::WPointF& point) const =0;
  virtual Wt::WString shapeName() const =0;
  virtual void paint(Wt::WPainter& painter) const =0;

  ShapeColor color() const {
    return color_;
  }

  double size() const {
    return size_;
  }

protected:
  Wt::WPointF center() const {
    return center_;
  }

private:
  Wt::WPointF center_;
  ShapeColor color_;
  double size_;
};

class Circle : public Shape
{
public:
  Circle(const Wt::WPointF& center, 
	 const ShapeColor& color, 
	 const double size);
  
  virtual bool contains(const Wt::WPointF& point) const;
  virtual Wt::WString shapeName() const;
  virtual void paint(Wt::WPainter& painter) const;
 
 private:
  double distanceTo(const double x1, const double y1, 
		    const double x2, const double y2) const;
};

class Rectangle : public Shape
{
public:
  Rectangle(const Wt::WPointF& center, 
	    const ShapeColor& color, 
	    const double size);

  virtual bool contains(const Wt::WPointF& point) const;
  virtual Wt::WString shapeName() const;
  virtual void paint(Wt::WPainter& painter) const;
};

#endif // SHAPE_H_
