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
#include "Server.h"

namespace http {
  namespace server {

WtReply::WtReply(const Request& request, const Wt::EntryPoint& entryPoint)
  : Reply(request),
    entryPoint_(entryPoint),
    sending_(false),
    fetchMoreData_(0)
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
    connection()->server()->controller()->handleRequest(r, &entryPoint_);
  }
}

void WtReply::setContentType(const std::string& type)
{
  contentType_ = type;
}

void WtReply::setLocation(const std::string& location)
{
  location_ = location;
}

bool WtReply::expectMoreData()
{
  return fetchMoreData_ != 0;
}

void WtReply::send(const std::string& text, CallbackFunction callBack,
		   void *cbData)
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED

  fetchMoreData_ = callBack;
  callBackData_ = cbData;

  nextCout_.assign(text);

  if (!sending_) {
    sending_ = true;

    Reply::send();
  }
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
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED

  cout_.swap(nextCout_);

  if (!responseSent_) {
    responseSent_ = true;
    if (!cout_.empty())
      return asio::buffer(cout_);
  } else
    cout_.clear();

  while (cout_.empty() && fetchMoreData_) {
    CallbackFunction f = fetchMoreData_;
    fetchMoreData_ = 0;
    (*f)(callBackData_);
    cout_.swap(nextCout_);
  }

  // This is the last packet, unless we wait for more data
  if (cout_.empty()) {
    responseSent_ = false;
    sending_ = false;
  }

  return asio::buffer(cout_);
}

  }
}
