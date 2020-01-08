// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SHAPES_WIDGET_H_
#define SHAPES_WIDGET_H_

#include "Shape.h"

#include <Wt/WPaintedWidget.h>
#include <Wt/WPaintDevice.h>

using namespace Wt;

class ShapesWidget : public WPaintedWidget
{
public:
  ShapesWidget();
  ~ShapesWidget();

  WString selectedColor();
  WString selectedShape();
  bool correctlyClicked(const WMouseEvent& me);
  void initShapes();
  
protected:
  virtual void paintEvent(WPaintDevice *paintDevice);

private:
  int randomInt(const unsigned max);
  double randomDouble();
  bool sameShapeAndColor(const Shape* s1, const Shape* s2);
  ShapeColor createRandomColor();
  std::unique_ptr<Shape> createRandomShape();
  void cleanupShapes();

private:
  std::vector<std::unique_ptr<Shape>> shapes_;
  Shape *toSelect_;
};

#endif //SHAPES_WIDGET_H_
