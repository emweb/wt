/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WSocketNotifier.h"
#include "Wt/WApplication.h"

#include "WebController.h"
#include "WebSession.h"

namespace Wt {

WSocketNotifier::WSocketNotifier(int socket, Type type)
  : socket_(socket),
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

  observing_ptr<WSocketNotifier> self(this);

  activated_.emit(socket_);

  if (self) {
    beingNotified_ = false;

    if (enabled_)
      WApplication::instance()->session()->controller()
        ->addSocketNotifier(this);
  }
}

}
