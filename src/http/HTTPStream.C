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

}
