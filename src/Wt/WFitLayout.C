/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WContainerWidget"
#include "Wt/WFitLayout"
#include "Wt/WLogger"

namespace Wt {

LOGGER("WFitLayout");

WFitLayout::WFitLayout(WWidget *parent)
  : WLayout()
{ 
  grid_.columns_.push_back(Impl::Grid::Section(0));
  grid_.rows_.push_back(Impl::Grid::Section(0));

  std::vector<Impl::Grid::Item> items;
  items.push_back(Impl::Grid::Item());
  grid_.items_.push_back(items);

  if (parent)
    setLayoutInParent(parent);
}

WFitLayout::~WFitLayout()
{ }

void WFitLayout::fitWidget(WContainerWidget *container, WWidget *widget)
{
  WFitLayout *l = new WFitLayout();
  container->setLayout(l);
  l->addWidget(widget);
}

void WFitLayout::addItem(WLayoutItem *item)
{
  if (grid_.items_[0][0].item_) {
    LOG_ERROR("addItem(): already have a widget");
    return;
  }

  grid_.items_[0][0].item_ = item;

  updateAddItem(item);
}

void WFitLayout::removeItem(WLayoutItem *item)
{
  if (item == grid_.items_[0][0].item_) {
    grid_.items_[0][0].item_ = 0;
    updateRemoveItem(item);
  }
}

WLayoutItem *WFitLayout::itemAt(int index) const
{
  return grid_.items_[0][0].item_;
}

void WFitLayout::clear()
{
  clearLayoutItem(grid_.items_[0][0].item_);
  grid_.items_[0][0].item_ = 0;
}

int WFitLayout::indexOf(WLayoutItem *item) const
{
  if (grid_.items_[0][0].item_ == item)
    return 0;
  else
    return -1;
}

int WFitLayout::count() const
{
  return grid_.items_[0][0].item_ ? 1 : 0;
}

}
