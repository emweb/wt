/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WStackedWidget"
#include "Utils.h"

#include <iostream>

namespace Wt {

WStackedWidget::WStackedWidget(WContainerWidget *parent)
  : WContainerWidget(parent),
    currentIndex_(-1)
{ }

void WStackedWidget::addWidget(WWidget *widget)
{
  insertWidget(widgets_.size(), widget);
}

int WStackedWidget::count() const
{
  return widgets_.size();
}

int WStackedWidget::currentIndex() const
{
  return currentIndex_;
}

WWidget *WStackedWidget::currentWidget() const
{
  return widgets_[currentIndex_];
}

int WStackedWidget::indexOf(WWidget *widget) const
{
  return Utils::indexOf(widgets_, widget);
}

void WStackedWidget::insertWidget(int index, WWidget *widget)
{
  // do not bother inserting in correct place since this does not matter
  // as only one will be visible at each time
  WContainerWidget::addWidget(widget);

  widgets_.insert(widgets_.begin() + index, widget);

  if (currentIndex_ == -1)
    currentIndex_ = 0;
}

void WStackedWidget::removeWidget(WWidget *widget)
{
  Utils::erase(widgets_, widget);

  if (currentIndex_ >= (int)widgets_.size())
    setCurrentIndex(widgets_.size() - 1);
}

void WStackedWidget::removeChild(WWidget *child)
{
  removeWidget(child);
  WContainerWidget::removeChild(child);
}

WWidget *WStackedWidget::widget(int index) const
{
  return widgets_[index];
}

void WStackedWidget::setCurrentIndex(int index)
{
  currentIndex_ = index;

  for (unsigned i = 0; i < (unsigned)widgets_.size(); ++i)
    widgets_[i]->setHidden(currentIndex_ != (int)i);
}

void WStackedWidget::setCurrentWidget(WWidget *widget)
{
  setCurrentIndex(indexOf(widget));
}

DomElement *WStackedWidget::createDomElement(WApplication *app)
{
  setCurrentIndex(currentIndex_);
  return WContainerWidget::createDomElement(app);
}

void WStackedWidget::getDomChanges(std::vector<DomElement *>& result,
				   WApplication *app)
{
  setCurrentIndex(currentIndex_);
  return WContainerWidget::getDomChanges(result, app);
}

}
