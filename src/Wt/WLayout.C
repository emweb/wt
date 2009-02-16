/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WLayout"
#include "Wt/WLayoutItemImpl"
#include "Wt/WWidget"
#include "Wt/WWidgetItem"

#include "WtException.h"

namespace Wt {

WLayout::WLayout()
  : margins_(0),
    impl_(0),
    hints_(0)
{ }

WLayout::~WLayout()
{
  delete impl_;
  delete hints_;
  delete[] margins_;
}

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

void WLayout::removeWidget(WWidget *w)
{
  WWidgetItem *widgetItem = findWidgetItem(w);

  if (widgetItem) {
    widgetItem->parentLayout()->removeItem(widgetItem);
    delete widgetItem;
  }
}

void WLayout::updateAddItem(WLayoutItem *item)
{
  if (item->layout_)
    throw WtException("Cannot add item to two Layouts");

  item->layout_ = this;

  if (impl_) {
    item->setParent(impl_->parent());
    impl_->updateAddItem(item);
  }
}

void WLayout::updateRemoveItem(WLayoutItem *item)
{
  if (impl_)
    impl_->updateRemoveItem(item);

  item->layout_ = 0;
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

void WLayout::setParent(WWidget *parent)
{
  assert(!impl_);

  int c = count();
  for (int i = 0; i < c; ++i) {
    WLayoutItem *item = itemAt(i);
    if (item)
      item->setParent(parent);
  }

  impl_ = parent->createLayoutItemImpl(this);

  if (hints_) {
    for (unsigned i = 0; i < hints_->size(); ++i)
      impl_->setHint((*hints_)[i].first, (*hints_)[i].second);
    delete hints_;
    hints_ = 0;
  }
}

void WLayout::setLayoutHint(const std::string& name, const std::string& value)
{
  if (impl_)
    impl_->setHint(name, value);
  else {
    if (!hints_)
      hints_ = new HintsList();
    hints_->push_back(std::make_pair(name, value));
  }
}

}
