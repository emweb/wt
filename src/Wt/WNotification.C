/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WNotification.h"

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WJavaScriptPreamble.h"
#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"

#ifndef WT_DEBUG_JS
#include "js/WNotification.min.js"
#endif

namespace Wt {

WNotification::WNotification(const WString& title, const WString& body)
  : title_(title),
    body_(body),
    clicked_(this, "clicked"),
    closed_(this, "closed"),
    shown_(this, "shown"),
    error_(this, "error")
{ }

void WNotification::setTitle(const WString& title)
{
  title_ = title;
}

void WNotification::setBody(const WString& body)
{
  body_ = body;
}

void WNotification::send()
{
  loadJavaScript();

  WApplication *app = WApplication::instance();
  WStringStream js;
 js << app->javaScriptClass() << ".WNotification.create("
     << jsRef() << ","
     << title_.jsStringLiteral() << ",{";
  if (!body_.empty()) {
    js << "body:"<< body_.jsStringLiteral();
  }
  js << "});";

  app->doJavaScript(js.str().c_str());
}

void WNotification::close()
{
  loadJavaScript();

  WApplication *app = WApplication::instance();
  WStringStream js;
  js << app->javaScriptClass() << ".WNotification.close(" << jsRef() << ");";
  WApplication::instance()->doJavaScript(js.str().c_str());
}

bool WNotification::supported()
{
  return WApplication::instance()->environment().supportsNotifications();
}

void WNotification::askPermission()
{
  WApplication *app = WApplication::instance();
  WStringStream js;
  js << "if ("<<WString("Notification").jsStringLiteral()<<" in window){"
     <<   "Notification.requestPermission((result) => {"
     <<     "Wt.emit("
     <<       app->javaScriptClass()
     <<       ","<<WString("Wt-updateNotificationPermission").jsStringLiteral()
     <<       ",result);"
     <<   "});"
     << "}";
  app->doJavaScript(js.str().c_str());
}

WNotification::Permission WNotification::permission()
{
  return WApplication::instance()->environment().notificationPermission();
}

Signal<WNotification::Permission>& WNotification::permissionUpdated()
{
  return WApplication::instance()->notificationPermissionChanged_;
}

std::string WNotification::jsRef()
{
  return WString(id()).jsStringLiteral();
}

void WNotification::loadJavaScript()
{
  WApplication *app = WApplication::instance();
  const char *THIS_JS = "js/WNotification.js";

  if (!app->javaScriptLoaded(THIS_JS)) {
    WStringStream js;
    LOAD_JAVASCRIPT(app, THIS_JS, "WNotificationSetup", appjs2);
    js << app->javaScriptClass() << ".WNotificationSetup("<< app->javaScriptClass() <<");\n";
    app->doJavaScript(js.str().c_str());
  }
}

}