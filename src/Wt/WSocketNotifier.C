/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WSocketNotifier"
#include "Wt/WApplication"

#include "WebController.h"

namespace Wt {

WSocketNotifier::WSocketNotifier(int socket, Type type, WObject *parent)
  : WObject(parent),
    activated(this),
    socket_(socket),
    type_(type),
    enabled_(false),
    beingNotified_(false),
    sessionId_(WApplication::instance()->sessionId())
{
  setEnabled(true);
}

WSocketNotifier::~WSocketNotifier()
{
  setEnabled(false);
}

void WSocketNotifier::setEnabled(bool enabled)
{
  if (enabled != enabled_) {
    enabled_ = enabled;

    if (!beingNotified_)
      if (enabled_)
	WebController::instance()->addSocketNotifier(this);
      else
	WebController::instance()->removeSocketNotifier(this);
  }
}

void WSocketNotifier::notify()
{
  beingNotified_ = true;
  activated.emit(socket_);
  beingNotified_ = false;

  if (enabled_)
    WebController::instance()->addSocketNotifier(this);
}

}
