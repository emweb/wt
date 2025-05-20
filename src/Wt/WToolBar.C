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
    orientation_(Orientation::Horizontal),
    lastGroup_(nullptr),
    nextUnrenderedGroup_(0)
{
  setImplementation(std::unique_ptr<WContainerWidget>(impl_ = new WContainerWidget()));
}

void WToolBar::setOrientation(Orientation orientation)
{
  if (orientation_ != orientation) {
    orientation_ = orientation;
    flags_.set(BIT_ORIENTATION_CHANGED);
    scheduleRender();
  }
}

WPushButton *WToolBar::addButton(std::unique_ptr<WPushButton> button,
                                 AlignmentFlag alignmentFlag)
{
  auto result = button.get();

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

  return result;
}

WSplitButton *WToolBar::addButton(std::unique_ptr<WSplitButton> button,
                                  AlignmentFlag alignmentFlag)
{
  auto result = button.get();

  widgets_.push_back(button.get());

  setCompact(false);
  lastGroup_ = nullptr;
  if (alignmentFlag == AlignmentFlag::Right)
    button->setAttributeValue("style", "float:right;");
  impl_->addWidget(std::move(button));

  return result;
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
    if (parent != impl_  && parent->count() == 0) {
      impl_->removeWidget(parent);
      if (lastGroup_ == parent) {
        lastGroup_ = nullptr;
      }
    }
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
    } else {
      if (impl_->count() > 0) {
        std::unique_ptr<WContainerWidget> group(new WContainerWidget());
        while (impl_->count() > 0) {
          auto w = impl_->removeWidget(impl_->widget(0));
          group->addWidget(std::move(w));
        }
        lastGroup_ = group.get();
        impl_->addWidget(std::move(group));
        flags_.set(BIT_MULTIPLE_GROUPS);
      }
    }

    flags_.set(BIT_COMPACT_CHANGED);
    scheduleRender();
  }
}

void WToolBar::render(WFlags<RenderFlag> flags)
{
  bool all = flags.test(RenderFlag::Full);
  if (isThemeStyleEnabled()) {
    if (flags_.test(BIT_COMPACT_CHANGED) || all) {
      if (compact_) {
        removeStyleClass("btn-toolbar");
        addStyleClass("btn-group");
      } else {
        removeStyleClass("btn-group");
        addStyleClass("btn-toolbar");
      }

      flags_.reset(BIT_COMPACT_CHANGED);
    }

    if (flags_.test(BIT_MULTIPLE_GROUPS)) {
      for (int i = nextUnrenderedGroup_; i < impl_->count(); ++i) {
        impl_->children()[i]->addStyleClass("btn-group me-2");
      }

      nextUnrenderedGroup_ = impl_->count();
    }

    if (flags_.test(BIT_ORIENTATION_CHANGED) || all) {
      if (orientation_ == Orientation::Vertical) {
        addStyleClass("btn-group-vertical");
      } else {
        removeStyleClass("btn-group-vertical");
      }

      flags_.reset(BIT_ORIENTATION_CHANGED);
    }
  }
}

WContainerWidget *WToolBar::lastGroup()
{
  if (!lastGroup_) {
    lastGroup_ = impl_->addWidget(std::make_unique<WContainerWidget>());
    scheduleRender();
  }

  return lastGroup_;
}

int WToolBar::count() const
{
  return widgets_.size();
}

WWidget *WToolBar::widget(int index) const
{
  if (index < static_cast<int>(widgets_.size()))
    return widgets_[index];
  else
    return nullptr;
}

}
