/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WText"
#include "Wt/WTabWidget"
#include "Wt/WMenu"
#include "Wt/WMenuItem"
#include "Wt/WStackedWidget"

#include "WebUtils.h"
#include "StdWidgetItemImpl.h"

namespace Wt {

WTabWidget::WTabWidget(WContainerWidget *parent)
  : WCompositeWidget(parent),
    currentChanged_(this)
{
  create();
}

void WTabWidget::create()
{
  setImplementation(layout_ = new WContainerWidget());

  menu_ = new WMenu(new WStackedWidget());

  layout_->addWidget(menu_);
  layout_->addWidget(menu_->contentsStack());

  setJavaScriptMember(WT_RESIZE_JS, StdWidgetItemImpl::secondResizeJS());
  setJavaScriptMember(WT_GETPS_JS, StdWidgetItemImpl::secondGetPSJS());

  menu_->itemSelected().connect(this, &WTabWidget::onItemSelected);
  menu_->itemClosed().connect(this, &WTabWidget::onItemClosed);
}

WMenuItem *WTabWidget::addTab(WWidget *child, const WString& label,
			      LoadPolicy loadPolicy)
{
  WMenuItem::LoadPolicy policy = WMenuItem::PreLoading;
  switch (loadPolicy) {
  case PreLoading: policy = WMenuItem::PreLoading; break;
  case LazyLoading: policy = WMenuItem::LazyLoading; break;
  }

  WMenuItem *result = new WMenuItem(label, child, policy);

  menu_->addItem(result);

  contentsWidgets_.push_back(child);

  return result;
}

void WTabWidget::removeTab(WWidget *child)
{
  int tabIndex = indexOf(child);

  if (tabIndex != -1) {
    contentsWidgets_.erase(contentsWidgets_.begin() + tabIndex);

    WMenuItem *item = menu_->itemAt(tabIndex);
    menu_->removeItem(item);

    item->takeContents();
    delete item;
  }
}

int WTabWidget::count() const
{
  return contentsWidgets_.size();
}

WWidget *WTabWidget::widget(int index) const
{
  return contentsWidgets_[index];
}

int WTabWidget::indexOf(WWidget *widget) const
{
  return Utils::indexOf(contentsWidgets_, widget);
}

void WTabWidget::setCurrentIndex(int index)
{
  menu_->select(index);
}

int WTabWidget::currentIndex() const
{
  return menu_->currentIndex();
}

void WTabWidget::setCurrentWidget(WWidget *widget)
{
  setCurrentIndex(indexOf(widget));
}

WWidget *WTabWidget::currentWidget() const
{
  return menu_->currentItem()->contents();
}

void WTabWidget::setTabEnabled(int index, bool enable)
{
  menu_->setItemDisabled(index, !enable);
}

bool WTabWidget::isTabEnabled(int index) const
{
  return !menu_->isItemDisabled(index);
}

void WTabWidget::setTabHidden(int index, bool hidden)
{
  menu_->setItemHidden(index, hidden);
}

bool WTabWidget::isTabHidden(int index) const
{
  return menu_->isItemHidden(index);
}

void WTabWidget::setTabCloseable(int index, bool closeable)
{
  menu_->itemAt(index)->setCloseable(closeable);
}

bool WTabWidget::isTabCloseable(int index)
{
  return menu_->itemAt(index)->isCloseable();
}

void WTabWidget::closeTab(int index)
{
  setTabHidden(index, true);
  tabClosed_.emit(index);
}

void WTabWidget::setTabText(int index, const WString& label)
{
  WMenuItem *item = menu_->itemAt(index);
  item->setText(label);
}

const WString& WTabWidget::tabText(int index) const
{
  WMenuItem *item = menu_->itemAt(index);
  return item->text();
}

void WTabWidget::setTabToolTip(int index, const WString& tip)
{
  WMenuItem *item = menu_->itemAt(index);
  item->setToolTip(tip);
}

WString WTabWidget::tabToolTip(int index) const
{
  WMenuItem *item = menu_->itemAt(index);
  return item->toolTip();
}

bool WTabWidget::internalPathEnabled() const
{
  return menu_->internalPathEnabled();
}

void WTabWidget::setInternalPathEnabled(const std::string& basePath)
{
  menu_->setInternalPathEnabled(basePath);
}

const std::string& WTabWidget::internalBasePath() const
{
  return menu_->internalBasePath();
}

void WTabWidget::setInternalBasePath(const std::string& path)
{
  menu_->setInternalBasePath(path);
}

void WTabWidget::onItemSelected(WMenuItem *item)
{
  currentChanged_.emit(menu_->currentIndex());
}

void WTabWidget::onItemClosed(WMenuItem *item)
{
  closeTab(menu_->indexOf(item));
}

WStackedWidget *WTabWidget::contentsStack() const
{
  return menu_->contentsStack();
}

}
