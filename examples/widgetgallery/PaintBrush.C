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
  dragging_ = false;

  resize(WLength(width), WLength(height));

  decorationStyle().setCursor("icons/pencil.cur", CrossCursor);

  mouseMoved().connect(SLOT(this, PaintBrush::drag));
  mouseWentUp().connect(SLOT(this, PaintBrush::mouseUp));
  mouseWentDown().connect(SLOT(this, PaintBrush::mouseDown));
  
  color_ = WColor(black);
}

void PaintBrush::paintEvent(WPaintDevice *paintDevice)
{
  WPainter painter(paintDevice);
  painter.setRenderHint(WPainter::Antialiasing);
  
  WPen pen;
  pen.setColor(color_);
  painter.setPen(pen);
  painter.drawPath(path_);

  path_ = WPainterPath(path_.currentPosition());
}

void PaintBrush::mouseDown(const WMouseEvent& e)
{
  WMouseEvent::Coordinates c = e.widget();
  path_ = WPainterPath(WPointF(c.x, c.y));
  dragging_ = true;
}

void PaintBrush::mouseUp(const WMouseEvent& e)
{
  dragging_ = false;
}

void PaintBrush::drag(const WMouseEvent& e)
{
  if (!dragging_)
    return;

  WMouseEvent::Coordinates c = e.widget();
  path_.lineTo(c.x, c.y);

  update(PaintUpdate);
}
