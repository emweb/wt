/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cassert>

#include "Wt/WApplication.h"
#include "Wt/WBoxLayout.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"
#include "Wt/WWebWidget.h"
#include "Wt/WWidgetItem.h"

#include "StdGridLayoutImpl2.h"
#include "FlexLayoutImpl.h"

namespace {
  class Spacer : public Wt::WWebWidget
  {
  public:
    Spacer() { setInline(false); }

  protected:
    virtual Wt::DomElementType domElementType() const override
    {
      return Wt::DomElementType::DIV;
    }
  };
}

namespace Wt {

LOGGER("WBoxLayout");

WBoxLayout::WBoxLayout(LayoutDirection dir)
  : direction_(dir)
{ }

void WBoxLayout::addItem(std::unique_ptr<WLayoutItem> item)
{
  insertItem(count(), std::move(item), 0, None);
}

std::unique_ptr<WLayoutItem> WBoxLayout::removeItem(WLayoutItem *item)
{
  std::unique_ptr<WLayoutItem> result;

  int index = indexOf(item);

  if (index != -1) {
    switch (direction_) {
    case LayoutDirection::RightToLeft:
      if ((impl() && implementation() != LayoutImplementation::Flex) || !implementationIsFlexLayout())
	index = grid_.columns_.size() - 1 - index;
    case LayoutDirection::LeftToRight: {
        result = std::move(grid_.items_[0][index].item_);
        grid_.columns_.erase(grid_.columns_.begin() + index);
        grid_.items_[0].erase(grid_.items_[0].begin() + index);
        break;
      }
    case LayoutDirection::BottomToTop:
      if ((impl() && implementation() != LayoutImplementation::Flex) || !implementationIsFlexLayout())
        index = grid_.rows_.size() - 1 - index;
    case LayoutDirection::TopToBottom: {
        result = std::move(grid_.items_[index][0].item_);
        grid_.rows_.erase(grid_.rows_.begin() + index);
        grid_.items_.erase(grid_.items_.begin() + index);
      }
    }

    itemRemoved(item);
  }

  return result;
}

WLayoutItem *WBoxLayout::itemAt(int index) const
{
  switch (direction_) {
  case LayoutDirection::RightToLeft:
    if ((impl() && implementation() != LayoutImplementation::Flex) || !implementationIsFlexLayout())
      index = grid_.columns_.size() - 1 - index;
  case LayoutDirection::LeftToRight:
    return grid_.items_[0][index].item_.get();
  case LayoutDirection::BottomToTop:
    if ((impl() && implementation() != LayoutImplementation::Flex) || !implementationIsFlexLayout())
      index = grid_.rows_.size() - 1 - index;
  case LayoutDirection::TopToBottom:
    return grid_.items_[index][0].item_.get();
  }

  assert(false);
  return nullptr;
}

int WBoxLayout::count() const
{
  return grid_.rows_.size() * grid_.columns_.size();
}

void WBoxLayout::setDirection(LayoutDirection direction)
{
  if (direction_ != direction) {
    direction_ = direction;
  }
}

void WBoxLayout::setSpacing(int size)
{
  grid_.horizontalSpacing_ = size;
  grid_.verticalSpacing_ = size;
}

void WBoxLayout::addWidget(std::unique_ptr<WWidget> widget, int stretch,
			   WFlags<AlignmentFlag> alignment)
{
  insertWidget(count(), std::move(widget), stretch, alignment);
}

void WBoxLayout::addLayout(std::unique_ptr<WLayout> layout, int stretch,
			   WFlags<AlignmentFlag> alignment)
{
  insertLayout(count(), std::move(layout), stretch, alignment);
}

void WBoxLayout::addSpacing(const WLength& size)
{
  insertSpacing(count(), size);
}

void WBoxLayout::addStretch(int stretch)
{
  insertStretch(count(), stretch);
}

void WBoxLayout::insertWidget(int index, std::unique_ptr<WWidget> widget,
			      int stretch,
			      WFlags<AlignmentFlag> alignment)
{
  if (widget->layoutSizeAware() && stretch == 0)
    stretch = -1;

  insertItem(index,
	     std::unique_ptr<WLayoutItem>(new WWidgetItem(std::move(widget))),
	     stretch, alignment);
}

void WBoxLayout::insertLayout(int index, std::unique_ptr<WLayout> layout,
			      int stretch,
			      WFlags<AlignmentFlag> alignment)
{
  insertItem(index, std::move(layout), stretch, alignment);
}

void WBoxLayout::insertSpacing(int index, const WLength& size)
{
  std::unique_ptr<WWidget> spacer(createSpacer(size));
  insertItem(index,
	     std::unique_ptr<WWidgetItem>(new WWidgetItem(std::move(spacer))),
	     0, None);
}

void WBoxLayout::insertStretch(int index, int stretch)
{
  std::unique_ptr<WWidget> spacer = createSpacer(WLength(0));
  insertItem(index,
	     std::unique_ptr<WWidgetItem>(new WWidgetItem(std::move(spacer))),
	     stretch, None);
}

bool WBoxLayout::setStretchFactor(WWidget *widget, int stretch)
{
  for (int i = 0; i < count(); ++i) {
    WLayoutItem *item = itemAt(i);
    if (item && item->widget() == widget) {
      setStretchFactor(i, stretch);

      return true;
    }
  }

  return false;
}

bool WBoxLayout::setStretchFactor(WLayout *layout, int stretch)
{
  for (int i = 0; i < count(); ++i) {
    WLayoutItem *item = itemAt(i);
    if (item && item->layout() == layout) {
      setStretchFactor(i, stretch);

      return true;
    }
  }

  return false;
}

void WBoxLayout::setStretchFactor(int i, int stretch)
{
  switch (direction_) {
  case LayoutDirection::RightToLeft:
    if ((impl() && implementation() != LayoutImplementation::Flex) || !implementationIsFlexLayout())
      i = grid_.columns_.size() - 1 - i;
  case LayoutDirection::LeftToRight:
    grid_.columns_[i].stretch_ = stretch;
    break;
  case LayoutDirection::BottomToTop:
    if ((impl() && implementation() != LayoutImplementation::Flex) || !implementationIsFlexLayout())
      i = grid_.rows_.size() - 1 - i;
  case LayoutDirection::TopToBottom:
    grid_.rows_[i].stretch_ = stretch;
  }
}

void WBoxLayout::insertItem(int index, std::unique_ptr<WLayoutItem> item,
			    int stretch, WFlags<AlignmentFlag> alignment)
{
  WLayoutItem *it = item.get();

  switch (direction_) {
  case LayoutDirection::RightToLeft:
    if ((impl() && implementation() != LayoutImplementation::Flex) || !implementationIsFlexLayout())
      index = grid_.columns_.size() - index;
  case LayoutDirection::LeftToRight:
    grid_.columns_.insert(grid_.columns_.begin() + index,
			  Impl::Grid::Section(stretch));
    if (grid_.items_.empty()) {
      grid_.items_.push_back(std::vector<Impl::Grid::Item>());
      grid_.rows_.push_back(Impl::Grid::Section());
      grid_.rows_[0].stretch_ = -1; // make height managed
    }
    grid_.items_[0].insert(grid_.items_[0].begin() + index,
			   Impl::Grid::Item(std::move(item), alignment));
    break;
  case LayoutDirection::BottomToTop:
    if ((impl() && implementation() != LayoutImplementation::Flex) || !implementationIsFlexLayout())
      index = grid_.rows_.size() - index;
  case LayoutDirection::TopToBottom:
    if (grid_.columns_.empty()) {
      grid_.columns_.push_back(Impl::Grid::Section());
      grid_.columns_[0].stretch_ = -1; // make width managed
    }
    grid_.rows_.insert(grid_.rows_.begin() + index,
		       Impl::Grid::Section(stretch));
    grid_.items_.insert(grid_.items_.begin() + index,
			std::vector<Impl::Grid::Item>());
    grid_.items_[index].push_back(Impl::Grid::Item(std::move(item), alignment));
    break;
  }

  itemAdded(it);
}

std::unique_ptr<WWidget> WBoxLayout::createSpacer(const WLength& size)
{
  std::unique_ptr<Spacer> spacer(new Spacer());

  if (size.toPixels() > 0) {
    if (direction_ == LayoutDirection::LeftToRight || 
	direction_ == LayoutDirection::RightToLeft)
      spacer->setMinimumSize(size, WLength::Auto);
    else
      spacer->setMinimumSize(WLength::Auto, size);
  }

  return std::move(spacer);
}

void WBoxLayout::setResizable(int index, bool enabled,
			      const WLength& initialSize)
{
  if (preferredImplementation() == LayoutImplementation::Flex) {
    LOG_WARN("Resize handles are not supported for flex layout implementation, "
             "using JavaScript implementation instead");
    setPreferredImplementation(LayoutImplementation::JavaScript);
  }

  switch (direction_) {
  case LayoutDirection::RightToLeft:
    if ((impl() && implementation() != LayoutImplementation::Flex) || !implementationIsFlexLayout())
      index = grid_.columns_.size() - 1 - index;
  case LayoutDirection::LeftToRight:
    grid_.columns_[index].resizable_ = enabled;
    grid_.columns_[index].initialSize_ = initialSize;
    break;
  case LayoutDirection::BottomToTop:
    if ((impl() && implementation() != LayoutImplementation::Flex) || !implementationIsFlexLayout())
      index = grid_.rows_.size() - 1 - index;
  case LayoutDirection::TopToBottom:
    grid_.rows_[index].resizable_ = enabled;
    grid_.rows_[index].initialSize_ = initialSize;
  }

  update(nullptr);
}

bool WBoxLayout::isResizable(int index) const
{
  switch (direction_) {
  case LayoutDirection::RightToLeft:
    if ((impl() && implementation() != LayoutImplementation::Flex) || !implementationIsFlexLayout())
      index = grid_.columns_.size() - 1 - index;
  case LayoutDirection::LeftToRight:
    return grid_.columns_[index].resizable_;
  case LayoutDirection::BottomToTop:
    if ((impl() && implementation() != LayoutImplementation::Flex) || !implementationIsFlexLayout())
      index = grid_.rows_.size() - 1 - index;
  case LayoutDirection::TopToBottom:
    return grid_.rows_[index].resizable_;
  }

  return false;
}

void WBoxLayout::iterateWidgets(const HandleWidgetMethod& method) const
{
  for (unsigned r = 0; r < grid_.rows_.size(); ++r) {
    for (unsigned c = 0; c < grid_.columns_.size(); ++c) {
      WLayoutItem *item = grid_.items_[r][c].item_.get();
      if (item)
	item->iterateWidgets(method);
    }
  }
}

void WBoxLayout::setParentWidget(WWidget *parent)
{
  WLayout::setParentWidget(parent);

  updateImplementation();
}

bool WBoxLayout::implementationIsFlexLayout() const
{
  const WEnvironment &env = WApplication::instance()->environment();
  return preferredImplementation() == LayoutImplementation::Flex &&
         !env.agentIsIElt(10);
}

void WBoxLayout::updateImplementation()
{
  if (!parentWidget())
    return;

  setImplementation();
}

void WBoxLayout::setImplementation()
{
  bool isFlexLayout = implementationIsFlexLayout();

  if (isFlexLayout)
    setImpl(cpp14::make_unique<FlexLayoutImpl>(this, grid_));
  else
    setImpl(cpp14::make_unique<StdGridLayoutImpl2>(this, grid_));
}

}
