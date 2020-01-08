// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef FCGI_STREAM_H_
#define FCGI_STREAM_H_

#include "WebStream.h"
#include "WebRequest.h"

struct FCGX_Request;

namespace Wt {

class FCGIStream final : public WebStream
{
public:
  FCGIStream();
  ~FCGIStream();

  virtual WebRequest *getNextRequest(int timeoutsec) override;
};

}

#endif // FCGI_STREAM_H_
