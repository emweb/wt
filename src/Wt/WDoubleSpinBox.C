/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WDoubleSpinBox"
#include "Wt/WDoubleValidator"
#include "Wt/WLocale"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

WDoubleSpinBox::WDoubleSpinBox(WContainerWidget *parent)
  : WAbstractSpinBox(parent),
    value_(-1),
    min_(0.0),
    max_(99.99),
    step_(1.0),
    precision_(2),
    valueChanged_(this)
{ 
  setValidator(createValidator());
  setValue(0.0);
}

void WDoubleSpinBox::setValue(double value)
{
  if (value_ != value) {
    value_ = value;
    setText(textFromValue());
  }
}

void WDoubleSpinBox::setMinimum(double minimum)
{
  min_ = minimum;

  changed_ = true;
  repaint();
}

void WDoubleSpinBox::setMaximum(double maximum)
{
  max_ = maximum;

  changed_ = true;
  repaint();
}

void WDoubleSpinBox::setRange(double minimum, double maximum)
{
  min_ = minimum;
  max_ = maximum;

  changed_ = true;
  repaint();
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
  return boost::lexical_cast<std::string>(min_) + ","
    + boost::lexical_cast<std::string>(max_) + ","
    + boost::lexical_cast<std::string>(step_);
}

void WDoubleSpinBox::updateDom(DomElement& element, bool all)
{
  if (all || changed_) {
    if (nativeControl()) {
      element.setAttribute("min", boost::lexical_cast<std::string>(min_));
      element.setAttribute("max", boost::lexical_cast<std::string>(max_));
      element.setAttribute("step", boost::lexical_cast<std::string>(step_));
    } else {
      /* Make sure the JavaScript validator is loaded */
      WDoubleValidator v;
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

WValidator *WDoubleSpinBox::createValidator()
{
  WDoubleValidator *validator = new WDoubleValidator();
  validator->setRange(min_, max_);
  return validator;
}

WString WDoubleSpinBox::textFromValue() const
{
  // FIXME, need to use WLocale here somehow !!

#ifndef WT_TARGET_JAVA
  // can't use round_str, because (1) precision is only a hint, and
  // (2) precision is limited to values < 7
  std::stringstream ss;
  ss.precision(precision_);
  ss << std::fixed << std::showpoint << value_;

  std::string result = ss.str();
#else
  char buf[30];

  std::string result = Utils::round_js_str(value_, precision_, buf);
#endif // WT_TARGET_JAVA

  if (!nativeControl())
    result = prefix().toUTF8() + result + suffix().toUTF8();

  return WString::fromUTF8(result);
}

bool WDoubleSpinBox::parseNumberValue(const std::string& text)
{
  try {
    char buf[30];

    // ??
    std::string currentV = Utils::round_css_str(value_, precision_, buf);

    if (currentV != text) // to prevent loss of precision
      value_ = WLocale::currentLocale().toDouble(text);

    return true;
  } catch (boost::bad_lexical_cast &e) {
    return false;
  }
}

WValidator::Result WDoubleSpinBox::validateRange() const
{
  WDoubleValidator validator;
  validator.setRange(min_, max_);
  return validator.validate(WString("{1}").arg(value_));
}

}
