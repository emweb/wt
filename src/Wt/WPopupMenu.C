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

#include "WebSession.h"
#include "WtException.h"

namespace Wt {

WPopupMenu::WPopupMenu()
  : WCompositeWidget(),
    aboutToHide(this),
    parentItem_(0),
    result_(0),
    recursiveEventLoop_(false)
{
  setImplementation(impl_ = new WContainerWidget());
  setPositionScheme(Absolute);
  setStyleClass("Wt-popupmenu");

  const char *CSS_RULES_NAME = "Wt::WPopupMenu";

  WApplication *app = WApplication::instance();

  if (!app->styleSheet().isDefined(CSS_RULES_NAME))
    app->styleSheet().addRule
      (".notselected .Wt-popupmenu", "visibility: hidden;", CSS_RULES_NAME);

  app->domRoot()->addWidget(this);

  hide();
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
  impl_->addWidget(item);
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

  for (int i = 0; i < impl_->count(); ++i) {
    WPopupMenuItem *item = dynamic_cast<WPopupMenuItem *>(impl_->widget(i));
    item->renderOut();
  }
}

void WPopupMenu::done(WPopupMenuItem *result)
{
  result_ = result;

  aboutToHide.emit();

  hide();

  globalClickConnection_.disconnect();
  globalEscapeConnection_.disconnect();

  WApplication::instance()->root()->clicked.senderRepaint();
  WApplication::instance()->root()->escapePressed.senderRepaint();

  if (recursiveEventLoop_) {
    WebSession *session = WApplication::instance()->session();
    recursiveEventLoop_ = false;
    session->unlockRecursiveEventLoop();
  }
}

void WPopupMenu::done()
{
  done(0);
}

void WPopupMenu::popup(WWidget *location)
{
  show();

  WApplication::instance()->doJavaScript
    (WT_CLASS ".positionAtWidget('" + formName() + "','" + location->formName()
     + "');");
}

void WPopupMenu::popup(const WMouseEvent& e)
{
  popup(WPoint(e.document().x, e.document().y));
}

void WPopupMenu::popup(const WPoint& p)
{
  result_ = 0;

  WApplication *app = WApplication::instance();

  // XXX
  // We rely here on the fact that no other widget is listening for
  // escape on the root()
  if (app->root()->escapePressed.isConnected())
    app->root()->escapePressed.emit();

  globalClickConnection_
    = app->root()->clicked.connect(SLOT(this, WPopupMenu::done));
  globalEscapeConnection_
    = app->root()->escapePressed.connect(SLOT(this, WPopupMenu::done));

  prepareRender(app);

  show();

  WApplication::instance()->doJavaScript
    (WT_CLASS ".positionXY('" + formName() + "',"
     + boost::lexical_cast<std::string>(p.x()) + ","
     + boost::lexical_cast<std::string>(p.y()) + ");");
}

void WPopupMenu::prepareRender(WApplication *app)
{
  if (app->environment().agentIE()) {
    app->doJavaScript(jsRef() + ".firstChild.style.width="
		      + jsRef() + ".firstChild.offsetWidth+'px';");
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
  session->doRecursiveEventLoop(std::string());

  return result_;
}

WPopupMenuItem *WPopupMenu::exec(const WMouseEvent& e)
{
  return exec(WPoint(e.document().x, e.document().y));
}

}
