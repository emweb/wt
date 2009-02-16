// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSERVER_PUSH_RESOURCE_H_
#define WSERVER_PUSH_RESOURCE_H_

#include "Wt/WResource"
#include "DomElement.h"

namespace Wt {

class WApplication;

class WT_API WServerPushResource : public WResource
{
public:
  WServerPushResource(WApplication *app);

  void triggerUpdate();

protected:
  virtual void handleRequest(const Http::Request& request,
			     Http::Response& response);

private:
  WApplication               *app_;
  Http::ResponseContinuation *continuation_;

  enum Method { Undefined, IFrame, XHRStream,
		XHRMultiPart, ServerSentEvents };
  Method method_;

  void initIFrameMethod(Http::Response& response);
  void initXhrStreamMethod(Http::Response& response);
  void initXhrMultiPartMethod(Http::Response& response);
  void initServerSentEventsMethod(Http::Response& response);

  void streamStringLiteralJSUpdate(std::ostream& s);
};

}

#endif // WSERVER_PUSH_RESOURCE_H_
