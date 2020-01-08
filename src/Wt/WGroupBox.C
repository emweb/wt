/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WGroupBox.h"

#include "DomElement.h"
#include "StdWidgetItemImpl.h"

namespace Wt {

WGroupBox::WGroupBox()
{ 
  init();
}

WGroupBox::WGroupBox(const WString& title)
  : title_(title)
{ 
  init();
}

void WGroupBox::init()
{
  setJavaScriptMember(WT_GETPS_JS, StdWidgetItemImpl::secondGetPSJS());
}

void WGroupBox::setTitle(const WString& title)
{
  title_ = title;
  titleChanged_ = true;
  repaint(RepaintFlag::SizeAffected);
}

void WGroupBox::updateDom(DomElement& element, bool all)
{
  if (all || titleChanged_) {
    DomElement *legend;
    if (all) {
      legend = DomElement::createNew(DomElementType::LEGEND);
      legend->setId(id() + "l");
    } else
      legend = DomElement::getForUpdate(id() + "l", DomElementType::LEGEND);

    legend->setProperty(Wt::Property::InnerHTML, escapeText(title_).toUTF8());
    element.addChild(legend);

    titleChanged_ = false;
  }

  WContainerWidget::updateDom(element, all);
}

void WGroupBox::propagateRenderOk(bool deep)
{
  titleChanged_ = false;

  WContainerWidget::propagateRenderOk(deep);
}

DomElementType WGroupBox::domElementType() const
{
  return DomElementType::FIELDSET;
}

void WGroupBox::refresh()
{
  if (title_.refresh()) {
    titleChanged_ = true;
    repaint(RepaintFlag::SizeAffected);
  }

  WContainerWidget::refresh();
}

int WGroupBox::firstChildIndex() const
{
  return 1; // Legend is before
}

}
