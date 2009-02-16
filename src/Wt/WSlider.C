/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WPaintedWidget"
#include "Wt/WPainter"
#include "Wt/WSlider"

namespace Wt {
  const int HANDLE_WIDTH = 17;
  const int HANDLE_HEIGHT = 21;

  class WSliderBackground : public WPaintedWidget
  {
  public:
    WSliderBackground(WSlider *slider)
      : WPaintedWidget(),
	slider_(slider)
    { }

  protected:
    void paintEvent(WPaintDevice *paintDevice);

  private:
    WSlider *slider_;
  };

  void WSliderBackground::paintEvent(WPaintDevice *paintDevice)
  {
    WPainter painter(paintDevice);

    WPen pen;
    pen.setCapStyle(FlatCap);

    int w, h;

    if (slider_->orientation() == Horizontal) {
      w = (int)width().value();
      h = (int)height().value();
    } else {
      w = (int)height().value();
      h = (int)width().value();

      painter.translate(0, w);
      painter.rotate(-90);
    }

    /*
     * Draw inset slider groove, as three lines
     */
    pen.setColor(WColor(0x89, 0x89, 0x89));
    painter.setPen(pen);
    
    painter.drawLine(HANDLE_WIDTH/2,     h/2 - 2 + 0.5,
		     w - HANDLE_WIDTH/2, h/2 - 2 + 0.5);

    pen.setColor(WColor(0xb7, 0xb7, 0xb7));
    painter.setPen(pen);

    painter.drawLine(HANDLE_WIDTH/2,     h/2 + 1 + 0.5,
		     w - HANDLE_WIDTH/2, h/2 + 1 + 0.5);

    pen.setColor(WColor(0xd7, 0xd7, 0xd7));
    pen.setWidth(2);
    painter.setPen(pen);

    painter.drawLine(HANDLE_WIDTH/2,     h/2,
		     w - HANDLE_WIDTH/2, h/2);

    /*
     * Draw ticks
     */
    if (slider_->tickPosition() != WSlider::NoTicks) {
      int tickInterval = slider_->tickInterval();
      int range = slider_->maximum() - slider_->minimum();
      if (tickInterval == 0)
	tickInterval = range / 2;

      double tickStep = ((double)w - HANDLE_WIDTH) / (range / tickInterval);

      pen.setWidth(1);
      painter.setPen(pen);

      int y1 = h / 4;
      int y2 = h / 2 - 4;
      int y3 = h / 2 + 4;
      int y4 = h - h/4;

      for (unsigned i = 0; ; ++i) {
	int x = HANDLE_WIDTH/2 + (int) (i * tickStep);

	if (x > w - HANDLE_WIDTH/2)
	  break;

	if (slider_->tickPosition() & WSlider::TicksAbove)
	  painter.drawLine(x + 0.5, y1, x + 0.5, y2);
	if (slider_->tickPosition() & WSlider::TicksBelow)
	  painter.drawLine(x + 0.5, y3, x + 0.5, y4);
      }
    }
  }

WSlider::WSlider(WContainerWidget *parent)
  : WCompositeWidget(parent),
    valueChanged(this),
    orientation_(Horizontal),
    tickInterval_(0),
    tickPosition_(NoTicks),
    minimum_(0),
    maximum_(99),
    value_(0)
{
  setImplementation(impl_ = new WContainerWidget());
  create();
}

WSlider::WSlider(Orientation orientation, WContainerWidget *parent)
  : WCompositeWidget(parent),
    valueChanged(this),
    orientation_(orientation),
    tickInterval_(0),
    tickPosition_(NoTicks),
    minimum_(0),
    maximum_(99),
    value_(0)
{
  setImplementation(impl_ = new WContainerWidget());
  create();
}

WSlider::~WSlider()
{ }

void WSlider::resize(const WLength& width, const WLength& height)
{
  WCompositeWidget::resize(width, height);
  background_->resize(width, height);
  update();
}

void WSlider::create()
{
  setPositionScheme(Relative);

  impl_->addWidget(background_ = new WSliderBackground(this));
  impl_->addWidget(handle_ = new WContainerWidget());

  handle_->setPopup(true);
  handle_->setPositionScheme(Absolute);

  if (orientation_ == Horizontal)
    resize(150, 50);
  else
    resize(50, 150);

  handle_->mouseWentDown.connect(mouseDownJS_);
  handle_->mouseMoved.connect(mouseMovedJS_);
  handle_->mouseWentUp.connect(mouseUpJS_);

  background_->clicked.connect(SLOT(this, WSlider::onSliderClick));

  update();
}

void WSlider::update()
{
  std::string resourcesURL = WApplication::resourcesUrl();

  background_->update();

  handle_->decorationStyle()
    .setBackgroundImage(resourcesURL + "slider-thumb-"
			+ (orientation_ == Horizontal ? 'h': 'v')
			+ ".gif");

  if (orientation_ == Horizontal) {
    handle_->resize(HANDLE_WIDTH, HANDLE_HEIGHT);
    handle_->setOffsets(height().value() / 2 + 2, Top);
  } else {
    handle_->resize(HANDLE_HEIGHT, HANDLE_WIDTH);
    handle_->setOffsets(width().value() / 2 - HANDLE_HEIGHT - 2, Left);
  }

  double l = (orientation_ == Horizontal ? width().value() : height().value());
  double pixelsPerUnit = (l - HANDLE_WIDTH) / range();

  std::string dir = (orientation_ == Horizontal ? "left" : "top");
  std::string u = (orientation_ == Horizontal ? "x" : "y");
  std::string maxS = boost::lexical_cast<std::string>(l - HANDLE_WIDTH);
  std::string ppU = boost::lexical_cast<std::string>(pixelsPerUnit);

  /*
   * Note: cancelling the mouseDown event prevents the selection behaviour
   */
  mouseDownJS_.setJavaScript
    ("function(obj, event) {"
     "  obj.setAttribute('down', " WT_CLASS ".widgetCoordinates(obj, event)."
     + u + "); "
        WT_CLASS ".cancelEvent(event);"
     "}");

  mouseMovedJS_.setJavaScript
    ("function(obj, event) {"
     "  var down = obj.getAttribute('down');"
     "  if (down != null && down != '') {"
     "    var objh = " + handle_->jsRef() + ";"
     "    var objb = " + background_->jsRef() + ";"
     "    var u = " WT_CLASS ".pageCoordinates(event)." + u + " - down;"
     "    var w = " WT_CLASS ".widgetPageCoordinates(objb)." + u + ";"
     "    var d = u-w;"
     "    d = (d<0?0:(d>" + maxS + "?" + maxS + ":d));"
     "    d = Math.round(d/" + ppU + ")*" + ppU + ";"
     "    objh.style." + dir + " = d + 'px';"
     "  }"
     "}");

  mouseUpJS_.setJavaScript
    ("function(obj, event) {"
     "  var down = obj.getAttribute('down');"
     "  if (down != null && down != '') {"
     "    obj.removeAttribute('down');"
     "    var objb = " + background_->jsRef() + ";"
     "    objb.onclick(event);"
     "  }"
     "}");

  updateSliderPosition();
}

void WSlider::onSliderClick(const WMouseEvent& event)
{
  double l = (orientation_ == Horizontal ? width().value() : height().value());
  double pixelsPerUnit = (l - HANDLE_WIDTH) / range();

  double u =
    ((orientation_ == Horizontal) ? event.widget().x : event.widget().y);
  
  u -= HANDLE_WIDTH / 2;

  setValue(minimum_ + (int)(u / pixelsPerUnit + 0.5));
  valueChanged.emit(value());
}

void WSlider::updateSliderPosition()
{
  double l = (orientation_ == Horizontal ? width().value() : height().value());
  double pixelsPerUnit = (l - HANDLE_WIDTH) / range();

  double u = ((double)value_ - minimum_) * pixelsPerUnit;
  handle_->setOffsets(u, orientation_ == Horizontal ? Left : Top);
}

void WSlider::setOrientation(Orientation orientation)
{
  orientation_ = orientation;

  update();
}

void WSlider::setTickPosition(TickPosition tickPosition)
{
  tickPosition_ = tickPosition;

  background_->update();
}

void WSlider::setTickInterval(int tickInterval)
{
  tickInterval_ = tickInterval;

  background_->update();
}

void WSlider::setMinimum(int minimum)
{
  minimum_ = minimum;
  value_ = std::max(minimum_, value_);
  maximum_ = std::max(minimum_ + 1, maximum_);

  update();
}

void WSlider::setMaximum(int maximum)
{
  maximum_ = maximum;
  value_ = std::min(maximum_, value_);
  minimum_ = std::min(maximum_ - 1, minimum_);

  update();
}

void WSlider::setRange(int minimum, int maximum)
{
  minimum_ = minimum;
  maximum_ = maximum;
  value_ = std::min(maximum_, std::max(minimum_, value_));

  update();
}

void WSlider::setValue(int value)
{
  value_ = std::min(maximum_, std::max(minimum_, value));
  updateSliderPosition();
}

}
