#include <boost/lexical_cast.hpp>
#include <iostream>

#include "PaintBrush.h"

#include <Wt/WCssDecorationStyle>
#include <Wt/WPainter>
#include <Wt/WPainterPath>
#include <Wt/WPointF>
#include <Wt/WRectF>

PaintBrush::PaintBrush(int width, int height, WContainerWidget *parent)
  : WPaintedWidget(parent)
{
  setSelectable(false);

  resize(WLength(width), WLength(height));

  decorationStyle().setCursor("icons/pencil.cur", CrossCursor);

  mouseDragged().connect(this, &PaintBrush::mouseDrag);
  mouseWentDown().connect(this, &PaintBrush::mouseDown);
  touchStarted().connect(this, &PaintBrush::touchStart);
  touchMoved().connect(this, &PaintBrush::touchMove);
  touchMoved().preventDefaultAction();
  
  color_ = WColor(black);

  // setPreferredMethod(PngImage);
}

void PaintBrush::paintEvent(WPaintDevice *paintDevice)
{
  WPainter painter(paintDevice);
  painter.setRenderHint(WPainter::Antialiasing);
  
  WPen pen;
  pen.setWidth(3);
  pen.setColor(color_);
  painter.setPen(pen);
  painter.drawPath(path_);

  path_ = WPainterPath(path_.currentPosition());
}

void PaintBrush::mouseDown(const WMouseEvent& e)
{
  Coordinates c = e.widget();
  path_ = WPainterPath(WPointF(c.x, c.y));
}

void PaintBrush::touchStart(const WTouchEvent& e)
{
  Coordinates c = e.touches()[0].widget();
  path_ = WPainterPath(WPointF(c.x, c.y));
}

void PaintBrush::mouseDrag(const WMouseEvent& e)
{
  Coordinates c = e.widget();
  path_.lineTo(c.x, c.y);

  update(PaintUpdate);
}

void PaintBrush::touchMove(const WTouchEvent& e)
{
  Coordinates c = e.touches()[0].widget();
  path_.lineTo(c.x, c.y);

  update(PaintUpdate);
}
