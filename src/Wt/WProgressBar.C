/*
 * Copyright (C) 2010 Thomas Suckow.
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 *   progressCompleted() and valueChanged() contributed by Omer Katz.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WProgressBar>

#include "DomElement.h"
#include "WebUtils.h"

using namespace Wt;

namespace Wt {

WProgressBar::WProgressBar(WContainerWidget *parent)
   : WInteractWidget(parent),
     min_(0),
     max_(100),
     value_(0),
     changed_(false)
{
  format_ = WString::fromUTF8("%.0f %%");

  setStyleClass("Wt-progressbar");
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
  repaint(RepaintInnerHtml);
}

void WProgressBar::setMinimum(double minimum)
{
  min_ = minimum;

  changed_ = true;
  repaint(RepaintInnerHtml);
}

void WProgressBar::setMaximum(double maximum)
{
  max_ = maximum;

  changed_ = true;
  repaint(RepaintInnerHtml);
}

void WProgressBar::setRange(double minimum, double maximum)
{
  min_ = minimum;
  max_ = maximum;

  changed_ = true;
  repaint(RepaintInnerHtml);
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
  return DomElement_DIV; // later support DomElement_PROGRESS
}

void WProgressBar::resize(const WLength& width, const WLength& height)
{
  WInteractWidget::resize(width, height);

  if (!height.isAuto())
    setAttributeValue("style", "line-height: " + height.cssText());
}

void WProgressBar::updateDom(DomElement& element, bool all)
{
  DomElement *bar = 0, *label = 0;

  if (all) {
    bar = DomElement::createNew(DomElement_DIV);
    bar->setId("bar" + id());
    bar->setProperty(PropertyClass, "Wt-pgb-bar " + valueStyleClass_);

    label = DomElement::createNew(DomElement_DIV);
    label->setId("lbl" + id());
    label->setProperty(PropertyClass, "Wt-pgb-label");
  }

  if (changed_ || all) {
    if (!bar)
      bar = DomElement::getForUpdate("bar" + id(), DomElement_DIV);
    if (!label)
      label = DomElement::getForUpdate("lbl" + id(), DomElement_DIV);

    bar->setProperty(PropertyStyleWidth,
		     boost::lexical_cast<std::string>(percentage()) + "%");

    WString s = text();
    removeScript(s);

    label->setProperty(PropertyInnerHTML, s.toUTF8());

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

