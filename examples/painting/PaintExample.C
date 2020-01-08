/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WContainerWidget.h>
#include <Wt/WGridLayout.h>
#include <Wt/WSlider.h>
#include <Wt/WText.h>
#include <iostream>

#include "PaintExample.h"
#include "ShapesWidget.h"

using namespace Wt;

PaintExample::PaintExample(bool showTitle)
  : WContainerWidget()
{
  std::string text;
  if (showTitle)
    text += "<h2>Paint example</h2>";

  text +=
    "<p>A simple example demonstrating cross-browser vector graphics."
    "</p>"
    "<p>The emweb logo below is painted using the Wt WPainter API from "
    "bezier paths, and rendered to the browser using inline SVG, inline VML "
    "or the HTML 5 &lt;canvas&gt; element."
    "</p>"
    "<p>"
    "The example also demonstrates the horizontal and vertical "
    "<a href=\"http://www.webtoolkit.eu/wt/doc/reference/html/classWt_1_1WSlider.html\" target=\"_blank\">"
    "WSlider</a> widgets. Here, "
    "the events of the WSlider widgets are used to scale and rotate the "
    "emweb logo."
    "</p>"
    "<p>"
    "To demonstrate the different rendering methods, a different backend is used for positive or negative "
    "angles (SVG or HTML canvas)."
    "</p>";

  this->addWidget(cpp14::make_unique<WText>(text));

  WContainerWidget *emweb = this->addWidget(cpp14::make_unique<WContainerWidget>());
  emweb->setMargin(WLength::Auto, Side::Left | Side::Right);

  auto layout = cpp14::make_unique<WGridLayout>();
  auto layout_ = emweb->setLayout(std::move(layout));

  std::unique_ptr<WSlider> scaleSlider(cpp14::make_unique<WSlider>());
  scaleSlider->setMinimum(0);
  scaleSlider->setMaximum(20);
  scaleSlider->setValue(10);
  scaleSlider->setTickInterval(5);
  scaleSlider->setTickPosition(WSlider::TicksBothSides);
  scaleSlider->resize(300, 50);
  scaleSlider->sliderMoved().connect(this, &PaintExample::scaleShape);

  layout_->addWidget(std::move(scaleSlider), 0, 1, AlignmentFlag::Center | AlignmentFlag::Middle);

  auto rotateSlider = cpp14::make_unique<WSlider>(Orientation::Vertical);
  rotateSlider->setMinimum(-30);
  rotateSlider->setMaximum(30);
  rotateSlider->setValue(0);
  rotateSlider->setTickInterval(10);
  rotateSlider->setTickPosition(WSlider::TicksBothSides);
  rotateSlider->resize(50, 400);
  rotateSlider->sliderMoved().connect(this, &PaintExample::rotateShape);

  layout_->addWidget(std::move(rotateSlider), 1, 0, AlignmentFlag::Center | AlignmentFlag::Middle);

  auto shapes = cpp14::make_unique<ShapesWidget>();
  shapes_ = shapes.get();
  shapes_->setAngle(0.0);
  shapes_->setRelativeSize(0.5);
  shapes_->setPreferredMethod(RenderMethod::HtmlCanvas);

  layout_->addWidget(std::move(shapes), 1, 1,
                    AlignmentFlag::Center | AlignmentFlag::Middle);
}

void PaintExample::rotateShape(int v)
{
  shapes_->setAngle(v / 2.0);

  // Being silly: test alternate rendering method
  shapes_->setPreferredMethod(v < 0 ? RenderMethod::InlineSvgVml
                              : RenderMethod::HtmlCanvas);
}

void PaintExample::scaleShape(int v)
{
  shapes_->setRelativeSize(0.1 + 0.9 * (v/20.0));
}
