/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#include <boost/lexical_cast.hpp>
#include <boost/pointer_cast.hpp>

// work-around for:
// http://groups.google.com/group/boost-list/browse_thread/thread/8eb0cc99bcda9d41?fwc=2&pli=1
#include <boost/asio.hpp>

#include "Wt/WServer"
#include "WtReply.h"
#include "StockReply.h"
#include "HTTPRequest.h"
#include "WebController.h"
#include "Server.h"
#include "Utils.h"

#include <fstream>

#define DEBUG_WS(a)
//#define DEBUG_WS(a) a

namespace http {
  namespace server {

WtReply::WtReply(const Request& request, const Wt::EntryPoint& entryPoint,
                 const Configuration &config)
  : Reply(request, config),
    entryPoint_(entryPoint),
    contentLength_(-1),
    bodyReceived_(0),
    sendingMessages_(false),
    sending_(false)
{
  urlScheme_ = request.urlScheme;

  status_ = (status_type)0;

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

WtReply::~WtReply()
{
  delete httpRequest_;

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
  ConnectionPtr connection = getConnection();

  if (!connection)
    return;

  if (state != Request::Error) {
    if (status_ != request_entity_too_large)
      cin_->write(begin, static_cast<std::streamsize>(end - begin));

    /*
     * For non-WebSocket requests, we create the HTTPRequest immediately
     * since it may be that the web application is interested in knowing
     * upload progress
     */
    if (!request().webSocketRequest) {
      if (!httpRequest_)
	httpRequest_ = new HTTPRequest(boost::dynamic_pointer_cast<WtReply>
				       (shared_from_this()), &entryPoint_);

      if (end - begin > 0) {
	bodyReceived_ += (end - begin);

	if (!connection->server()->controller()->requestDataReceived
	    (httpRequest_, bodyReceived_, request().contentLength)) {
	  status_ = request_entity_too_large;
	  setCloseConnection();
	  state = Request::Error;
	}
      }
    }
  }

  /*
   * If the state == Request::Partial, we need to wait for more data.
   * Request::Partial is only for purposes above (notifying of upload
   * progress)
   */
  if (state != Request::Partial) {
    if (state == Request::Error) {
      if (status_ < 300) {
	if (status_ == switching_protocols) {
	  /*
	   * We already committed the reply -- all we can (and should)
	   * do now is close the connection.
	   */
	  connection->close();
	  return;
	} else
	  status_ = bad_request;
      }

      setCloseConnection();
    } else if (request().webSocketRequest) {
      DEBUG_WS(std::cerr << "WebSocket request: " << status_ << std::endl);
      setCloseConnection();

      if (status_ == ok) {
	std::string origin = request().getHeader("Origin");
	std::string host = request().getHeader("Host");

	if (host.empty()) {
	  DEBUG_WS(std::cerr << "WebSocket: missing Host " << std::endl);
	  status_ = bad_request;
	} else {
	  status_ = switching_protocols;
	  addHeader("Connection", "Upgrade");
	  addHeader("Upgrade", "WebSocket");
	  if (!origin.empty())
	    addHeader("Sec-WebSocket-Origin", origin);

	  std::string location
	    = request().urlScheme + "://" + host + request().request_path
	    + "?" + request().request_query;
	  addHeader("Sec-WebSocket-Location", location);

	  /*
	   * We defer reading the rest of the handshake until after we
	   * have sent the 101: some intermediaries may be holding back this
	   * data because they are still in HTTP mode
	   */
	  responseSent_ = true;
	  sending_ = true;

	  fetchMoreDataCallback_
	    = boost::bind(&WtReply::readRestWebSocketHandshake, this);

	  DEBUG_WS(std::cerr << "WebSocket: Sending 101" << std::endl);

	  Reply::send();
	  return;
	}
      } else { // status_ == switching_protocols
	/*
	 * We got the nonce and the expected challenge response is
	 * available in in(). This should be copied to out() by the
	 * web session.
	 */
	if (state == Request::Complete) {
	  connection->server()->controller()->handleRequest(httpRequest_);
	} else {
	  std::cerr << "Unreachable code?" << std::endl;
	}

	return;
      }
    }

    if (status_ >= 300) {
      setRelay(ReplyPtr(new StockReply(request(), status_, configuration())));
      Reply::send();
      return;
    }

    cin_->seekg(0); // rewind
    responseSent_ = false;

    connection->server()->controller()->handleRequest(httpRequest_);
  }
}

void WtReply::readRestWebSocketHandshake()
{
  ConnectionPtr connection = getConnection();

  if (connection) {
    DEBUG_WS(std::cerr << "WebSocket: creating HTTPRequest" << std::endl);

    httpRequest_ = new HTTPRequest(boost::dynamic_pointer_cast<WtReply>
				   (shared_from_this()), &entryPoint_);
    httpRequest_->setWebSocketRequest(true);

    DEBUG_WS(std::cerr << "WebSocket: reading handshake" << std::endl);
    connection->handleReadBody();
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

    DEBUG_WS(std::cerr << "WebSocket: new message" << std::endl);

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

bool WtReply::waitMoreData() const
{
  return httpRequest_ != 0 && !httpRequest_->done();
}

void WtReply::send(const std::string& text, CallbackFunction callBack,
		   bool responseComplete)
{
  ConnectionPtr connection = getConnection();

  if (!connection)
    return;

  fetchMoreDataCallback_ = callBack;

  if (request().webSocketRequest && text.empty()) {
    DEBUG_WS(std::cerr << "WebSocket: closed by app" << std::endl);

    connection->close();
    return;
  }

  if (request().webSocketRequest && sendingMessages_) {
    DEBUG_WS(std::cerr << "WebSocket: sending message" << std::endl);
    nextCout_.clear();
    nextCout_ += (char)0;
    nextCout_ += text;
    nextCout_ += (char)0xFF;
  } else {
    nextCout_.assign(text);
    sendingMessages_ = true;
  }

  responseSent_ = false;

  if (!sending_) {
    if (!status_) {
      if (!transmitting() && fetchMoreDataCallback_) {
	/*
	 * We haven't got a response status, so we can't send anything really.
	 * Instead, we immediately invoke the fetchMoreDataCallback_
	 *
	 * This is used in a resource continuation which indicates to wait
	 * for more data before sending anything at all.
	 */
	CallbackFunction f = fetchMoreDataCallback_;
	fetchMoreDataCallback_ = 0;
	f();
	return;
      } else {
	/*
	 * The old behaviour was to assume 200 ok by default.
	 */
	status_ = ok;
      }
    }

    sending_ = true;
    Reply::send();
  }
}

void WtReply::readWebSocketMessage(CallbackFunction callBack)
{
  DEBUG_WS(std::cerr << "WebSocket: reading message" << std::endl);

  ConnectionPtr connection = getConnection();

  assert(request().webSocketRequest);

  if (readMessageCallback_)
    return;

  readMessageCallback_ = callBack;

  if (!connection) {
    Buffer b;
    consumeWebSocketMessage(b.begin(), b.begin(), Request::Error);
  } else {
    if (&cin_mem_ != cin_) {
      dynamic_cast<std::fstream *>(cin_)->close();
      delete cin_;
      cin_ = &cin_mem_;
    }

    cin_mem_.str("");

    connection->server()->service().post
      (boost::bind(&Connection::handleReadBody, connection));
  }
}

bool WtReply::readAvailable()
{
  ConnectionPtr connection = getConnection();

  if (connection)
    return connection->readAvailable();
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
