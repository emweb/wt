/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <assert.h>

#include "Wt/WSocketNotifier"

#include "HTTPStream.h"
#include "Server.h"

namespace Wt {

HTTPStream::HTTPStream()
  : WebStream(true)
{ }

WebRequest *HTTPStream::getNextRequest(int timeoutsec)
{
  assert(false);
  return 0;
}

void HTTPStream::addSocketNotifier(WSocketNotifier *notifier)
{
  switch (notifier->type()) {
  case WSocketNotifier::Read:
    http::server::Server::instance()->select_read(notifier->socket());
    break;
  case WSocketNotifier::Write:
    http::server::Server::instance()->select_write(notifier->socket());
    break;
  case WSocketNotifier::Exception:
    http::server::Server::instance()->select_except(notifier->socket());
    break;
  }    
}

void HTTPStream::removeSocketNotifier(WSocketNotifier *notifier)
{
  http::server::Server::instance()->stop_select(notifier->socket());
}

}
