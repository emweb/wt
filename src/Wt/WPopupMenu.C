/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WException"
#include "Wt/WPoint"
#include "Wt/WPopupMenu"
#include "Wt/WPopupMenuItem"
#include "Wt/WTemplate"

#include "WebSession.h"

#ifndef WT_DEBUG_JS
#include "js/WPopupMenu.min.js"
#endif

namespace Wt {

WPopupMenu::WPopupMenu()
  : WCompositeWidget(),
    parentItem_(0),
    result_(0),
    aboutToHide_(this),
    triggered_(this),
    cancel_(this, "cancel"),
    recursiveEventLoop_(false),
    autoHideDelay_(-1)
{
  const char *TEMPLATE =
    "${shadow-x1-x2}"
    "${contents}";

  setImplementation(impl_ = new WTemplate(WString::fromUTF8(TEMPLATE)));
  impl_->setLoadLaterWhenInvisible(false);

  setPositionScheme(Absolute);
  setStyleClass("Wt-popupmenu Wt-outset");

  impl_->bindString("shadow-x1-x2", WTemplate::DropShadow_x1_x2);
  WContainerWidget *content = new WContainerWidget();
  content->setStyleClass("content");
  impl_->bindWidget("contents", content);

  const char *CSS_RULES_NAME = "Wt::WPopupMenu";

  WApplication *app = WApplication::instance();

  if (!app->styleSheet().isDefined(CSS_RULES_NAME))
    app->styleSheet().addRule
      (".Wt-notselected .Wt-popupmenu", "visibility: hidden;", CSS_RULES_NAME);

  app->domRoot()->addWidget(this);

  hide();
}

WContainerWidget *WPopupMenu::contents() const
{
  return dynamic_cast<WContainerWidget *>(impl_->resolveWidget("contents"));
}

void WPopupMenu::setMaximumSize(const WLength& width,
				const WLength& height)
{
  WCompositeWidget::setMaximumSize(width, height);
  contents()->setMaximumSize(width, height);
}

void WPopupMenu::setMinimumSize(const WLength& width,
				const WLength& height)
{
  WCompositeWidget::setMinimumSize(width, height);
  contents()->setMinimumSize(width, height);
}

WPopupMenuItem *WPopupMenu::addItem(const WString& text)
{
  return addItem(std::string(), text);
}

WPopupMenuItem *WPopupMenu::addItem(const std::string& iconPath,
				    const WString& text)
{
  WPopupMenuItem *item = new WPopupMenuItem(iconPath, text);
  add(item);
  return item;
}

WPopupMenuItem *WPopupMenu::addMenu(const WString& text, WPopupMenu *menu)
{
  return addMenu(std::string(), text, menu);
}

WPopupMenuItem *WPopupMenu::addMenu(const std::string& iconPath,
				    const WString& text, WPopupMenu *menu)
{
  WPopupMenuItem *item = addItem(iconPath, text);
  item->setPopupMenu(menu);
  return item;
}

void WPopupMenu::add(WPopupMenuItem *item)
{
  contents()->addWidget(item);
}

void WPopupMenu::addSeparator()
{
  add(new WPopupMenuItem(true));
}

WPopupMenu *WPopupMenu::topLevelMenu()
{
  return parentItem_ ? parentItem_->topLevelMenu() : this;
}

void WPopupMenu::setHidden(bool hidden, const WAnimation& animation)
{
  WCompositeWidget::setHidden(hidden, animation);

  if (hidden)
    renderOutAll();
}

void WPopupMenu::renderOutAll()
{
  WContainerWidget *c = contents();
  for (int i = 0; i < c->count(); ++i) {
    WPopupMenuItem *item = dynamic_cast<WPopupMenuItem *>(c->widget(i));
    item->renderOut();
  }
}

void WPopupMenu::done(WPopupMenuItem *result)
{
  result_ = result;

  hide();

  WApplication *app = WApplication::instance();

  app->root()->clicked().disconnect(globalClickConnection_);
  app->globalEscapePressed().disconnect(globalEscapeConnection_);
  app->popExposedConstraint(this);

  recursiveEventLoop_ = false;

  triggered_.emit(result_);

  aboutToHide_.emit();
}

void WPopupMenu::done()
{
  done(0);
}

void WPopupMenu::popup(WWidget *location, Orientation orientation)
{
  popupImpl();
  positionAt(location, orientation);
}

void WPopupMenu::popupToo(WWidget *location)
{
  show();
  positionAt(location, Horizontal);
}

void WPopupMenu::popup(const WMouseEvent& e)
{
  popup(WPoint(e.document().x, e.document().y));
}

void WPopupMenu::popupImpl()
{
  renderOutAll();

  result_ = 0;

  WApplication *app = WApplication::instance();

  // XXX
  // We rely here on the fact that no other widget is listening for
  // escape on the root()
  if (app->globalEscapePressed().isConnected())
    app->globalEscapePressed().emit();

  globalClickConnection_
    = app->root()->clicked().connect(this, &WPopupMenu::done);
  globalEscapeConnection_
    = app->globalEscapePressed().connect(this, &WPopupMenu::done);

  app->pushExposedConstraint(this);

  prepareRender(app);

  show();
}

void WPopupMenu::popup(const WPoint& p)
{
  popupImpl();

  // make sure we are not confused by client-side being positioned properly
  setOffsets(42, Left | Top);
  setOffsets(-10000, Left | Top);

  WApplication::instance()->doJavaScript
    (WT_CLASS ".positionXY('" + id() + "',"
     + boost::lexical_cast<std::string>(p.x()) + ","
     + boost::lexical_cast<std::string>(p.y()) + ");");
}

void WPopupMenu::prepareRender(WApplication *app)
{
  if (app->environment().agentIsIE()) {
    app->doJavaScript(jsRef() + ".lastChild.style.width="
		      + jsRef() + ".lastChild.offsetWidth+'px';");
  }

  // FIXME: we should really also prepareRender() of submenus when shown...

  if (autoHideDelay_ >= 0) {
    if (!cancel_.isConnected()) {
      LOAD_JAVASCRIPT(app, "js/WPopupMenu.js", "WPopupMenu", wtjs1);

      doJavaScript("new " WT_CLASS ".WPopupMenu("
		   + app->javaScriptClass() + "," + jsRef() + ","
		   + boost::lexical_cast<std::string>(autoHideDelay_)
		   + ");");
      cancel_.connect(this, &WPopupMenu::done);
    }
  }
}

WPopupMenuItem *WPopupMenu::exec(const WPoint& p)
{
  if (recursiveEventLoop_)
    throw WException("WPopupMenu::exec(): already being executed.");

  WApplication *app = WApplication::instance();
  recursiveEventLoop_ = true;

  popup(p);

  if (app->environment().isTest()) {
    app->environment().popupExecuted().emit(this);
    if (recursiveEventLoop_)
      throw WException("Test case must close popup menu.");
  } else {
    do {
      app->session()->doRecursiveEventLoop();
    } while (recursiveEventLoop_);
  }

  return result_;
}

WPopupMenuItem *WPopupMenu::exec(const WMouseEvent& e)
{
  return exec(WPoint(e.document().x, e.document().y));
}

WPopupMenuItem *WPopupMenu::exec(WWidget *location, Orientation orientation)
{
  if (recursiveEventLoop_)
    throw WException("WPopupMenu::exec(): already being executed.");

  WebSession *session = WApplication::instance()->session();
  recursiveEventLoop_ = true;

  popup(location, orientation);
  do {
    session->doRecursiveEventLoop();
  } while (recursiveEventLoop_);
 
  return result_;
}

void WPopupMenu::setAutoHide(bool enabled, int autoHideDelay)
{
  if (enabled)
    autoHideDelay_ = autoHideDelay;
  else
    autoHideDelay_ = -1;
}

bool WPopupMenu::isExposed(WWidget *w)
{
  if (WCompositeWidget::isExposed(w))
    return true;

  if (w == WApplication::instance()->root())
    return true;

  WContainerWidget *c = contents();
  for (int i = 0; i < c->count(); ++i) {
    WPopupMenuItem *item = dynamic_cast<WPopupMenuItem *>(c->widget(i));
    if (item->popupMenu())
      if (item->popupMenu()->isExposed(w))
	return true;
  }

  // Signal outside of the menu: a top level menu will simply close
  // and let it be handled.
  if (!parentItem_) {
    done();
    return true;
  } else
    return false;
}

}
