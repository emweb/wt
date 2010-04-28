/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "StdLayoutImpl.h"
#include "StdLayoutItemImpl.h"

#include "Wt/WContainerWidget"
#include "Wt/WLayoutItem"
#include "Wt/WLayout"

namespace Wt {

StdLayoutItemImpl::StdLayoutItemImpl()
{ }

StdLayoutItemImpl::~StdLayoutItemImpl()
{ }
  
StdLayoutImpl *StdLayoutItemImpl::parentLayoutImpl() const
{
  WLayoutItem *i = layoutItem();
  
  if (i->parentLayout())
    return dynamic_cast<StdLayoutImpl *>(i->parentLayout()->impl());
  else
    return 0;
}

WContainerWidget *StdLayoutItemImpl::container() const
{
  StdLayoutImpl *p = parentLayoutImpl();
  if (p)
    return p->container();
  else
    return 0;
}

WWidget *StdLayoutItemImpl::parentWidget() const
{
  return container();
}

}
