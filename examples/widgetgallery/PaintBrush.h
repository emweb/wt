// This may look like C code, but it's really -*- C++ -*-

#include <Wt/WPaintedWidget>
#include <Wt/WPainterPath>
#include <Wt/WEvent>

#include <vector>

#ifndef PAINTBRUSH_H_
#define PAINTBRUSH_H_

using namespace Wt;

class PaintBrush : public WPaintedWidget
{
public:
  PaintBrush(int width, int height, WContainerWidget *parent = 0);

  void clear() {
    update();
  }

  void setColor(const WColor& c) {
    color_ = c;
  }

protected:
  virtual void paintEvent(WPaintDevice *paintDevice);

private:
  WPainterPath path_;
  WColor color_;

  void mouseDown(const WMouseEvent& e); 
  void mouseDrag(const WMouseEvent& e);
  void touchStart(const WTouchEvent& e);
  void touchMove(const WTouchEvent& e);
};

#endif // PAINTBRUSH_H_
