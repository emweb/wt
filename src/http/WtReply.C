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
#include "WebUtils.h"
#include "FileUtils.h"

#include <fstream>

namespace Wt {
  LOGGER("wthttp");
}

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

  if (request.contentLength > config.maxMemoryRequestSize()) {
    requestFileName_ = Wt::FileUtils::createTempFileName();
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
  consumeRequestBody(begin, end, state);
}

void WtReply::consumeRequestBody(Buffer::const_iterator begin,
				 Buffer::const_iterator end,
				 Request::State state)
{
  ConnectionPtr connection = getConnection();

  if (!connection)
    return;

  if (request().webSocketVersion < 0) {
    /*
     * A normal HTTP request
     */
    if (state != Request::Error) {
      if (status() != request_entity_too_large)
	cin_->write(begin, static_cast<std::streamsize>(end - begin));

      /*
       * We create the HTTPRequest immediately since it may be that
       * the web application is interested in knowing upload progress
       */
      if (!httpRequest_)
	httpRequest_ = new HTTPRequest(boost::dynamic_pointer_cast<WtReply>
				       (shared_from_this()), &entryPoint_);

      if (end - begin > 0) {
	bodyReceived_ += (end - begin);

	if (!connection->server()->controller()->requestDataReceived
	    (httpRequest_, bodyReceived_, request().contentLength)) {
	  setStatus(request_entity_too_large);
	  setCloseConnection();
	  state = Request::Error;
	}
      }
    }

    if (state == Request::Error) {
      if (status() < 300)
	setStatus(bad_request); // or internal error ?

      setCloseConnection();
    }

    if (state != Request::Partial) {
      if (status() >= 300) {
	setRelay(ReplyPtr(new StockReply(request(),
					 status(), configuration())));
	Reply::send();
      } else {
	cin_->seekg(0); // rewind
	responseSent_ = false;

	connection->server()->controller()->handleRequest(httpRequest_);
      }
    }
  } else {
    /*
     * WebSocket connection request
     */
    setCloseConnection();

    switch (state) {
    case Request::Error:
      if (status() == Reply::switching_protocols) {
	/*
	 * We already committed the reply -- all we can (and should)
	 * do now is close the connection.
	 */
	connection->close();
      } else {
	if (status() < 300)
	  setStatus(bad_request);

	setRelay(ReplyPtr(new StockReply(request(), status(),
					 configuration())));

	Reply::send();
      }

      break;
    case Request::Partial:
      /*
       * We defer reading the rest of the handshake until after we
       * have sent the 101: some intermediaries may be holding back
       * this data because they are still in HTTP mode
       */
      responseSent_ = true;
      sending_ = true;

      /*
       * We already create the HTTP request because waitMoreData() depends
       * on it.
       */
      httpRequest_ = new HTTPRequest(boost::dynamic_pointer_cast<WtReply>
                                     (shared_from_this()), &entryPoint_);
      httpRequest_->setWebSocketRequest(true);
      
      fetchMoreDataCallback_
	= boost::bind(&WtReply::readRestWebSocketHandshake, this);

      LOG_DEBUG("ws: sending 101, deferring handshake");

      Reply::send();

      break;
    case Request::Complete:
      /*
       * The client handshake has been parsed and is ready.
       */
      cin_mem_.write(begin, static_cast<std::streamsize>(end - begin));

      if (!httpRequest_) {
	httpRequest_ = new HTTPRequest(boost::dynamic_pointer_cast<WtReply>
				       (shared_from_this()), &entryPoint_);
	httpRequest_->setWebSocketRequest(true);
      }

      LOG_DEBUG("ws: accepting connection");
      connection->server()->controller()->handleRequest(httpRequest_);
    }
  }
}

void WtReply::readRestWebSocketHandshake()
{
  ConnectionPtr connection = getConnection();

  if (connection)
    connection->handleReadBody();
}

void WtReply::consumeWebSocketMessage(ws_opcode opcode,
				      Buffer::const_iterator begin,
				      Buffer::const_iterator end,
				      Request::State state)
{
  cin_mem_.write(begin, static_cast<std::streamsize>(end - begin));

  if (state != Request::Partial) {
    if (state == Request::Error)
      cin_mem_.str("");
    else
      cin_mem_.seekg(0);

    switch (opcode) {
    case connection_close:
      cin_mem_.str("");

      /* fall through */
    case text_frame:
      {
	CallbackFunction cb = readMessageCallback_;
	readMessageCallback_ = 0;
	cb();

	break;
      }
    case ping:
      /* trouble */

      break;
    case continuation:
    case pong:
    case binary_frame:
      break;
    }
  }
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
  if (status() < 300)
    setStatus(found);
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

  bool webSocket = request().webSocketVersion >= 0;

  if (webSocket) {
    if (!sendingMessages_) {
      /*
       * This finishes the server handshake. For 00-protocol, we copy
       * the computed handshake nonce in the output.
       */
      nextCout_.assign(cin_mem_.str());

      sendingMessages_ = true;
    } else {
      /*
       * This should be sent as a websocket message.
       */
      if (text.empty()) {
	LOG_DEBUG("ws: closed by app");

	/*
	 * FIXME: send a close frame
	 */
	connection->close();
	return;
      }

      LOG_DEBUG("ws: sending a message, length = " << (long long)text.length());

      nextCout_.clear();

      switch (request().webSocketVersion) {
      case 0:
	nextCout_ += (char)0;
	nextCout_ += text;
	nextCout_ += (char)0xFF;

	break;
      case 7:
      case 8:
      case 13:
	{
	  nextCout_ += (char)0x81;

	  std::size_t payloadLength = text.length();

	  if (payloadLength < 126)
	    nextCout_ += (char)payloadLength;
	  else if (payloadLength < (1 << 16)) {
	    nextCout_ += (char)126;
	    nextCout_ += (char)(payloadLength >> 8);
	    nextCout_ += (char)(payloadLength);
	  } else {
	    nextCout_ += (char)127;
	    for (unsigned i = 0; i < 8; ++i)
	      nextCout_ += (char)(payloadLength >> ((7-i) * 8));
	  }

	  nextCout_ += text;
	}
	break;
      default:
	LOG_ERROR("ws: encoding for version " <<
		  request().webSocketVersion << " is not implemented");
	connection->close();
	return;
      }
    }
  } else {
    nextCout_.assign(text);
  }

  responseSent_ = false;

  if (!sending_) {
    if (status() == no_status) {
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
	setStatus(ok);
      }
    }

    sending_ = true;
    Reply::send();
  }
}

void WtReply::readWebSocketMessage(CallbackFunction callBack)
{
  ConnectionPtr connection = getConnection();

  assert(request().webSocketVersion >= 0);

  if (readMessageCallback_)
    return;

  readMessageCallback_ = callBack;

  if (!connection) {
    /*
     * Simulate a connection_close to the application
     */
    Buffer b;
    consumeWebSocketMessage(connection_close, b.begin(), b.begin(),
			    Request::Complete);
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
  LOG_DEBUG("(sending: " << sending_ << ", reponseSent_: " << responseSent_
	    << ") nextContentBuffer: " << nextCout_.length());

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
