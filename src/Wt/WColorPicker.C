#include <Wt/WColor.h>
#include <Wt/WColorPicker.h>
#include <Wt/WSignal.h>
#include <Wt/WString.h>
#include <web/WebUtils.h>

#include <string>

WColorPicker::WColorPicker()
{
    setInline(true);
    setFormObject(true);
    setAttributeValue("type", "color");
}

WColorPicker::WColorPicker(const Wt::WColor& color)
{
    setInline(true);
    setFormObject(true);
    setAttributeValue("type", "color");
    setValue(color);
}

Wt::WColor WColorPicker::value() const
{
    return color_;
}

void WColorPicker::setValue(const Wt::WColor& value)
{
    color_ = value;
    doJavaScript(jsRef() + ".value = " + WWebWidget::jsStringLiteral(value.cssText()) + ";");
}

Wt::EventSignal<>& WColorPicker::colorInput()
{
    return *voidEventSignal(INPUT_SIGNAL, true);
}

Wt::DomElementType WColorPicker::domElementType() const
{
    return Wt::DomElementType::INPUT;
}

void WColorPicker::setFormData(const FormData& formData)
{
    if (isReadOnly())
        return;

    if (!Wt::Utils::isEmpty(formData.values))
    {
        const std::string& value = formData.values[0];
        color_ = {value};
    }
}

WT_USTRING WColorPicker::valueText() const
{
    return value().cssText();
}

void WColorPicker::setValueText(const WT_USTRING& value)
{
    setValue({value});
}
