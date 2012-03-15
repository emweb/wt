/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WException"
#include "Wt/WLayout"
#include "Wt/WLayoutItemImpl"
#include "Wt/WWidget"
#include "Wt/WWidgetItem"

namespace Wt {

WLayout::WLayout()
  : margins_(0),
    impl_(0),
    hints_(0)
{ }

WLayout::~WLayout()
{
  if (!parentLayout())
    setParent(0);

  delete impl_;
  delete hints_;
  delete[] margins_;
}

#ifndef WT_TARGET_JAVA

void WLayout::getContentsMargins(int *left, int *top, int *right, int *bottom)
  const
{
  if (margins_) {
    *left = margins_[0];
    *top = margins_[1];
    *right = margins_[2];
    *bottom = margins_[3];
  } else {
    *left = 9;
    *right = 9;
    *top = 9;
    *bottom = 9;
  }
}

#else // WT_TARGET_JAVA

int WLayout::getContentsMargin(Side side) const
{
  if (!margins_)
    return 9;

  switch (side) {
  case Left:
    return margins_[0];
  case Top:
    return margins_[1];
  case Right:
    return margins_[2];
  case Bottom:
    return margins_[3];
  default:
    return 9;
  }
}
#endif // WT_TARGET_JAVA

void WLayout::setContentsMargins(int left, int top, int right, int bottom)
{
  if (!margins_)
    margins_ = new int[4];

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

void WLayout::addWidget(WWidget *w)
{
  addItem(new WWidgetItem(w));
}

bool WLayout::removeWidget(WWidget *w)
{
  WWidgetItem *widgetItem = findWidgetItem(w);

  if (widgetItem) {
    widgetItem->parentLayout()->removeItem(widgetItem);
    delete widgetItem;
    return true;
  } else
    return false;
}

void WLayout::updateAddItem(WLayoutItem *item)
{
  if (item->parentLayout())
    throw WException("Cannot add item to two Layouts");

  item->setParentLayout(this);

  if (impl_) {
    item->setParentWidget(impl_->parentWidget());
    impl_->updateAddItem(item);
  }
}

void WLayout::updateRemoveItem(WLayoutItem *item)
{
  if (impl_)
    impl_->updateRemoveItem(item);

  item->setParentLayout(0);
}

void WLayout::update(WLayoutItem *item)
{
  if (impl_)
    impl_->update(item);
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

  return 0;
}

void WLayout::setLayoutInParent(WWidget *parent)
{
  parent->setLayout(this);
}

void WLayout::setParentWidget(WWidget *parent)
{
  if (!this->parent())
    setParent(parent);

  assert(!impl_);

  int c = count();
  for (int i = 0; i < c; ++i) {
    WLayoutItem *item = itemAt(i);
    if (item)
      item->setParentWidget(parent);
  }

  impl_ = parent->createLayoutItemImpl(this);

  if (hints_) {
    for (unsigned i = 0; i < hints_->size(); ++i)
      impl_->setHint((*hints_)[i].name, (*hints_)[i].value);
    delete hints_;
    hints_ = 0;
  }
}

void WLayout::setParentLayout(WLayout *layout)
{
  if (layout)
    layout->addChild(this);
  else
    parent()->removeChild(this);
}

WLayout *WLayout::parentLayout() const
{
  return dynamic_cast<WLayout *>(parent());
}

void WLayout::setLayoutHint(const std::string& name, const std::string& value)
{
  if (impl_)
    impl_->setHint(name, value);
  else {
    if (!hints_)
      hints_ = new HintsList();
    hints_->push_back(Hint(name, value));
  }
}

void WLayout::clearLayoutItem(WLayoutItem *item)
{    
  if (item) {
    WWidget* widget = 0;
    
    if (item->layout()) {
      //clear removes all widgets and sublayouts of this layout,
      //which is not executed by the dtor
      item->layout()->clear();
    } else {
      widget = item->widget();
    }
    
    removeItem(item);
    delete item;
    
    delete widget;
  }
}

}
