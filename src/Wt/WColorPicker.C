#include <Wt/WColor.h>
#include <Wt/WColorPicker.h>
#include <Wt/WDllDefs.h>
#include <Wt/WFormWidget.h>
#include <Wt/WObject.h>
#include <Wt/WSignal.h>
#include <Wt/WString.h>
#include <Wt/WWebWidget.h>

#include <web/WebUtils.h>

#include <string>

namespace Wt {

LOGGER("WColorPicker");

WColorPicker::WColorPicker()
{
  setInline(true);
  setFormObject(true);
  setAttributeValue("type", "color");
}

WColorPicker::WColorPicker(const WColor& color)
{
  setInline(true);
  setFormObject(true);
  setAttributeValue("type", "color");
  setColor(color);
}

WColor WColorPicker::color() const
{
  return color_;
}

void WColorPicker::setColor(const WColor& value)
{
  color_ = value;
  doJavaScript(jsRef() + ".value = " + WWebWidget::jsStringLiteral(value.cssText()) + ";");
}

EventSignal<>& WColorPicker::colorInput()
{
  return *voidEventSignal(INPUT_SIGNAL, true);
}

DomElementType WColorPicker::domElementType() const
{
  return DomElementType::INPUT;
}

void WColorPicker::setFormData(const FormData& formData)
{
  if (isReadOnly())
    return;

  if (!Utils::isEmpty(formData.values))
  {
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
