/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/asio.hpp>

#include "WebMain.h"

#include <boost/bind.hpp>

#include <Wt/WIOService>
#include <Wt/WServer>

#include "Configuration.h"
#include "WebController.h"
#include "WebStream.h"

namespace Wt {

WebMain::WebMain(WServer *server, WebStream *stream,
		 std::string singleSessionId)
  : server_(server),
    stream_(stream),
    singleSessionId_(singleSessionId),
    shutdown_(false)
{ }

WebMain::~WebMain()
{ }

void WebMain::shutdown()
{
  shutdown_ = true;
  controller().shutdown();
}

void WebMain::run()
{
  server_->ioService().start();

  WebRequest *request = stream_->getNextRequest(10);

  if (request)
    controller().handleRequest(request);
  else
    if (!singleSessionId_.empty()) {
      server_->log("error") << "No initial request ?";
      return;
    }

  for (;;) {
    bool haveMoreSessions = controller().expireSessions();

    if (!haveMoreSessions && !singleSessionId_.empty()) {
      server_->log("notice") << "Dedicated session process exiting cleanly.";
      break;
    }

    WebRequest *request = stream_->getNextRequest(5);

    if (shutdown_) {
      server_->log("notice") << "Shared session server exiting cleanly.";
      break;
    }

    if (request)
      server_->ioService().post(boost::bind(&WebController::handleRequest,
					    &controller(), request));
  }

  server_->ioService().stop();
}

}
