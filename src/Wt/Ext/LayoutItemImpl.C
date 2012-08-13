/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Ext/LayoutItemImpl.h"
#include "Wt/Ext/LayoutImpl.h"
#include "Wt/Ext/Container"
#include "Wt/WLayoutItem"
#include "Wt/WLayout"
#include "Wt/WLogger"

namespace Wt {

LOGGER("Ext.LayoutItemImpl");

  namespace Ext {

LayoutItemImpl::LayoutItemImpl()
{ }

LayoutItemImpl::~LayoutItemImpl()
{ }

void LayoutItemImpl::update(WLayoutItem *item)
{
  assert(false);
}

LayoutImpl *LayoutItemImpl::parentLayoutImpl() const
{
  WLayoutItem *i = layoutItem();

  if (i->parentLayout())
    return dynamic_cast<LayoutImpl *>(i->parentLayout()->impl());
  else
    return 0;
}

void LayoutItemImpl::addConfig(std::ostream& config)
{
  LayoutImpl *p = parentLayoutImpl();
  if (p)
    p->addLayoutConfig(this, config);
}

Container* LayoutItemImpl::container() const
{
  LayoutImpl *p = parentLayoutImpl();
  if (p)
    return p->container();
  else
    return 0;
}

WWidget *LayoutItemImpl::parentWidget() const
{
  return container();
}

void LayoutItemImpl::getLayoutChanges(const std::string& parentId,
				      std::vector<DomElement *>& result)
{ }

void LayoutItemImpl::setHint(const std::string& name, const std::string& value)
{
  LOG_ERROR("unrecognized hint '" << name << "'");
}

  }
}
