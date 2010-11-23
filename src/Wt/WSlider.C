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

const Wt::WFlags<WSlider::TickPosition> WSlider::NoTicks = 0;
const Wt::WFlags<WSlider::TickPosition> WSlider::TicksBothSides
  = TicksAbove | TicksBelow;

  namespace {
    const int HANDLE_WIDTH = 17;
    const int HANDLE_HEIGHT = 21;
  }

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

    int w, h;

    if (slider_->orientation() == Horizontal) {
      w = (int)width().toPixels();
      h = (int)height().toPixels();
    } else {
      w = (int)height().toPixels();
      h = (int)width().toPixels();

      painter.translate(0, w);
      painter.rotate(-90);
    }

    /*
     * Draw inset slider groove, as three lines
     */
    WPen p1;
    p1.setCapStyle(FlatCap);
    p1.setColor(WColor(0x89, 0x89, 0x89));
    painter.setPen(p1);

    painter.drawLine(WSlider::HANDLE_WIDTH/2,     h/2 - 2 + 0.5,
		     w - WSlider::HANDLE_WIDTH/2, h/2 - 2 + 0.5);

    WPen p2;
    p2.setCapStyle(FlatCap);
    p2.setColor(WColor(0xb7, 0xb7, 0xb7));
    painter.setPen(p2);

    painter.drawLine(WSlider::HANDLE_WIDTH/2,     h/2 + 1 + 0.5,
		     w - WSlider::HANDLE_WIDTH/2, h/2 + 1 + 0.5);

    WPen p3;
    p3.setCapStyle(FlatCap);
    p3.setColor(WColor(0xd7, 0xd7, 0xd7));
    p3.setWidth(2);
    painter.setPen(p3);

    painter.drawLine(WSlider::HANDLE_WIDTH/2,     h/2,
		     w - WSlider::HANDLE_WIDTH/2, h/2);

    /*
     * Draw ticks
     */
    if (slider_->tickPosition()) {
      int tickInterval = slider_->tickInterval();
      int range = slider_->maximum() - slider_->minimum();
      if (tickInterval == 0)
	tickInterval = range / 2;

      double tickStep = ((double)w - WSlider::HANDLE_WIDTH)
	/ (range / tickInterval);

      WPen pen;
      pen.setColor(WColor(0xd7, 0xd7, 0xd7));
      pen.setCapStyle(FlatCap);
      pen.setWidth(1);
      painter.setPen(pen);

      int y1 = h / 4;
      int y2 = h / 2 - 4;
      int y3 = h / 2 + 4;
      int y4 = h - h/4;

      for (unsigned i = 0; ; ++i) {
	int x = WSlider::HANDLE_WIDTH/2 + (int) (i * tickStep);

	if (x > w - WSlider::HANDLE_WIDTH/2)
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
    orientation_(Horizontal),
    tickInterval_(0),
    tickPosition_(0),
    minimum_(0),
    maximum_(99),
    value_(0),
    valueChanged_(this),
    sliderMoved_(this, "moved"),
    sliderReleased_(this, "released")
{
  setImplementation(impl_ = new WContainerWidget());
  create();
}

WSlider::WSlider(Orientation orientation, WContainerWidget *parent)
  : WCompositeWidget(parent),
    orientation_(orientation),
    tickInterval_(0),
    tickPosition_(0),
    minimum_(0),
    maximum_(99),
    value_(0),
    valueChanged_(this),
    sliderMoved_(this, "moved"),
    sliderReleased_(this, "released")
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

void WSlider::layoutSizeChanged(int width, int height)
{
  WCompositeWidget::resize(WLength::Auto, WLength::Auto);
  background_->resize(width, height);
  update();
}

WLength WSlider::w() const
{
  return background_->width();
}

WLength WSlider::h() const
{
  return background_->height();
}

void WSlider::create()
{
  impl_->setStyleClass("Wt-slider");

  setPositionScheme(Relative);

  impl_->addWidget(background_ = new WSliderBackground(this));
  impl_->addWidget(handle_ = new WContainerWidget());

  handle_->setPopup(true);
  handle_->setPositionScheme(Absolute);

  if (orientation_ == Horizontal)
    resize(150, 50);
  else
    resize(50, 150);

  handle_->mouseWentDown().connect(mouseDownJS_);
  handle_->mouseMoved().connect(mouseMovedJS_);
  handle_->mouseWentUp().connect(mouseUpJS_);

  background_->clicked().connect(this, &WSlider::onSliderClick);
  sliderReleased_.connect(this, &WSlider::onSliderReleased);

  setLayoutSizeAware(true);

  update();
}

void WSlider::update()
{
  std::string resourcesURL = WApplication::resourcesUrl();

  background_->update();

  handle_->setStyleClass(std::string("handle-")
			 + (orientation_ == Horizontal ? 'h': 'v'));

  if (orientation_ == Horizontal) {
    handle_->resize(HANDLE_WIDTH, HANDLE_HEIGHT);
    handle_->setOffsets(h().toPixels() / 2 + 2, Top);
  } else {
    handle_->resize(HANDLE_HEIGHT, HANDLE_WIDTH);
    handle_->setOffsets(w().toPixels() / 2 - HANDLE_HEIGHT - 2, Left);
  }

  double l = (orientation_ == Horizontal ? w() : h()).toPixels();
  double pixelsPerUnit = (l - HANDLE_WIDTH) / range();

  std::string dir = (orientation_ == Horizontal ? "left" : "top");
  std::string u = (orientation_ == Horizontal ? "x" : "y");
  std::string U = (orientation_ == Horizontal ? "X" : "Y");
  std::string maxS = boost::lexical_cast<std::string>(l - HANDLE_WIDTH);
  std::string ppU = boost::lexical_cast<std::string>(pixelsPerUnit);
  std::string minimumS = boost::lexical_cast<std::string>(minimum_);
  std::string maximumS = boost::lexical_cast<std::string>(maximum_);

  /*
   * Note: cancelling the mouseDown event prevents the selection behaviour
   */
  mouseDownJS_.setJavaScript
    ("function(obj, event) {"
     """obj.setAttribute('down', " WT_CLASS ".widgetCoordinates(obj, event)."
     + u + "); "
        WT_CLASS ".cancelEvent(event);"
     "}");

  // = 'u' position relative to background, corrected for slider
  std::string computeD =
    ""  "var objh = " + handle_->jsRef() + ","
    ""      "objb = " + background_->jsRef() + ","
    ""      "u = WT.pageCoordinates(event)." + u + " - down,"
    ""      "w = WT.widgetPageCoordinates(objb)." + u + ","
    ""      "d = u-w;";

  mouseMovedJS_.setJavaScript
    ("function(obj, event) {"
     """var down = obj.getAttribute('down');"
     """var WT = " WT_CLASS ";"
     """if (down != null && down != '') {"
     + computeD +
     ""  "d = Math.max(0, Math.min(d, " + maxS + "));"
     ""  "var v = Math.round(d/" + ppU + ");"
     ""  "var intd = v*" + ppU + ";"
     ""  "if (Math.abs(WT.pxself(objh, '" + dir + "') - intd) > 1) {"
     ""    "objh.style." + dir + " = intd + 'px';" +
     sliderMoved_.createCall(orientation_ == Horizontal ?
			     "v + " + minimumS
			     : maximumS + " - v") + 
     ""  "}"
     """}"
     "}");

  mouseUpJS_.setJavaScript
    ("function(obj, event) {"
     """var down = obj.getAttribute('down');"
     """var WT = " WT_CLASS ";"
     """if (down != null && down != '') {"
     + computeD +
     """d += " + boost::lexical_cast<std::string>(HANDLE_WIDTH / 2) + ";" +
     sliderReleased_.createCall("d") + 
     ""  "obj.removeAttribute('down');"
     """}"
     "}");

  updateSliderPosition();
}

void WSlider::onSliderClick(const WMouseEvent& event)
{
  onSliderReleased(orientation_ == Horizontal
		   ? event.widget().x : event.widget().y);
}

void WSlider::onSliderReleased(int u)
{
  if (orientation_ == Horizontal)
    u -= HANDLE_WIDTH / 2;
  else
    u = (int)h().toPixels() - (u + HANDLE_WIDTH / 2);

  double l = (orientation_ == Horizontal ? w() : h()).toPixels();
  double pixelsPerUnit = (l - HANDLE_WIDTH) / range();

  double v = std::max(minimum_,
		      std::min(maximum_,
			       minimum_ + (int)((double)u / pixelsPerUnit
						+ 0.5)));

  sliderMoved_.emit(static_cast<int>(v));

  setValue(static_cast<int>(v));
  valueChanged_.emit(value());  
}

void WSlider::updateSliderPosition()
{
  double l = (orientation_ == Horizontal ? w() : h()).toPixels();
  double pixelsPerUnit = (l - HANDLE_WIDTH) / range();

  double u = ((double)value_ - minimum_) * pixelsPerUnit;

  if (orientation_ == Horizontal)
    handle_->setOffsets(u, Left);
  else
    handle_->setOffsets(h().toPixels() - HANDLE_WIDTH - u, Top);
}

void WSlider::setOrientation(Orientation orientation)
{
  orientation_ = orientation;

  update();
}

void WSlider::setTickPosition(WFlags<TickPosition> tickPosition)
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

void WSlider::signalConnectionsChanged()
{
  WCompositeWidget::signalConnectionsChanged();

  update();
}

}
