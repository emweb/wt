#ifndef WCOLORPICKER_H_
#define WCOLORPICKER_H_

#include <Wt/WColor.h>
#include <Wt/WFormWidget.h>

class WColorPicker : public Wt::WFormWidget
{
public:
    WColorPicker();
    WColorPicker(const Wt::WColor& color);

    Wt::WColor value() const;
    void setValue(const Wt::WColor& value);

    Wt::EventSignal<>& colorInput();

    virtual WT_USTRING valueText() const override;
    virtual void setValueText(const WT_USTRING& value) override;

private:
    Wt::WColor color_;

    static constexpr const char* INPUT_SIGNAL{"input"};

protected:
    virtual Wt::DomElementType domElementType() const override;
    virtual void setFormData(const FormData& formData) override;
};

#endif  // WCOLORPICKER_H_
