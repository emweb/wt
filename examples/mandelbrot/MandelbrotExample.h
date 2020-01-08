// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef MANDELBROT_EXAMPLE_H_
#define MANDELBROT_EXAMPLE_H_

#include <Wt/WContainerWidget.h>
#include <Wt/WGlobal.h>

using namespace Wt;

class MandelbrotImage;

class MandelbrotExample : public WContainerWidget
{
public:
  MandelbrotExample();

private:
  MandelbrotImage *mandelbrot_;

  WText *viewPortText_;

  void moveLeft();
  void moveRight();
  void moveUp();
  void moveDown();
  void zoomIn();
  void zoomOut();

  void updateViewPortText();
};

#endif // MANDELBROT_EXAMPLE_H_
