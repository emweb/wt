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
  setCompact(false);
  lastGroup_ = nullptr;
  if (alignmentFlag == AlignmentFlag::Right)
    button->setAttributeValue("style", "float:right;");
  impl_->addWidget(std::move(button));
}

void WToolBar::addWidget(std::unique_ptr<WWidget> widget,
			 AlignmentFlag alignmentFlag)
{
  setCompact(false);
  lastGroup_ = nullptr;
  if (alignmentFlag == AlignmentFlag::Right)
    widget->setAttributeValue("style", "float:right;");
  impl_->addWidget(std::move(widget));
}

std::unique_ptr<WWidget> WToolBar::removeWidget(WWidget *widget)
{
  WWidget *p = widget->parent();
  if (p == impl_)
    return impl_->removeWidget(widget);
  else {
    int i = impl_->indexOf(p);
    if (i >= 0) {
      WContainerWidget *cw = dynamic_cast<WContainerWidget *>(p);
      if (cw) {
	std::unique_ptr<WWidget> result = cw->removeWidget(widget);
	if (cw->count() == 0)
	  cw->parent()->removeWidget(cw);
	return result;
      }
    }
  }

  return std::unique_ptr<WWidget>();
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
	group->setStyleClass("btn-group");
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
    lastGroup_ = impl_->addWidget(cpp14::make_unique<WContainerWidget>());
    lastGroup_->addStyleClass("btn-group");
  }

  return lastGroup_;
}

int WToolBar::count() const
{
  if (compact_)
    return impl_->count();
  else {
    int result = 0;
    for (int i = 0; i < impl_->count(); ++i) {
      WWidget *w = impl_->widget(i);

      if (dynamic_cast<WSplitButton *>(w))
	++result;
      else {
	WContainerWidget *group = dynamic_cast<WContainerWidget *>(w);

	result += group->count();
      }
    }

    return result;
  }
}

WWidget *WToolBar::widget(int index) const
{
  if (compact_)
    return impl_->widget(index);
  else {
    int current = 0;
    for (int i = 0; i < impl_->count(); ++i) {
      WWidget *w = impl_->widget(i);

      if (dynamic_cast<WSplitButton *>(w)) {
	if (index == current)
	  return w;
	++current;
      } else {
	WContainerWidget *group = dynamic_cast<WContainerWidget *>(w);

	if (index < current + group->count())
	  return group->widget(index - current);

	current += group->count();
      }
    }

    return nullptr;
  }
}

}
