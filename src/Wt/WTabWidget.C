/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAnchor.h"
#include "Wt/WApplication.h"
#include "Wt/WText.h"
#include "Wt/WTabWidget.h"
#include "Wt/WMenu.h"
#include "Wt/WMenuItem.h"
#include "Wt/WStackedWidget.h"

#include "WebUtils.h"
#include "StdWidgetItemImpl.h"

namespace Wt {

WTabWidget::WTabWidget()
{
  create();
}

void WTabWidget::create()
{
  layout_ = new WContainerWidget();
  setImplementation(std::unique_ptr<WWidget>(layout_));

  std::unique_ptr<WStackedWidget> stack(new WStackedWidget());
  menu_ = new WMenu(stack.get());
  layout_->addWidget(std::unique_ptr<WWidget>(menu_));
  layout_->addWidget(std::move(stack));

  setJavaScriptMember(WT_RESIZE_JS, StdWidgetItemImpl::secondResizeJS());
  setJavaScriptMember(WT_GETPS_JS, StdWidgetItemImpl::secondGetPSJS());

  menu_->itemSelected().connect(this, &WTabWidget::onItemSelected);
  menu_->itemClosed().connect(this, &WTabWidget::onItemClosed);
}

WMenuItem *WTabWidget::addTab(std::unique_ptr<WWidget> child,
			      const WString& label,
			      ContentLoading loadPolicy)
{
  return insertTab(count(), std::move(child), label, loadPolicy);
}

WMenuItem *WTabWidget::insertTab(int index,
                                 std::unique_ptr<WWidget> child,
                                 const WString &label,
                                 ContentLoading loadPolicy)
{
  contentsWidgets_.insert(contentsWidgets_.begin() + index, child.get());
  std::unique_ptr<WMenuItem> item
    (new WMenuItem(label, std::move(child), loadPolicy));
  WMenuItem *result = item.get();
  menu_->insertItem(index, std::move(item));
  return result;
}

std::unique_ptr<WWidget> WTabWidget::removeTab(WWidget *child)
{
  int tabIndex = indexOf(child);

  if (tabIndex != -1) {
    contentsWidgets_.erase(contentsWidgets_.begin() + tabIndex);

    WMenuItem *item = menu_->itemAt(tabIndex);
    std::unique_ptr<WWidget> result = item->removeContents();
    menu_->removeItem(item);
    return result;
  } else
    return std::unique_ptr<WWidget>();
}

int WTabWidget::count() const
{
  return contentsWidgets_.size();
}

WWidget *WTabWidget::widget(int index) const
{
  return contentsWidgets_[index];
}

WMenuItem *WTabWidget::itemAt(int index) const
{
  return menu_->itemAt(index);
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

WMenuItem *WTabWidget::currentItem() const
{
  return menu_->currentItem();
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

WString WTabWidget::tabText(int index) const
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

void WTabWidget::setOverflow(Overflow value,
	WFlags<Orientation> orientation)
{
  layout_->setOverflow(value, orientation);
}

}
