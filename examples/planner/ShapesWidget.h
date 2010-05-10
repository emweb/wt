// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SHAPES_WIDGET_H_
#define SHAPES_WIDGET_H_

#include "Shape.h"

#include <Wt/WPaintedWidget>
#include <Wt/WPaintDevice>

class ShapesWidget : public Wt::WPaintedWidget
{
public:
  ShapesWidget(Wt::WContainerWidget* parent);
  ~ShapesWidget();

  Wt::WString selectedColor();
  Wt::WString selectedShape();
  bool correctlyClicked(const Wt::WMouseEvent& me);
  void initShapes();
  
protected:
  virtual void paintEvent(Wt::WPaintDevice *paintDevice);

private:
  int randomInt(const unsigned max);
  double randomDouble();
  bool sameShapeAndColor(const Shape* s1, const Shape* s2);
  ShapeColor createRandomColor();
  Shape* createRandomShape();
  void cleanupShapes();

private:
  std::vector<Shape*> shapes_;
  Shape* toSelect_;
};

#endif //SHAPES_WIDGET_H_
