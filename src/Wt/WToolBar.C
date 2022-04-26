/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WContainerWidget.h"
#include "Wt/WLogger.h"
#include "Wt/WPushButton.h"
#include "Wt/WSplitButton.h"
#include "Wt/WToolBar.h"

#include "web/WebUtils.h"

#include <algorithm>

namespace Wt {

LOGGER("WToolBar");

WToolBar::WToolBar()
  : WCompositeWidget(),
    compact_(true),
    lastGroup_(nullptr)
{
  setImplementation(std::unique_ptr<WContainerWidget>(impl_ = new WContainerWidget()));
  setStyleClass("btn-group");
}

void WToolBar::setOrientation(Orientation orientation)
{
  if (orientation == Orientation::Vertical)
    addStyleClass("btn-group-vertical");
  else
    removeStyleClass("btn-group-vertical");
}

void WToolBar::addButton(std::unique_ptr<WPushButton> button,
                         AlignmentFlag alignmentFlag)
{
  widgets_.push_back(button.get());

  if (compact_){
    if (alignmentFlag == AlignmentFlag::Right)
      button->setAttributeValue("style", "float:right;");
    impl_->addWidget(std::move(button));
  } else{
    if (alignmentFlag == AlignmentFlag::Right)
      lastGroup()->setAttributeValue("style", "float:right;");
    lastGroup()->addWidget(std::move(button));
  }
}

void WToolBar::addButton(std::unique_ptr<WSplitButton> button,
                         AlignmentFlag alignmentFlag)
{
  widgets_.push_back(button.get());

  setCompact(false);
  lastGroup_ = nullptr;
  if (alignmentFlag == AlignmentFlag::Right)
    button->setAttributeValue("style", "float:right;");
  impl_->addWidget(std::move(button));
}

void WToolBar::addWidget(std::unique_ptr<WWidget> widget,
                         AlignmentFlag alignmentFlag)
{
  widgets_.push_back(widget.get());

  setCompact(false);
  lastGroup_ = nullptr;
  if (alignmentFlag == AlignmentFlag::Right)
    widget->setAttributeValue("style", "float:right;");
  impl_->addWidget(std::move(widget));
}

std::unique_ptr<WWidget> WToolBar::removeWidget(WWidget *widget)
{
  auto idx = Utils::indexOf(widgets_, widget);
  if (idx != -1) {
    auto parent = static_cast<WContainerWidget*>(widget->parent());

    auto retval = parent->removeWidget(widget);
    if (parent != impl_  && parent->count() == 0)
      impl_->removeWidget(parent);
    widgets_.erase(widgets_.begin() + idx);

    return retval;
  } else
    return nullptr;
}

void WToolBar::addSeparator()
{
  setCompact(false);
  lastGroup_ = nullptr;
}

void WToolBar::setCompact(bool compact)
{
  if (compact != compact_) {
    compact_ = compact;

    if (compact) {
      if (impl_->count() > 0)
        LOG_INFO("setCompact(true): not implemented");
      setStyleClass("btn-group");
    } else {
      setStyleClass("btn-toolbar");
      if (impl_->count() > 0) {
        std::unique_ptr<WContainerWidget> group(new WContainerWidget());
        group->setStyleClass("btn-group me-2");
        while (impl_->count() > 0) {
          auto w = impl_->removeWidget(impl_->widget(0));
          group->addWidget(std::move(w));
        }
        lastGroup_ = group.get();
        impl_->addWidget(std::move(group));
      }
    }
  }
}

WContainerWidget *WToolBar::lastGroup()
{
  if (!lastGroup_) {
    lastGroup_ = impl_->addWidget(std::make_unique<WContainerWidget>());
    lastGroup_->addStyleClass("btn-group me-2");
  }

  return lastGroup_;
}

int WToolBar::count() const
{
  return widgets_.size();
}

WWidget *WToolBar::widget(int index) const
{
  if (index < widgets_.size())
    return widgets_[index];
  else
    return nullptr;
}

}
