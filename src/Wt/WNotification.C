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

WLink WNotification::defaultIconLink_ = WLink();
WLink WNotification::defaultBadgeLink_ = WLink();

WNotification::WNotification(const WString& title, const WString& body)
  : title_(title),
    body_(body),
    iconLink_(defaultIconLink_),
    badgeLink_(defaultBadgeLink_),
    silent_(false),
    requireInteraction_(false),
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

void WNotification::setIcon(const WLink& iconLink)
{
  iconLink_ = iconLink;
}

void WNotification::setBadge(const WLink& badgeLink)
{
  badgeLink_ = badgeLink;
}

void WNotification::setSilent(bool enable)
{
  silent_ = enable;
}

void WNotification::setRequireInteraction(bool enable)
{
  requireInteraction_ = enable;
}

void WNotification::send()
{
  if (WNotification::supported()) {
    update();
    if (WNotification::permission() == Permission::Default) {
      askPermission();
      conn_ = permissionUpdated().connect(this, &WNotification::sendJs);
    } else {
      sendJs();
    }
  }
}

void WNotification::close()
{
  if (WNotification::supported()) {
    if (WNotification::permission() == Permission::Granted) {
      loadJavaScript();

      WApplication *app = WApplication::instance();
      WStringStream js;
      js << app->javaScriptClass() << ".WNotification.close(" << jsRef() << ");";
      WApplication::instance()->doJavaScript(js.str().c_str());
    }
    conn_.disconnect();
  }
}

bool WNotification::supported()
{
  return WApplication::instance()->environment().supportsNotifications();
}

void WNotification::askPermission()
{
  WApplication *app = WApplication::instance();
  if (!app->notificationPermissionAsked_) {
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
    app->notificationPermissionAsked_ = true;
  }
}

void WNotification::setDefaultIcon(const WLink& iconLink)
{
  defaultIconLink_ = iconLink;
}

void WNotification::setDefaultBadge(const WLink& badgeLink)
{
  defaultBadgeLink_ = badgeLink;
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

void WNotification::update()
{
  if (WNotification::permission() != Permission::Denied) {
    WApplication *app = WApplication::instance();
    WStringStream js;
    js << app->javaScriptClass() << ".WNotification.create("
      << jsRef() << ","
      << title_.jsStringLiteral() << ",{"
      << "tag:"<< WString(id()).jsStringLiteral()<<",";
    if (!body_.empty()) {
      js << "body:"<< body_.jsStringLiteral()<<",";
    }
    if (!icon().isNull()) {
      std::string url = icon().resolveUrl(app);
      url = app->encodeUntrustedUrl(url);
      js << "icon:"<< WString(url).jsStringLiteral()<<",";
    }
    if (!badge().isNull()) {
      std::string url = badge().resolveUrl(app);
      url = app->encodeUntrustedUrl(url);
      js << "badge:"<< WString(url).jsStringLiteral()<<",";
    }
    if (silent_) {
      js << "silent:true,";
    }
    if (requireInteraction_) {
      js << "requireInteraction:true,";
    }
    js << "});";

    js_ = js.str();
  }
}

void WNotification::sendJs()
{
  if (WNotification::permission() == Permission::Granted) {
    loadJavaScript();
    WApplication::instance()->doJavaScript(js_.c_str());
  }
  conn_.disconnect();
}

}