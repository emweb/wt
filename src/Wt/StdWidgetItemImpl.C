/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <cassert>

#include "DomElement.h"
#include "StdWidgetItemImpl.h"

#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"
#include "Wt/WTheme.h"
#include "Wt/WWidgetItem.h"

#ifndef WT_DEBUG_JS
#include "js/WtResize.min.js"
#endif

namespace Wt {

LOGGER("WWidgetItem");

StdWidgetItemImpl::StdWidgetItemImpl(WWidgetItem *item)
  : item_(item)
{ }

StdWidgetItemImpl::~StdWidgetItemImpl()
{ }

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

const std::string StdWidgetItemImpl::id() const
{
  return item_->widget()->id();
}

DomElement *StdWidgetItemImpl::createDomElement(DomElement *parent,
						bool fitWidth, bool fitHeight,
						WApplication *app)
{
  WWidget *w = item_->widget();

  w->setInline(false);

  DomElement *d = w->createSDomElement(app);
  DomElement *result = d;

  if (app->environment().agentIsIElt(9) &&
      (d->type() == DomElementType::TEXTAREA ||
       d->type() == DomElementType::SELECT ||
       d->type() == DomElementType::INPUT ||
       d->type() == DomElementType::BUTTON)) {
    d->removeProperty(Property::StyleDisplay);
  }

  // FIXME IE9 does border-box perhaps ?
  if (!app->environment().agentIsIElt(9) && 
      w->javaScriptMember(WWidget::WT_RESIZE_JS).empty() &&
      d->type() != DomElementType::TABLE /* buggy in Chrome, see #1856 */ &&
      app->theme()->canBorderBoxElement(*d))
    d->setProperty(Property::StyleBoxSizing, "border-box");

  return result;
}

WLayoutItem *StdWidgetItemImpl::layoutItem() const
{
  return item_;
}

}
