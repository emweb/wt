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
    willPopup_(false),
    hideOnSelect_(true),
    autoHideDelay_(-1)
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
    if (b)
      b->setMenu(nullptr);
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
    button_->addStyleClass("dropdown-toggle");
  }
}

void WPopupMenu::popupAtButton()
{
  if (!isHidden())
    return;

  if (!topLevel_ || topLevel_ == this) {
    button_->addStyleClass("active", true);
    if (parentItem())
      parentItem()->addStyleClass("open");

    popup(button_);
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
    button_->removeStyleClass("active", true);
    if (parentItem())
      parentItem()->removeStyleClass("open");
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
  if (willPopup_)
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

  positionAt(location, orientation);
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

  willPopup_ = true;
  scheduleRender();
}

void WPopupMenu::popup(const WPoint& p)
{
  popupImpl();

  // make sure we are not confused by client-side being positioned properly
  setOffsets(42, Side::Left | Side::Top);
  setOffsets(-10000, Side::Left | Side::Top);

  doJavaScript(WT_CLASS ".positionXY('" + id() + "',"
	       + std::to_string(p.x()) + ","
	       + std::to_string(p.y()) + ");");
}

void WPopupMenu::prepareRender(WApplication *app)
{
  if (!cancel_.isConnected()) {
    LOAD_JAVASCRIPT(app, "js/WPopupMenu.js", "WPopupMenu", wtjs1);

    WStringStream s;

    s << "new " WT_CLASS ".WPopupMenu("
      << app->javaScriptClass() << ',' << jsRef() << ','
      << autoHideDelay_ << ");";

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
}

void WPopupMenu::renderSelected(WMenuItem *item, bool selected)
{ }

void WPopupMenu::getSDomChanges(std::vector<DomElement *> &result, WApplication *app)
{
  WMenu::getSDomChanges(result, app);
  willPopup_ = false;
}
void WPopupMenu::render(WFlags<RenderFlag> flags)
{
  WMenu::render(flags);
  willPopup_ = false;
}

std::string WPopupMenu::renderRemoveJs(bool recursive)
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

}
