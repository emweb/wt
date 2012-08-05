/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ShapesWidget.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <Wt/WPainter>
#include <Wt/WGlobal>

using namespace Wt;

ShapesWidget::ShapesWidget(WContainerWidget* parent) 
  : WPaintedWidget(parent),
    toSelect_(0)
{}

ShapesWidget::~ShapesWidget()
{
  cleanupShapes();
}

WString ShapesWidget::selectedColor()
{
  return toSelect_->color().colorName();
}

WString ShapesWidget::selectedShape()
{
  return toSelect_->shapeName();
}

bool ShapesWidget::correctlyClicked(const WMouseEvent& me)
{
  return toSelect_->contains(WPointF(me.widget().x, me.widget().y));
}
  
double ShapesWidget::randomDouble()
{
  return rand() / (double(RAND_MAX) + 1); 
}

int ShapesWidget::randomInt(const unsigned max) 
{
  return (rand() % max);
}

void ShapesWidget::cleanupShapes() 
{
  for (unsigned i = 0; i < shapes_.size(); i++) {
    delete shapes_[i];
  }
  shapes_.clear();
  toSelect_ = 0;
}

void ShapesWidget::initShapes()
{
  cleanupShapes();
  
  const unsigned numberOfShapes = 5;

  unsigned i = 1;
	
  toSelect_ = createRandomShape();
  while (i < numberOfShapes) {
    Shape* s = createRandomShape();
    if (!sameShapeAndColor(s, toSelect_)) {
      shapes_.push_back(s);
      i++;
    } else {
      delete s;
    }
  }

  shapes_.insert(shapes_.begin() + randomInt((int)shapes_.size()), toSelect_);
}

void ShapesWidget::paintEvent(WPaintDevice *paintDevice) 
{
  WPainter painter(paintDevice);

  for (unsigned i = 0; i < shapes_.size(); i++) {
    Shape* s = shapes_[i];
    s->paint(painter);
  }
}

bool ShapesWidget::sameShapeAndColor(const Shape* s1, const Shape* s2)
{
  return s1->shapeName() == s2->shapeName() 
    && s1->color() == s2->color();
}

ShapeColor ShapesWidget::createRandomColor() 
{
  static const unsigned amountOfColors = 4;
  
  switch (randomInt(amountOfColors)) {
  case 0:
    return ShapeColor(red, WString::tr("captcha.red")); 
  case 1:
    return ShapeColor(green, WString::tr("captcha.green")); 
  case 2:
    return ShapeColor(blue, WString::tr("captcha.blue")); 
  case 3:
    return ShapeColor(yellow, WString::tr("captcha.yellow")); 
  default:
    return ShapeColor(black, WString("Invalid color")); 
  }
}

Shape* ShapesWidget::createRandomShape()
{
  double size = 6 + randomDouble() * 6;

  ShapeColor c = createRandomColor();
		
  double x = size + randomDouble() * (width().value() - 2 * size);
  double y = size + randomDouble() * (height().value() - 2 * size);

  const unsigned amountOfShapes = 2;
  
  unsigned shapeId = randomInt(amountOfShapes);
  if (shapeId == 0)
    return new Circle(WPointF(x, y), c, size);
  else
    return new Rectangle(WPointF(x, y), c, size);
}
