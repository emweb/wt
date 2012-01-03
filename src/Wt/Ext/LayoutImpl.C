/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/Ext/LayoutImpl.h"
#include "Wt/Ext/Container"
#include "Wt/Ext/TabWidget"
#include "Wt/WApplication"
#include "Wt/WLayout"
#include "Wt/WLayoutItem"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {
  namespace Ext {

LayoutImpl::LayoutImpl(WLayout *layout)
  : layout_(layout),
    container_(0)
{ }

LayoutImpl::~LayoutImpl()
{ 
  WApplication *app = WApplication::instance();

  app->doJavaScript(app->javaScriptClass()
		    + ".deleteExtW('" + id() + "');");
}

LayoutItemImpl *LayoutImpl::getImpl(WLayoutItem *item)
{
  return dynamic_cast<LayoutItemImpl *>(item->impl());
}

void LayoutImpl::updateAddItem(WLayoutItem *item)
{
  Container *c = container();

  if (c) {
    getImpl(item)->containerAddWidgets(c);

    if (c->isRendered()) {
      c->setLayoutChanged();
      itemsAdded_.push_back(item);
    }
  }
}

void LayoutImpl::updateRemoveItem(WLayoutItem *item)
{
  Container *c = container();

  if (c && c->isRendered()) {
    addUpdateJS(containerRef()
		+ ".remove('" + getImpl(item)->componentId() + "');");
  }

  Utils::erase(itemsAdded_, item);
}

std::string LayoutImpl::containerRef() const
{
  if (container_)
    return container_->elRef();
  else
    return elRef();
}

void LayoutImpl::setContainer(Container *c)
{
  container_ = c;
  containerAddWidgets(container_);
}

Container *LayoutImpl::container() const
{
  if (container_)
    return container_;
  else
    return LayoutItemImpl::container();
}

WLayoutItem *LayoutImpl::layoutItem() const
{
  return layout_;
}

void LayoutImpl::containerAddWidgets(Container *container)
{
  int c = layout_->count();

  for (int i = 0; i < c; ++i) {
    WLayoutItem *item = layout_->itemAt(i);
    if (item)
      getImpl(item)->containerAddWidgets(container);
  }
}

void LayoutImpl::createComponents(DomElement *parentContainer)
{
  int c = layout_->count();

  for (int i = 0; i < c; ++i) {
    WLayoutItem *item = layout_->itemAt(i);
    if (item)
      getImpl(item)->createComponent(parentContainer);
  }
}

void LayoutImpl::createComponent(DomElement *parentContainer)
{
  DomElement *nested = DomElement::createNew(DomElement_DIV);
  nested->setId("l" + id());
  createComponents(nested);
  parentContainer->addChild(nested);

  std::stringstream js;

  js << "var " << elVar() << '=' << elRef() << "=new Ext.Panel({id:'"
     << elVar() << "',border:false,contentEl:'l" << id() << '\'';

  addConfig(js);
  createConfig(js);

  js << "});";

  parentContainer->callJavaScript(js.str());
}

std::string LayoutImpl::componentVar() const
{
  return elVar();
}

std::string LayoutImpl::componentId() const
{
  return elVar();
}

void LayoutImpl::createConfig(std::ostream& config)
{
  int c = layout_->count();

  if (c) {
    config << ",items:";

    if (c > 1)
      config << '[';

    for (int i = 0; i < c; ++i) {
      WLayoutItem *item = layout_->itemAt(i);
      if (item) {
	if (i != 0)
	  config << ",";

	config << getImpl(item)->componentVar();
      }
    }

    if (c > 1)
      config << ']';
  }

  itemsAdded_.clear();
  jsUpdates_.clear();
}

void LayoutImpl::addLayoutConfig(LayoutItemImpl *item, std::ostream& config)
{ }

std::string LayoutImpl::elRef() const
{
  return /* WApplication::instance()->javaScriptClass() + '.' + */
    "ExtW['" + id() + "']";
}

std::string LayoutImpl::elVar() const
{
  return "el" + id();
}

void LayoutImpl::addUpdateJS(const std::string& js)
{
  if (!js.empty()) {
    jsUpdates_ += js;
    container()->setLayoutChanged();
  }
}

void LayoutImpl::getLayoutChanges(const std::string& parentId,
				  std::vector<DomElement *>& result)
{
  if (!jsUpdates_.empty() || !itemsAdded_.empty()) {
    DomElement *e = DomElement::getForUpdate(parentId,  DomElement_DIV);

    std::string addUpdates = "var " + elVar() + "=" + containerRef() + ";";

    for (unsigned i = 0; i < itemsAdded_.size(); ++i) {
      getImpl(itemsAdded_[i])->createComponent(e);
      std::string cvar = getImpl(itemsAdded_[i])->componentVar();
      addUpdates += elVar() + ".add(" + cvar + ");";
      // FIXME: does not work correctly when the container is not shown
      // for example because it is in an unactivated tab ?
    }
    if (!itemsAdded_.empty() && !dynamic_cast<TabWidget *>(container()))
      addUpdates += elVar() + ".doLayout();";

    itemsAdded_.clear();

    e->callJavaScript(jsUpdates_ + addUpdates);
    jsUpdates_.clear();

    result.push_back(e);
  }

  int c = layout_->count();
  for (int i = 0; i < c; ++i) {
    WLayoutItem *item = layout_->itemAt(i);
    if (item)
      getImpl(item)->getLayoutChanges("l" + id(), result);
  }
}

  }
}
