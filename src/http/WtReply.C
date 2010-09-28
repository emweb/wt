/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#include <boost/lexical_cast.hpp>
#include <boost/pointer_cast.hpp>

#include "Wt/WServer"
#include "WtReply.h"
#include "StockReply.h"
#include "HTTPRequest.h"
#include "WebController.h"
#include "Server.h"
#include "Utils.h"

#include <fstream>

namespace http {
  namespace server {

WtReply::WtReply(const Request& request, const Wt::EntryPoint& entryPoint,
                 const Configuration &config)
  : Reply(request),
    entryPoint_(entryPoint),
    sending_(false),
    status_(ok),
    contentLength_(-1),
    fetchMoreData_(0)
{
  if (request.contentLength > config.maxMemoryRequestSize()) {
    requestFileName_ = Wt::Utils::createTempFileName();
    // First, create the file
    std::ofstream o(requestFileName_.c_str());
    o.close();
    // Now, open it for read/write
    cin_ = new std::fstream(requestFileName_.c_str(),
      std::ios::in | std::ios::out | std::ios::binary);
    if (!*cin_) {
      // Give up, spool to memory
      requestFileName_ = "";
      delete cin_;
      cin_ = &cin_mem_;
    }
  } else {
    cin_ = &cin_mem_;
  }
}

WtReply::~WtReply()
{
  if (&cin_mem_ != cin_) {
    dynamic_cast<std::fstream *>(cin_)->close();
    delete cin_;
  }
  if (requestFileName_ != "") {
    unlink(requestFileName_.c_str());
  }
}

void WtReply::consumeRequestBody(Buffer::const_iterator begin,
				 Buffer::const_iterator end,
				 bool endOfRequest)
{
  /*
   * Copy everything to a buffer.
   */
  cin_->write(begin, static_cast<std::streamsize>(end - begin));

  if (endOfRequest) {
    cin_->flush();
    cin_->seekg(0); // rewind
    responseSent_ = false;
    HTTPRequest *r = new HTTPRequest(boost::dynamic_pointer_cast<WtReply>
				     (shared_from_this()), &entryPoint_);
    connection()->server()->controller()->server_->handleRequest(r);
  }
}

void WtReply::setStatus(int status)
{
  status_ = (status_type)status;
}

void WtReply::setContentLength(boost::intmax_t length)
{
  contentLength_ = length;
}

void WtReply::setContentType(const std::string& type)
{
  contentType_ = type;
}

void WtReply::setLocation(const std::string& location)
{
  location_ = location;
  if (status_ < 300)
    status_ = found;
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
  return status_;
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
  return contentLength_;
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
