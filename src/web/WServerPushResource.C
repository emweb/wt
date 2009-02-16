/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/Http/Request"
#include "Wt/Http/Response"

#include "WServerPushResource.h"
#include "WebSession.h"
#include "WebRenderer.h"
#include "WtException.h"

#include <iostream>

namespace Wt {

WServerPushResource::WServerPushResource(WApplication *app)
  : app_(app),
    continuation_(0),
    method_(Undefined)
{ }

void WServerPushResource::triggerUpdate()
{
  if (continuation_)
    continuation_->doContinue();
}

void WServerPushResource::streamStringLiteralJSUpdate(std::ostream& s)
{
  /* FIXME: expand the render interface to also use EscapeOStream */
  if (app_->domRoot_) {
    std::stringstream ss;
    app_->session()->renderer().streamJavaScriptUpdate(ss, -1, false);
    DomElement::jsStringLiteral(s, ss.str(), '\'');
  }
}

void WServerPushResource::initIFrameMethod(Http::Response& response)
{
  method_ = IFrame;
  response.setMimeType("text/html; charset=UTF-8");
  response.out() << "<html><head><script src='"
		 << WApplication::instance()->resourcesUrl() << "iframe.js'"
		 << " type='text/javascript' charset='UTF-8'></script></head>"
		 << "<body onLoad='reload();'>" << std::endl;
  for (unsigned i = 0; i < 100; ++i)
    response.out() << "<span></span>";
}

void WServerPushResource::initXhrStreamMethod(Http::Response& response)
{
  method_ = XHRStream;
  response.setMimeType("text/plain; charset=UTF-8");

#ifndef WT_TARGET_JAVA
  response.out() << std::string(256, '.') << "\r\n\r\n";
#else // WT_TARGET_JAVA
  for (int i = 0; i < 256; ++i)
    response.out() << '.';
  response.out() << "\r\n\r\n";
#endif // WT_TARGET_JAVA
}

void WServerPushResource::initXhrMultiPartMethod(Http::Response& response)
{
  method_ = XHRMultiPart;
  response.setMimeType("multipart/x-mixed-replace; charset=UTF-8; "
			  "boundary=\""
			  + WApplication::instance()->sessionId() + "\"");
}

void WServerPushResource::initServerSentEventsMethod(Http::Response& response)
{
  method_ = ServerSentEvents;
  response.setMimeType("application/x-dom-event-stream; charset=UTF-8;");
}

void WServerPushResource::handleRequest(const Http::Request& request,
					Http::Response& response)
{
  continuation_ = request.continuation();

  if (!continuation_) {
    const std::string *transport = request.getParameter("transport");
    if (transport) {
      if (*transport == "iframe")
	initIFrameMethod(response);
      else if (*transport == "xhr_stream") 
	initXhrStreamMethod(response);
      else if (*transport == "xhr_multipart")
	initXhrMultiPartMethod(response);
      else if (*transport == "server_sent_events")
	initServerSentEventsMethod(response);
      else
	throw WtException("WServerPushResource: unknown transport: '"
			  + *transport + "'");
    }
  }

  std::ostream& stream = response.out();

  switch(method_) {
  case IFrame:
    stream << "<script>e(";
    streamStringLiteralJSUpdate(stream);
    stream << ");</script>" << std::endl;
    
    break;
  case XHRStream:
    streamStringLiteralJSUpdate(stream);
    stream << "\r\n|O|\r\n";
 
    break;
  case XHRMultiPart:
    {
      std::stringstream s;
      streamStringLiteralJSUpdate(s);

      stream << "Content-Type: application/json\r\n"
	     << "Content-length: " << s.str().length()
	     << "\r\n\r\n" << s.str()
	     << "\r\n--" << WApplication::instance()->sessionId() << "\r\n";
    }

    break;
  case ServerSentEvents:
    stream << "Event: orbited\n"
	   << "data: ";
    streamStringLiteralJSUpdate(stream);
    stream << "\n\n";

    break;
  default:
    throw WtException("WServerPushResource: method not set.");
  }

  continuation_ = response.createContinuation(Http::ServerEventContinuation);
}

}
