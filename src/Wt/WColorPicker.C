/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WColor.h>
#include <Wt/WColorPicker.h>
#include <Wt/WDllDefs.h>
#include <Wt/WFormWidget.h>
#include <Wt/WObject.h>
#include <Wt/WSignal.h>
#include <Wt/WString.h>
#include <Wt/WWebWidget.h>

#include "ColorUtils.h"
#include "DomElement.h"
#include "WebUtils.h"

#include <string>

namespace Wt {

const char *WColorPicker::INPUT_SIGNAL = "input";

WColorPicker::WColorPicker()
  : color_(StandardColor::Black),
    colorChanged_(false)
{
  setInline(true);
  setFormObject(true);
}

WColorPicker::WColorPicker(const WColor& color)
  : color_(color),
    colorChanged_(false)
{
  setInline(true);
  setFormObject(true);
}

WColor WColorPicker::color() const
{
  return color_;
}

void WColorPicker::setColor(const WColor& value)
{
  if (value != color_) {
    color_ = value;
    colorChanged_ = true;
    repaint();
  }
}

EventSignal<>& WColorPicker::colorInput()
{
  return *voidEventSignal(INPUT_SIGNAL, true);
}

void WColorPicker::updateDom(DomElement& element, bool all)
{
  if (all) {
    element.setAttribute("type", "color");
  }

  if (colorChanged_ || all) {
    element.setProperty(Property::Value, Color::colorToHex(color_));
    colorChanged_ = false;
  }

  WFormWidget::updateDom(element, all);
}

DomElementType WColorPicker::domElementType() const
{
  return DomElementType::INPUT;
}

void WColorPicker::propagateRenderOk(bool deep)
{
  colorChanged_ = false;
}

void WColorPicker::setFormData(const FormData& formData)
{
  if (colorChanged_ || isReadOnly())
    return;

  if (!Utils::isEmpty(formData.values)) {
    const std::string& value = formData.values[0];
    color_ = WColor(value);
  }
}

WT_USTRING WColorPicker::valueText() const
{
  return color().cssText();
}

void WColorPicker::setValueText(const WT_USTRING& value)
{
  setColor(WColor(value));
}

}
