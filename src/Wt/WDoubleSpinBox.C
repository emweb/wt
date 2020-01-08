/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WDoubleSpinBox.h"
#include "Wt/WDoubleValidator.h"
#include "Wt/WLocale.h"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

WDoubleSpinBox::WDoubleSpinBox()
  : setup_(false),
    value_(-1),
    min_(0.0),
    max_(99.99),
    step_(1.0),
    precision_(2)
{
  setValidator(createValidator());
  setValue(0.0);
}

void WDoubleSpinBox::setValue(double value)
{
  if (value_ != value || text() != textFromValue()) {
    value_ = value;
    setText(textFromValue());
  }
}

void WDoubleSpinBox::setMinimum(double minimum)
{
  min_ = minimum;

  std::shared_ptr<WDoubleValidator> v
    = std::dynamic_pointer_cast<WDoubleValidator>(validator());
  if (v)
    v->setBottom(min_);

  changed_ = true;
  repaint();
}

void WDoubleSpinBox::setMaximum(double maximum)
{
  max_ = maximum;

  std::shared_ptr<WDoubleValidator> v
    = std::dynamic_pointer_cast<WDoubleValidator>(validator());
  if (v)
    v->setTop(max_);

  changed_ = true;
  repaint();
}

void WDoubleSpinBox::setRange(double minimum, double maximum)
{
  setMinimum(minimum);
  setMaximum(maximum);
}

void WDoubleSpinBox::setSingleStep(double step)
{
  step_ = step;

  changed_ = true;
  repaint();
}

int WDoubleSpinBox::decimals() const
{
  return precision_;
}

void WDoubleSpinBox::setDecimals(int decimals)
{
  precision_ = decimals;

  setText(textFromValue());
}

std::string WDoubleSpinBox::jsMinMaxStep() const
{
  return std::to_string(min_) + "," + std::to_string(max_) + ","
    + std::to_string(step_);
}

void WDoubleSpinBox::updateDom(DomElement& element, bool all)
{
  if (all || changed_) {
    if (nativeControl()) {
      element.setAttribute("min", std::to_string(min_));
      element.setAttribute("max", std::to_string(max_));
      element.setAttribute("step", std::to_string(step_));
    } else {
      /* Make sure the JavaScript validator is loaded */
      WDoubleValidator v ;
      v.javaScriptValidate();
    }
  }

  WAbstractSpinBox::updateDom(element, all);
}

void WDoubleSpinBox::signalConnectionsChanged()
{
  if (valueChanged_.isConnected() && !valueChangedConnection_) {
    valueChangedConnection_ = true;
    changed().connect(this, &WDoubleSpinBox::onChange);
  }

  WAbstractSpinBox::signalConnectionsChanged();
}

void WDoubleSpinBox::onChange()
{
  valueChanged_.emit(value());
}

std::unique_ptr<WValidator> WDoubleSpinBox::createValidator()
{
  std::unique_ptr<WDoubleValidator> validator(new WDoubleValidator());
  validator->setMandatory(true);
  validator->setRange(min_, max_);
#ifndef WT_TARGET_JAVA
  return std::move(validator); // FreeBSD wanted this std::move
#else // WT_TARGET_JAVA
  return validator;
#endif // WT_TARGET_JAVA
}

WT_USTRING WDoubleSpinBox::textFromValue() const
{
  std::string result = WLocale::currentLocale().toFixedString(value_, precision_).toUTF8();

  if (!nativeControl())
    result = prefix().toUTF8() + result + suffix().toUTF8();

  return WT_USTRING::fromUTF8(result);
}

bool WDoubleSpinBox::parseNumberValue(const std::string& text)
{
  try {
    if (textFromValue().toUTF8() != text) // to prevent loss of precision
      value_ = WLocale::currentLocale().toDouble(WT_USTRING::fromUTF8(text));

    return true;
  } catch (std::exception &e) {
    return false;
  }
}

void WDoubleSpinBox::refresh()
{
  setText(textFromValue());
  WAbstractSpinBox::refresh();
}

WValidator::Result WDoubleSpinBox::validateRange() const
{
  WDoubleValidator validator;
  validator.setRange(min_, max_);
  std::string badRangeText = WString::tr("Wt.WDoubleValidator.BadRange").toUTF8();
  Wt::Utils::replace(badRangeText, "{1}", "{1}" + suffix().toUTF8());
  Wt::Utils::replace(badRangeText, "{2}", "{2}" + suffix().toUTF8());
  validator.setInvalidTooLargeText(WString(badRangeText));
  validator.setInvalidTooSmallText(WString(badRangeText));
  return validator.validate(WString("{1}").arg(value_));
}

void WDoubleSpinBox::render(WFlags<RenderFlag> flags)
{
  WAbstractSpinBox::render(flags);

  if (!setup_ && flags.test(RenderFlag::Full))
    setup();
}

void WDoubleSpinBox::setup()
{
  setup_ = true;
  doJavaScript(jsRef() + ".wtObj"
      ".setIsDoubleSpinBox(true);");
}

}
