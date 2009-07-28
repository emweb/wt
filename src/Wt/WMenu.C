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
    c->setOverflow(WContainerWidget::OverflowAuto);

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

#ifdef WT_WITH_OLD_INTERNALPATH_API
    if (app->oldInternalPathAPI())
      internalPathChanged(basePath_);
    else
#endif // WT_WITH_OLD_INTERNALPATH_API
      internalPathChanged(app->internalPath());

    updateItems();
  }
}

void WMenu::setInternalBasePath(const std::string& basePath)
{
  std::string bp = Utils::terminate(basePath, '/');
  if (basePath_ != bp) {
    basePath_ = bp;

    if (internalPathEnabled_) {
      WApplication *app = wApp;
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

    select(current_);
  }
}

void WMenu::select(int index)
{
  selectVisual(index);

  if (index != -1) {
    items_[index]->loadContents();
    itemSelected_.emit(items_[current_]);
  }
}

void WMenu::selectVisual(int index)
{
  previousCurrent_ = current_;
  previousStackIndex_ = contentsStack_->currentIndex();

  current_ = index;

  for (unsigned i = 0; i < items_.size(); ++i)
    items_[i]->renderSelected((int)i == current_);

  if (index == -1)
    return;

  WWidget *contents = items_[current_]->contents();
  if (contents)
    contentsStack_->setCurrentWidget(contents);

  if (internalPathEnabled_) {
    WApplication *app = wApp;

    previousInternalPath_ = app->internalPath();

    std::string newPath = basePath_;
    std::string pc = items_[current_]->pathComponent();
    if (pc.empty()) {
      if (newPath.length() > 1)
	newPath = newPath.substr(0, newPath.length() - 1);
    } else
      newPath += pc;

    // unless we are resetting to basePath, we avoid removing a more
    // specific path
    if (newPath == basePath_ || !app->internalPathMatches(newPath))
      app->setInternalPath(newPath);
  }

  itemSelectRendered_.emit(items_[current_]);
}

void WMenu::internalPathChanged(std::string path)
{
  WApplication *app = wApp;

  if (
#ifdef WT_WITH_OLD_INTERNALPATH_API
      (app->oldInternalPathAPI() && path == basePath_) ||
#endif // WT_WITH_OLD_INTERNALPATH_API 
      app->internalPathMatches(basePath_))
    setFromState(app->internalPathNextPart(basePath_));
}

void WMenu::setFromState(const std::string& value)
{
  std::string v = value;

  for (unsigned i = 0; i < items_.size(); ++i) {
    if (items_[i]->pathComponent() == v
	|| items_[i]->pathComponent() == (v + '/')) {
      if (contentsStack_->currentWidget() != items_[i]->contents())
	select(i);
      return;
    }
  }

  if (!value.empty())
    wApp->log("error") << "WMenu: unknown path: '"<< value << "'";

  select(-1);
}

void WMenu::select(WMenuItem *item)
{
  select(indexOf(item));
}

void WMenu::selectVisual(WMenuItem *item)
{
  selectVisual(indexOf(item));
}

int WMenu::indexOf(WMenuItem *item)
{
  return Utils::indexOf(items_, item);
}

void WMenu::undoSelectVisual()
{
  std::string prevPath = previousInternalPath_;
  int prevStackIndex = previousStackIndex_;

  selectVisual(previousCurrent_);

  if (internalPathEnabled_)
    wApp->setInternalPath(prevPath);

  contentsStack_->setCurrentIndex(prevStackIndex);
}

WMenuItem *WMenu::currentItem() const
{
  return current_ >= 0 ? items_[current_] : 0;
}

}
