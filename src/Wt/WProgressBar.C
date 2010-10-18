/*
 * Copyright (C) 2010 Thomas Suckow.
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WProgressBar>
#include <Wt/WText>
#include <Wt/WContainerWidget>

#include "DomElement.h"

using namespace Wt;

namespace Wt {

WProgressBar::WProgressBar(WContainerWidget *parent)
   : WInteractWidget(parent),
     min_(0),
     max_(100),
     value_(0),
     changed_(false)
{
  setStyleClass("Wt-progressbar");
  setInline(true);
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

WString WProgressBar::text() const
{
  return WString::fromUTF8
    (boost::lexical_cast<std::string>(static_cast<int>(percentage())));
}

double WProgressBar::percentage() const
{
  return (value() - minimum()) * 100 / (maximum() - minimum());
}

DomElementType WProgressBar::domElementType() const
{
  return DomElement_DIV; // later support DomElement_PROGRESS
}

void WProgressBar::updateDom(DomElement& element, bool all)
{
  DomElement *bar = 0, *label = 0;

  if (all) {
    bar = DomElement::createNew(DomElement_DIV);
    bar->setId("bar" + id());
    bar->setProperty(PropertyClass, "Wt-pgb-bar");

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

