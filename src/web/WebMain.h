// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WEB_MAIN_H_
#define WT_WEB_MAIN_H_

#include "Wt/WServer.h"

namespace Wt {

class WebStream;
/*
 * This controls a main loop for connectors which are not
 * event-driven. It reads incoming requests from a webstream, and
 * schedules the handling of them into a thread pool.
 *
 * It also deal with event scheduling.
 */
class WT_API WebMain
{
public:
  WebMain(WServer *server, WebStream *stream,
	  std::string singleSessionId = std::string());
  ~WebMain();

  WebController& controller() { return *server_->controller(); }

  void run();
  void shutdown();

private:
  WServer *server_;
  WebStream *stream_;
  std::string singleSessionId_;

  bool shutdown_;
};

}

#endif // WEB_MAIN_H_
