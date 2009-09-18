/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WGroupBox"

#include "DomElement.h"
#include "Utils.h"

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
    legend->setId(id() + "l");
    legend->setProperty(Wt::PropertyInnerHTML, escapeText(title_).toUTF8());
    element.addChild(legend);

    titleChanged_ = false;
  }

  WContainerWidget::updateDom(element, all);
}

void WGroupBox::propagateSetEnabled(bool enabled)
{
  if (enabled)
    setStyleClass(WString::fromUTF8(Utils::eraseWord(styleClass().toUTF8(), "Wt-disabled")));
  else
    setStyleClass(WString::fromUTF8(Utils::addWord(styleClass().toUTF8(), "Wt-disabled")));

  WInteractWidget::propagateSetEnabled(enabled);
}

void WGroupBox::propagateRenderOk(bool deep)
{
  titleChanged_ = false;

  WContainerWidget::propagateRenderOk(deep);
}

void WGroupBox::getDomChanges(std::vector<DomElement *>& result,
			      WApplication *app)
{
  DomElement *e = DomElement::getForUpdate(this, domElementType());
  updateDom(*e, false);
  result.push_back(e);

  if (titleChanged_) {
    DomElement *legend
      = DomElement::getForUpdate(id() + "l", DomElement_LEGEND);
    legend->setProperty(Wt::PropertyInnerHTML, escapeText(title_).toUTF8());

    titleChanged_ = false;

    result.push_back(legend);
  }
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

int WGroupBox::firstChildIndex() const
{
  return 1; // Legend is before
}

}
