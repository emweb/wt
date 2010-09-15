
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAnchor"
#include "Wt/WBreak"
#include "Wt/WText"
#include "Wt/WApplication"
#include "Wt/WTabWidget"
#include "Wt/WMenu"
#include "Wt/WMenuItem"
#include "Wt/WStackedWidget"
#include "Wt/WVBoxLayout"

#include "Utils.h"

namespace Wt {

namespace {

class TabWidgetItem : public WMenuItem
{
public:
  TabWidgetItem(const WString& text, WWidget *contents, LoadPolicy loadPolicy);

protected:
  virtual WWidget *createItemWidget();
  virtual void updateItemWidget(WWidget *itemWidget);
  virtual SignalBase& activateSignal();
};

TabWidgetItem::TabWidgetItem(const WString& text, WWidget *contents,
			     LoadPolicy loadPolicy)
  : WMenuItem(text, contents, loadPolicy)
{ }

void TabWidgetItem::updateItemWidget(WWidget *itemWidget)
{
  if (!isCloseable()) {
    WContainerWidget *c = dynamic_cast<WContainerWidget *>(itemWidget);
    WWidget *label = 0;
    if (!isDisabled())
      label = dynamic_cast<WAnchor *>(c->children()[0]);
    else
      label = dynamic_cast<WText *>(c->children()[0]);

    WMenuItem::updateItemWidget(label);
  }
  else
    WMenuItem::updateItemWidget(itemWidget);
}

WWidget *TabWidgetItem::createItemWidget()
{
  if (!isCloseable()) {
    WContainerWidget *c = new WContainerWidget();
    c->setInline(true);
    c->addWidget(WMenuItem::createItemWidget());

    return c;
  } else
    return WMenuItem::createItemWidget();
}

SignalBase& TabWidgetItem::activateSignal()
{
  WContainerWidget *c = dynamic_cast<WContainerWidget *>(itemWidget());

  return dynamic_cast<WInteractWidget *>(c->children()[0])->clicked();
}

}

WTabWidget::WTabWidget(WContainerWidget *parent)
  : WCompositeWidget(parent),
    currentChanged_(this)
{
  create(AlignJustify);
}

WTabWidget::WTabWidget(WFlags<AlignmentFlag> layoutAlignment,
		       WContainerWidget *parent)
  : WCompositeWidget(parent),
    currentChanged_(this)
{
  create(layoutAlignment);
}

void WTabWidget::create(WFlags<AlignmentFlag> layoutAlignment)
{
  setImplementation(layout_ = new WContainerWidget());

  WT_DEBUG( setObjectName("WTabWidget") );

  menu_ = new WMenu(new WStackedWidget(), Horizontal);
  menu_->setRenderAsList(true);

  WBreak *clear = new WBreak();
  clear->setStyleClass("Wt-tabs-clear");

  WContainerWidget *menuDiv = new WContainerWidget();
  menuDiv->setStyleClass("Wt-tabs");
  menuDiv->addWidget(menu_);
  menuDiv->addWidget(clear);

  layout_->addWidget(menuDiv);
  layout_->addWidget(menu_->contentsStack());

  setJavaScriptMember
    (WT_RESIZE_JS, std::string() +
     "function(self, w, h) {"
     """self.style.height= h + 'px';"
     """var c = self.firstChild;"
     """var t = self.lastChild;"
     """h -= c.offsetHeight;"
     """if (h > 0)"
     ""  "t." + WT_RESIZE_JS + "(t, w, h);"
     "};");

  menu_->itemSelected().connect(this, &WTabWidget::onItemSelected);
}

WMenuItem *WTabWidget::addTab(WWidget *child, const WString& label,
			      LoadPolicy loadPolicy)
{
  WMenuItem::LoadPolicy policy = WMenuItem::PreLoading;
  switch (loadPolicy) {
  case PreLoading: policy = WMenuItem::PreLoading; break;
  case LazyLoading: policy = WMenuItem::LazyLoading; break;
  }

  WMenuItem *result = new TabWidgetItem(label, child, policy);

  menu_->addItem(result);

  contentsWidgets_.push_back(child);

  return result;
}

void WTabWidget::removeTab(WWidget *child)
{
  int tabIndex = indexOf(child);

  if (tabIndex != -1) {
    contentsWidgets_.erase(contentsWidgets_.begin() + tabIndex);

    WMenuItem *item = menu_->items()[tabIndex];
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
  TabWidgetItem *item = dynamic_cast<TabWidgetItem *>(menu_->items()[index]);
  item->setDisabled(!enable);
}

bool WTabWidget::isTabEnabled(int index) const
{
  TabWidgetItem *item = dynamic_cast<TabWidgetItem *>(menu_->items()[index]);
  return !item->isDisabled();
}

void WTabWidget::setTabHidden(int index, bool hidden)
{
  TabWidgetItem *item = dynamic_cast<TabWidgetItem *>(menu_->items()[index]);
  item->setHidden(hidden);
}

bool WTabWidget::isTabHidden(int index) const
{
  TabWidgetItem *item = dynamic_cast<TabWidgetItem *>(menu_->items()[index]);
  return item->isHidden();
}

void WTabWidget::setTabCloseable(int index, bool closeable)
{
  TabWidgetItem *item = dynamic_cast<TabWidgetItem *>(menu_->items()[index]);
  item->setCloseable(closeable);
}

bool WTabWidget::isTabCloseable(int index)
{
  TabWidgetItem *item = dynamic_cast<TabWidgetItem *>(menu_->items()[index]);
  return item->isCloseable();
}

void WTabWidget::closeTab(int index)
{
  setTabHidden(index, true);
  tabClosed_.emit(index);
}

void WTabWidget::setTabText(int index, const WString& label)
{
  TabWidgetItem *item = dynamic_cast<TabWidgetItem *>(menu_->items()[index]);
  item->setText(label);
}

const WString& WTabWidget::tabText(int index) const
{
  TabWidgetItem *item = dynamic_cast<TabWidgetItem *>(menu_->items()[index]);
  return item->text();
}

void WTabWidget::setTabToolTip(int index, const WString& tip)
{
  TabWidgetItem *item = dynamic_cast<TabWidgetItem *>(menu_->items()[index]);
  item->setToolTip(tip);
}

const WString& WTabWidget::tabToolTip(int index) const
{
  TabWidgetItem *item = dynamic_cast<TabWidgetItem *>(menu_->items()[index]);
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

}
