/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WSpinBox>
#include <Wt/WIntValidator>
#include <Wt/WLocale>

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

WSpinBox::WSpinBox(WContainerWidget *parent)
  : WAbstractSpinBox(parent),
    value_(-1),
    min_(0),
    max_(99),
    step_(1),
    valueChanged_(this)
{ 
  setValidator(createValidator());
  setValue(0);
}

void WSpinBox::setValue(int value)
{
  if (value_ != value) {
    value_ = value;
    setText(textFromValue());
  }
}

void WSpinBox::setMinimum(int minimum)
{
  min_ = minimum;

  WIntValidator *v = dynamic_cast<WIntValidator *>(validator());
  if (v)
    v->setBottom(min_);

  changed_ = true;
  repaint();
}

void WSpinBox::setMaximum(int maximum)
{
  max_ = maximum;

  WIntValidator *v = dynamic_cast<WIntValidator *>(validator());
  if (v)
    v->setTop(max_);

  changed_ = true;
  repaint();
}

void WSpinBox::setRange(int minimum, int maximum)
{
  setMinimum(minimum);
  setMaximum(maximum);
}

void WSpinBox::setSingleStep(int step)
{
  step_ = step;

  changed_ = true;
  repaint();
}

int WSpinBox::decimals() const
{
  return 0;
}

std::string WSpinBox::jsMinMaxStep() const 
{
  return boost::lexical_cast<std::string>(min_) + ","
    + boost::lexical_cast<std::string>(max_) + ","
    + boost::lexical_cast<std::string>(step_);
}

void WSpinBox::updateDom(DomElement& element, bool all)
{
  if (all || changed_) {
    if (nativeControl()) {
      element.setAttribute("min", boost::lexical_cast<std::string>(min_));
      element.setAttribute("max", boost::lexical_cast<std::string>(max_));
      element.setAttribute("step", boost::lexical_cast<std::string>(step_));
    } else {
      /* Make sure the JavaScript validator is loaded */
      WIntValidator v;
      v.javaScriptValidate();
    }
  }

  WAbstractSpinBox::updateDom(element, all);
}

void WSpinBox::signalConnectionsChanged()
{
  if (valueChanged_.isConnected() && !valueChangedConnection_) {
    valueChangedConnection_ = true;
    changed().connect(this, &WSpinBox::onChange);
  }

  WAbstractSpinBox::signalConnectionsChanged();
}

void WSpinBox::onChange()
{
  valueChanged_.emit(value());
}

WValidator *WSpinBox::createValidator()
{
  WIntValidator *validator = new WIntValidator();
  validator->setMandatory(true);
  validator->setRange(min_, max_);
  return validator;
}

WT_USTRING WSpinBox::textFromValue() const
{
  if (nativeControl())    
    return WLocale::currentLocale().toString(value_);
  else {
    std::string text = prefix().toUTF8()
      + WLocale::currentLocale().toString(value_).toUTF8()
      + suffix().toUTF8();

    return WT_USTRING::fromUTF8(text);
  }
}

bool WSpinBox::parseNumberValue(const std::string& text)
{
  try {
    value_ = WLocale::currentLocale().toInt(WT_USTRING::fromUTF8(text));
    return true;
  } catch (boost::bad_lexical_cast &e) {
    return false;
  }
}

WValidator::Result WSpinBox::validateRange() const
{
  WIntValidator validator;
  validator.setRange(min_, max_);
  std::string badRangeText = WString::tr("Wt.WIntValidator.BadRange").toUTF8();
  Wt::Utils::replace(badRangeText, "{1}", "{1}" + suffix().toUTF8());
  Wt::Utils::replace(badRangeText, "{2}", "{2}" + suffix().toUTF8());
  validator.setInvalidTooLargeText(WString(badRangeText));
  validator.setInvalidTooSmallText(WString(badRangeText));
  return validator.validate(WString("{1}").arg(value_));
}

}
