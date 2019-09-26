/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WException.h"
#include "Wt/WLayout.h"
#include "Wt/WLayoutImpl.h"
#include "Wt/WWidget.h"
#include "Wt/WWidgetItem.h"

#include "Wt/StdGridLayoutImpl2.h"
#include "Wt/FlexLayoutImpl.h"

namespace Wt {

LayoutImplementation WLayout::defaultImplementation_ = LayoutImplementation::Flex;

WLayout::WLayout()
  : parentLayout_(nullptr),
    parentWidget_(nullptr),
    preferredImplementation_(defaultImplementation_)
{
  margins_[0] = margins_[1] = margins_[2] = margins_[3] = 9;
}

WLayout::~WLayout()
{ }

void WLayout::setPreferredImplementation(LayoutImplementation implementation)
{
  if (preferredImplementation_ != implementation) {
    preferredImplementation_ = implementation;
    if (impl_ && this->implementation() != preferredImplementation())
      updateImplementation();
  }
}

void WLayout::setDefaultImplementation(LayoutImplementation implementation)
{
  defaultImplementation_ = implementation;
}

LayoutImplementation WLayout::implementation() const
{
  if (dynamic_cast<StdGridLayoutImpl2*>(impl_.get()))
    return LayoutImplementation::JavaScript;
  if (dynamic_cast<FlexLayoutImpl*>(impl_.get()))
    return LayoutImplementation::Flex;
  return preferredImplementation_;
}

LayoutImplementation WLayout::preferredImplementation() const
{
  return preferredImplementation_;
}

#ifndef WT_TARGET_JAVA

void WLayout::getContentsMargins(int *left, int *top, int *right, int *bottom)
  const
{
  *left = margins_[0];
  *top = margins_[1];
  *right = margins_[2];
  *bottom = margins_[3];
}

#else // WT_TARGET_JAVA

int WLayout::getContentsMargin(Side side) const
{
  switch (side) {
  case Side::Left:
    return margins_[0];
  case Side::Top:
    return margins_[1];
  case Side::Right:
    return margins_[2];
  case Side::Bottom:
    return margins_[3];
  default:
    return 9;
  }
}
#endif // WT_TARGET_JAVA

void WLayout::setContentsMargins(int left, int top, int right, int bottom)
{
  margins_[0] = left;
  margins_[1] = top;
  margins_[2] = right;
  margins_[3] = bottom;
}

int WLayout::indexOf(WLayoutItem *item) const
{
  int c = count();
  for (int i = 0; i < c; ++i)
    if (itemAt(i) == item)
      return i;

  return -1;
}

void WLayout::addWidget(std::unique_ptr<WWidget> w)
{
  addItem(std::unique_ptr<WWidgetItem>(new WWidgetItem(std::move(w))));
}

std::unique_ptr<WWidget> WLayout::removeWidget(WWidget *w)
{
  WWidgetItem *widgetItem = findWidgetItem(w);

  if (widgetItem) {
    auto wi = widgetItem->parentLayout()->removeItem(widgetItem);
    return widgetItem->takeWidget();
  } else
    return std::unique_ptr<WWidget>();
}

void WLayout::itemAdded(WLayoutItem *item)
{
  item->setParentLayout(this);

  WWidget *w = parentWidget();
  if (w)
    item->setParentWidget(w);

  if (impl_)
    impl_->itemAdded(item);
}

void WLayout::itemRemoved(WLayoutItem *item)
{  
  if (impl_)
    impl_->itemRemoved(item);

  item->setParentWidget(nullptr);

  item->setParentLayout(nullptr);
}

void WLayout::update(WLayoutItem *item)
{
  if (impl_)
    impl_->update();
}

WWidgetItem *WLayout::findWidgetItem(WWidget *widget)
{
  int c = count();

  for (int i = 0; i < c; ++i) {
    WLayoutItem *item = itemAt(i);
    if (item) {
      WWidgetItem *result = item->findWidgetItem(widget);

      if (result)
	return result;
    }
  }

  return nullptr;
}

void WLayout::setParentWidget(WWidget *parent)
{
  parentWidget_ = parent;

  int c = count();

  for (int i = 0; i < c; ++i) {
    WLayoutItem *item = itemAt(i);
    if (item)
      item->setParentWidget(parent);
  }

  if (!parent)
    impl_.reset();
}

void WLayout::setParentLayout(WLayout *layout)
{
  parentLayout_ = layout;
}

WLayout *WLayout::parentLayout() const
{
  return parentLayout_;
}

WWidget *WLayout::parentWidget() const
{
  if (parentWidget_)
    return parentWidget_;
  else if (parentLayout_)
    return parentLayout_->parentWidget();
  else
    return nullptr;
}

void WLayout::setImpl(std::unique_ptr<WLayoutImpl> impl)
{
  impl_ = std::move(impl);
}

bool WLayout::implementationIsFlexLayout() const
{
  // If StdGridLayoutImpl2 is the only option, just return false
  return false;
}

void WLayout::updateImplementation()
{
  // If StdGridLayoutImpl2 is the only option, don't update anything
}

}
