/*
 * Copyright (C) 2008, 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WBootstrap5Theme.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WCssTheme.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"
#include "Wt/WPaintDevice.h"
#include "Wt/WPaintedWidget.h"
#include "Wt/WPainter.h"
#include "Wt/WSlider.h"
#include "Wt/WStringStream.h"

#include "DomElement.h"
#include "WebUtils.h"

#include <cmath>
#include <memory>

/*
 * FIXME: move styling to the theme classes
 */
namespace Wt {
LOGGER("WSlider");

const Wt::WFlags<WSlider::TickPosition> WSlider::NoTicks = None;
const Wt::WFlags<WSlider::TickPosition> WSlider::TicksBothSides
  = WSlider::TickPosition::TicksAbove | WSlider::TickPosition::TicksBelow;

class TickList final : public WWebWidget
{
public:
  TickList(WSlider* slider)
    : slider_(slider)
  {
  }

  void doUpdateDom(DomElement& element, bool all)
  {
    if (all) {
      // Container for all the options
      DomElement *list = DomElement::createNew(domElementType());
      list->setId(element.id() + "dl");

      int tickInterval = slider_->tickInterval();
      int range = slider_->maximum() - slider_->minimum();
      if (range == 0) {
        return;
      }

      if (tickInterval == 0) {
        tickInterval = range / 2;
      }

      int numTicks =  range / tickInterval + 1;

      if (numTicks < 1) {
        return;
      }

      for (int i = 0; i < numTicks; ++i) {
        int value = slider_->minimum() + i * tickInterval;

        DomElement *option = DomElement::createNew(DomElementType::OPTION);
        option->setProperty(Property::Value, std::to_string(value));
        list->addChild(option);
      }
      element.addChild(list);
    }
  }

  DomElementType domElementType() const
  {
    return DomElementType::DATALIST;
  }

private:
  WSlider* slider_ = nullptr;
};

const char *WSlider::INPUT_SIGNAL = "input";

class PaintedSlider final : public WPaintedWidget
{
public:
  PaintedSlider(WSlider *slider);
  virtual ~PaintedSlider();

  void connectSlots();
  void updateState();
  void updateSliderPosition();
  void doUpdateDom(DomElement& element, bool all);

  void sliderResized(const WLength& width, const WLength& height);

protected:
  virtual void paintEvent(WPaintDevice *paintDevice) override;

private:
  WSlider *slider_;

  JSignal<int> sliderReleased_;
  JSlot mouseDownJS_, mouseMovedJS_, mouseUpJS_;

  std::unique_ptr<WInteractWidget> handle_, fill_;

  int range() const { return slider_->maximum() - slider_->minimum(); }
  double w() const;
  double h() const;

  void onSliderClick(const WMouseEvent& event);
  void onSliderReleased(int u);

  int getClosestNumberByStep(int value, int step);
};

void PaintedSlider::paintEvent(WPaintDevice *paintDevice)
{
  int tickInterval = slider_->tickInterval();
  int r = range();

  if (r == 0) {
    // Empty range, don't paint anything
    return;
  }

  if (tickInterval == 0)
    tickInterval = r / 2;

  int numTicks = tickInterval == 0 ? 2 : r / tickInterval + 1;
  if (numTicks < 1)
    return;

  int w = 0, h = 0;

  switch (slider_->orientation()) {
  case Orientation::Horizontal:
    w = (int)paintDevice->width().toPixels();
    h = (int)paintDevice->height().toPixels();
    break;
  case Orientation::Vertical:
    w = (int)paintDevice->height().toPixels();
    h = (int)paintDevice->width().toPixels();
  }

  double tickStep = ((double)w + 10 - slider_->handleWidth()) / (numTicks - 1);

  WPainter painter(paintDevice);

  for (int i = 0; i < numTicks; ++i) {
    int v = slider_->minimum() + i * tickInterval;
    int x = -5 + slider_->handleWidth()/2 + (int) (i * tickStep);

    switch (slider_->orientation()) {
    case Orientation::Horizontal:
      slider_->paintTick(painter, v, x, h/2);
      break;
    case Orientation::Vertical:
      slider_->paintTick(painter, v, h/2, w - x);
    }
  }
}

PaintedSlider::PaintedSlider(WSlider *slider)
  : WPaintedWidget(),
    slider_(slider),
    sliderReleased_(this, "released")
{
  setStyleClass("Wt-slider-bg");

  slider_->addStyleClass
    (std::string("Wt-slider-")
     + (slider_->orientation() == Orientation::Horizontal ? "h" : "v"));

  if (slider_->positionScheme() == PositionScheme::Static) {
    slider_->setPositionScheme(PositionScheme::Relative);
    slider_->setOffsets(0, Side::Left | Side::Top);
  }

  auto fill = std::unique_ptr<WInteractWidget>(new WContainerWidget());
  manageWidget(fill_, std::move(fill));
  auto handle = slider_->createHandle();
  manageWidget(handle_, std::move(handle));

  fill_->setPositionScheme(PositionScheme::Absolute);
  fill_->setStyleClass("fill");

  handle_->setPositionScheme(PositionScheme::Absolute);
  handle_->setStyleClass("handle");

  handle_->setCanReceiveFocus(true);
  slider_->setCanReceiveFocus(true);

  connectSlots();
}

void PaintedSlider::connectSlots()
{
  if (Wt::WApplication::instance()->environment().ajax()) {
    handle_->mouseWentDown().connect(mouseDownJS_);
    handle_->touchStarted().connect(mouseDownJS_);
    handle_->mouseMoved().connect(mouseMovedJS_);
    handle_->touchMoved().connect(mouseMovedJS_);
    handle_->mouseWentUp().connect(mouseUpJS_);
    handle_->touchEnded().connect(mouseUpJS_);

    slider_->clicked().connect(this, &PaintedSlider::onSliderClick);

    sliderReleased_.connect(this, &PaintedSlider::onSliderReleased);
  }
}

PaintedSlider::~PaintedSlider()
{
  manageWidget(fill_, std::unique_ptr<WInteractWidget>());
  manageWidget(handle_, std::unique_ptr<WInteractWidget>());
}

double PaintedSlider::w() const
{
  return width().toPixels()
    + (slider_->orientation() == Orientation::Horizontal ? 10 : 0);
}

double PaintedSlider::h() const
{
  return height().toPixels()
    + (slider_->orientation() == Orientation::Vertical ? 10 : 0);
}

void PaintedSlider::updateState()
{
  bool rtl = WApplication::instance()->layoutDirection() ==
    LayoutDirection::RightToLeft;

  Orientation o = slider_->orientation();

  // Have 5px margin for the handle VS the slider
  const int handleOffset = 5;
  const int widgetLength = o == Orientation::Horizontal ? static_cast<int>(h()) : static_cast<int>(w());
  int calculatedOffset = -(widgetLength / 2) + slider_->handleWidth() + handleOffset;
  auto theme = Wt::WApplication::instance()->theme();
  auto cssTheme = std::dynamic_pointer_cast<WCssTheme>(theme);
  if (cssTheme && cssTheme->name() == "polished") {
    calculatedOffset = 0;
  }

  if (o == Orientation::Horizontal) {
    handle_->resize(slider_->handleWidth(), h());
    handle_->setOffsets(calculatedOffset, Side::Top);
  } else {
    handle_->resize(w(), slider_->handleWidth());
    handle_->setOffsets(calculatedOffset, Side::Left);
  }

  double l = o == Orientation::Horizontal ? w() : h();
  double pixelsPerUnit = (l - slider_->handleWidth()) / range();

  std::string dir;
  std::string size;
  if (o == Orientation::Horizontal) {
    dir = rtl ? "right" : "left";
    size = "width";
  } else {
    dir = "top";
    size = "height";
  }

  char u = (o == Orientation::Horizontal ? 'x' : 'y');

  double max = l - slider_->handleWidth();
  bool horizontal = o == Orientation::Horizontal;

  char buf[30]; // Buffer for round_js_str

  /*
   * Note: cancelling the mouseDown event prevents the selection behaviour
   */
  WStringStream mouseDownJS;
  mouseDownJS << "obj.setAttribute('down', " WT_CLASS
              <<                     ".widgetCoordinates(obj, event)." << u
              <<                  ");";

  WStringStream computeD; // = 'u' position relative to background, corrected for slider
  computeD << "var objh = " << handle_->jsRef() << ","
           <<     "objf = " << fill_->jsRef() << ","
           <<     "objb = " << slider_->jsRef() << ","
           <<     "minVal = " << slider_->minimum() << ","
           <<     "maxVal = " << slider_->maximum() << ","
           <<     "stepVal = " << slider_->step() << ","
           <<     "page_u = WT.pageCoordinates(event)." << u << ","
           <<     "widget_page_u = WT.widgetPageCoordinates(objb)." << u << ","
           <<     "pos = page_u - widget_page_u,"
           <<     "rtl = " << rtl << ","
           <<     "horizontal = " << horizontal << ";"
           <<     "if (rtl && horizontal)";
  computeD <<       "pos = " << Utils::round_js_str(l, 3, buf) << " - pos;";
  computeD <<     "var d = pos - down;";
  // Get max d value and make D relative against the slider's value
  computeD <<     "let sliderV = Math.abs(maxVal - minVal);";
  computeD <<     "let scaleFactor = " << Utils::round_js_str(max, 3, buf) << " / sliderV;";
  computeD <<     "let scaledD = d / scaleFactor;";
  computeD <<     "let absD = Math.abs(scaledD);";
  computeD <<     "let signD = scaledD < 0 ? -1 : 1;";
  computeD <<     "let lowDelta = absD - (absD % stepVal);";
  computeD <<     "let highDelta = lowDelta + stepVal;";
  computeD <<     "if (absD- lowDelta < highDelta - absD) {";
  computeD <<       "d = lowDelta * signD;";
  computeD <<     "} else {";
  computeD <<       "d = highDelta *signD;";
  computeD <<     "}";
  computeD <<     "d = d * scaleFactor;";

  WStringStream mouseMovedJS;
  mouseMovedJS << "var down = obj.getAttribute('down');"
               << "var WT = " WT_CLASS ";"
               << "if (down != null && down != '') {"
               <<    computeD.str();
  mouseMovedJS <<   "d = Math.max(0, Math.min(d, " << Utils::round_js_str(max, 3, buf) << "));";
  mouseMovedJS <<   "var v = Math.round(d/" << Utils::round_js_str(pixelsPerUnit, 3, buf) << ");";
  mouseMovedJS <<   "var intd = v*" << Utils::round_js_str(pixelsPerUnit, 3, buf) << ";";
  mouseMovedJS <<   "if (Math.abs(WT.pxself(objh, '" << dir
               <<                 "') - intd) > 1) {"
               <<     "objf.style." << size << " = ";
  if (o == Orientation::Vertical) {
    mouseMovedJS << '(' << Utils::round_js_str(max, 3, buf);
    mouseMovedJS << " - intd + " << (slider_->handleWidth() / 2)
                 << ")";
  } else
    mouseMovedJS << "intd + " << (slider_->handleWidth() / 2);
  mouseMovedJS <<       " + 'px';"
               <<     "objh.style." << dir << " = intd + 'px';"
               <<     "var vs = ";
  if (o == Orientation::Horizontal)
    mouseMovedJS << "v + " << slider_->minimum();
  else
    mouseMovedJS << slider_->maximum() << " - v";
  mouseMovedJS <<     ";"
               <<     "var f = objb.onValueChange;"
               <<     "if (f) f(vs);";

  if (slider_->sliderMoved().needsUpdate(true)) {
#ifndef WT_TARGET_JAVA
    mouseMovedJS << slider_->sliderMoved().createCall({"vs"});
#else
    mouseMovedJS << slider_->sliderMoved().createCall("vs");
#endif
  }

  mouseMovedJS <<   "}"
               << "}";

  WStringStream mouseUpJS;
  mouseUpJS << "var down = obj.getAttribute('down');"
            << "var WT = " WT_CLASS ";"
            << "if (down != null && down != '') {"
            <<    computeD.str()
            <<   "d += " << (slider_->handleWidth() / 2) << ";"
#ifndef WT_TARGET_JAVA
            <<    sliderReleased_.createCall({"Math.round(d)"})
#else
            <<    sliderReleased_.createCall("Math.round(d)")
#endif
            <<   "obj.removeAttribute('down');"
            << "}";

  bool enabled = !slider_->isDisabled();

  mouseDownJS_.setJavaScript(std::string("function(obj, event) {")
                             + (enabled ? mouseDownJS.str() : "")
                             + "}");
  mouseMovedJS_.setJavaScript(std::string("function(obj, event) {")
                              + (enabled ? mouseMovedJS.str() : "")
                              + "}");
  mouseUpJS_.setJavaScript(std::string("function(obj, event) {")
                           + (enabled ? mouseUpJS.str() : "")
                           + "}");

  update();
  updateSliderPosition();
}

void PaintedSlider::doUpdateDom(DomElement& element, bool all)
{
  if (all) {
    WApplication *app = WApplication::instance();

    DomElement *west = DomElement::createNew(DomElementType::DIV);
    west->setProperty(Property::Class, "Wt-w");
    element.addChild(west);

    DomElement *east = DomElement::createNew(DomElementType::DIV);
    east->setProperty(Property::Class, "Wt-e");
    element.addChild(east);

    element.addChild(createSDomElement(app));
    element.addChild(fill_->createSDomElement(app));
    element.addChild(handle_->createSDomElement(app));
  }
}

void PaintedSlider::sliderResized(const WLength& width, const WLength& height)
{
  if (slider_->orientation() == Orientation::Horizontal) {
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
  int x = event.widget().x;
  int y = event.widget().y;

  if (WApplication::instance()->layoutDirection() ==
      LayoutDirection::RightToLeft)
    x = (int)(w() - x);

  onSliderReleased(slider_->orientation() == Orientation::Horizontal ? x : y);
}

void PaintedSlider::onSliderReleased(int u)
{
  if (slider_->orientation() == Orientation::Horizontal)
    u -= slider_->handleWidth() / 2;
  else
    u = (int)h() - (u + slider_->handleWidth() / 2);

  double l = (slider_->orientation() == Orientation::Horizontal) ? w() : h();

  double pixelsPerUnit = (l - slider_->handleWidth()) / range();

  double v = std::max(slider_->minimum(),
                      std::min(slider_->maximum(),
                               slider_->minimum()
                               + (int)((double)u / pixelsPerUnit + 0.5)));

  v = getClosestNumberByStep(static_cast<int>(v), slider_->step());

  // TODO changed() ?
  slider_->sliderMoved().emit(static_cast<int>(v));

  slider_->setValue(static_cast<int>(v));
  slider_->valueChanged().emit(slider_->value());

  updateSliderPosition();
}

void PaintedSlider::updateSliderPosition()
{
  double l = (slider_->orientation() == Orientation::Horizontal) ? w() : h();
  double pixelsPerUnit = (l - slider_->handleWidth()) / range();

  double u = ((double)slider_->value() - slider_->minimum()) * pixelsPerUnit;

  if (slider_->orientation() == Orientation::Horizontal) {
    handle_->setOffsets(u, Side::Left);
    fill_->setWidth(u + slider_->handleWidth() / 2);
  } else {
    handle_->setOffsets(h() - slider_->handleWidth() - u, Side::Top);
    fill_->setHeight(u + slider_->handleWidth() / 2);
  }
  handle_->setFocus(true);
}

int PaintedSlider::getClosestNumberByStep(int value, int step)
{
  int absValue = std::abs(value);
  int sign = value < 0 ? -1 : 1;
  int lowDelta = absValue - (absValue % step);
  int highDelta = lowDelta + step;

  if (absValue - lowDelta < highDelta - absValue) {
    return lowDelta * sign;
  } else {
    return highDelta * sign;
  }
}

WSlider::WSlider()
  : orientation_(Orientation::Horizontal),
    tickInterval_(0),
    tickPosition_(None),
    preferNative_(false),
    changed_(false),
    changedConnected_(false),
    handleWidth_(20),
    minimum_(0),
    maximum_(99),
    value_(0),
    step_(1),
    sliderMoved_(this, "moved", true)
{
  resize(150, 50);
}

WSlider::WSlider(Orientation orientation)
  : orientation_(orientation),
    tickInterval_(0),
    tickPosition_(None),
    preferNative_(false),
    changed_(false),
    changedConnected_(false),
    handleWidth_(20),
    minimum_(0),
    maximum_(99),
    value_(0),
    step_(1),
    sliderMoved_(this, "moved", true)
{
  if (orientation == Orientation::Horizontal)
    resize(150, 50);
  else
    resize(50, 150);
}

WSlider::~WSlider()
{
  manageWidget(paintedSlider_, std::unique_ptr<PaintedSlider>());
  manageWidget(tickList_, std::unique_ptr<TickList>());
}

EventSignal<>& WSlider::input()
{
  return *voidEventSignal(INPUT_SIGNAL, true);
}

void WSlider::enableAjax()
{
  if (paintedSlider_)
    paintedSlider_->connectSlots();
}

void WSlider::setNativeControl(bool nativeControl)
{
  preferNative_ = nativeControl;
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

  if (paintedSlider_) {
    paintedSlider_->sliderResized(width, height);
  }
}

void WSlider::setOrientation(Orientation orientation)
{
  orientation_ = orientation;

  if (paintedSlider_)
    paintedSlider_->updateState();
}

void WSlider::setTickPosition(WFlags<TickPosition> tickPosition)
{
  if (nativeControl()) {
    LOG_WARN("setTickLength(): Cannot set the tick length of a native widget.");
    return;
  }

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

void WSlider::setTickLength(const Wt::WLength& length)
{
  if (nativeControl()) {
    LOG_WARN("setTickLength(): Cannot set the tick length of a native widget.");
    return;
  }

  tickLength_ = length;

  if (paintedSlider_) {
    paintedSlider_->updateState();
  }
}

void WSlider::setHandleWidth(int handleWidth)
{
  handleWidth_ = handleWidth;
}

std::unique_ptr<WInteractWidget> WSlider::createHandle()
{
  return std::unique_ptr<WInteractWidget>(new WContainerWidget());
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

void WSlider::setStep(int step)
{
  if (step <= 0) {
    LOG_WARN("setStep() is called with a bad step value. This must be greater than 0.");
    return;
  }

  step_ = step;
  value_ = getClosestNumberByStep(value(), step);

  update();
  onChange();
}

void WSlider::setValue(int value)
{
  value_ = std::min(maximum_, std::max(minimum_, value));

  if (paintedSlider_)
    paintedSlider_->updateSliderPosition();
  else {
    update();
    onChange();
  }
}

void WSlider::signalConnectionsChanged()
{
  WFormWidget::signalConnectionsChanged();

  update();
}

void WSlider::onChange()
{
  updateSliderProperties();
  valueChanged_.emit(value_);
  sliderMoved_.emit(value_);
}

void WSlider::updateSliderProperties()
{
  if (preferNative_) {
    scheduleRender();
  }
}

DomElementType WSlider::domElementType() const
{
  return paintedSlider_ ? DomElementType::DIV : DomElementType::INPUT;
}

void WSlider::render(WFlags<RenderFlag> flags)
{
  /*
   * In theory we are a bit late here to decide what we want to become:
   * somebody could already have asked the domElementType()
   */
  if (flags.test(RenderFlag::Full)) {
    bool useNative = nativeControl();

    if (!useNative) {
      if (!paintedSlider_) {
        auto paintedSlider = std::make_unique<PaintedSlider>(this);
        manageWidget(paintedSlider_, std::move(paintedSlider));
        paintedSlider_->sliderResized(width(), height());
      }
    } else {
      auto tickList = std::make_unique<TickList>(this);
      manageWidget(tickList_, std::move(tickList));
    }

    setLayoutSizeAware(!useNative);
    setFormObject(useNative);
  }

  WFormWidget::render(flags);
}

void WSlider::updateDom(DomElement& element, bool all)
{
  if (preferNative_) {
    if (orientation() == Orientation::Horizontal) {
      element.removeProperty(Property::Orient);
      element.removeProperty(Property::StyleWebkitAppearance);
    } else {
      // Firefox
      element.setProperty(Property::Orient, "vertical");
      // Chrome/Safari
      element.setProperty(Property::StyleWebkitAppearance, "slider-vertical");
    }

    if (tickList_) {
      // Get parent of widget, since the native version is an `input`,
      // which is a type that cannot have children.
      tickList_->doUpdateDom(element, all);

      element.setAttribute("list", id() + "dl");
    }
    element.setAttribute("step", std::to_string(step()));
    element.setAttribute("value", std::to_string(value()));
  }

  if (paintedSlider_)
    paintedSlider_->doUpdateDom(element, all);
  else {
    if (all || changed_) {
      element.setAttribute("type", "range");
      element.setProperty(Wt::Property::Value, std::to_string(value_));
      element.setAttribute("min", std::to_string(minimum_));
      element.setAttribute("max", std::to_string(maximum_));

      if (!changedConnected_
          && (valueChanged_.isConnected() || sliderMoved_.isConnected())) {
        changedConnected_ = true;
        changed().connect(this, &WSlider::onChange);
      } else if (!inputConnected_ && (valueChanged_.isConnected() || sliderMoved_.isConnected())) {
        changedConnected_ = true;
        input().connect(this, &WSlider::onChange);
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
  if (changed_ || isReadOnly())
    return;

  if (!Utils::isEmpty(formData.values)) {
    const std::string& value = formData.values[0];
    try {
      value_ = Utils::stoi(value);
    } catch (std::exception& e) { }
  }
}

WT_USTRING WSlider::valueText() const
{
  return WT_USTRING::fromUTF8(std::to_string(value_));
}

void WSlider::setValueText(const WT_USTRING& value)
{
  try {
    value_ = Utils::stoi(value.toUTF8());
  } catch (std::exception& e) {
  }
}

void WSlider::setDisabled(bool disabled)
{
  if (paintedSlider_)
    paintedSlider_->setDisabled(disabled);

  WFormWidget::setDisabled(disabled);

  if (paintedSlider_)
    paintedSlider_->updateState();
}

void WSlider::paintTick(WPainter& painter, int value, int x, int y)
{
  if (!tickPosition_.empty()) {
    int h = 0;

    if (orientation_ == Orientation::Horizontal)
      h = (int)painter.device()->height().toPixels();
    else
      h = (int)painter.device()->width().toPixels();

    WPen pen;
    pen.setColor(WColor(0xd7, 0xd7, 0xd7));
    pen.setCapStyle(PenCapStyle::Flat);
    pen.setWidth(1);
    painter.setPen(pen);

    int y1 = h / 4;
    int y2 = h / 2 - 4;
    int y3 = h / 2 + 4;
    int y4 = h - h/4;

    // Apply tick length if not default
    if (!tickLength().isAuto()) {
      y1 = y2 - static_cast<int>(tickLength().toPixels());
      y4 = y3 + static_cast<int>(tickLength().toPixels());
    }

    switch (orientation_) {
    case Orientation::Horizontal:
      if (tickPosition_.test(WSlider::TickPosition::TicksAbove))
        painter.drawLine(x + 0.5, y1, x + 0.5, y2);
      if (tickPosition_.test(WSlider::TickPosition::TicksBelow))
        painter.drawLine(x + 0.5, y3, x + 0.5, y4);

      break;
    case Orientation::Vertical:
      if (tickPosition_.test(WSlider::TickPosition::TicksAbove))
        painter.drawLine(y1, y + 0.5, y2, y + 0.5);
      if (tickPosition_.test(WSlider::TickPosition::TicksBelow))
        painter.drawLine(y3, y + 0.5, y4, y + 0.5);
    }
  }
}

int WSlider::getClosestNumberByStep(int value, int step)
{
  int absValue = std::abs(value);
  int sign = value < 0 ? -1 : 1;
  int lowDelta = absValue - (absValue % step);
  int highDelta = lowDelta + step;

  if (absValue - lowDelta < highDelta - absValue) {
    return lowDelta * sign;
  } else {
    return highDelta * sign;
  }
}
}
