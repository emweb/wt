/*
 * Copyright (C) 2010 Thomas Suckow.
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 *   progressCompleted() and valueChanged() contributed by Omer Katz.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WProgressBar.h>
#include <Wt/WTheme.h>

#include "DomElement.h"
#include "WebUtils.h"

using namespace Wt;

namespace Wt {

WProgressBar::WProgressBar()
  : min_(0),
    max_(100),
    value_(0),
    changed_(false)
{
  format_ = WString::fromUTF8("%.0f %%");
  setInline(true);
}

void WProgressBar::setValueStyleClass(const std::string& valueStyleClass)
{
  valueStyleClass_ = valueStyleClass;
}

void WProgressBar::setValue(double value)
{
  value_ = value;

  valueChanged_.emit(value_);
  
  if (value_ == max_)
    progressCompleted_.emit();
  
  changed_ = true;
  repaint();
}

void WProgressBar::setMinimum(double minimum)
{
  min_ = minimum;

  changed_ = true;
  repaint();
}

void WProgressBar::setMaximum(double maximum)
{
  max_ = maximum;

  changed_ = true;
  repaint();
}

void WProgressBar::setRange(double minimum, double maximum)
{
  min_ = minimum;
  max_ = maximum;

  changed_ = true;
  repaint();
}

void WProgressBar::setState(double minimum, double maximum, double value)
{
  min_ = minimum;
  max_ = maximum;

  if (value_ != value) {
    value_ = value;

    if (value_ == max_)
      progressCompleted_.emit();
  }
}

void WProgressBar::setFormat(const WString& format)
{
  format_ = format;
}

WString WProgressBar::text() const
{
  return Utils::formatFloat(format_, percentage());
}

double WProgressBar::percentage() const
{
  double max = maximum(), min = minimum();

  if (max - min != 0)
    return (value() - min) * 100 / (max - min);
  else
    return 0;
}

DomElementType WProgressBar::domElementType() const
{
  return DomElementType::DIV; // later support DomElementType::PROGRESS
}

void WProgressBar::resize(const WLength& width, const WLength& height)
{
  WInteractWidget::resize(width, height);

  if (!height.isAuto())
    setAttributeValue("style", "line-height: " + height.cssText());
}

void WProgressBar::updateBar(DomElement& bar)
{
  bar.setProperty(Property::StyleWidth, std::to_string(percentage()) + "%");
}

void WProgressBar::updateDom(DomElement& element, bool all)
{
  DomElement *bar = nullptr, *label = nullptr;

  if (all) {
    WApplication *app = WApplication::instance();

    bar = DomElement::createNew(DomElementType::DIV);
    bar->setId("bar" + id());
    bar->setProperty(Property::Class, valueStyleClass_);
    app->theme()->apply(this, *bar, ProgressBarBar);

    label = DomElement::createNew(DomElementType::DIV);
    label->setId("lbl" + id());
    app->theme()->apply(this, *label, ProgressBarLabel);
  }

  if (changed_ || all) {
    if (!bar)
      bar = DomElement::getForUpdate("bar" + id(), DomElementType::DIV);
    if (!label)
      label = DomElement::getForUpdate("lbl" + id(), DomElementType::DIV);

    updateBar(*bar);

    WString s = text();
    removeScript(s);

    label->setProperty(Property::InnerHTML, s.toUTF8());

    changed_ = false;
  }

  if (bar)
    element.addChild(bar);

  if (label)
    element.addChild(label);

  WInteractWidget::updateDom(element, all);
}

void WProgressBar::propagateRenderOk(bool deep)
{
  changed_ = false;

  WInteractWidget::propagateRenderOk(deep);
}

}

