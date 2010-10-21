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
#include "Wt/WWidgetItem"

namespace Wt {

StdWidgetItemImpl::StdWidgetItemImpl(WWidgetItem *item)
  : item_(item)
{ }

StdWidgetItemImpl::~StdWidgetItemImpl()
{
  containerAddWidgets(0);
}

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
  if (container)
    container->addWidget(item_->widget());
  else {
    WContainerWidget *wc
      = dynamic_cast<WContainerWidget *>(item_->widget()->parent());

    if (wc)
      wc->removeFromLayout(item_->widget());
  }
}

DomElement *StdWidgetItemImpl::createDomElement(bool fitWidth, bool fitHeight,
						WApplication *app)
{
  WWidget *w = item_->widget();

  DomElement *d = w->createSDomElement(app);
  DomElement *result = d;

  int marginRight = 0, marginBottom = 0;

  if (fitWidth)
    marginRight = (w->boxPadding(Horizontal) + w->boxBorder(Horizontal)) * 2;

  if (fitHeight)
    marginBottom = (w->boxPadding(Vertical) + w->boxBorder(Vertical)) * 2;

  bool forceDiv
    = (fitHeight && d->type() == DomElement_SELECT
       && d->getAttribute("size").empty());

  if (marginRight || marginBottom || forceDiv) {
    result = DomElement::createNew(DomElement_DIV);
    result->setProperty(PropertyClass, "Wt-wrapdiv");
    std::stringstream style;

    if (app->environment().agentIsIElt(9) && !forceDiv) {
      style << "margin-top:-1px;";
      marginBottom -= 1;
    }

    if (marginRight)
      style << "margin-right:" << marginRight << "px;";

    if (marginBottom)
      style << "margin-bottom:" << marginBottom << "px;";

    result->setProperty(PropertyStyle, style.str());
  }

  /*
   * Known issues:
   *  - textarea does not interpret height 100%, and thus it does not
   *    work inside the wrapped div, on IE6/7 -> fixed in the JavaScript code
   *  - select does not interpret height that is set on IE6
   *    it does work on IE7 !
   */
  if (fitHeight && d->getProperty(PropertyStyleHeight).empty())
    if (   d->type() == DomElement_DIV
	|| d->type() == DomElement_UL
	|| d->type() == DomElement_INPUT
	|| d->type() == DomElement_TABLE
	|| d->type() == DomElement_TEXTAREA)
      d->setProperty(PropertyStyleHeight, "100%");

  // on IE, a select is reduced to width 0 when setting width: 100% when nothing
  // else in that column takes up space: that is a very bad thing...
  if (fitWidth && d->getProperty(PropertyStyleWidth).empty()) {
    if ((d->type() == DomElement_BUTTON
	 || (d->type() == DomElement_INPUT
	     && d->getAttribute("type") != "radio"
	     && d->getAttribute("type") != "checkbox")
	 || (d->type() == DomElement_SELECT
	     && !app->environment().agentIsIE())
	 || d->type() == DomElement_TEXTAREA))
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
