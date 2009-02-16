/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#include <boost/lexical_cast.hpp>
#include <boost/pointer_cast.hpp>

#include "WtReply.h"
#include "StockReply.h"
#include "HTTPRequest.h"
#include "WebController.h"

namespace http {
namespace server {

WtReply::WtReply(const Request& request, const Wt::EntryPoint& entryPoint)
  : Reply(request),
    entryPoint_(entryPoint)
{ }

WtReply::~WtReply()
{ }

void WtReply::consumeRequestBody(Buffer::const_iterator begin,
				 Buffer::const_iterator end,
				 bool endOfRequest)
{
  /*
   * Copy everything to a buffer.
   */
  cin_.append(begin, end);

  if (endOfRequest) {
    responseSent_ = false;
    HTTPRequest *r = new HTTPRequest(boost::dynamic_pointer_cast<WtReply>
				     (shared_from_this()));
    Wt::WebController::instance()->handleRequest(r, &entryPoint_);
  }
}

void WtReply::setContentType(const std::string type)
{
  contentType_ = type;
}

void WtReply::setLocation(const std::string location)
{
  location_ = location;
}

void WtReply::setCout(const std::string text)
{
  cout_.assign(text);

  responseSent_ = false;
}

Reply::status_type WtReply::responseStatus()
{
  return (location_.empty() ? ok : moved_permanently);
}

std::string WtReply::contentType()
{
  return contentType_;
}

std::string WtReply::location()
{
  return location_;
}

boost::intmax_t WtReply::contentLength()
{
  return -1;
}

asio::const_buffer WtReply::nextContentBuffer()
{
  /*
   * We have the whole response in memory. Just have swap ready if
   * your server is serving big stuff. The trade-off is ok: we trade memory
   * for threads, since this is happening asynchronously.
   */
  if (!responseSent_) {
    responseSent_ = true;
    return asio::buffer(cout_);
  } else
    return emptyBuffer;
}

}
}
