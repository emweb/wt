#ifndef WCOLORPICKER_H_
#define WCOLORPICKER_H_

#include <Wt/WColor.h>
#include <Wt/WFormWidget.h>

namespace Wt {

class WT_API WColorPicker : public WFormWidget
{
public:
    WColorPicker();
    WColorPicker(const WColor& color);

    WColor value() const;
    void setValue(const WColor& value);

    EventSignal<>& colorInput();

    virtual WT_USTRING valueText() const override;
    virtual void setValueText(const WT_USTRING& value) override;

private:
    WColor color_;
    static constexpr const char* INPUT_SIGNAL = "input";

protected:
    virtual DomElementType domElementType() const override;
    virtual void setFormData(const FormData& formData) override;
};

}

#endif  // WCOLORPICKER_H_
