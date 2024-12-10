/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WNotification.h"

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"

namespace Wt {

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

}