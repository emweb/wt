/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WWidgetItem.h"

#include "Wt/WContainerWidget.h"
#include "Wt/WException.h"
#include "Wt/WLayout.h"
#include "Wt/WLayoutItemImpl.h"

#include "StdWidgetItemImpl.h"
#include "FlexLayoutImpl.h"
#include "FlexItemImpl.h"

namespace Wt {

WWidgetItem::WWidgetItem(std::unique_ptr<WWidget> widget)
  : widget_(std::move(widget)),
    parentLayout_(nullptr)
{ }

WWidgetItem::~WWidgetItem()
{
  setParentWidget(nullptr);
}

WWidgetItem *WWidgetItem::findWidgetItem(WWidget *widget)
{
  if (widget_.get() == widget)
    return this;
  else
    return nullptr;
}

void WWidgetItem::setParentWidget(WWidget *parent)
{
  if (!widget_)
    return;

  if (parent) {
    WContainerWidget *pc = dynamic_cast<WContainerWidget *>(parent);

    if (widget_->parent()) {
      if (widget_->parent() != pc)
	throw WException("Cannot move a WWidgetItem to another container");
    } else
      pc->widgetAdded(widget_.get());

    bool flexLayout = parentLayout_->implementationIsFlexLayout();

    if (flexLayout)
      impl_ = std::make_unique<FlexItemImpl>(this);
    else
      impl_ = std::make_unique<StdWidgetItemImpl>(this);
  } else {
    WContainerWidget *pc = dynamic_cast<WContainerWidget *>(widget_->parent());

    if (pc) {
      assert(impl_);
      bool flex = dynamic_cast<FlexItemImpl*>(impl());
      pc->widgetRemoved(widget_.get(), flex);
    }

    impl_.reset();
  }
}

std::unique_ptr<WWidget> WWidgetItem::takeWidget()
{
  std::unique_ptr<WWidget> result = std::move(widget_);
  impl_.reset();
  return result;
}

void WWidgetItem::setParentLayout(WLayout *parentLayout)
{
  parentLayout_ = parentLayout;
}

void WWidgetItem::iterateWidgets(const HandleWidgetMethod& method) const
{
  if (widget_)
#ifndef WT_TARGET_JAVA
    method(widget_.get());
#else
    method.handle(widget_.get());
#endif
}

WWidget *WWidgetItem::parentWidget() const
{
  if (parentLayout_)
    return parentLayout_->parentWidget();
  else
    return nullptr;
}

}
