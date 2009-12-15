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

    if (!beingNotified_)
      if (enabled_)
        WApplication::instance()->session()->controller()->addSocketNotifier(this);
      else
	WApplication::instance()->session()->controller()->removeSocketNotifier(this);
  }
}

void WSocketNotifier::notify()
{
  beingNotified_ = true;
  activated_.emit(socket_);
  beingNotified_ = false;

  if (enabled_)
    WApplication::instance()->session()->controller()->addSocketNotifier(this);
}

}
