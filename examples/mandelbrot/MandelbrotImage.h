// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef MANDELBROT_IMAGE_H_
#define MANDELBROT_IMAGE_H_

#include <Wt/WVirtualImage.h>

using namespace Wt;

namespace Wt {
  class WRasterImage;
}

class MandelbrotImage : public WVirtualImage
{
public:
  MandelbrotImage(int width, int height,
		  int64_t virtualWidth, int64_t virtualHeight,
		  double bx1, double by1,
		  double bx2, double by2);

  void zoomIn();
  void zoomOut();

  void generate(int64_t x, int64_t y, WRasterImage *img);

  double currentX1() const;
  double currentY1() const;
  double currentX2() const;
  double currentY2() const;

private:
  double bx1_, by1_, bwidth_, bheight_;
  int maxDepth_;
  double bailOut2_;

  virtual std::unique_ptr<WResource> render(int64_t x, int64_t y, int w, int h);
  double calcPixel(double x, double y);

  double convertPixelX(int64_t x) const;
  double convertPixelY(int64_t y) const;
};

#endif // MANDELBROT_IMAGE_H_
