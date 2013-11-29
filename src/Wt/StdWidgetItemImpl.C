/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <cassert>

#include "DomElement.h"
#include "StdWidgetItemImpl.h"

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WLogger"
#include "Wt/WWidgetItem"
#include "Wt/WTheme"

#ifndef WT_DEBUG_JS
#include "js/WtResize.min.js"
#endif

namespace Wt {

LOGGER("WWidgetItem");

StdWidgetItemImpl::StdWidgetItemImpl(WWidgetItem *item)
  : item_(item)
{ }

StdWidgetItemImpl::~StdWidgetItemImpl()
{
  containerAddWidgets(0);
}

const char *StdWidgetItemImpl::childrenResizeJS()
{
  WApplication *app = WApplication::instance();
  LOAD_JAVASCRIPT(app, "js/WtResize.js", "ChildrenResize", wtjs10);

  return WT_CLASS ".ChildrenResize";
}

const char *StdWidgetItemImpl::childrenGetPSJS()
{
  WApplication *app = WApplication::instance();
  LOAD_JAVASCRIPT(app, "js/WtResize.js", "ChildrenGetPS", wtjs11);

  return WT_CLASS ".ChildrenGetPS";
}

const char *StdWidgetItemImpl::secondResizeJS()
{
  WApplication *app = WApplication::instance();
  LOAD_JAVASCRIPT(app, "js/WtResize.js", "LastResize", wtjs12);

  return WT_CLASS ".LastResize";
}

const char *StdWidgetItemImpl::secondGetPSJS()
{
  WApplication *app = WApplication::instance();
  LOAD_JAVASCRIPT(app, "js/WtResize.js", "LastGetPS", wtjs13);

  return WT_CLASS ".LastGetPS";
}

WLayoutItem *StdWidgetItemImpl::layoutItem() const
{
  return item_;
}

int StdWidgetItemImpl::minimumWidth() const
{
  if (item_->widget()->isHidden())
    return 0;
  else
    return static_cast<int>(item_->widget()->minimumWidth().toPixels());
}

int StdWidgetItemImpl::minimumHeight() const
{
  if (item_->widget()->isHidden())
    return 0;
  else
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

const std::string StdWidgetItemImpl::id() const
{
  return item_->widget()->id();
}

DomElement *StdWidgetItemImpl::createDomElement(bool fitWidth, bool fitHeight,
						WApplication *app)
{
  WWidget *w = item_->widget();

  w->setInline(false);

  DomElement *d = w->createSDomElement(app);
  DomElement *result = d;

  if (app->environment().agentIsIElt(9) &&
      (d->type() == DomElement_TEXTAREA || d->type() == DomElement_SELECT
       || d->type() == DomElement_INPUT || d->type() == DomElement_BUTTON)) {
    d->removeProperty(PropertyStyleDisplay);
  }

  // FIXME IE9 does border-box perhaps ?
  if (!app->environment().agentIsIElt(9) && 
      w->javaScriptMember(WWidget::WT_RESIZE_JS).empty() &&
      d->type() != DomElement_TABLE /* buggy in Chrome, see #1856 */ &&
      app->theme()->canBorderBoxElement(*d))
    d->setProperty(PropertyStyleBoxSizing, "border-box");

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
  LOG_ERROR("unrecognized hint '" << name << "'");
}

}
