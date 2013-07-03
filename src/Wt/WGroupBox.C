/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WGroupBox"

#include "DomElement.h"
#include "StdWidgetItemImpl.h"

namespace Wt {

WGroupBox::WGroupBox(WContainerWidget *parent)
  : WContainerWidget(parent),
    titleChanged_(false)
{ 
  init();
}

WGroupBox::WGroupBox(const WString& title, WContainerWidget *parent)
  : WContainerWidget(parent),
    title_(title),
    titleChanged_(false)
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
  repaint(RepaintSizeAffected);
}

void WGroupBox::updateDom(DomElement& element, bool all)
{
  if (all || titleChanged_) {
    DomElement *legend;
    if (all) {
      legend = DomElement::createNew(DomElement_LEGEND);
      legend->setId(id() + "l");
    } else
      legend = DomElement::getForUpdate(id() + "l", DomElement_LEGEND);

    legend->setProperty(Wt::PropertyInnerHTML, escapeText(title_).toUTF8());
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
  return DomElement_FIELDSET;
}

void WGroupBox::refresh()
{
  if (title_.refresh()) {
    titleChanged_ = true;
    repaint(RepaintSizeAffected);
  }

  WContainerWidget::refresh();
}

int WGroupBox::firstChildIndex() const
{
  return 1; // Legend is before
}

}
