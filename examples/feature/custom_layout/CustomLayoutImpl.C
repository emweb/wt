/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "CustomLayoutImpl.h"

#include <web/DomElement.h>
#include <web/WebUtils.h>
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLayout.h>
#include <Wt/WLogger.h>

using namespace Wt;

CustomLayoutImpl::CustomLayoutImpl(WLayout *layout)
  : StdLayoutImpl(layout)
{ }

CustomLayoutImpl::~CustomLayoutImpl()
{
  WApplication *app = WApplication::instance();

  if (parentLayoutImpl() == nullptr) {
    if (container() == app->root()) {
      app->setBodyClass("");
      app->setHtmlClass("");
    }
  }
}

int CustomLayoutImpl::minimumWidth() const
{
  return items_.size() ? 50 : 0;
}

int CustomLayoutImpl::minimumHeight() const
{
  return items_.size() * 50;
}

int CustomLayoutImpl::maximumWidth() const
{
  return items_.size() ? 50 : 0;
}

int CustomLayoutImpl::maximumHeight() const
{
  return items_.size()/2 * 50;
}

void CustomLayoutImpl::itemAdded(WLayoutItem *item)
{
  items_.push_back(item);
  addedItems_.push_back(item);
  update();
}

void CustomLayoutImpl::itemRemoved(WLayoutItem *item)
{
  Utils::erase(items_, item);
  Utils::erase(addedItems_, item);
  removedItems_.push_back(getImpl(item)->id());
  update();
}

void CustomLayoutImpl::updateDom(DomElement& parent)
{
  WApplication *app = WApplication::instance();

  DomElement *div = DomElement::getForUpdate(elId_, DomElementType::DIV);

  for (unsigned i = 0; i < addedItems_.size(); ++i) {
    DomElement *el = createElement(addedItems_[i], app);
    div->addChild(el);
  }

  addedItems_.clear();

  for (unsigned i = 0; i < removedItems_.size(); ++i) {
    div->callJavaScript(WT_CLASS ".remove('" + removedItems_[i] + "');", false);
    div->callJavaScript(WT_CLASS ".remove('" + removedItems_[i] + "');", true);
  }
  removedItems_.clear();

  parent.addChild(div);
}

DomElement *CustomLayoutImpl::createDomElement(DomElement *parent,
                                             WT_MAYBE_UNUSED bool fitWidth,
                                             WT_MAYBE_UNUSED bool fitHeight,
                                             WApplication *app)
{
  addedItems_.clear();
  removedItems_.clear();

  DomElement *result;

  if (layout()->parentLayout() == nullptr) {
    /*
     * If it is a top-level layout (as opposed to a nested layout),
     * configure overflow of the container.
     */
    if (container() == app->root()) {
      /*
       * Reset body,html default paddings and so on if we are doing layout
       * in the entire document.
       */
      app->setBodyClass(app->bodyClass() + " Wt-layout");
      app->setHtmlClass(app->htmlClass() + " Wt-layout");

      parent->setProperty(Property::StyleBoxSizing, "border-box");
    }

    result = parent;
    parent->setProperty(Property::StyleMinWidth, std::to_string(minimumWidth())+"px");
    parent->setProperty(Property::StyleMaxWidth, std::to_string(maximumWidth())+"px");
    parent->setProperty(Property::StyleMinHeight, std::to_string(minimumHeight())+"px");
    parent->setProperty(Property::StyleMaxHeight, std::to_string(maximumHeight())+"px");
    elId_ = container()->id();
  } else {
    result = DomElement::createNew(DomElementType::DIV);
    elId_ = id();
    result->setId(elId_);
    result->setProperty(Property::StyleMinWidth, std::to_string(minimumWidth())+"px");
    result->setProperty(Property::StyleMaxWidth, std::to_string(maximumWidth())+"px");
    result->setProperty(Property::StyleMinHeight, std::to_string(minimumHeight())+"px");
    result->setProperty(Property::StyleMaxHeight, std::to_string(maximumHeight())+"px");
  }

  for (int i = 0; i < items_.size(); ++i) {
    DomElement *el = createElement(items_[i], app);
    result->addChild(el);
  }

  return result;
}

DomElement *CustomLayoutImpl::createElement(WLayoutItem *item, WApplication *app)
{
  DomElement *el = getImpl(item)->createDomElement(nullptr, false, false, app);

  el->setProperty(Property::StyleMinWidth, "50px");
  el->setProperty(Property::StyleMaxWidth, "50px");
  el->setProperty(Property::StyleMinHeight, "50px");
  el->setProperty(Property::StyleMaxHeight, "50px");

  return el;
}

bool CustomLayoutImpl::itemResized(WLayoutItem *item)
{
  return true;
}

bool CustomLayoutImpl::parentResized()
{
  return false;
}