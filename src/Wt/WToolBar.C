/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WContainerWidget"
#include "Wt/WPushButton"
#include "Wt/WSplitButton"
#include "Wt/WToolBar"

namespace Wt {

LOGGER("WToolBar");

WToolBar::WToolBar(WContainerWidget *parent)
  : WCompositeWidget(parent),
    compact_(true),
    lastGroup_(0)
{
  setImplementation(impl_ = new WContainerWidget());
  setStyleClass("btn-group");
}

void WToolBar::setOrientation(Orientation orientation){
  if(orientation == Vertical)
    addStyleClass("btn-group-vertical");
  else
    removeStyleClass("btn-group-vertical");
}

void WToolBar::addButton(WPushButton *button, AlignmentFlag alignmentFlag)
{
  if (compact_){
    impl_->addWidget(button);
    if(alignmentFlag == AlignRight)
      button->setAttributeValue("style", "float:right;");
  } else{
    if(alignmentFlag == AlignRight)
      lastGroup()->setAttributeValue("style", "float:right;");
    lastGroup()->addWidget(button);
  }
}

void WToolBar::addButton(WSplitButton *button, AlignmentFlag alignmentFlag)
{
  setCompact(false);
  lastGroup_ = 0;
  if(alignmentFlag == AlignRight)
    button->setAttributeValue("style", "float:right;");
  impl_->addWidget(button);
}

void WToolBar::addWidget(WWidget *widget, AlignmentFlag alignmentFlag)
{
  setCompact(false);
  lastGroup_ = 0;
  if(alignmentFlag == AlignRight)
    widget->setAttributeValue("style", "float:right;");
  impl_->addWidget(widget);
}

void WToolBar::addSeparator()
{
  setCompact(false);
  lastGroup_ = 0;
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
      WContainerWidget *group = new WContainerWidget();
      group->setStyleClass("btn-group");
      while (impl_->count() > 0) {
	WWidget *w = impl_->widget(0);
	impl_->removeWidget(w);
	group->addWidget(w);
      }
      impl_->addWidget(group);
      lastGroup_ = group;
    }
  }
}

WContainerWidget *WToolBar::lastGroup()
{
  if (!lastGroup_) {
    lastGroup_ = new WContainerWidget(impl_);
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

    return 0;
  }
}

}
