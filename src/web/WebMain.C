/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "WebMain.h"

#include <boost/bind.hpp>

#include <Wt/WIOService>
#include <Wt/WServer>

#include "WebController.h"
#include "WebStream.h"

namespace Wt {

LOGGER("WebMain");

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
      LOG_ERROR("no initial request ?");
      return;
    }

  for (;;) {
    bool haveMoreSessions = controller().expireSessions();

    if (!haveMoreSessions && !singleSessionId_.empty())
      break;

    WebRequest *request = stream_->getNextRequest(5);

    if (shutdown_)
      break;

    if (request) {
      // Attention: WIOService::post() uses a global strand to ensure keeping
      // the order in which events were posted. For processing WebRequests,
      // this is very bad since at most one handleRequest() will be executed
      // simultaneously. Additionaly, this breaks recursive event loops.
      // Asio's io_service::post does no such thing, so calling the function
      // of the superclass if fine.
      static_cast<boost::asio::io_service&>(server_->ioService())
	.post(boost::bind(&WebController::handleRequest,
	      &controller(), request));
    }
  }

  server_->ioService().stop();
}

}
