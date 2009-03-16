/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "DomElement.h"
#include "StdWidgetItemImpl.h"
#include "StdGridLayoutImpl.h"

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WLogger"
#include "Wt/WTextEdit"
#include "Wt/WWidgetItem"

namespace {
  bool isTextArea(Wt::WWidget *w) {
    return dynamic_cast<Wt::WTextArea *>(w)
      && !dynamic_cast<Wt::WTextEdit *>(w);
  }
}

namespace Wt {

StdWidgetItemImpl::StdWidgetItemImpl(WWidgetItem *item)
  : item_(item)
{ }

StdWidgetItemImpl::~StdWidgetItemImpl()
{ }

WLayoutItem *StdWidgetItemImpl::layoutItem() const
{
  return item_;
}

int StdWidgetItemImpl::minimumHeight() const
{
  return static_cast<int>(item_->widget()->minimumHeight().toPixels());
}

void StdWidgetItemImpl::containerAddWidgets(WContainerWidget *container)
{
  container->addWidget(item_->widget());
}

DomElement *StdWidgetItemImpl::createDomElement(bool fitWidth, bool fitHeight,
						int& additionalVerticalPadding,
						WApplication *app)
{
  DomElement *d = item_->widget()->webWidget()->createSDomElement(app);
  DomElement *result = d;

  // Safari does height properly (like Opera) ?
  if (   !app->environment().agentIE()
      && !app->environment().agentOpera()
      && (   (isTextArea(item_->widget()) && (fitWidth || fitHeight))
	  || (d->type() == DomElement_INPUT && fitWidth)
	  || (d->type() == DomElement_BUTTON && fitWidth
	      && app->environment().agentKonqueror()))) {
    /*
     * Browsers ignore the border so width/height 100% overflows the container.
     * Thus we wrap it in a container that is slightly less wide/high.
     */
    result = DomElement::createNew(DomElement_DIV);
    std::string style = "height:100%;";

    if (fitWidth)
      style += "margin-right:8px;";

    if (fitHeight && d->type() == DomElement_TEXTAREA) {
      style += "height:100%;";
      additionalVerticalPadding = 5;
    }

    result->setAttribute("style", style);
  }

  if (fitHeight && d->getProperty(PropertyStyleHeight).empty())
    if (   d->type() == DomElement_DIV
	|| d->type() == DomElement_UL
	|| d->type() == DomElement_TABLE
	|| d->type() == DomElement_TEXTAREA)
      d->setProperty(PropertyStyleHeight, "100%");

  if (fitWidth && d->getProperty(PropertyStyleWidth).empty()) {
    if (d->type() == DomElement_BUTTON
	|| d->type() == DomElement_INPUT
	|| d->type() == DomElement_TEXTAREA)
      d->setProperty(PropertyStyleWidth, "100%");
  }

  if (result != d)
    result->addChild(d);

  return result;
}

void StdWidgetItemImpl::updateAddItem(WLayoutItem *)
{
  assert(false);
}

void StdWidgetItemImpl::updateRemoveItem(WLayoutItem *)
{
  assert(false);
}

void StdWidgetItemImpl::update(WLayoutItem *)
{
  assert(false);
}

void StdWidgetItemImpl::setHint(const std::string& name,
				const std::string& value)
{
  WApplication::instance()->log("error")
    << "WWidgetItem: unrecognized hint '" << name << "'";
}

}
