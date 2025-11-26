/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WPoint.h"
#include "Wt/WPopupMenu.h"
#include "Wt/WPushButton.h"
#include "Wt/WTemplate.h"

#include "WebSession.h"

#ifndef WT_DEBUG_JS
#include "js/WPopupMenu.min.js"
#endif

namespace Wt {

WPopupMenu::WPopupMenu(WStackedWidget *contentsStack)
  : WMenu(contentsStack),
    topLevel_(nullptr),
    result_(nullptr),
    location_(nullptr),
    button_(nullptr),
    aboutToHide_(),
    triggered_(),
    cancel_(this, "cancel"),
    recursiveEventLoop_(false),
    hideOnSelect_(true),
    open_(false),
    adjustFlags_(AllOrientations),
    autoHideDelay_(-1),
    renderedAutoHideDelay_(-1),
    autoHideBehaviour_(AutoHideBehaviour::HideAllEnabled)
{
  const char *CSS_RULES_NAME = "Wt::WPopupMenu";

  WApplication *app = WApplication::instance();

  if (!app->styleSheet().isDefined(CSS_RULES_NAME))
    app->styleSheet().addRule
      (".Wt-notselected .Wt-popupmenu", "visibility: hidden;", CSS_RULES_NAME);

  app->addGlobalWidget(this);
  // Set high ZIndex so WPopupMenu is above pretty much every dialog by default
  webWidget()->setBaseZIndex(110000);
  setPopup(true);

  hide();
}

WPopupMenu::~WPopupMenu()
{
  if (button_) {
    WPushButton *b = dynamic_cast<WPushButton *>(button_);
    if (b && b->menu() == this) {
      b->setMenu(nullptr);
    }
  }

  if (isGlobalWidget()) {
    wApp->removeGlobalWidget(this);
  }
}

void WPopupMenu::setButton(WInteractWidget *button)
{
  button_ = button;

  if (button_) {
    button_->clicked().connect(this, &WPopupMenu::popupAtButton);
    button_->addThemeStyle("dropdown-toggle");
  }
}

void WPopupMenu::popupAtButton()
{
  if (!isHidden())
    return;

  if (!topLevel_ || topLevel_ == this) {
    open_ = true;
    flags_.set(BIT_OPEN_CHANGED);
    popup(button_);

    scheduleRender();
  }
}

void WPopupMenu::setMaximumSize(const WLength& width,
                                const WLength& height)
{
  WCompositeWidget::setMaximumSize(width, height);
  ul()->setMaximumSize(width, height);
}

void WPopupMenu::setMinimumSize(const WLength& width,
                                const WLength& height)
{
  WCompositeWidget::setMinimumSize(width, height);
  ul()->setMinimumSize(width, height);
}

void WPopupMenu::setHidden(bool hidden, const WAnimation& animation)
{
  WApplication *app = WApplication::instance();

  if (hidden != isHidden()) {
    handleFocusOnHide(hidden);
  }

  WCompositeWidget::setHidden(hidden, animation);

  if (cancel_.isConnected() ||
      WApplication::instance()->session()->renderer().preLearning())
    doJavaScript(jsRef() + ".wtObj.setHidden("
                 + (hidden ? "1" : "0") + ");");
}

void WPopupMenu::done(WMenuItem *result)
{
  if (isHidden())
    return;

  if (location_ && location_ == button_) {
    open_ = false;
    flags_.set(BIT_OPEN_CHANGED);
    scheduleRender();
  }

  location_ = nullptr;
  result_ = result;

  bool shouldHide = !result || static_cast<WPopupMenu*>(result->parentMenu())->hideOnSelect();

  if (shouldHide)
    hide();

  recursiveEventLoop_ = false;

  if (result_)
    triggered_.emit(result_);

  if (shouldHide)
    aboutToHide_.emit();
}

void WPopupMenu::cancel()
{
  if (flags_.test(BIT_WILL_POPUP))
    return;

  if (!isHidden())
    done(nullptr);
}

void WPopupMenu::popup(WWidget *location, Orientation orientation)
{
  location_ = location;

  popupImpl();

  doJavaScript(jsRef() + ".wtObj.popupAt("
               + location->jsRef() + ");");

  positionAt(location, orientation, adjustFlags_);
}

void WPopupMenu::popup(const WMouseEvent& e)
{
  popup(WPoint(e.document().x, e.document().y));
}

void WPopupMenu::popupImpl()
{
  result_ = nullptr;

  WApplication *app = WApplication::instance();
  prepareRender(app);

  show();

  flags_.set(BIT_WILL_POPUP);
  scheduleRender();
}

void WPopupMenu::popup(const WPoint& p)
{
  popupImpl();

  // make sure we are not confused by client-side being positioned properly
  setOffsets(42, Side::Left | Side::Top);
  setOffsets(-10000, Side::Left | Side::Top);

  std::string canAdjustX = adjustFlags_.test(Orientation::Horizontal) ? "true" : "false";
  std::string canAdjustY = adjustFlags_.test(Orientation::Vertical) ? "true" : "false";

  doJavaScript(WT_CLASS ".positionXY('" + id() + "',"
               + std::to_string(p.x()) + ","
               + std::to_string(p.y()) + ","
               + canAdjustX + ","
               + canAdjustY + ");");
}

void WPopupMenu::prepareRender(WApplication *app)
{
  if (!cancel_.isConnected()) {
    LOAD_JAVASCRIPT(app, "js/WPopupMenu.js", "WPopupMenu", wtjs1);

    WStringStream s;

    s << "new " WT_CLASS ".WPopupMenu("
      << app->javaScriptClass() << ',' << jsRef() << ','
      << autoHideDelay_ << ','
      << static_cast<int>(autoHideBehaviour_) << ");";

    setJavaScriptMember(" WPopupMenu", s.str());

    cancel_.connect(this, &WPopupMenu::cancel);

    connectSignals(this);
  }

  adjustPadding();
}

void WPopupMenu::setCurrent(int index)
{
  // a popup menu does not have a 'current' item, unless it's involved
  // in a contents stack ... but then it will render oddly ?
  if (contentsStack())
    WMenu::setCurrent(index);
}

void WPopupMenu::connectSignals(WPopupMenu * const topLevel)
{
  topLevel_ = topLevel;
  itemSelected().connect(topLevel, &WPopupMenu::done);

  for (int i = 0; i < count(); ++i) {
    WMenuItem *item = itemAt(i);
    WPopupMenu *subMenu = dynamic_cast<WPopupMenu *>(item->menu());

    if (subMenu)
      subMenu->connectSignals(topLevel);
  }
}

void WPopupMenu::adjustPadding()
{
  bool needPadding = false;

  for (int i = 0; i < count(); ++i) {
    WMenuItem *item = itemAt(i);
    if (!item->icon().empty() || item->isCheckable()) {
      needPadding = true;
      break;
    }
  }

  for (int i = 0; i < count(); ++i) {
    WMenuItem *item = itemAt(i);
    item->setItemPadding(needPadding);
    WPopupMenu *subMenu = dynamic_cast<WPopupMenu *>(item->menu());

    if (subMenu)
      subMenu->adjustPadding();
  }
}

WMenuItem *WPopupMenu::exec(const WPoint& p)
{
  if (recursiveEventLoop_)
    throw WException("WPopupMenu::exec(): already being executed.");

  popup(p);
  exec();

  return result_;
}

void WPopupMenu::exec()
{
  WApplication *app = WApplication::instance();
  recursiveEventLoop_ = true;

  if (app->environment().isTest()) {
    app->environment().popupExecuted().emit(this);
    if (recursiveEventLoop_)
      throw WException("Test case must close popup menu.");
  } else {
    do {
      app->waitForEvent();
    } while (recursiveEventLoop_);
  }
}

WMenuItem *WPopupMenu::exec(const WMouseEvent& e)
{
  return exec(WPoint(e.document().x, e.document().y));
}

WMenuItem *WPopupMenu::exec(WWidget *location, Orientation orientation)
{
  if (recursiveEventLoop_)
    throw WException("WPopupMenu::exec(): already being executed.");

  popup(location, orientation);
  exec();

  return result_;
}

void WPopupMenu::setAutoHide(bool enabled, int autoHideDelay)
{
  if (enabled)
    autoHideDelay_ = autoHideDelay;
  else
    autoHideDelay_ = -1;

  flags_.set(BIT_AUTO_HIDE_CHANGED);
  scheduleRender();
}

void WPopupMenu::setAutoHideBehaviour(AutoHideBehaviour behaviour)
{
  autoHideBehaviour_ = behaviour;
  flags_.set(BIT_AUTO_HIDE_BEHAVIOR_CHANGED);
  scheduleRender();
}

void WPopupMenu::renderSelected(WT_MAYBE_UNUSED WMenuItem* item, WT_MAYBE_UNUSED bool selected)
{ }

void WPopupMenu::getSDomChanges(std::vector<DomElement *> &result, WApplication *app)
{
  WMenu::getSDomChanges(result, app);
   flags_.reset(BIT_WILL_POPUP);
}
void WPopupMenu::render(WFlags<RenderFlag> flags)
{
  if (flags_.test(BIT_OPEN_CHANGED)) {
    if (button_ && button_->isThemeStyleEnabled()) {
      button_->toggleStyleClass("active", open_, true);
    }
    if (parentItem() && isThemeStyleEnabled()) {
      parentItem()->toggleStyleClass("open", open_);
    }

    flags_.reset(BIT_OPEN_CHANGED);
  }

  if (cancel_.isConnected() && flags_.test(BIT_AUTO_HIDE_BEHAVIOR_CHANGED)) {
    WStringStream s;

    s << jsRef() << ".wtObj.setAutoHideBehaviour("
      << static_cast<int>(autoHideBehaviour_) << ");";

    doJavaScript(s.str());

    flags_.reset(BIT_AUTO_HIDE_BEHAVIOR_CHANGED);
  }

  if (flags_.test(BIT_AUTO_HIDE_CHANGED)) {
    const std::string styleClassBase = "Wt-AutoHideDelay-";
    if (renderedAutoHideDelay_ != -1) {
      removeStyleClass(styleClassBase + std::to_string(renderedAutoHideDelay_));
    }
    if (autoHideDelay_ != -1) {
      addStyleClass(styleClassBase + std::to_string(autoHideDelay_));
    }
    renderedAutoHideDelay_ = autoHideDelay_;
    flags_.reset(BIT_AUTO_HIDE_CHANGED);
  }

  if (adjustFlags_.test(Orientation::Horizontal)) {
    addStyleClass("Wt-AdjustX");
  } else {
    removeStyleClass("Wt-AdjustX");
  }

  if (adjustFlags_.test(Orientation::Vertical)) {
    addStyleClass("Wt-AdjustY");
  } else {
    removeStyleClass("Wt-AdjustY");
  }

  WMenu::render(flags);
  flags_.reset(BIT_WILL_POPUP);
}

std::string WPopupMenu::renderRemoveJs(WT_MAYBE_UNUSED bool recursive)
{
  // Removal of WPopupMenu may not be simplified, because
  // it may have been reparented by JavaScript
  auto result = WMenu::renderRemoveJs(true);
  result += WT_CLASS ".remove('" + id() + "');";
  return result;
}

void WPopupMenu::setHideOnSelect(bool enabled)
{
  hideOnSelect_ = enabled;
}

void WPopupMenu::setAdjust(WFlags<Orientation> adjustOrientations)
{

  adjustFlags_ = adjustOrientations;
  refresh();
  scheduleRerender(true);
}

}
