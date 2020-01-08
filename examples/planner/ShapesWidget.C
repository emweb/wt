/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ShapesWidget.h"

#include <cstdlib>

#include <Wt/WPainter.h>
#include <Wt/WGlobal.h>

ShapesWidget::ShapesWidget()
  : WPaintedWidget(),
    toSelect_(nullptr)
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
  return std::rand() / (double(RAND_MAX) + 1);
}

int ShapesWidget::randomInt(const unsigned max) 
{
  return (std::rand() % max);
}

void ShapesWidget::cleanupShapes() 
{
  shapes_.clear();
  toSelect_ = nullptr;
}

void ShapesWidget::initShapes()
{
  cleanupShapes();
  
  const unsigned numberOfShapes = 5;

  unsigned i = 1;
	

  auto toSelectPtr = createRandomShape();
  toSelect_ = toSelectPtr.get();
  while (i < numberOfShapes) {
    std::unique_ptr<Shape> s = createRandomShape();
    if (!sameShapeAndColor(s.get(), toSelect_)) {
      shapes_.push_back(std::move(s));
      i++;
    }
  }
  shapes_.insert(shapes_.begin() + randomInt((int)shapes_.size()), std::move(toSelectPtr));
}

void ShapesWidget::paintEvent(WPaintDevice *paintDevice) 
{
  WPainter painter(paintDevice);

  for (auto& shape : shapes_)
    shape->paint(painter);
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
    return ShapeColor(WColor(StandardColor::Red), WString::tr("captcha.red"));
  case 1:
    return ShapeColor(WColor(StandardColor::Green), WString::tr("captcha.green"));
  case 2:
    return ShapeColor(WColor(StandardColor::Blue), WString::tr("captcha.blue"));
  case 3:
    return ShapeColor(WColor(StandardColor::Yellow), WString::tr("captcha.yellow"));
  default:
    return ShapeColor(WColor(StandardColor::Black), WString("Invalid color"));
  }
}

std::unique_ptr<Shape> ShapesWidget::createRandomShape()
{
  double size = 6 + randomDouble() * 6;

  ShapeColor c = createRandomColor();
		
  double x = size + randomDouble() * (width().value() - 2 * size);
  double y = size + randomDouble() * (height().value() - 2 * size);

  const unsigned amountOfShapes = 2;
  
  unsigned shapeId = randomInt(amountOfShapes);
  if (shapeId == 0)
    return cpp14::make_unique<Circle>(WPointF(x, y), c, size);
  else
    return cpp14::make_unique<Rectangle>(WPointF(x, y), c, size);
}
