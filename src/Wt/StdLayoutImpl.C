/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "StdLayoutImpl.h"

#include "Wt/WContainerWidget.h"
#include "Wt/WLayout.h"
#include "Wt/WLayoutItem.h"

namespace Wt {

StdLayoutImpl::StdLayoutImpl(WLayout *layout)
  : layout_(layout)
{ }

StdLayoutImpl::~StdLayoutImpl()
{ }

WLayoutItem *StdLayoutImpl::layoutItem() const
{
  return layout_;
}

StdLayoutItemImpl *StdLayoutImpl::getImpl(WLayoutItem *item)
{
  return dynamic_cast<StdLayoutItemImpl *>(item->impl());
}

}
