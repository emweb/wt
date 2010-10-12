/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Http/Response"
#include "Wt/Http/ResponseContinuation"
#include "Wt/WResource"
#include "WebRequest.h"

namespace Wt {
  namespace Http {

void Response::setStatus(int status)
{
  if (response_)
    response_->setStatus(status);
}

void Response::setContentLength(::uint64_t length)
{
  if (response_)
    response_->setContentLength(length);
}

void Response::setMimeType(const std::string& mimeType)
{
  if (response_)
    response_->setContentType(mimeType);
}

void Response::addHeader(const std::string& name, const std::string& value)
{
  if (response_)
    response_->addHeader(name, value);
}

ResponseContinuation *Response::createContinuation()
{
  if (!continuation_)
    continuation_ = new ResponseContinuation(resource_, response_);
  else
    continuation_->resource_ = resource_;

  return continuation_;
}

ResponseContinuation *Response::continuation() const
{
  if (continuation_ && continuation_->resource_)
    return continuation_;
  else
    return 0;
}

WT_BOSTREAM& Response::out()
{
  if (out_)
    return *out_;
  else
    return response_->out();
}

Response::Response(WResource *resource, WebResponse *response,
		   ResponseContinuation *continuation)
  : resource_(resource),
    response_(response),
    continuation_(continuation),
    out_(0)
{ }

Response::Response(WResource *resource, WT_BOSTREAM& out)
  : resource_(resource),
    response_(0),
    continuation_(0),
    out_(&out)
{ }

  }
}
