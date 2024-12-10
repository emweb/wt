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
    body_(body)
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
  defineJavaScript();
}

void WNotification::close()
{
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

void WNotification::defineJavaScript()
{
  WApplication *app = WApplication::instance();
  const char *THIS_JS = "js/WNotification.js";
  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WNotification", appjs1);
  }

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

}