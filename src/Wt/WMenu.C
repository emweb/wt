/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WBreak"
#include "Wt/WLogger"
#include "Wt/WMenu"
#include "Wt/WMenuItem"
#include "Wt/WStackedWidget"
#include "Wt/WTable"
#include "Wt/WTableCell"
#include "Wt/WText"

#include "WtException.h"
#include "Utils.h"

namespace Wt {

WMenu::WMenu(Orientation orientation, WContainerWidget *parent)
  : WCompositeWidget(parent),
    contentsStack_(0),
    orientation_(orientation),
    internalPathEnabled_(false),
    emitPathChange_(false),
    itemSelected_(this),
    itemSelectRendered_(this),
    itemClosed_(this),
    current_(-1)
{
  setRenderAsList(false);
}

WMenu::WMenu(WStackedWidget *contentsStack, Orientation orientation,
	     WContainerWidget *parent)
  : WCompositeWidget(parent),
    contentsStack_(contentsStack),
    orientation_(orientation),
    internalPathEnabled_(false),
    emitPathChange_(false),
    itemSelected_(this),
    itemSelectRendered_(this),
    itemClosed_(this),
    current_(-1)
{
  setRenderAsList(false);
}

void WMenu::setRenderAsList(bool enable)
{
  if (enable) {
    WContainerWidget *c = new WContainerWidget();
    //WBreak *w = new WBreak(c);
    //w->setAttributeValue("style","clear:both;");
    c->setList(true);

    setImplementation(impl_ = c);
  } else {
    setImplementation(impl_ = new WTable());
  }

  renderAsList_ = enable;
}

WMenu::~WMenu()
{
  for (unsigned i = 0; i < items_.size(); ++i) {
    items_[i]->setMenu(0);
    delete items_[i];
  }
}

void WMenu::setInternalPathEnabled(const std::string& basePath)
{
  if (!internalPathEnabled_) {
    internalPathEnabled_ = true;

    WApplication *app = wApp;

    basePath_
      = Utils::terminate(basePath.empty() ? app->internalPath() : basePath,
			 '/');

    app->internalPathChanged().connect(this, &WMenu::internalPathChanged);

    previousInternalPath_ = app->internalPath();

#ifdef WT_WITH_OLD_INTERNALPATH_API
    if (app->oldInternalPathAPI())
      internalPathChanged(basePath_);
    else
#endif // WT_WITH_OLD_INTERNALPATH_API
      internalPathChanged(app->internalPath());

    updateItems();
  }
}

void WMenu::enableAjax()
{
  for (unsigned i = 0; i < items_.size(); ++i) {
    WMenuItem *item = items_[i];
    item->enableAjax();
  }

  WCompositeWidget::enableAjax();
}

void WMenu::setInternalBasePath(const std::string& basePath)
{
  std::string bp = Utils::terminate(basePath, '/');
  if (basePath_ != bp) {
    basePath_ = bp;

    if (internalPathEnabled_) {
      WApplication *app = wApp;
      previousInternalPath_ = app->internalPath();
#ifdef WT_WITH_OLD_INTERNALPATH_API
      if (app->oldInternalPathAPI())
	internalPathChanged(basePath_);
      else
#endif // WT_WITH_OLD_INTERNALPATH_API
	internalPathChanged(app->internalPath());

      updateItems();
    }
  }
}

void WMenu::updateItems()
{
  for (unsigned i = 0; i < items_.size(); ++i) {
    WMenuItem *item = items_[i];
    item->updateItemWidget(item->itemWidget());
    item->resetLearnedSlots();
  }
}

WMenuItem *WMenu::addItem(const WString& name, WWidget *contents,
			  WMenuItem::LoadPolicy policy)
{
  return addItem(new WMenuItem(name, contents, policy));
}

WMenuItem *WMenu::addItem(WMenuItem *item)
{
  item->setMenu(this);
  items_.push_back(item);

  if (renderAsList_) {
    WContainerWidget *p = dynamic_cast<WContainerWidget *>(impl_);
    WContainerWidget *li = new WContainerWidget();
    p->insertWidget(p->count()/* - 1 */, li);
    li->addWidget(item->itemWidget());
  } else {
    WTable *layout = dynamic_cast<WTable *>(impl_);
    WTableCell *parent
      = layout->elementAt((orientation_ == Vertical) ? items_.size() - 1 : 0, 0);

    WWidget *w = item->itemWidget();
    parent->addWidget(w);

    // separate horizontal items so wrapping will occur inbetween items.
    if (orientation_ == Horizontal) {
      w->setInline(true);
      new WText(" ", parent);
    }
  }

  for (unsigned i = 0; i < items_.size(); ++i)
    items_[i]->resetLearnedSlots();

  if (contentsStack_) {
    WWidget *contents = item->contents();
    if (contents)
      contentsStack_->addWidget(contents);

    if (contentsStack_->count() == 1) {
      current_ = 0;
      if (contents)
	contentsStack_->setCurrentWidget(contents);

      items_[0]->renderSelected(true);
      items_[0]->loadContents();
    } else
      item->renderSelected(false);
  } else
    item->renderSelected(false);

  item->renderHidden(item->isHidden());

  if (internalPathEnabled_) {
    WApplication *app = wApp;

    if (app->internalPathMatches(basePath_ + item->pathComponent()))
      select(items_.size() - 1, false);
  }

  return item;
}

void WMenu::removeItem(WMenuItem *item)
{
  int itemIndex = indexOf(item);

  if (itemIndex != -1) {
    items_.erase(items_.begin() + itemIndex);

    if (renderAsList_) {
      WContainerWidget *li
	= dynamic_cast<WContainerWidget *>(item->itemWidget()->parent());
      li->removeWidget(item->itemWidget());
      delete li;
    } else {
      WTableCell *parent =
        dynamic_cast<WTableCell *>(item->itemWidget()->parent());

      if (orientation_ == Horizontal) {
        WWidget *itemWidget = item->itemWidget();
        WWidget *separator = parent->widget(parent->indexOf(itemWidget) + 1);

        parent->removeWidget(itemWidget);
        delete separator;
      } else {
        WTable *table = parent->table();
        parent->removeWidget(item->itemWidget());
        table->deleteRow(parent->row());
      }
    }

    if (contentsStack_ && item->contents())
      contentsStack_->removeWidget(item->contents());

    item->setMenu(0);

    if (itemIndex <= current_ && current_ >= 0)
      --current_;

    for (unsigned i = 0; i < items_.size(); ++i)
      items_[i]->resetLearnedSlots();

    select(current_, true);
  }
}

void WMenu::select(int index)
{
  select(index, true);
}

void WMenu::select(int index, bool changePath)
{
  selectVisual(index, changePath);

  if (index != -1) {
    if (isItemHidden(index))
      setItemHidden(index, false);

    items_[index]->loadContents();
    itemSelected_.emit(items_[current_]);

    if (changePath && emitPathChange_) {
      WApplication *app = wApp;
      app->internalPathChanged().emit(app->internalPath());
      emitPathChange_ = false;
    }
  }
}

void WMenu::selectVisual(int index, bool changePath)
{
  previousCurrent_ = current_;

  if (contentsStack_)
    previousStackIndex_ = contentsStack_->currentIndex();

  current_ = index;

  if (changePath && internalPathEnabled_ && current_ != -1) {
    WApplication *app = wApp;
    previousInternalPath_ = app->internalPath();

    std::string newPath = basePath_ + items_[current_]->pathComponent();
    if (newPath != app->internalPath())
      emitPathChange_ = true;

    // The change is emitted in select()
    app->setInternalPath(newPath);
  }

  for (unsigned i = 0; i < items_.size(); ++i)
    items_[i]->renderSelected((int)i == current_);

  if (index == -1)
    return;

  if (contentsStack_) {
    WWidget *contents = items_[current_]->contents();
    if (contents)
      contentsStack_->setCurrentWidget(contents);
  }

  itemSelectRendered_.emit(items_[current_]);
}

void WMenu::setItemHidden(int index, bool hidden)
{
  items_[index]->setHidden(hidden);
}

void WMenu::doSetHiddenItem(int index, bool hidden)
{
  if (hidden) {
    int nextItem = nextAfterHide(index);
    if (nextItem != current_)
      select(nextItem);
  }

  items_[index]->renderHidden(hidden);
}

void WMenu::doSetHiddenItem(WMenuItem *item, bool hidden)
{
  doSetHiddenItem(indexOf(item), hidden);
}

int WMenu::nextAfterHide(int index)
{
  if (current_ == index) {
    // Try to find visible item to the right of the current.
    for (unsigned i = current_ + 1; i < items_.size(); ++i)
      if (!isItemHidden(i))
        return i;

    // Try to find visible item to the left of the current.
    for (int i = current_ - 1; i >= 0; --i)
      if (!isItemHidden(i))
        return i;
  }

  return current_;
}

void WMenu::setItemHidden(WMenuItem *item, bool hidden)
{
  setItemHidden(indexOf(item), hidden);
}

bool WMenu::isItemHidden(int index) const
{
  return items_[index]->isHidden();
}

bool WMenu::isItemHidden(WMenuItem *item) const
{
  return isItemHidden(indexOf(item));
}

void WMenu::setItemDisabled(int index, bool disabled)
{
  items_[index]->setDisabled(disabled);
}

void WMenu::setItemDisabled(WMenuItem* item, bool disabled)
{
  setItemDisabled(indexOf(item), disabled);
}

bool WMenu::isItemDisabled(int index) const
{
  return items_[index]->isDisabled();
}

bool WMenu::isItemDisabled(WMenuItem *item) const
{
  return isItemDisabled(indexOf(item));
}

void WMenu::close(int index)
{
  WMenuItem *item = items_[index];
  if (item->isCloseable()) {
    item->hide();
    itemClosed_.emit(item);
  }
}

void WMenu::close(WMenuItem *item)
{
  close(indexOf(item));
}

void WMenu::internalPathChanged(const std::string& path)
{
  WApplication *app = wApp;

  if (
#ifdef WT_WITH_OLD_INTERNALPATH_API
      (app->oldInternalPathAPI() && path == basePath_) ||
#endif // WT_WITH_OLD_INTERNALPATH_API 
      app->internalPathMatches(basePath_)) {

    std::string value = app->internalPathNextPart(basePath_);

    for (unsigned i = 0; i < items_.size(); ++i) {
      if (items_[i]->pathComponent() == value
	  || items_[i]->pathComponent() == (value + '/')) {
	items_[i]->setFromInternalPath(path);
	return;
      }
    }

    if (!value.empty())
      wApp->log("error") << "WMenu: unknown path: '"<< value << "'";
    else
      select(-1, false);
  }
}

void WMenu::select(WMenuItem *item)
{
  select(indexOf(item), true);
}

void WMenu::selectVisual(WMenuItem *item)
{
  selectVisual(indexOf(item), true);
}

int WMenu::indexOf(WMenuItem *item) const
{
  return Utils::indexOf(items_, item);
}

void WMenu::undoSelectVisual()
{
  std::string prevPath = previousInternalPath_;
  int prevStackIndex = previousStackIndex_;

  selectVisual(previousCurrent_, true);

  if (internalPathEnabled_) {
    WApplication *app = wApp;
    app->setInternalPath(prevPath);
  }

  if (contentsStack_)
    contentsStack_->setCurrentIndex(prevStackIndex);
}

WMenuItem *WMenu::currentItem() const
{
  return current_ >= 0 ? items_[current_] : 0;
}

void WMenu::recreateItem(int index)
{
  WMenuItem *item = items_[index];

  if (renderAsList_) {
    WContainerWidget *li =
      dynamic_cast<WContainerWidget *>(item->itemWidget()->parent());
    li->addWidget(item->recreateItemWidget());
  } else {
    WTableCell *parent =
      dynamic_cast<WTableCell *>(item->itemWidget()->parent());

    if (orientation_ == Horizontal) {
      const int pos = parent->indexOf(item->itemWidget());
      WWidget *newItemWidget = item->recreateItemWidget();
      parent->insertWidget(pos, newItemWidget);
      newItemWidget->setInline(true);
    } else
      parent->addWidget(item->recreateItemWidget());
  }

  item->renderSelected(current_ == index);
  item->renderHidden(item->isHidden());

  for (unsigned i = 0; i < items_.size(); ++i)
    items_[i]->resetLearnedSlots();
}

void WMenu::recreateItem(WMenuItem *item)
{
  recreateItem(indexOf(item));
}

}
