/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WStackedWidget"

#include "StdGridLayoutImpl.h"

#include <iostream>

namespace Wt {

WStackedWidget::WStackedWidget(WContainerWidget *parent)
  : WContainerWidget(parent),
    currentIndex_(-1)
{
  WT_DEBUG( setObjectName("WStackedWidget") );
  setJavaScriptMember(WT_RESIZE_JS, StdGridLayoutImpl::childrenResizeJS());
}

void WStackedWidget::addWidget(WWidget *widget)
{
  WContainerWidget::addWidget(widget);

  if (currentIndex_ == -1)
    currentIndex_ = 0;
}

int WStackedWidget::currentIndex() const
{
  return currentIndex_;
}

WWidget *WStackedWidget::currentWidget() const
{
  if (currentIndex_ >= 0)
    return widget(currentIndex_);
  else
    return 0;
}

void WStackedWidget::insertWidget(int index, WWidget *widget)
{
  WContainerWidget::insertWidget(index, widget);

  if (currentIndex_ == -1)
    currentIndex_ = 0;
}

void WStackedWidget::removeChild(WWidget *child)
{
  WContainerWidget::removeChild(child);

  if (currentIndex_ >= count())
    setCurrentIndex(count() - 1);
}

void WStackedWidget::setCurrentIndex(int index)
{
  currentIndex_ = index;

  for (int i = 0; i < count(); ++i)
    widget(i)->setHidden(currentIndex_ != i);
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
  WContainerWidget::getDomChanges(result, app);
}

}
