// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WNOTIFICATION_H_
#define WT_WNOTIFICATION_H_

#include "Wt/WObject.h"
#include "Wt/WJavaScript.h"
#include "Wt/WLink.h"
#include "Wt/WSignal.h"
#include "Wt/WString.h"

namespace Wt {

class WT_API WNotification : public WObject
{
public:
  /*! \brief Enumeration representing the permission status of notifications.
  *
  * For an application to show notifications, it must first ask the authorization of the
  * user. This enumeration represent the different possible permisson status that the
  * application can have.
  */
  enum class Permission {
    Default, //!< Permission not yet denied or granted by the user. Notifications won't be displayed.
    Granted, //!< Permission granted by the user. Notifications can be displayed.
    Denied   //!< Permission denied by the user. Notifications won't be displayed.
  };

  explicit WNotification(const WString& title = WString(), const WString& body = WString());

  void setTitle(const WString& title);
  WString title() const { return title_; }

  void setBody(const WString& body);
  WString body() const { return body_; }

  void setIcon(const WLink& iconLink);
  WLink icon() const { return iconLink_; }

  void setBadge(const WLink& badgeLink);
  WLink badge() const { return badgeLink_; }

  void setSilent(bool enable = true);
  bool silent() const { return silent_; }

  void setRequireInteraction(bool enable = true);
  bool requireInteraction() const { return requireInteraction_; }

  void send();
  void close();

  JSignal<>& clicked() { return clicked_; }
  JSignal<>& closed() { return closed_; }
  JSignal<>& shown() { return shown_; }
  JSignal<>& error() { return error_; }

  static void setDefaultIcon(const WLink& iconLink);
  static WLink defaultIcon() { return defaultIconLink_; }

  static void setDefaultBadge(const WLink& badgeLink);
  static WLink defaultBadge() { return defaultBadgeLink_; }

  static bool supported();
  static void askPermission();
  static Permission permission();
  static Signal<Permission>& permissionUpdated();

private:
  WString title_;
  WString body_;
  WLink iconLink_;
  WLink badgeLink_;
  bool silent_;
  bool requireInteraction_;

  JSignal<> clicked_, closed_, shown_, error_;

  std::string js_;
  Signals::connection conn_;

  std::string jsRef();
  void loadJavaScript();
  void update();
  void sendJs();

  static WLink defaultIconLink_;
  static WLink defaultBadgeLink_;
};


}
#endif //WT_WNOTIFICATION_H_