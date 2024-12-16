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
/*! \class WNotification Wt/WNotification.h Wt/WNotification.h
 *  \brief A class that represent a notification.
 *
 * A notification is a message sent to the user that can be read
 * from outside the webpage. For a page to be allowed to send
 * notifications, permission must be explicitly given by the user
 * to the application. The permission can be asked to the user with
 * askPermission() or will be asked automatically the first time send()
 * is called in the application. It is possible to listen to the
 * permissionUpdated() signal to react to the user granting or denying
 * permission to send notifications.
 *
 * If the user has previously granted permission to the page to display
 * notifications, the browser will remember it, and %Wt will not ask
 * for that permission again.
 *
 * The WNotification will not be sent to the user if the permission
 * has not yet been granted. Instead, the notification will wait for
 * the user to accept notifications before being sent. This will
 * however not happen if the notification was closed or destroyed.
 *
 * A WNotification represents a notification with a tag. The value of
 * this tag is the id of the WNotification. If the same WNotification
 * is sent multiple times, this will update the notification instead
 * of creating a new one, as long as it was not closed.
 *
 * You can react to a notification being closed or clicked by listening
 * to the closed() and clicked() signals, this does however require
 * that the WNotification has not been destroyed before the event
 * happens.
 *
 * Notifications can have an icon and a badge. Those can be set using
 * setIcon() and setBadge(), but since these are likely the same for
 * all the notifications sent by your application, it is possible to
 * set a default icon and a default badge for all notifications with
 * setDefaultIcon() and setDefaultBadge().
 *
 * \sa send(), close()
 *
 * \note Sending notifications requires that:
 *       - JavaScript is enabled
 *       - the browser supports notifications
 *       - HTTPS is used
 */
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

  /*! \brief Creates a new notification.
   *
   * Creates a new notification with the given title and body.
   */
  explicit WNotification(const WString& title = WString(), const WString& body = WString());

  /*! \brief Sets the title of the notification.
   *
   * This sets the title of the notification.
   *
   * By default, the title is an empty string.
   */
  void setTitle(const WString& title);

  /*! \brief Returns the title of the notification.
   *
   * \sa setTitle()
   */
  WString title() const { return title_; }

  /*! \brief Sets the body of the notification.
   *
   * This sets the body of the notification.
   *
   * By default, the notification has no body, which is
   * represented by an empty string.
   */
  void setBody(const WString& body);

  /*! \brief Returns the body of the notification.
   *
   * Returns the body of the notification or an empty string if the
   * notification has no body.
   *
   * \sa setBody()
   */
  WString body() const { return body_; }

  /*! \brief Sets the link to the notification's icon.
   *
   * This sets the link to the notification's icon which is the image
   * displayed on the left of the notification.
   *
   * By default, the icon is set to the default icon.
   *
   * \sa setDefaultIcon()
   */
  void setIcon(const WLink& iconLink);

  /*! \brief Returns the link to the notification's icon.
   *
   * Returns the link to the notification's icon or a \p null link if the
   * notification uses the default icon.
   *
   * By default, the icon is set to the default icon.
   *
   * \sa setIcon()
   */
  WLink icon() const { return iconLink_; }

  /*! \brief Sets the link to the notification's badge.
   *
   * This sets the link to the notification's badge which is the image
   * displayed when there is not enough space to display the
   * notification.
   *
   * By default, the badge is set to the default badge.
   *
   * \sa setDefaultBadge()
   */
  void setBadge(const WLink& badgeLink);

  /*! \brief Returns the link to the notification's badge.
   *
   * Returns the link to the notification's badge.
   *
   * By default, the badge is set to the default badge.
   *
   * \sa setBadge(), setDefaultBadge()
   */
  WLink badge() const { return badgeLink_; }

  /*! \brief Sets whether the notification should be silent or not.
   *
   * This sets whether the notification should be silent or not. A
   * silent notification will not make any noise or vibration.
   *
   * By default, the notification is not silent.
   *
   * \note A non silent notification may still make no noise or
   *       vibration as it will respect the user's configuration.
   */
  void setSilent(bool enable = true);

  /*! \brief Returns whether the notification should be silent or not.
   *
   * \sa setSilent()
   */
  bool silent() const { return silent_; }

  /*! \brief Sets whether the notification require interaction to become inactive or not.
   *
   * This sets whether the notification should remain active until the
   * user click's on it or dismiss it.
   *
   * By default, this is set to false.
   */
  void setRequireInteraction(bool enable = true);

  /*! \brief Returns whether the notification require interaction to become inactive or not.
   *
   * \sa setRequireInteraction()
   */
  bool requireInteraction() const { return requireInteraction_; }

  /*! \brief Sends the notification.
   *
   * Sends the notification to the user. This only happens if the
   * permission to send notification has been granted by the user.
   *
   * In case the user has not denied or granted notification to
   * the user, the first call to send will instead ask for
   * permission to send notification. The notification will then
   * be sent after the user granted permission to send notification,
   * unless this notification was closed or destroyed.
   *
   * Calling send() multiple times on the same notification will not
   * create multiple notification but replace the previous with
   * the new one, as long as the previous one has not been closed.
   * This means that the user will not be notified again for this
   * notification unless the previous one has been closed.
   *
   * \sa close()
   */
  void send();

  /*! \brief Closes the notification.
   *
   * Closes the notification. This does nothing if the notification
   * was not sent or was already closed.
   *
   * \sa send()
   */
  void close();

  /*! \brief Signal emitted when the notification is clicked.
   */
  JSignal<>& clicked() { return clicked_; }

  /*! \brief Signal emitted when the notification is closed.
   */
  JSignal<>& closed() { return closed_; }

  /*! \brief Signal emitted when the notification is shown.
   */
  JSignal<>& shown() { return shown_; }

  /*! \brief Signal emitted when the notification was not shown due to an error.
   */
  JSignal<>& error() { return error_; }

  /*! \brief Sets the link to the notifications default icon.
   *
   * By default, this is a \p null link which means no default icon is
   * used.
   *
   * \sa setIcon()
   */
  static void setDefaultIcon(const WLink& iconLink);

  /*! \brief Returns the link to the notifications default icon.
   *
   * \sa setDefaultIcon()
   */
  static WLink defaultIcon() { return defaultIconLink_; }

  /*! \brief Sets the link to the notifications default badge.
   *
   * By default, this is a \p null link which means no default badge is
   * used.
   *
   * \sa setBadge()
   */
  static void setDefaultBadge(const WLink& badgeLink);

  /*! \brief Returns the link to the notifications default badge.
   *
   * \sa setDefaultBadge()
   */
  static WLink defaultBadge() { return defaultBadgeLink_; }

  /*! \brief Returns whether the user supports notifications or not.
   */
  static bool supported();

  /*! \brief Ask permission to send notifications to the user.
   *
   * This ask permission to the user to send notifications. This will
   * only be done once per application. More calls to this function will
   * not do anything.
   *
   * \sa permissionUpdated()
   *
   * \note Browsers ignores this if the permission is already denied or
   *       granted.
   */
  static void askPermission();

  /*! \brief Returns the status of the permission to send notifications.
   *
   * \sa askPermission()
   */
  static Permission permission();

  /*! \brief Signal emitted when the permission to send notification has been updated.
   *
   * Signal emitted when the permission to send notification has been
   * updated. This happens when the user answered to askPermission(), and
   * the new permission status is given by the signal as parameter to
   * all connected functions.
   *
   * \sa askPermission()
   */
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