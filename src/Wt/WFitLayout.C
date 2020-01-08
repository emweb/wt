/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WEnvironment.h"
#include "Wt/WFitLayout.h"
#include "Wt/WLogger.h"

#include "StdGridLayoutImpl2.h"
#include "FlexLayoutImpl.h"

namespace Wt {

LOGGER("WFitLayout");

WFitLayout::WFitLayout()
{ 
  grid_.columns_.push_back(Impl::Grid::Section(0));
  grid_.rows_.push_back(Impl::Grid::Section(0));

  std::vector<Impl::Grid::Item> items;
  items.push_back(Impl::Grid::Item());
  grid_.items_.push_back(std::move(items));
}

WFitLayout::~WFitLayout()
{ }

void WFitLayout::fitWidget(WContainerWidget *container,
			   std::unique_ptr<WWidget> widget)
{
  std::unique_ptr<WFitLayout> l(new WFitLayout());
  l->addWidget(std::move(widget));
  container->setLayout(std::move(l));
}

void WFitLayout::addItem(std::unique_ptr<WLayoutItem> item)
{
  if (grid_.items_[0][0].item_) {
    LOG_ERROR("addItem(): already have a widget");
    return;
  }

  WLayoutItem *it = item.get();
  grid_.items_[0][0].item_ = std::move(item);
  itemAdded(it);
}

std::unique_ptr<WLayoutItem> WFitLayout::removeItem(WLayoutItem *item)
{
  if (item == grid_.items_[0][0].item_.get()) {
    auto result = std::move(grid_.items_[0][0].item_);
    itemRemoved(item);
    return result;
  } else
    return std::unique_ptr<WLayoutItem>();
}

WLayoutItem *WFitLayout::itemAt(int index) const
{
  return grid_.items_[0][0].item_.get();
}

int WFitLayout::indexOf(WLayoutItem *item) const
{
  if (grid_.items_[0][0].item_.get() == item)
    return 0;
  else
    return -1;
}

int WFitLayout::count() const
{
  return grid_.items_[0][0].item_ ? 1 : 0;
}


void WFitLayout::iterateWidgets(const HandleWidgetMethod& method) const
{
  if (grid_.items_[0][0].item_)
    grid_.items_[0][0].item_->iterateWidgets(method);
}

void WFitLayout::setParentWidget(WWidget *parent)
{
  WLayout::setParentWidget(parent);

  if (parent) {
    updateImplementation();
  }
}

void WFitLayout::updateImplementation()
{
  if (!parentWidget())
    return;

  bool isFlexLayout = implementationIsFlexLayout();

  if (isFlexLayout)
    setImpl(cpp14::make_unique<FlexLayoutImpl>(this, grid_));
  else
    setImpl(cpp14::make_unique<StdGridLayoutImpl2>(this, grid_));
}

bool WFitLayout::implementationIsFlexLayout() const
{
  const WEnvironment &env = WApplication::instance()->environment();
  return preferredImplementation() == LayoutImplementation::Flex &&
         !env.agentIsIElt(10);
}

}
