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
  virtual ~WServerPushResource();

  void triggerUpdate();

protected:
  virtual const std::string resourceMimeType() const;
  virtual bool streamResourceData(std::ostream& stream,
				  const ArgumentMap& arguments);
  virtual void setArguments(const ArgumentMap& arguments);

private:
  WApplication *app_;
  bool initialDataSent_;
  bool closing_;

  enum Method { Undefined, IFrame, XHRStream,
		XHRMultiPart, ServerSentEvents };
  Method method_;

  void sendInitialData(std::ostream& stream);
  void streamStringLiteralJSUpdate(std::ostream& s);
};

}

#endif // WSERVER_PUSH_RESOURCE_H_
