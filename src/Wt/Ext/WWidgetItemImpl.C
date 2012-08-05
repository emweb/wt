/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <sstream>

#include "Wt/Ext/WWidgetItemImpl.h"
#include "Wt/Ext/Container"
#include "Wt/WApplication"
#include "Wt/WWidgetItem"

#include "DomElement.h"

namespace Wt {
  namespace Ext {

WWidgetItemImpl::WWidgetItemImpl(WWidgetItem *item)
  : item_(item)
{ 
  item_->widget()->setInline(false);

  Container *c = dynamic_cast<Container *>(item_->widget());
  if (c) {
    var_ = c->elVar();
    id_ = c->extId();
  } else {
    var_ = "wi" + item_->widget()->id();
    id_ = var_;
  }
}

WWidgetItemImpl::~WWidgetItemImpl()
{ }

WLayoutItem *WWidgetItemImpl::layoutItem() const
{
  return item_;
}

void WWidgetItemImpl::createComponent(DomElement *parentContainer)
{
  DomElement *e = item_->widget()->createSDomElement(WApplication::instance());

  e->setProperty(PropertyStyle, e->getProperty(PropertyStyle)
		  + "position:absolute;left:-10000px;top:-10000px;"
		  "visibility:hidden;");
  Container *c = dynamic_cast<Container *>(item_->widget());

  if (!c) {
    std::stringstream js;

    js << "var " << var_ << "=new Ext.BoxComponent({id:'"
       << id_ << "',applyTo:'" << item_->widget()->id() << "'";
    addConfig(js);
    Container::setSizeConfig(js, item_->widget());
    js << "});{var s="
       << item_->widget()->jsRef() << ";s.style.position='';"
      "s.style.left='';"
      "s.style.top='';"
      "s.style.visibility='';}";

    e->callJavaScript(js.str());
  }

  parentContainer->addChild(e);
}

std::string WWidgetItemImpl::componentVar() const
{
  return var_;
}

std::string WWidgetItemImpl::componentId() const
{
  return id_;
}

void WWidgetItemImpl::containerAddWidgets(Container *container)
{
  container->add(item_->widget());
}

void WWidgetItemImpl::updateAddItem(WLayoutItem *)
{
  assert(false);
}

void WWidgetItemImpl::updateRemoveItem(WLayoutItem *)
{
  assert(false);
}


  }
}
