/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
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

WMenu::WMenu(WStackedWidget *contentsStack, Orientation orientation,
	     WContainerWidget *parent)
  : WCompositeWidget(parent),
    contentsStack_(contentsStack),
    orientation_(orientation),
    internalPathEnabled_(false),
    itemSelected_(this),
    itemSelectRendered_(this),
    current_(-1)
{
  setRenderAsList(false);
}

void WMenu::setRenderAsList(bool enable)
{
  if (enable) {
    WContainerWidget *c = new WContainerWidget();
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

    app->internalPathChanged().connect(SLOT(this, WMenu::internalPathChanged));

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
  if (internalPathEnabled_)
    updateItems();

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
    WContainerWidget *li = new WContainerWidget(p);
    li->addWidget(item->itemWidget());
  } else {
    WTable *layout = dynamic_cast<WTable *>(impl_);
    WTableCell *parent
      = layout->elementAt((orientation_ == Vertical) ? items_.size() : 0, 0);

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
    } else
      std::cerr << "WMenu::removeItem() only implemented when "
	"renderAsList == true" << std::endl;

    contentsStack_->removeWidget(item->contents());
    item->setMenu(0);

    if (itemIndex <= current_ && current_ > 0)
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
  bool emitPathChange = false;

  WApplication *app = 0;

  if (changePath && internalPathEnabled_) {
    app = wApp;
    emitPathChange = previousInternalPath_ != app->internalPath();
  }

  selectVisual(index, changePath);

  if (index != -1) {
    items_[index]->loadContents();
    itemSelected_.emit(items_[current_]);

    if (changePath && emitPathChange)
      app->internalPathChanged().emit(app->internalPath());
  }
}

void WMenu::selectVisual(int index, bool changePath)
{
  previousCurrent_ = current_;
  previousStackIndex_ = contentsStack_->currentIndex();

  current_ = index;

  if (internalPathEnabled_ && current_ != -1) {
    WApplication *app = wApp;

    previousInternalPath_ = app->internalPath();
    std::string newPath = basePath_ + items_[current_]->pathComponent();

    if (changePath)
      // The change is emitted in select()
      app->setInternalPath(newPath);
  }

  for (unsigned i = 0; i < items_.size(); ++i)
    items_[i]->renderSelected((int)i == current_);

  if (index == -1)
    return;

  WWidget *contents = items_[current_]->contents();
  if (contents)
    contentsStack_->setCurrentWidget(contents);

  itemSelectRendered_.emit(items_[current_]);
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

int WMenu::indexOf(WMenuItem *item)
{
  return Utils::indexOf(items_, item);
}

void WMenu::undoSelectVisual()
{
  std::string prevPath = previousInternalPath_;
  int prevStackIndex = previousStackIndex_;

  selectVisual(previousCurrent_, true);

  if (internalPathEnabled_)
    wApp->setInternalPath(prevPath);

  contentsStack_->setCurrentIndex(prevStackIndex);
}

WMenuItem *WMenu::currentItem() const
{
  return current_ >= 0 ? items_[current_] : 0;
}

}
