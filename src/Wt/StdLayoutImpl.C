/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "StdLayoutImpl.h"

#include "Wt/WContainerWidget"
#include "Wt/WLayout"
#include "Wt/WLayoutItem"

namespace Wt {

StdLayoutImpl::StdLayoutImpl(WLayout *layout)
  : layout_(layout),
    container_(0)
{ }

StdLayoutImpl::~StdLayoutImpl()
{
  if (container_)
    container_->layoutChanged(true, true);
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
  for (int i = c->count(); i > 0; --i) {
    /*
     * See original remark: 94337b3e062f925ba8e7afc74059a41f8488bb75
     * But, this should only be done if the widget is not part
     * of the layout
     */
    WWidget *w = c->widget(i - 1);
    if (!layout_->findWidgetItem(w))
	c->removeWidget(w);
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

}
