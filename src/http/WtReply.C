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
    contentLength_(-1),
    bodyReceived_(0),
    sendingMessages_(false),
    sending_(false)
{
  urlScheme_ = request.urlScheme;

  status_ = ok;

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

  httpRequest_ = 0;
}

void WtReply::release()
{
  delete httpRequest_;
  httpRequest_ = 0;
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

void WtReply::consumeData(Buffer::const_iterator begin,
			  Buffer::const_iterator end,
			  Request::State state)
{
  if (readMessageCallback_)
    consumeWebSocketMessage(begin, end, state);
  else
    consumeRequestBody(begin, end, state);
}

void WtReply::consumeRequestBody(Buffer::const_iterator begin,
				 Buffer::const_iterator end,
				 Request::State state)
{
  if (status_ != request_entity_too_large)
    cin_->write(begin, static_cast<std::streamsize>(end - begin));

  if (!request().isWebSocketRequest()) {
    if (!httpRequest_)
      httpRequest_ = new HTTPRequest(boost::dynamic_pointer_cast<WtReply>
				     (shared_from_this()), &entryPoint_);

    if (end - begin > 0) {
      bodyReceived_ += (end - begin);

      if (!connection()->server()->controller()->requestDataReceived
	  (httpRequest_, bodyReceived_, request().contentLength)) {
	status_ = request_entity_too_large;
	setCloseConnection();
	state = Request::Error;
      }
    }
  }

  if (state != Request::Partial) {
    if (request().isWebSocketRequest()) {
      if (status_ == ok) {
	assert(state == Request::Complete);

	std::string origin = request().headerMap.find("Origin")->second;

	status_ = switching_protocols;
	addHeader("Connection", "Upgrade");
	addHeader("Upgrade", "WebSocket");
	addHeader("Sec-WebSocket-Origin", origin);

	std::string location
	  = "ws" + origin.substr(4)
	  + request().request_path + "?" + request().request_query;
	addHeader("Sec-WebSocket-Location", location);

	/*
	 * We defer reading the rest of the handshake until after we
	 * have sent the 101: some intermediaries may be holding back this
	 * data because they are still in HTTP mode
	 */
	setWaitMoreData(true);
	responseSent_ = true;
	sending_ = true;
	Reply::send();

	// This will read more data, starting with the hand-shake.
	// The computed handshake response will be passed to the next
	// invocation of this method.
	connection()->handleReadBody();
      } else {
	/*
	 * We got the nonce and the expected challenge response is
	 * available in in(). This should be copied to out() by the
	 * web.
	 */
	if (state == Request::Complete) {
	  HTTPRequest *r = new HTTPRequest(boost::dynamic_pointer_cast<WtReply>
					   (shared_from_this()), &entryPoint_);

	  connection()->server()->controller()->server_->handleRequest(r);
	} else {
	  setWaitMoreData(false);
	  setCloseConnection();
	  Reply::send();
	  return;
	}
      }

      return;
    }

    if (status_ >= 300) {
      release();
      setRelay(ReplyPtr(new StockReply(request(), status_)));
      Reply::send();
      return;
    }

    assert(state == Request::Complete);

    cin_->seekg(0); // rewind
    responseSent_ = false;

    HTTPRequest *r = httpRequest_;
    httpRequest_ = 0;
    connection()->server()->controller()->server_->handleRequest(r);
  }
}

void WtReply::consumeWebSocketMessage(Buffer::const_iterator begin,
				      Buffer::const_iterator end,
				      Request::State state)
{
  cin_mem_.write(begin, static_cast<std::streamsize>(end - begin));

  if (state != Request::Partial) {
    if (state == Request::Error)
      cin_mem_.str("");
    else {
      cin_mem_.seekg(0);
    }

    CallbackFunction cb = readMessageCallback_;
    readMessageCallback_ = 0;
    cb();
  }
}

void WtReply::setStatus(int status)
{
  status_ = (status_type)status;
}

void WtReply::setContentLength(::int64_t length)
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

void WtReply::send(const std::string& text, CallbackFunction callBack)
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED

  if (!connection())
    return;

  fetchMoreDataCallback_ = callBack;

  if (request().isWebSocketRequest() && sendingMessages_) {
    // std::cerr << this << " Sending frame of length "
    //           << text.length() << std::endl;
    nextCout_.clear();
    nextCout_ += (char)0;
    nextCout_ += text;
    nextCout_ += (char)0xFF;
  } else {
    // std::cerr << this << "Sending response of length "
    //           << text.length() << std::endl;
    nextCout_.assign(text);
    sendingMessages_ = true;
  }

  responseSent_ = false;

  if (!sending_) {
    sending_ = true;
    Reply::send();
  }
}

void WtReply::readWebSocketMessage(CallbackFunction callBack)
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED

  assert(request().isWebSocketRequest());

  if (readMessageCallback_)
    return;

  readMessageCallback_ = callBack;

  if (&cin_mem_ != cin_) {
    dynamic_cast<std::fstream *>(cin_)->close();
    delete cin_;
    cin_ = &cin_mem_;
  }

  cin_mem_.str("");

  if (connection())
    connection()->handleReadBody();
}

bool WtReply::readAvailable()
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED

  if (connection())
    return connection()->readAvailable();
  else
    return false;
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

::int64_t WtReply::contentLength()
{
  return contentLength_;
}

asio::const_buffer WtReply::nextContentBuffer()
{
  // std::cerr << this << "(sending: " << sending_
  // 	       << ", reponseSent_: " << responseSent_
  //	       << ") nextContentBuffer: " << nextCout_.length() << std::endl;
  cout_.clear();
  cout_.swap(nextCout_);

  if (!responseSent_) {
    responseSent_ = true;
    if (!cout_.empty())
      return asio::buffer(cout_);
  } else
    cout_.clear();

  while (cout_.empty() && fetchMoreDataCallback_) {
    CallbackFunction f = fetchMoreDataCallback_;
    fetchMoreDataCallback_ = 0;
    f();
    cout_.swap(nextCout_);
  }

  if (cout_.empty())
    sending_ = false;

  return asio::buffer(cout_);
}

  }
}
