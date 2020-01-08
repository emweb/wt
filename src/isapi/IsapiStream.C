/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "IsapiStream.h"
#include "IsapiRequest.h"
#include "Server.h"

namespace Wt {
  namespace isapi {

IsapiStream::IsapiStream(IsapiServer *server)
  : server_(server)
{
}

IsapiStream::~IsapiStream()
{ }

WebRequest *IsapiStream::getNextRequest(int timeoutsec)
{
  return server_->popRequest(timeoutsec);
}

}
}
