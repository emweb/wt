/*
 * Copyright (C) 2008, 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WPaintedWidget"
#include "Wt/WPainter"
#include "Wt/WSlider"

#include "DomElement.h"
#include "Utils.h"

namespace Wt {

const Wt::WFlags<WSlider::TickPosition> WSlider::NoTicks = 0;
const Wt::WFlags<WSlider::TickPosition> WSlider::TicksBothSides
  = TicksAbove | TicksBelow;

class PaintedSlider : public WPaintedWidget
{
public:
  PaintedSlider(WSlider *slider);

  void updateState();
  void updateSliderPosition();
  void doUpdateDom(DomElement& element, bool all);

  void sliderResized(const WLength& width, const WLength& height);

protected:
  void paintEvent(WPaintDevice *paintDevice);

private:
  WSlider *slider_;

  JSignal<int> sliderReleased_;
  JSlot mouseDownJS_, mouseMovedJS_, mouseUpJS_;

  WContainerWidget *handle_;

  int range() const { return slider_->maximum() - slider_->minimum(); }
  double w() const;
  double h() const;

  void onSliderClick(const WMouseEvent& event);
  void onSliderReleased(int u);

  static const int HANDLE_WIDTH = 20;
};

void PaintedSlider::paintEvent(WPaintDevice *paintDevice)
{
  if (slider_->tickPosition()) {
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

    int tickInterval = slider_->tickInterval();
    int r = range();

    if (tickInterval == 0)
      tickInterval = r / 2;

    double tickStep = ((double)w - (HANDLE_WIDTH - 10)) / (r / tickInterval);

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
      int x = (HANDLE_WIDTH - 10)/2 + (int) (i * tickStep);

      if (x > w - (HANDLE_WIDTH - 10)/2)
	break;

      if (slider_->tickPosition() & WSlider::TicksAbove)
	painter.drawLine(x + 0.5, y1, x + 0.5, y2);
      if (slider_->tickPosition() & WSlider::TicksBelow)
	painter.drawLine(x + 0.5, y3, x + 0.5, y4);
    }
  }
}

PaintedSlider::PaintedSlider(WSlider *slider)
  : WPaintedWidget(),
    slider_(slider),
    sliderReleased_(this, "released")
{
  setStyleClass("Wt-slider-bg");

  slider_->setStyleClass(std::string("Wt-slider-")
			 + (slider_->orientation() == Horizontal ? "h" : "v"));
  slider_->setPositionScheme(Relative);

  addChild(handle_ = new WContainerWidget());
  handle_->setPopup(true);
  handle_->setPositionScheme(Absolute);

  handle_->mouseWentDown().connect(mouseDownJS_);
  handle_->mouseMoved().connect(mouseMovedJS_);
  handle_->mouseWentUp().connect(mouseUpJS_);

  slider->clicked().connect(this, &PaintedSlider::onSliderClick);
  sliderReleased_.connect(this, &PaintedSlider::onSliderReleased);
}

double PaintedSlider::w() const
{
  return width().toPixels() + (slider_->orientation() == Horizontal ? 10 : 0);
}

double PaintedSlider::h() const
{
  return height().toPixels() + (slider_->orientation() == Vertical ? 10 : 0);
}

void PaintedSlider::updateState()
{
  std::string resourcesURL = WApplication::resourcesUrl();

  Orientation o = slider_->orientation();

  handle_->setStyleClass("handle");

  if (o == Horizontal) {
    handle_->resize(HANDLE_WIDTH, h());
    handle_->setOffsets(0, Top);
  } else {
    handle_->resize(w(), HANDLE_WIDTH);
    handle_->setOffsets(0, Left);
  }

  double l = o == Horizontal ? w() : h();
  double pixelsPerUnit = (l - HANDLE_WIDTH) / range();

  std::string dir = (o == Horizontal ? "left" : "top");
  std::string u = (o == Horizontal ? "x" : "y");
  std::string U = (o == Horizontal ? "X" : "Y");
  std::string maxS = boost::lexical_cast<std::string>(l - HANDLE_WIDTH);
  std::string ppU = boost::lexical_cast<std::string>(pixelsPerUnit);
  std::string minimumS = boost::lexical_cast<std::string>(slider_->minimum());
  std::string maximumS = boost::lexical_cast<std::string>(slider_->maximum());

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
    ""      "objb = " + jsRef() + ","
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
     slider_->sliderMoved().createCall(o == Horizontal ? "v + " + minimumS
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

  update();
  updateSliderPosition();
}

void PaintedSlider::doUpdateDom(DomElement& element, bool all)
{
  if (all) {
    WApplication *app = WApplication::instance();

    element.addChild(createSDomElement(app));
    element.addChild(((WWebWidget *)handle_)->createSDomElement(app));

    DomElement *west = DomElement::createNew(DomElement_DIV);
    west->setProperty(PropertyClass, "Wt-w");
    element.addChild(west);

    DomElement *east = DomElement::createNew(DomElement_DIV);
    east->setProperty(PropertyClass, "Wt-e");
    element.addChild(east);
  }
}

void PaintedSlider::sliderResized(const WLength& width, const WLength& height)
{
  if (slider_->orientation() == Horizontal) {
    WLength w = width;
    if (!w.isAuto())
      w = WLength(w.toPixels() - 10);

    resize(w, height);
  } else {
    WLength h = height;
    if (!h.isAuto())
      h = WLength(h.toPixels() - 10);

    resize(width, h);    
  }

  updateState();
}
 
void PaintedSlider::onSliderClick(const WMouseEvent& event)
{
  onSliderReleased(slider_->orientation() == Horizontal
		   ? event.widget().x : event.widget().y);
}

void PaintedSlider::onSliderReleased(int u)
{
  if (slider_->orientation() == Horizontal)
    u -= HANDLE_WIDTH / 2;
  else
    u = (int)h() - (u + HANDLE_WIDTH / 2);

  double l = (slider_->orientation() == Horizontal) ? w() : h();

  double pixelsPerUnit = (l - HANDLE_WIDTH) / range();

  double v = std::max(slider_->minimum(),
		      std::min(slider_->maximum(),
			       slider_->minimum() 
			       + (int)((double)u / pixelsPerUnit + 0.5)));

  // TODO changed() ?
  slider_->sliderMoved().emit(static_cast<int>(v));

  slider_->setValue(static_cast<int>(v));
  slider_->valueChanged().emit(slider_->value());  

  updateSliderPosition();
}

void PaintedSlider::updateSliderPosition()
{
  double l = (slider_->orientation() == Horizontal) ? w() : h();
  double pixelsPerUnit = (l - HANDLE_WIDTH) / range();

  double u = ((double)slider_->value() - slider_->minimum()) * pixelsPerUnit;

  if (slider_->orientation() == Horizontal)
    handle_->setOffsets(u, Left);
  else
    handle_->setOffsets(h() - HANDLE_WIDTH - u, Top);
}

WSlider::WSlider(WContainerWidget *parent)
  : WFormWidget(parent),
    orientation_(Horizontal),
    tickInterval_(0),
    tickPosition_(0),
    preferNative_(false),
    changed_(false),
    changedConnected_(false),
    minimum_(0),
    maximum_(99),
    value_(0),
    valueChanged_(this),
    sliderMoved_(this, "moved"),
    paintedSlider_(0)
{ 
  resize(150, 50);
}

WSlider::WSlider(Orientation orientation, WContainerWidget *parent)
  : WFormWidget(parent),
    orientation_(orientation),
    tickInterval_(0),
    tickPosition_(0),
    preferNative_(false),
    changed_(false),
    changedConnected_(false),
    minimum_(0),
    maximum_(99),
    value_(0),
    valueChanged_(this),
    sliderMoved_(this, "moved"),
    paintedSlider_(0)
{ 
  if (orientation == Horizontal)
    resize(150, 50);
  else
    resize(50, 150);
}

WSlider::~WSlider()
{ }

void WSlider::setNativeControl(bool nativeControl)
{
  preferNative_ = nativeControl;
}

bool WSlider::nativeControl() const
{
  if (preferNative_) {
    const WEnvironment& env = WApplication::instance()->environment();
    if ((env.agentIsChrome() && env.agent() >= WEnvironment::Chrome5)
	|| (env.agentIsSafari() && env.agent() >= WEnvironment::Safari4)
	|| (env.agentIsOpera() && env.agent() >= WEnvironment::Opera10))
      return true;
  }

  return false;
}

void WSlider::resize(const WLength& width, const WLength& height)
{
  WFormWidget::resize(width, height);

  if (paintedSlider_)
    paintedSlider_->sliderResized(width, height);
}

void WSlider::layoutSizeChanged(int width, int height)
{
  WFormWidget::resize(WLength::Auto, WLength::Auto);

  if (paintedSlider_)
    paintedSlider_->sliderResized(width, height);
}

void WSlider::setOrientation(Orientation orientation)
{
  orientation_ = orientation;

  if (paintedSlider_)
    paintedSlider_->updateState();
}

void WSlider::setTickPosition(WFlags<TickPosition> tickPosition)
{
  tickPosition_ = tickPosition;

  if (paintedSlider_)
    paintedSlider_->updateState();
}

void WSlider::setTickInterval(int tickInterval)
{
  tickInterval_ = tickInterval;

  if (paintedSlider_)
    paintedSlider_->updateState();
}

void WSlider::update()
{
  if (paintedSlider_)
    paintedSlider_->updateState();
  else {
    changed_ = true;
    repaint();
  }
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

  update();
}

void WSlider::signalConnectionsChanged()
{
  WFormWidget::signalConnectionsChanged();

  update();
}

void WSlider::onChange()
{
  valueChanged_.emit(value_);
  sliderMoved_.emit(value_);
}

DomElementType WSlider::domElementType() const
{
  return paintedSlider_ ? DomElement_DIV : DomElement_INPUT;
}

void WSlider::render(WFlags<RenderFlag> flags)
{
  /*
   * In theory we are a bit late here to decide what we want to become:
   * somebody could already have asked the domElementType()
   */
  if (flags & RenderFull) {
    bool useNative = nativeControl();

    if (!useNative) {
      if (!paintedSlider_) {
	addChild(paintedSlider_ = new PaintedSlider(this));
	paintedSlider_->sliderResized(width(), height());
      }
    } else {
      delete paintedSlider_;
      paintedSlider_ = 0;
    }

    setLayoutSizeAware(!useNative);
    setFormObject(useNative);
  }

  WFormWidget::render(flags);
}

void WSlider::updateDom(DomElement& element, bool all)
{
  if (paintedSlider_)
    paintedSlider_->doUpdateDom(element, all);
  else {
    if (all || changed_) {
      element.setAttribute("type", "range");
      element.setProperty(Wt::PropertyValue,
			  boost::lexical_cast<std::string>(value_));
      element.setAttribute("min",
			   boost::lexical_cast<std::string>(minimum_));
      element.setAttribute("max",
			   boost::lexical_cast<std::string>(maximum_));

      if (!changedConnected_
	  && (valueChanged_.isConnected() || sliderMoved_.isConnected())) {
	changedConnected_ = true;
	changed().connect(this, &WSlider::onChange);
      }

      changed_ = false;
    }
  }

  WFormWidget::updateDom(element, all);
}

void WSlider::setFormData(const FormData& formData)
{
  // if the value was updated through the API, then ignore the update from
  // the browser, this happens when an action generated multiple events,
  // and we do not want to revert the changes made through the API
  if (changed_)
    return;

  if (!Utils::isEmpty(formData.values)) {
    const std::string& value = formData.values[0];
    try {
      value_ = boost::lexical_cast<int>(value);
    } catch (boost::bad_lexical_cast& e) { }
  }
}

}
