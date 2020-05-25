// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef ISAPI_STREAM_H_
#define ISAPI_STREAM_H_

#include "WebStream.h"
#include "WebRequest.h"

class FCGX_Request;

namespace Wt {
  namespace isapi {

class IsapiServer;

class IsapiStream : public WebStream
{
public:
  IsapiStream(IsapiServer *server);
  ~IsapiStream();

  virtual WebRequest *getNextRequest(int timeoutsec);

private:
  IsapiServer *server_;
};

}
}

#endif // ISAPI_STREAM_H_
