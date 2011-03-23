/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include <Wt/WDoubleSpinBox>
#include <Wt/WDoubleValidator>

#include "DomElement.h"
#include "Utils.h"

namespace Wt {

WDoubleSpinBox::WDoubleSpinBox(WContainerWidget *parent)
  : WAbstractSpinBox(parent),
    value_(-1),
    min_(0.0),
    max_(99.99),
    step_(1.0),
    precision_(2)
{ 
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
  repaint(RepaintInnerHtml);
}

void WDoubleSpinBox::setMaximum(double maximum)
{
  max_ = maximum;

  changed_ = true;
  repaint(RepaintInnerHtml);
}

void WDoubleSpinBox::setRange(double minimum, double maximum)
{
  min_ = minimum;
  max_ = maximum;

  changed_ = true;
  repaint(RepaintInnerHtml);
}

void WDoubleSpinBox::setSingleStep(double step)
{
  step_ = step;

  changed_ = true;
  repaint(RepaintInnerHtml);
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
  char buf[30];

  std::string result = Utils::round_str(value_, precision_, buf);

  if (!nativeControl())
    result = prefix().toUTF8() + result + suffix().toUTF8();

  return WString::fromUTF8(result);
}

bool WDoubleSpinBox::parseNumberValue(const std::string& text)
{
  try {
    char buf[30];

    std::string currentV = Utils::round_str(value_, precision_, buf);

    if (currentV != text) // to prevent loss of precision
      value_ = boost::lexical_cast<double>(text);

    return true;
  } catch (boost::bad_lexical_cast &e) {
    return false;
  }
}

}
