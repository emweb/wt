/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WPoint"
#include "Wt/WPopupMenu"
#include "Wt/WPopupMenuItem"
#include "Wt/WTemplate"

#include "WebSession.h"
#include "WtException.h"

namespace Wt {

WPopupMenu::WPopupMenu()
  : WCompositeWidget(),
    parentItem_(0),
    result_(0),
    aboutToHide_(this),
    recursiveEventLoop_(false)
{
  const char *TEMPLATE =
    "${shadow-x1-x2}"
    "${contents}";

  setImplementation(impl_ = new WTemplate(WString::fromUTF8(TEMPLATE)));
  impl_->setLoadLaterWhenInvisible(false);

  setPositionScheme(Absolute);
  setStyleClass("Wt-popupmenu Wt-outset");

  impl_->bindString("shadow-x1-x2", WTemplate::DropShadow_x1_x2);
  impl_->bindWidget("contents", new WContainerWidget());

  const char *CSS_RULES_NAME = "Wt::WPopupMenu";

  WApplication *app = WApplication::instance();

  if (!app->styleSheet().isDefined(CSS_RULES_NAME))
    app->styleSheet().addRule
      (".Wt-notselected .Wt-popupmenu", "visibility: hidden;", CSS_RULES_NAME);

  app->domRoot()->addWidget(this);

  hide();
}

WContainerWidget *WPopupMenu::contents()
{
  return dynamic_cast<WContainerWidget *>(impl_->resolveWidget("contents"));
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

void WPopupMenu::setHidden(bool hidden)
{
  WCompositeWidget::setHidden(hidden);

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

  WApplication::instance()->root()->clicked()
    .disconnect(globalClickConnection_);
  WApplication::instance()->globalEscapePressed()
    .disconnect(globalEscapeConnection_);

  recursiveEventLoop_ = false;

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
}

WPopupMenuItem *WPopupMenu::exec(const WPoint& p)
{
  if (recursiveEventLoop_)
    throw WtException("WPopupMenu::exec(): already in recursive event loop.");

  WebSession *session = WApplication::instance()->session();
  recursiveEventLoop_ = true;

  popup(p);
  do {
    session->doRecursiveEventLoop();
  } while (recursiveEventLoop_);
 
  return result_;
}

WPopupMenuItem *WPopupMenu::exec(const WMouseEvent& e)
{
  return exec(WPoint(e.document().x, e.document().y));
}

WPopupMenuItem *WPopupMenu::exec(WWidget *location, Orientation orientation)
{
  if (recursiveEventLoop_)
    throw WtException("WPopupMenu::exec(): already in recursive event loop.");

  WebSession *session = WApplication::instance()->session();
  recursiveEventLoop_ = true;

  popup(location, orientation);
  do {
    session->doRecursiveEventLoop();
  } while (recursiveEventLoop_);
 
  return result_;
}


}
