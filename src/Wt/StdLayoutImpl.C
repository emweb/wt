/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "DomElement.h"
#include "StdLayoutImpl.h"

#include "Wt/WContainerWidget"
#include "Wt/WLayout"
#include "Wt/WLayoutItem"
#include "Wt/WApplication"
#include "Wt/WLogger"

namespace Wt {

StdLayoutImpl::StdLayoutImpl(WLayout *layout)
  : layout_(layout),
    container_(0)
{ }

StdLayoutImpl::~StdLayoutImpl()
{
  if (container_)
    container_->layoutChanged(true);
}

StdLayoutItemImpl *StdLayoutImpl::getImpl(WLayoutItem *item)
{
  return dynamic_cast<StdLayoutItemImpl *>(item->impl());
}

void StdLayoutImpl::updateAddItem(WLayoutItem *item)
{
  WContainerWidget *c = container();

  if (c) {
    getImpl(item)->containerAddWidgets(c);

    update(item);
  }
}

void StdLayoutImpl::updateRemoveItem(WLayoutItem *item)
{
  WContainerWidget *c = container();

  if (c) {
    update(item);

    getImpl(item)->containerAddWidgets(0);
  }
}

void StdLayoutImpl::setContainer(WContainerWidget *c)
{
  if (c->count()) {
    while (c->count())
      c->removeWidget(c->widget(0));
  }

  container_ = c;
  containerAddWidgets(container_);
}

WContainerWidget *StdLayoutImpl::container() const
{
  if (container_)
    return container_;
  else
    return StdLayoutItemImpl::container();
}

WLayoutItem *StdLayoutImpl::layoutItem() const
{
  return layout_;
}

void StdLayoutImpl::containerAddWidgets(WContainerWidget *container)
{
  int c = layout_->count();

  for (int i = 0; i < c; ++i) {
    WLayoutItem *item = layout_->itemAt(i);
    if (item)
      getImpl(item)->containerAddWidgets(container);
  }
}

void StdLayoutImpl::update(WLayoutItem *item)
{
  WContainerWidget *c = container();

  if (c)
    c->layoutChanged();
}

}
