/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WGroupBox"

#include "DomElement.h"

namespace Wt {

WGroupBox::WGroupBox(WContainerWidget *parent)
  : WContainerWidget(parent),
    titleChanged_(false)
{ }

WGroupBox::WGroupBox(const WString& title, WContainerWidget *parent)
  : WContainerWidget(parent),
    title_(title),
    titleChanged_(false)
{ }

void WGroupBox::setTitle(const WString& title)
{
  title_ = title;
  titleChanged_ = true;
  repaint(RepaintInnerHtml);
}

void WGroupBox::updateDom(DomElement& element, bool all)
{
  if (all) {
    DomElement *legend = DomElement::createNew(DomElement_LEGEND);
    legend->setId(formName() + "l");
    legend->setProperty(Wt::PropertyInnerHTML, escapeText(title_).toUTF8());
    element.addChild(legend);

    titleChanged_ = false;
  } else if (titleChanged_) {
    DomElement *legend
      = DomElement::getForUpdate(formName() + "l", DomElement_LEGEND);
    legend->setProperty(Wt::PropertyInnerHTML, escapeText(title_).toUTF8());

    titleChanged_ = false;
  }

  WContainerWidget::updateDom(element, all);
}

DomElementType WGroupBox::domElementType() const
{
  return DomElement_FIELDSET;
}

void WGroupBox::refresh()
{
  if (title_.refresh()) {
    titleChanged_ = true;
    repaint(RepaintInnerHtml);
  }

  WContainerWidget::refresh();
}

bool WGroupBox::wasEmpty() const
{
  return false; // LEGEND is always there
}

}
