// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SHAPES_WIDGET_H_
#define SHAPES_WIDGET_H_

#include <Wt/WPaintedWidget.h>

namespace Wt {
  class WPainter;
}

using namespace Wt;

class ShapesWidget : public WPaintedWidget
{
public:
  ShapesWidget();

  void setAngle(double angle);
  double angle() const { return angle_; }

  void setRelativeSize(double size);
  double relativeSize() const { return size_; }

protected:
  virtual void paintEvent(WPaintDevice *paintDevice) override;

private:
  double angle_;
  double size_;

  void drawEmwebLogo(WPainter& p);
  void drawEmwebE(WPainter& p);
  void drawEmwebMW(WPainter& p);
};

#endif // SHAPES_WIDGET_H_
