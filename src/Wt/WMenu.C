/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WLogger"
#include "Wt/WMenu"
#include "Wt/WMenuItem"
#include "Wt/WStackedWidget"
#include "Wt/WTemplate"

#include "WebUtils.h"

/*
 * TODO:
 *  - disable everything selection-related for menu items that have a
 *    popup menu
 */
namespace {

  /*
   * Returns the length of the longest matching sub path.
   *
   * ("a", "b") -> -1
   * ("a", "") -> 0
   * ("a/ab", "a/ac") -> -1
   * ("a", "a") -> 1
   */
  int match(const std::string& path, const std::string& component) {
    if (component.length() > path.length())
      return -1;

    int length = std::min(component.length(), path.length());

    int current = -1;

    for (int i = 0; i < length; ++i) {
      if (component[i] != path[i]) {
	return current;
      } else if (component[i] == '/')
	current = i;
    }

    return length;
  }
}

namespace Wt {

LOGGER("WMenu");

WMenu::WMenu(Orientation orientation, WContainerWidget *parent)
  : WCompositeWidget(parent),
    contentsStack_(0),
    itemSelected_(this),
    itemSelectRendered_(this),
    itemClosed_(this)
{ 
  init();
}

WMenu::WMenu(WContainerWidget *parent)
  : WCompositeWidget(parent),
    contentsStack_(0),
    itemSelected_(this),
    itemSelectRendered_(this),
    itemClosed_(this)
{ 
  init();
}

WMenu::WMenu(WStackedWidget *contentsStack, Orientation orientation,
	     WContainerWidget *parent)
  : WCompositeWidget(parent),
    contentsStack_(contentsStack),
    itemSelected_(this),
    itemSelectRendered_(this),
    itemClosed_(this)
{
  init();
}

WMenu::WMenu(WStackedWidget *contentsStack, WContainerWidget *parent)
  : WCompositeWidget(parent),
    contentsStack_(contentsStack),
    itemSelected_(this),
    itemSelectRendered_(this),
    itemClosed_(this)
{
  init();
}

void WMenu::init()
{
  internalPathEnabled_ = false;
  emitPathChange_ = false;
  parentItem_ = 0;
  needSelectionEventUpdate_ = false;
  current_ = -1;

  if (contentsStack_) {
#ifndef WT_CNOR
    contentsStackConnection_
      = contentsStack_->destroyed().connect(this, &WMenu::contentsDestroyed);
#endif // WT_CNOR

    contentsStack_->childrenChanged().connect(this,
					      &WMenu::updateSelectionEvent);
  }

  setImplementation(ul_ = new WContainerWidget());
  ul_->setList(true);
}

void WMenu::load()
{
  bool wasLoaded = loaded();

  WCompositeWidget::load();

  if (wasLoaded)
    return;

  if (currentItem())
    currentItem()->loadContents();
}

void WMenu::contentsDestroyed()
{
  for (int i = 0; i < count(); ++i)
    itemAt(i)->purgeContents();
}

void WMenu::setRenderAsList(bool enable)
{ 
  LOG_ERROR("WMenu::setRenderAsList() has been deprecated.");
}

WMenu::~WMenu()
{
  contentsStackConnection_.disconnect();
}

void WMenu::setInternalPathEnabled(const std::string& basePath)
{
  WApplication *app = WApplication::instance();

  basePath_ = basePath.empty() ? app->internalPath() : basePath;
  basePath_ = Utils::append(Utils::prepend(basePath_, '/'), '/');

  if (!internalPathEnabled_) {
    internalPathEnabled_ = true;
    app->internalPathChanged().connect(this, &WMenu::handleInternalPathChange);
  }

  previousInternalPath_ = app->internalPath();
  internalPathChanged(app->internalPath());

  updateItemsInternalPath();
}

void WMenu::handleInternalPathChange(const std::string& path)
{
  if (!parentItem_ || !parentItem_->internalPathEnabled())
    internalPathChanged(path);
}

void WMenu::setInternalBasePath(const std::string& basePath)
{
  setInternalPathEnabled(basePath);
}

void WMenu::updateItemsInternalPath()
{
  for (int i = 0; i < count(); ++i) {
    WMenuItem *item = itemAt(i);
    item->updateInternalPath();
  }

  updateSelectionEvent();
}

WMenuItem *WMenu::addItem(const WString& name, WWidget *contents,
			  WMenuItem::LoadPolicy policy)
{
  return addItem(std::string(), name, contents, policy);
}

WMenuItem *WMenu::addItem(const std::string& iconPath, const WString& name,
			  WWidget *contents, WMenuItem::LoadPolicy policy)
{
  WMenuItem *item = new WMenuItem(iconPath, name, contents, policy);
  addItem(item);
  return item;
}

WMenuItem *WMenu::addMenu(const WString& text, WMenu *menu)
{
  return addMenu(std::string(), text, menu);
}

WMenuItem *WMenu::addMenu(const std::string& iconPath,
			  const WString& text, WMenu *menu)
{
  WMenuItem *item = new WMenuItem(iconPath, text, 0, WMenuItem::LazyLoading);
  item->setMenu(menu);
  addItem(item);
  return item;
}

WMenuItem *WMenu::addSeparator()
{
  WMenuItem *item = new WMenuItem(true, WString::Empty);
  addItem(item);
  return item;
}

WMenuItem *WMenu::addSectionHeader(const WString& text)
{
  WMenuItem *result = new WMenuItem(false, text);
  addItem(result);
  return result;
}

void WMenu::addItem(WMenuItem *item)
{
  insertItem(ul()->count(), item);
}

WMenuItem *WMenu::insertItem(int index, const WString& name, WWidget *contents,
                          WMenuItem::LoadPolicy policy)
{
  return insertItem(index, std::string(), name, contents, policy);
}

WMenuItem *WMenu::insertItem(int index, const std::string& iconPath,
                             const WString& name, WWidget *contents,
                             WMenuItem::LoadPolicy policy)
{
  WMenuItem *item = new WMenuItem(iconPath, name, contents, policy);
  insertItem(index, item);
  return item;
}

WMenuItem *WMenu::insertMenu(int index, const WString& text, WMenu *menu)
{
  return insertMenu(index, std::string(), text, menu);
}

WMenuItem *WMenu::insertMenu(int index, const std::string& iconPath,
                          const WString& text, WMenu *menu)
{
  WMenuItem *item = new WMenuItem(iconPath, text, 0, WMenuItem::LazyLoading);
  item->setMenu(menu);
  insertItem(index, item);
  return item;
}
void WMenu::insertItem(int index, WMenuItem *item)
{
  item->setParentMenu(this);

  ul()->insertWidget(index, item);

  if (contentsStack_) {
    WWidget *contents = item->contents();
    if (contents) {
      contentsStack_->addWidget(contents);

      if (contentsStack_->count() == 1) {
	setCurrent(0);

	if (contents)
	  contentsStack_->setCurrentWidget(contents);

	renderSelected(item, true);
      } else
	renderSelected(item, false);
    } else
      renderSelected(item, false);
  } else
    renderSelected(item, false);

  itemPathChanged(item);
}

void WMenu::itemPathChanged(WMenuItem *item)
{
  if (internalPathEnabled_ && item->internalPathEnabled()) {
    WApplication *app = wApp;

    if (app->internalPathMatches(basePath_ + item->pathComponent()))
      item->setFromInternalPath(app->internalPath());
  }
}

void WMenu::removeItem(WMenuItem *item)
{
  WContainerWidget *items = ul();

  if (item->parent() == items) {
    int itemIndex = items->indexOf(item);
    items->removeWidget(item);

    if (contentsStack_ && item->contents())
      contentsStack_->removeWidget(item->contents());

    item->setParentMenu(0);

    if (itemIndex <= current_ && current_ >= 0)
      --current_;

    select(current_, true);
  }
}

void WMenu::select(int index)
{
  select(index, true);
}

void WMenu::setCurrent(int index)
{
  current_ = index;
}

void WMenu::select(int index, bool changePath)
{
  int last = current_;
  setCurrent(index);

  selectVisual(current_, changePath, true);

  if (index != -1) {
    WMenuItem *item = itemAt(index);
    item->show();
    if (loaded())
      item->loadContents();

    DeletionTracker guard(this);

    if (changePath && emitPathChange_) {
      WApplication *app = wApp;
      app->internalPathChanged().emit(app->internalPath());
      if (guard.deleted())
        return;
      emitPathChange_ = false;
    }

    if (last != index) {
      item->triggered().emit(item);
      if (!guard.deleted()) {
        // item may have been deleted too
        if (ul()->indexOf(item) != -1)
          itemSelected_.emit(item);
        else
          select(-1);
      }
    }
  }
}

void WMenu::selectVisual(int index, bool changePath, bool showContents)
{
  if (contentsStack_)
    previousStackIndex_ = contentsStack_->currentIndex();

  WMenuItem *item = index >= 0 ? itemAt(index) : 0;

  if (changePath && internalPathEnabled_ &&
      index != -1 && item->internalPathEnabled()) {
    WApplication *app = wApp;
    previousInternalPath_ = app->internalPath();

    std::string newPath = basePath_ + item->pathComponent();
    if (newPath != app->internalPath())
      emitPathChange_ = true;

    // The change is emitted in select()
    app->setInternalPath(newPath);
  }

  for (int i = 0; i < count(); ++i)
    renderSelected(itemAt(i), (int)i == index);

  if (index == -1)
    return;

  if (showContents && contentsStack_) {
    WWidget *contents = item->contents();
    if (contents)
      contentsStack_->setCurrentWidget(contents);
  }

  itemSelectRendered_.emit(item);
}

void WMenu::renderSelected(WMenuItem *item, bool selected)
{
  item->renderSelected(selected);
}

void WMenu::setItemHidden(int index, bool hidden)
{
  itemAt(index)->setHidden(hidden);
}

void WMenu::onItemHidden(int index, bool hidden)
{
  if (hidden) {
    int nextItem = nextAfterHide(index);
    if (nextItem != current_)
      select(nextItem);
  }
}

int WMenu::nextAfterHide(int index)
{
  if (current_ == index) {
    // Try to find visible item to the right of the current.
    for (int i = current_ + 1; i < count(); ++i)
      if (!isItemHidden(i) && itemAt(i)->isEnabled())
        return i;

    // Try to find visible item to the left of the current.
    for (int i = current_ - 1; i >= 0; --i)
      if (!isItemHidden(i) && itemAt(i)->isEnabled())
        return i;
  }

  return current_;
}

void WMenu::setItemHidden(WMenuItem *item, bool hidden)
{
  item->setHidden(hidden);
}

bool WMenu::isItemHidden(int index) const
{
  return isItemHidden(itemAt(index));
}

bool WMenu::isItemHidden(WMenuItem *item) const
{
  return item->isHidden();
}

void WMenu::setItemDisabled(int index, bool disabled)
{
  setItemDisabled(itemAt(index), disabled);
}

void WMenu::setItemDisabled(WMenuItem* item, bool disabled)
{
  item->setDisabled(disabled);
}

bool WMenu::isItemDisabled(int index) const
{
  return isItemDisabled(itemAt(index));
}

bool WMenu::isItemDisabled(WMenuItem *item) const
{
  return item->isDisabled();
}

void WMenu::close(int index)
{
  close(itemAt(index));
}

void WMenu::close(WMenuItem *item)
{
  if (item->isCloseable()) {
    item->hide();
    itemClosed_.emit(item);
  }
}

void WMenu::internalPathChanged(const std::string& path)
{
  WApplication *app = wApp;

  if (app->internalPathMatches(basePath_)) {
    std::string subPath = app->internalSubPath(basePath_);

    int bestI = -1, bestMatchLength = -1;

    for (int i = 0; i < count(); ++i) {
      if (!itemAt(i)->isEnabled() || itemAt(i)->isHidden())
	continue;

      int matchLength = match(subPath, itemAt(i)->pathComponent());

      if (matchLength > bestMatchLength) {
	bestMatchLength = matchLength;
	bestI = i;
      }
    }

    if (bestI != -1)
      itemAt(bestI)->setFromInternalPath(path);
    else {
      if (!subPath.empty())
	LOG_WARN("unknown path: '"<< subPath << "'");
      else
	select(-1, false);
    }
  }
}

void WMenu::select(WMenuItem *item)
{
  select(indexOf(item), true);
}

void WMenu::selectVisual(WMenuItem *item)
{
  selectVisual(indexOf(item), true, true);
}

int WMenu::indexOf(WMenuItem *item) const
{
  return ul()->indexOf(item);
}

void WMenu::undoSelectVisual()
{
  std::string prevPath = previousInternalPath_;
  int prevStackIndex = previousStackIndex_;

  selectVisual(current_, true, true);

  if (internalPathEnabled_) {
    WApplication *app = wApp;
    app->setInternalPath(prevPath);
  }

  if (contentsStack_)
    contentsStack_->setCurrentIndex(prevStackIndex);
}

WMenuItem *WMenu::currentItem() const
{
  return current_ >= 0 ? itemAt(current_) : 0;
}

void WMenu::render(WFlags<RenderFlag> flags)
{
  if (needSelectionEventUpdate_) {
    for (int i = 0; i < count(); ++i)
      itemAt(i)->resetLearnedSlots();

    needSelectionEventUpdate_ = false;
  }

  WCompositeWidget::render(flags);
}

void WMenu::updateSelectionEvent()
{
  needSelectionEventUpdate_ = true;
  scheduleRender();
}

int WMenu::count() const
{
  return ul()->count();
}

WMenuItem *WMenu::itemAt(int index) const
{
  return dynamic_cast<WMenuItem *>(ul()->widget(index));
}

std::vector<WMenuItem *> WMenu::items() const
{
  std::vector<WMenuItem *> result;

  for (int i = 0; i < count(); ++i)
    result.push_back(itemAt(i));

  return result;
}

}
