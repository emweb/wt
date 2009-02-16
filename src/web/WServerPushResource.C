/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WContainerWidget"

#include "WServerPushResource.h"
#include "WebSession.h"
#include "WebRenderer.h"
#include "WtException.h"

#include <iostream>

namespace Wt {

WServerPushResource::WServerPushResource(WApplication *app)
  : app_(app),
    initialDataSent_(false),
    closing_(false),
    method_(Undefined)
{ }

WServerPushResource::~WServerPushResource()
{
  closing_ = true;
  if (initialDataSent_)
    flush();
}

void WServerPushResource::setArguments(const ArgumentMap& arguments)
{
  if (method_ == Undefined) {
    ArgumentMap::const_iterator i = arguments.find("transport");
    if (i == arguments.end())
      throw WtException("WServerPushResource expects a transport");

    std::string transport = i->second[0];

    if (transport == "iframe")
      method_ = IFrame;
    else if (transport == "xhr_stream")
      method_ = XHRStream;
    else if (transport == "xhr_multipart")
      method_ = XHRMultiPart;
    else if (transport == "server_sent_events")
      method_ = ServerSentEvents;
    else
      throw WtException("WServerPushResource: unknown transport: '"
			+ transport + "'");
  }

  initialDataSent_ = false;
}

const std::string WServerPushResource::resourceMimeType() const
{
  switch (method_) {
  case IFrame:
    return "text/html; charset=UTF-8";
  case XHRStream:
    return "text/plain; charset=UTF-8";
  case XHRMultiPart:
    return "multipart/x-mixed-replace; charset=UTF-8; boundary=\""
      + WApplication::instance()->sessionId() + "\"";
  case ServerSentEvents:
    return "application/x-dom-event-stream; charset=UTF-8;";
  default:
    throw WtException("WServerPushResource: method not set.");
  }
}

void WServerPushResource::triggerUpdate()
{
  if (initialDataSent_)
    flush();
}

void WServerPushResource::sendInitialData(std::ostream& stream)
{
  switch (method_) {
  case IFrame:
    stream << "<html><head><script src='"
	   << WApplication::instance()->resourcesUrl() << "iframe.js'"
	   << " type='text/javascript' charset='UTF-8'></script></head>"
	   << "<body onLoad='reload();'>" << std::endl;
    for (unsigned i = 0; i < 100; ++i)
      stream << "<span></span>";

    break;
  case XHRStream:
    stream << std::string(256, '.') << "\r\n\r\n";

    break;
  case XHRMultiPart:
  case ServerSentEvents:

    break;
  default:
    throw WtException("WServerPushResource: method not set.");
  }
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

bool WServerPushResource::streamResourceData(std::ostream& stream,
					     const ArgumentMap& arguments)
{
  if (closing_)
    return true;
  else {
    if (!initialDataSent_) {
      sendInitialData(stream);

      initialDataSent_ = true;
    }

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
    return false;
  }
}

}
