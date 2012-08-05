/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/Ext/Splitter"
#include "Wt/Ext/SplitterHandle"
#include "Wt/WException"
#include "Wt/WTable"

#include "DomElement.h"

namespace Wt {
  namespace Ext {

Splitter::Splitter(WContainerWidget *parent)
  : WContainerWidget(parent),
    orientation_(Horizontal),
    handleWidth_(4)
{
  WContainerWidget::addWidget(container_ = new WContainerWidget());

  container_->resize(WLength(100, WLength::Percentage),
		     WLength(100, WLength::Percentage));
  container_->setPositionScheme(Relative);

  setHideWithOffsets();
}

Splitter::Splitter(Orientation orientation, WContainerWidget *parent)
  : WContainerWidget(parent),
    orientation_(orientation),
    handleWidth_(4)
{
  WContainerWidget::addWidget(container_ = new WContainerWidget());

  container_->resize(WLength(100, WLength::Percentage),
		     WLength(100, WLength::Percentage));
  container_->setPositionScheme(Relative);

  setHideWithOffsets();
}

void Splitter::setOrientation(Orientation orientation)
{
  orientation_ = orientation;
}

void Splitter::setHandleWidth(int pixels)
{
  handleWidth_ = pixels;
}

void Splitter::addWidget(WWidget *widget)
{
  insertWidget(children_.size(), widget);
}

void Splitter::insertBefore(WWidget *widget, WWidget *before)
{
  insertWidget(indexOf(before), widget);
}

void Splitter::insertWidget(int index, WWidget *widget)
{
  children_.insert(children_.begin() + index, widget);

  if (children_.size() != 1) {
    SplitterHandle *handle = new SplitterHandle(this);

    if ((int)children_.size() - 1 == index) {
      handles_.insert(handles_.begin() + (index - 1), handle);
      container_->insertWidget(index * 2 - 1, handle);
    }

    container_->insertWidget(index * 2, widget);

    if ((int)children_.size() - 1 != index) {
      handles_.insert(handles_.begin() + index, handle);
      container_->insertWidget(index * 2 + 1, handle);
    }

  } else
    container_->insertWidget(0, widget);

  widget->setPositionScheme(Absolute);
  widget->setInline(false);
}

SplitterHandle *Splitter::handle(int index) const
{
  if (index == 0)
    return 0;
  else
    return handles_[index - 1];
}

WWidget *Splitter::widgetBefore(const SplitterHandle *handle) const
{
  std::size_t index
    = std::find(handles_.begin(), handles_.end(), handle) - handles_.begin();

  return children_[index];
}

WWidget *Splitter::widgetAfter(const SplitterHandle *handle) const
{
  std::size_t index
    = std::find(handles_.begin(), handles_.end(), handle) - handles_.begin();

  return children_[index + 1];
}

SplitterHandle *Splitter::splitterBefore(const SplitterHandle *handle) const
{
  std::size_t index
    = std::find(handles_.begin(), handles_.end(), handle) - handles_.begin();

  if (index > 0)
    return handles_[index - 1];
  else
    return 0;
}

SplitterHandle *Splitter::splitterAfter(const SplitterHandle *handle) const
{
  std::size_t index
    = std::find(handles_.begin(), handles_.end(), handle) - handles_.begin();

  if (index == handles_.size() - 1)
    return 0;
  else
    return handles_[index + 1];
}

DomElement *Splitter::createDomElement(WApplication *app)
{
  /*
   * Adjust margins & offsets
   */
  int x = 0;

  Side side = orientation_ == Horizontal ? Left : Top;

  for (unsigned i = 0; i < children_.size(); ++i) {
    if (i != 0) {
      handles_[i-1]->setOffsets(x, side);

      if (orientation_ == Horizontal)
	handles_[i-1]->resize(WLength(handleWidth_), WLength());
      else
	handles_[i-1]->resize(WLength(), WLength(handleWidth_));

      x += handleWidth_;
      children_[i]->setMargin(WLength(x), side);
    }

    WLength l = orientation_ == Horizontal
      ? children_[i]->width()
      : children_[i]->height();

    if (l.isAuto())
      throw WException("Splitter requires all widgets to have their "
		       "width or height set using WWidget::resize().");

    x += (int)l.value();
  }

  DomElement *result = WContainerWidget::createDomElement(app);
  result->setProperty(Wt::PropertyStyleOverflowX, "hidden");

  return result;
}

  }
}
