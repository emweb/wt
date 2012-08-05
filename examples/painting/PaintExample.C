/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WContainerWidget>
#include <Wt/WGridLayout>
#include <Wt/WSlider>
#include <Wt/WText>

#include "PaintExample.h"
#include "ShapesWidget.h"

using namespace Wt;

PaintExample::PaintExample(WContainerWidget *root, bool showTitle)
  : WContainerWidget(root)
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
    "WSlider</a> widgets. Here,"
    "the events of the WSlider widgets are used to scale and rotate the "
    "emweb logo."
    "</p>"
    "<p>"
    "In non-IE browsers, a different backend is used for positive or negative "
    "angles (SVG or HTML canvas)."
    "</p>";

  new WText(text, this);

  WContainerWidget *emweb = new WContainerWidget(this);
  emweb->setMargin(WLength::Auto, Left | Right);

  WGridLayout *layout = new WGridLayout();
  emweb->setLayout(layout);

  WSlider *scaleSlider = new WSlider(Horizontal);
  scaleSlider->setMinimum(0);
  scaleSlider->setMaximum(20);
  scaleSlider->setValue(10);
  scaleSlider->setTickInterval(5);
  scaleSlider->setTickPosition(WSlider::TicksBothSides);
  scaleSlider->resize(300, 50);
  scaleSlider->sliderMoved().connect(this, &PaintExample::scaleShape);

  layout->addWidget(scaleSlider, 0, 1, AlignCenter | AlignMiddle);

  WSlider *rotateSlider = new WSlider(Vertical);
  rotateSlider->setMinimum(-30);
  rotateSlider->setMaximum(30);
  rotateSlider->setValue(0);
  rotateSlider->setTickInterval(10);
  rotateSlider->setTickPosition(WSlider::TicksBothSides);
  rotateSlider->resize(50, 400);
  rotateSlider->sliderMoved().connect(this, &PaintExample::rotateShape);

  layout->addWidget(rotateSlider, 1, 0, AlignCenter | AlignMiddle);

  shapes_ = new ShapesWidget();
  shapes_->setAngle(0.0);
  shapes_->setRelativeSize(0.5);
  shapes_->setPreferredMethod(WPaintedWidget::HtmlCanvas);

  layout->addWidget(shapes_, 1, 1, AlignCenter | AlignMiddle);
}

void PaintExample::rotateShape(int v)
{
  shapes_->setAngle(v / 2.0);

  // Being silly: test alternate rendering method
  shapes_->setPreferredMethod(v < 0 ? WPaintedWidget::InlineSvgVml
			      : WPaintedWidget::HtmlCanvas);
}

void PaintExample::scaleShape(int v)
{
  shapes_->setRelativeSize(0.1 + 0.9 * (v/20.0));
}
