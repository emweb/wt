/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WPushButton>
#include <Wt/WTable>
#include <Wt/WTableCell>
#include <Wt/WText>
#include <boost/lexical_cast.hpp>

#include "MandelbrotExample.h"
#include "MandelbrotImage.h"

MandelbrotExample::MandelbrotExample(WContainerWidget *parent)
  : WContainerWidget(parent)
{
  new WText("<h2>Wt Mandelbrot example</h2>"
	    "<p>The image below is a WVirtualImage that renders the "
	    "classic Mandelbrot fractal.</p>"
	    "<p>It is drawn as a grid of many smaller images, "
	    "computed on the fly, as you scroll around "
	    "through the virtual image. You can scroll the image using the "
	    "buttons, or by dragging the mouse.</p>", this);

  WTable *layout = new WTable(this);

  mandelbrot_ = new MandelbrotImage(400, 400,
				    3000, 3000,
				    -2,
				    -1.5,
				    1,
				    1.5, layout->elementAt(0, 0));

  WContainerWidget *buttons = new WContainerWidget(layout->elementAt(0, 0));
  buttons->resize(400, WLength());
  buttons->setContentAlignment(AlignCenter);

  (new WPushButton("Left", buttons))
    ->clicked.connect(SLOT(this, MandelbrotExample::moveLeft));
  (new WPushButton("Right", buttons))
    ->clicked.connect(SLOT(this, MandelbrotExample::moveRight));
  (new WPushButton("Up", buttons))
    ->clicked.connect(SLOT(this, MandelbrotExample::moveUp));
  (new WPushButton("Down", buttons))
    ->clicked.connect(SLOT(this, MandelbrotExample::moveDown));

  new WBreak(buttons);

  (new WPushButton("Zoom in", buttons))
    ->clicked.connect(SLOT(this, MandelbrotExample::zoomIn));
  (new WPushButton("Zoom out", buttons))
    ->clicked.connect(SLOT(this, MandelbrotExample::zoomOut));

  viewPortText_ = new WText(layout->elementAt(0, 1));
  layout->elementAt(0, 1)->setPadding(10);

  updateViewPortText();

  mandelbrot_->viewPortChanged
    .connect(SLOT(this, MandelbrotExample::updateViewPortText));
}

void MandelbrotExample::moveLeft()
{
  mandelbrot_->scroll(-50, 0);
}

void MandelbrotExample::moveRight()
{
  mandelbrot_->scroll(50, 0);
}

void MandelbrotExample::moveUp()
{
  mandelbrot_->scroll(0, -50);
}

void MandelbrotExample::moveDown()
{
  mandelbrot_->scroll(0, 50);
}

void MandelbrotExample::zoomIn()
{
  mandelbrot_->zoomIn();
}

void MandelbrotExample::zoomOut()
{
  mandelbrot_->zoomOut();
}

void MandelbrotExample::updateViewPortText()
{
  viewPortText_->setText
    ("Current viewport: ("
     + boost::lexical_cast<std::string>(mandelbrot_->currentX1()) + ","
     + boost::lexical_cast<std::string>(mandelbrot_->currentY1()) + ") to ("
     + boost::lexical_cast<std::string>(mandelbrot_->currentX2()) + ","
     + boost::lexical_cast<std::string>(mandelbrot_->currentY2()) + ")");
}

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new WApplication(env);
  app->setTitle("Wt Mandelbrot example");

  app->root()->addWidget(new MandelbrotExample());
  return app;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}
