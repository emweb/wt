/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WWidget"
#include "Wt/WWidgetItem"
#include "Wt/WLayoutItemImpl"

namespace Wt {

WWidgetItem::WWidgetItem(WWidget *widget)
  : widget_(widget),
    parentLayout_(0),
    impl_(0)
{ 
  widget_->setHasParent(true);
}

WWidgetItem::~WWidgetItem()
{
  widget_->setHasParent(false);

  delete impl_;
}

WWidgetItem *WWidgetItem::findWidgetItem(WWidget *widget)
{
  if (widget_ == widget)
    return this;
  else
    return 0;
}

void WWidgetItem::setParentWidget(WWidget *parent)
{ 
  assert(!impl_);
  impl_ = parent->createLayoutItemImpl(this);
}

void WWidgetItem::setParentLayout(WLayout *parentLayout)
{
  parentLayout_ = parentLayout;
}

}
