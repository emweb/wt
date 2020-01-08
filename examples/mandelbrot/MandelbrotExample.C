/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WText.h>
#include <Wt/WAny.h>

#include "MandelbrotExample.h"
#include "MandelbrotImage.h"

MandelbrotExample::MandelbrotExample()
  : WContainerWidget()
{
  this->addWidget(cpp14::make_unique<WText>("<div style=\"height:1px; width: 1px;\"/>"
	    "<h2>Wt Mandelbrot example</h2>"
	    "<p>The image below is a WVirtualImage that renders the "
	    "classic Mandelbrot fractal.</p>"
	    "<p>It is drawn as a grid of many smaller images, "
	    "computed on the fly, as you scroll around "
	    "through the virtual image. You can scroll the image using the "
	    "buttons, or by dragging the mouse.</p>"));

  WTable *layout = this->addWidget(cpp14::make_unique<WTable>());

  auto mandelbrotPtr = cpp14::make_unique<MandelbrotImage>(400, 400,
				    3000, 3000,
				    -2,
				    -1.5,
				    1,
				    1.5);
  mandelbrot_ = mandelbrotPtr.get();

  layout->elementAt(0, 0)->addWidget(std::move(mandelbrotPtr));

  WContainerWidget *buttons = layout->elementAt(0,0)
      ->addWidget(cpp14::make_unique<WContainerWidget>());
  buttons->resize(400, WLength::Auto);
  buttons->setContentAlignment(AlignmentFlag::Center);

  (buttons->addWidget(cpp14::make_unique<WPushButton>("Left")))
    ->clicked().connect(this, &MandelbrotExample::moveLeft);
  (buttons->addWidget(cpp14::make_unique<WPushButton>("Right")))
    ->clicked().connect(this, &MandelbrotExample::moveRight);
  (buttons->addWidget(cpp14::make_unique<WPushButton>("Up")))
    ->clicked().connect(this, &MandelbrotExample::moveUp);
  (buttons->addWidget(cpp14::make_unique<WPushButton>("Down")))
    ->clicked().connect(this, &MandelbrotExample::moveDown);

  buttons->addWidget(cpp14::make_unique<WBreak>());

  (buttons->addWidget(cpp14::make_unique<WPushButton>("Zoom in")))
    ->clicked().connect(this, &MandelbrotExample::zoomIn);
  (buttons->addWidget(cpp14::make_unique<WPushButton>("Zoom out")))
    ->clicked().connect(this, &MandelbrotExample::zoomOut);

  viewPortText_ = layout->elementAt(0,1)->addWidget(cpp14::make_unique<WText>());
  layout->elementAt(0, 1)->setPadding(10);

  updateViewPortText();

  mandelbrot_->viewPortChanged()
    .connect(this, &MandelbrotExample::updateViewPortText);
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
     + asString(mandelbrot_->currentX1()).toUTF8() + ","
     + asString(mandelbrot_->currentY1()).toUTF8() + ") to ("
     + asString(mandelbrot_->currentX2()).toUTF8() + ","
     + asString(mandelbrot_->currentY2()).toUTF8() + ")");
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  auto app = cpp14::make_unique<WApplication>(env);
  app->setTitle("Wt Mandelbrot example");

  auto mandelbrot = cpp14::make_unique<MandelbrotExample>();
  mandelbrot->setPadding(8);
  app->root()->addWidget(std::move(mandelbrot));

  app->styleSheet().addRule("html, body",
			    "border: 0px; margin: 0px; height: 100%;");
  return app;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}
