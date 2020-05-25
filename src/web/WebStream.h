// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEB_STREAM_H_
#define WEB_STREAM_H_

#include <Wt/WDllDefs.h>

namespace Wt {

class WebRequest;
class WSocketNotifier;

/*
 * Class that implements a stream of (http)-requests.
 *
 * An instance may be passed to the WebController, which will call the
 * method getNextRequest() to get the next request.
 */
class WT_API WebStream
{
public:
  WebStream();

  virtual ~WebStream();

  /*
   * Get the next request, return 0 when a timeout occurs.
   * Otherwise throws an exception.
   */
  virtual WebRequest *getNextRequest(int timeoutsec) = 0;
};

}

#endif // WEB_STREAM_H_
