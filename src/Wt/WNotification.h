// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WNOTIFICATION_H_
#define WT_WNOTIFICATION_H_

#include "Wt/WObject.h"
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

  void send();
  void close();

  static bool supported();
  static void askPermission();
  static Permission permission();
  static Signal<Permission>& permissionUpdated();

private:
  WString title_;
  WString body_;

  std::string jsRef();
  void defineJavaScript();
};


}
#endif //WT_WNOTIFICATION_H_