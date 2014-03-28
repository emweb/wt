/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WException"
#include "Wt/WPoint"
#include "Wt/WPopupMenu"
#include "Wt/WPushButton"
#include "Wt/WTemplate"

#include "WebSession.h"

#ifndef WT_DEBUG_JS
#include "js/WPopupMenu.min.js"
#endif

namespace Wt {

WPopupMenu::WPopupMenu(WStackedWidget *contentsStack)
  : WMenu(contentsStack),
    topLevel_(0),
    result_(0),
    location_(0),
    button_(0),
    aboutToHide_(this),
    triggered_(this),
    cancel_(this, "cancel"),
    recursiveEventLoop_(false),
    autoHideDelay_(-1)
{
  const char *CSS_RULES_NAME = "Wt::WPopupMenu";

  WApplication *app = WApplication::instance();

  if (!app->styleSheet().isDefined(CSS_RULES_NAME))
    app->styleSheet().addRule
      (".Wt-notselected .Wt-popupmenu", "visibility: hidden;", CSS_RULES_NAME);

  app->addGlobalWidget(this);
  setPopup(true);

  hide();
}

WPopupMenu::~WPopupMenu()
{
  if (button_) {
    WPushButton *b = dynamic_cast<WPushButton *>(button_);
    if (b)
      b->setMenu(0);
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
    doJavaScript("jQuery.data(" + jsRef() + ", 'obj').setHidden("
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

  location_ = 0;
  result_ = result;

  hide();

  recursiveEventLoop_ = false;

  if (result_)
    triggered_.emit(result_);

  aboutToHide_.emit();
}

void WPopupMenu::cancel()
{
  if (!isHidden())
    done(0);
}

void WPopupMenu::popup(WWidget *location, Orientation orientation)
{
  location_ = location;

  popupImpl();

  doJavaScript("jQuery.data(" + jsRef() + ", 'obj').popupAt("
	       + location->jsRef() + ");");

  positionAt(location, orientation);
}

void WPopupMenu::popup(const WMouseEvent& e)
{
  popup(WPoint(e.document().x, e.document().y));
}

void WPopupMenu::popupImpl()
{
  result_ = 0;

  WApplication *app = WApplication::instance();
  prepareRender(app);

  show();
}

void WPopupMenu::popup(const WPoint& p)
{
  popupImpl();

  // make sure we are not confused by client-side being positioned properly
  setOffsets(42, Left | Top);
  setOffsets(-10000, Left | Top);

  doJavaScript(WT_CLASS ".positionXY('" + id() + "',"
	       + boost::lexical_cast<std::string>(p.x()) + ","
	       + boost::lexical_cast<std::string>(p.y()) + ");");
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

}
