/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "DeferredWidget.h"
#include "Notification.h"
#include "TopicTemplate.h"

#include <Wt/WMenu.h>

Notification::Notification()
{
#if 0
  addText(tr("notification-intro"), this);
#endif
}

void Notification::populateSubMenu(Wt::WMenu *menu)
{
  menu->addItem("WNotification",
                deferCreate([this]{ return wnotification(); }))
    ->setPathComponent("");

}


#include "examples/AskNotificationPermission.cpp"
#include "examples/NotificationSend.cpp"
#include "examples/NotificationChat.cpp"

std::unique_ptr<Wt::WWidget> Notification::wnotification()
{
  auto result = std::make_unique<TopicTemplate>("notification-wnotification");

  result->bindWidget("AskNotificationPermission", AskNotificationPermission());
  result->bindWidget("NotificationSend", NotificationSend());
  result->bindWidget("NotificationChat", NotificationChat());

  return std::move(result);
}