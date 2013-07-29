/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WSocketNotifier"
#include "Wt/WApplication"

#include "WebController.h"
#include "WebSession.h"

namespace Wt {

WSocketNotifier::WSocketNotifier(int socket, Type type, WObject *parent)
  : WObject(parent),
    socket_(socket),
    type_(type),
    enabled_(false),
    beingNotified_(false),
    sessionId_(WApplication::instance()->sessionId()),
    activated_(this)
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

    if (!beingNotified_) {
      WebController *controller
	= WApplication::instance()->session()->controller();
      if (enabled_)
        controller->addSocketNotifier(this);
      else
	controller->removeSocketNotifier(this);
    }
  }
}

void WSocketNotifier::dummy()
{
}

void WSocketNotifier::notify()
{
  beingNotified_ = true;

  /*
   * use this connection to know if the notifier was killed while
   * processing the notification
   */
  Wt::Signals::connection alive
    = activated_.connect(this, &WSocketNotifier::dummy);

  activated_.emit(socket_);

  if (alive.connected()) {
    alive.disconnect();

    beingNotified_ = false;

    if (enabled_)
      WApplication::instance()->session()->controller()
        ->addSocketNotifier(this);
  }
}

}
