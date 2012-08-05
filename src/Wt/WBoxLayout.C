/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cassert>

#include "Wt/WBoxLayout"
#include "Wt/WWebWidget"
#include "Wt/WWidgetItem"

namespace {
  class Spacer : public Wt::WWebWidget
  {
  public:
    Spacer() { setInline(false); }

  protected:
    virtual Wt::DomElementType domElementType() const {
      return Wt::DomElement_DIV;
    }
  };
}

namespace Wt {

WBoxLayout::WBoxLayout(Direction dir, WWidget *parent)
  : WLayout(),
    direction_(dir)
{
  if (parent)
    setLayoutInParent(parent);
}

void WBoxLayout::addItem(WLayoutItem *item)
{
  insertItem(count(), item, 0, 0);
}

void WBoxLayout::removeItem(WLayoutItem *item)
{
  int index = indexOf(item);

  if (index != -1) {
    switch (direction_) {
    case RightToLeft:
      index = grid_.columns_.size() - 1 - index;
    case LeftToRight:
      grid_.columns_.erase(grid_.columns_.begin() + index);
      grid_.items_[0].erase(grid_.items_[0].begin() + index);
      break;
    case BottomToTop:
      index = grid_.rows_.size() - 1 - index;
    case TopToBottom:
      grid_.rows_.erase(grid_.rows_.begin() + index);
      grid_.items_.erase(grid_.items_.begin() + index);
    }

    updateRemoveItem(item);
  }
}

WLayoutItem *WBoxLayout::itemAt(int index) const
{
  switch (direction_) {
  case RightToLeft:
    index = grid_.columns_.size() - 1 - index;    
  case LeftToRight:
    return grid_.items_[0][index].item_;
  case BottomToTop:
    index = grid_.rows_.size() - 1 - index;
  case TopToBottom:
    return grid_.items_[index][0].item_;
  }

  assert(false);
  return 0;
}

void WBoxLayout::clear()
{
  while (count()) {
    WLayoutItem *item = itemAt(count() - 1);
    clearLayoutItem(item);
  }
}

int WBoxLayout::count() const
{
  return grid_.rows_.size() * grid_.columns_.size();
}

void WBoxLayout::setDirection(Direction direction)
{
  if (direction_ != direction) {
    direction_ = direction;

    // FIXME: modify the grid
  }
}

void WBoxLayout::setSpacing(int size)
{
  grid_.horizontalSpacing_ = size;
  grid_.verticalSpacing_ = size;
}

void WBoxLayout::addWidget(WWidget *widget, int stretch,
			   WFlags<AlignmentFlag> alignment)
{
  insertWidget(count(), widget, stretch, alignment);
}

void WBoxLayout::addLayout(WLayout *layout, int stretch,
			   WFlags<AlignmentFlag> alignment)
{
  insertLayout(count(), layout, stretch, alignment);
}

void WBoxLayout::addSpacing(const WLength& size)
{
  insertSpacing(count(), size);
}

void WBoxLayout::addStretch(int stretch)
{
  insertStretch(count(), stretch);
}

void WBoxLayout::insertWidget(int index, WWidget *widget, int stretch,
			      WFlags<AlignmentFlag> alignment)
{
  if (widget->layoutSizeAware() && stretch == 0)
    stretch = -1;

  insertItem(index, new WWidgetItem(widget), stretch, alignment);
}

void WBoxLayout::insertLayout(int index, WLayout *layout, int stretch,
			      WFlags<AlignmentFlag> alignment)
{
  insertItem(index, layout, stretch, alignment);
}

void WBoxLayout::insertSpacing(int index, const WLength& size)
{
  WWidget *spacer = createSpacer(size);
  insertItem(index, new WWidgetItem(spacer), 0, 0);
}

void WBoxLayout::insertStretch(int index, int stretch)
{
  WWidget *spacer = createSpacer(WLength(0));
  insertItem(index, new WWidgetItem(spacer), stretch, 0);
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
  case RightToLeft:
    i = grid_.columns_.size() - 1 - i;
  case LeftToRight:
    grid_.columns_[i].stretch_ = stretch;
    break;
  case BottomToTop:
    i = grid_.rows_.size() - 1 - i;
  case TopToBottom:
    grid_.rows_[i].stretch_ = stretch;
  }
}

void WBoxLayout::insertItem(int index, WLayoutItem *item, int stretch,
			    WFlags<AlignmentFlag> alignment)
{
  switch (direction_) {
  case RightToLeft:
    index = grid_.columns_.size() - index;
  case LeftToRight:
    grid_.columns_.insert(grid_.columns_.begin() + index,
			  Impl::Grid::Section(stretch));
    if (grid_.items_.empty()) {
      grid_.items_.push_back(std::vector<Impl::Grid::Item>());
      grid_.rows_.push_back(Impl::Grid::Section());
      grid_.rows_[0].stretch_ = -1; // make height managed
    }
    grid_.items_[0].insert(grid_.items_[0].begin() + index,
			   Impl::Grid::Item(item, alignment));
    break;
  case BottomToTop:
    index = grid_.rows_.size() - index;
  case TopToBottom:
    if (grid_.columns_.empty()) {
      grid_.columns_.push_back(Impl::Grid::Section());
      grid_.columns_[0].stretch_ = -1; // make width managed
    }
    grid_.rows_.insert(grid_.rows_.begin() + index,
		       Impl::Grid::Section(stretch));
    grid_.items_.insert(grid_.items_.begin() + index,
			std::vector<Impl::Grid::Item>());
    grid_.items_[index].push_back(Impl::Grid::Item(item, alignment));
    break;
  }

  updateAddItem(item);
}

WWidget *WBoxLayout::createSpacer(const WLength& size)
{
  Spacer *spacer = new Spacer();
  if (size.toPixels() > 0) {
    if (direction_ == LeftToRight || direction_ == RightToLeft)
      spacer->setMinimumSize(size, WLength::Auto);
    else
      spacer->setMinimumSize(WLength::Auto, size);
  }

  return spacer;
}

void WBoxLayout::setResizable(int index, bool enabled,
			      const WLength& initialSize)
{
  switch (direction_) {
  case RightToLeft:
    index = grid_.columns_.size() - 1 - index;
  case LeftToRight:
    grid_.columns_[index].resizable_ = enabled;
    grid_.columns_[index].initialSize_ = initialSize;
    break;
  case BottomToTop:
    index = grid_.rows_.size() - 1 - index;
  case TopToBottom:
    grid_.rows_[index].resizable_ = enabled;
    grid_.rows_[index].initialSize_ = initialSize;
  }

  update(0);
}

bool WBoxLayout::isResizable(int index) const
{
  switch (direction_) {
  case RightToLeft:
    index = grid_.columns_.size() - 1 - index;
  case LeftToRight:
    return grid_.columns_[index].resizable_;
  case BottomToTop:
    index = grid_.rows_.size() - 1 - index;
  case TopToBottom:
    return grid_.rows_[index].resizable_;
  }

  return false;
}

}
