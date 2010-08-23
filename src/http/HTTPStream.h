// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef HTTP_STREAM_H_
#define HTTP_STREAM_H_

#include "WebStream.h"
#include "WebRequest.h"

namespace Wt {

class HTTPStream : public WebStream
{
public:
  HTTPStream();

  virtual WebRequest *getNextRequest(int timeoutsec);

};

}

#endif // HTTP_STREAM_H_
