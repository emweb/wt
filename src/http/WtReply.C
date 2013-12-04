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

namespace misc_strings {
  const char char0x0 = 0x0;
  const char char0xFF = (char)0xFF;
  const char char0x81 = (char)0x81;
}

WtReply::WtReply(const Request& request, const Wt::EntryPoint& entryPoint,
                 const Configuration &config)
  : Reply(request, config),
    entryPoint_(entryPoint),
    out_(&out_buf_),
    sending_(0),
    contentLength_(-1),
    bodyReceived_(0),
    sendingMessages_(false)
{
  urlScheme_ = request.urlScheme;

  if (request.contentLength > config.maxMemoryRequestSize()) {
    requestFileName_ = Wt::FileUtils::createTempFileName();
    // First, make sure the file exists
    std::ofstream o(requestFileName_.c_str());
    o.close();
    // Now, open it for read/write to verify that the file was
    // properly created
    in_ = new std::fstream(requestFileName_.c_str(),
      std::ios::in | std::ios::out | std::ios::binary);
    // To avoid an OWASP DoS attack (slow POST), we don't keep
    // file descriptors open here.
    static_cast<std::fstream *>(in_)->close();
  } else {
    in_ = &in_mem_;
  }

  httpRequest_ = 0;
}

WtReply::~WtReply()
{
  delete httpRequest_;

  if (&in_mem_ != in_) {
    dynamic_cast<std::fstream *>(in_)->close();
    delete in_;
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
      if (status() != request_entity_too_large) {
	// in_ may be a file stream, or a memory stream. File streams are
	// closed inbetween receiving parts -> open it
	std::fstream *f_in = dynamic_cast<std::fstream *>(in_);
        if (f_in) {
          f_in->open(requestFileName_.c_str(),
            std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
          if (!*f_in) {
            LOG_ERROR("error opening spool file for request that exceeds "
              "max-memory-request-size: " << requestFileName_);
            // Give up
            setStatus(internal_server_error);
            setCloseConnection();
            state = Request::Error;
          }
        }
	in_->write(begin, static_cast<std::streamsize>(end - begin));
        if (f_in) {
          f_in->close();
        }
      }
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
	  delete httpRequest_;
	  httpRequest_ = 0;

	  setStatus(request_entity_too_large);
	  setCloseConnection();
	  state = Request::Error;
	}
      }
    } else {
      delete httpRequest_;
      httpRequest_ = 0;
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
        if (dynamic_cast<std::fstream *>(in_)) {
          dynamic_cast<std::fstream *>(in_)->open(requestFileName_.c_str(),
            std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
          if (!*in_) {
            LOG_ERROR("error opening spooled request " << requestFileName_);
            setStatus(internal_server_error);
            setCloseConnection();
            state = Request::Error;
          }
        }

	in_->seekg(0); // rewind

	connection->server()->service().post
	  (connection->strand().wrap
	   (boost::bind(&Wt::WebController::handleRequest,
			connection->server()->controller(),
			httpRequest_)));
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

	setRelay(ReplyPtr(new StockReply(request(), status(), configuration())));

	Reply::send();
      }

      break;
    case Request::Partial:
      /*
       * We defer reading the rest of the handshake until after we
       * have sent the 101: some intermediaries may be holding back
       * this data because they are still in HTTP mode
       */

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
      in_mem_.write(begin, static_cast<std::streamsize>(end - begin));

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
  in_mem_.write(begin, static_cast<std::streamsize>(end - begin));

  if (state != Request::Partial) {
    if (state == Request::Error) {
      in_mem_.str("");
      in_mem_.clear();
    } else
      in_mem_.seekg(0);

    switch (opcode) {
    case connection_close:
      LOG_DEBUG("WtReply::consumeWebSocketMessage(): rx close");
      in_mem_.str("");
      in_mem_.clear();

      /* fall through */
    case continuation:
      LOG_DEBUG("WtReply::consumeWebSocketMessage(): rx continuation");

    case text_frame:
      {
	LOG_DEBUG("WtReply::consumeWebSocketMessage(): rx text_frame");

	/*
	 * FIXME: check that we have received the entire message.
	 *  If yes: call the callback; else resume reading (expecting
	 *  continuation frames in that case)
	 */
	Wt::WebRequest::ReadCallback cb = readMessageCallback_;
	readMessageCallback_ = 0;
	ConnectionPtr connection = getConnection();
	connection->server()->service().post
	  (boost::bind(cb, Wt::WebRequest::MessageEvent));

	break;
      }
    case ping:
      {
	LOG_DEBUG("WtReply::consumeWebSocketMessage(): rx ping");

	Wt::WebRequest::ReadCallback cb = readMessageCallback_;
	readMessageCallback_ = 0;
	ConnectionPtr connection = getConnection();
	connection->server()->service().post
	  (boost::bind(cb, Wt::WebRequest::PingEvent));

	break;
      }
      break;
    case binary_frame:
      LOG_ERROR("ws: binary_frame received, don't know what to do.");

      /* fall through */
    case pong:
      {
	LOG_DEBUG("WtReply::consumeWebSocketMessage(): rx pong");

	/*
	 * We do not need to send a response; resume reading, keeping the
	 * same read callback
	 */
	Wt::WebRequest::ReadCallback cb = readMessageCallback_;
	readMessageCallback_ = 0;
	readWebSocketMessage(cb);
      }

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

void WtReply::send(const Wt::WebRequest::WriteCallback& callBack,
		   bool responseComplete)
{
  LOG_DEBUG("WtReply::send(): " << sending_);

  ConnectionPtr connection = getConnection();

  if (!connection)
    return;

  fetchMoreDataCallback_ = callBack;

  if (sending_ == 0) {
    if (status() == no_status) {
      if (!transmitting() && fetchMoreDataCallback_) {
	/*
	 * We haven't got a response status, so we can't send anything really.
	 * Instead, we immediately invoke the fetchMoreDataCallback_
	 *
	 * This is used in a resource continuation which indicates to wait
	 * for more data before sending anything at all.
	 */
	LOG_DEBUG("Invoking callback (no status)");

	Wt::WebRequest::WriteCallback f = fetchMoreDataCallback_;
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

    Reply::send();
  }
}

void WtReply::readWebSocketMessage(const Wt::WebRequest::ReadCallback& callBack)
{
  LOG_DEBUG("readWebSocketMessage(): " << readMessageCallback_
	    << ", " << callBack);

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
    if (&in_mem_ != in_) {
      dynamic_cast<std::fstream *>(in_)->close();
      delete in_;
      in_ = &in_mem_;
    }

    in_mem_.str("");
    in_mem_.clear();

    connection->server()->service().post
      (connection->strand().wrap
       (boost::bind(&Connection::handleReadBody, connection)));
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

void WtReply::formatResponse(std::vector<asio::const_buffer>& result)
{
  assert(sending_ > 0);

  bool webSocket = request().webSocketVersion >= 0;
  if (webSocket) {
    std::size_t size = sending_;

    LOG_DEBUG("ws: sending a message, length = " << size);

    switch (request().webSocketVersion) {
    case 0:
      result.push_back(asio::buffer(&misc_strings::char0x0, 1));
      result.push_back(out_buf_.data());
      result.push_back(asio::buffer(&misc_strings::char0xFF, 1));

      break;
    case 7:
    case 8:
    case 13:
      {
	result.push_back(asio::buffer(&misc_strings::char0x81, 1));

	std::size_t payloadLength = size;

	if (payloadLength < 126) {
	  gatherBuf_[0] = (char)payloadLength;
	  result.push_back(asio::buffer(gatherBuf_, 1));
	} else if (payloadLength < (1 << 16)) {
	  gatherBuf_[0] = (char)126;
	  gatherBuf_[1] = (char)(payloadLength >> 8);
	  gatherBuf_[2] = (char)(payloadLength);
	  result.push_back(asio::buffer(gatherBuf_, 3));
	} else {
	  unsigned j = 0;
	  gatherBuf_[j++] = (char)127;

	  const unsigned SizeTLength = sizeof(payloadLength);

	  for (unsigned i = 8; i > SizeTLength; --i)
	    gatherBuf_[j++] = (char)0x0;

	  for (unsigned i = 0; i < SizeTLength; ++i)
	    gatherBuf_[j++] = (char)(payloadLength
				     >> ((SizeTLength - 1 - i) * 8));

	  result.push_back(asio::buffer(gatherBuf_, 9));
	}

	result.push_back(out_buf_.data());
      }
      break;
    default:
      LOG_ERROR("ws: encoding for version " <<
		request().webSocketVersion << " is not implemented");

      sending_ = 0;
      // FIXME: set something to close the connection
      return;
    }
  } else
    result.push_back(out_buf_.data());
}

void WtReply::nextContentBuffers(std::vector<asio::const_buffer>& result)
{
  LOG_DEBUG("sent: " << sending_);

  out_buf_.consume(sending_);

  sending_ = out_buf_.size();

  LOG_DEBUG("avail now: " << sending_);

  bool webSocket = request().webSocketVersion >= 0;

  if (webSocket && !sendingMessages_) {
    /*
     * This finishes the server handshake. For 00-protocol, we copy
     * the computed handshake nonce in the output.
     */
    if (request().webSocketVersion == 0) {
      std::string s = in_mem_.str();
      memcpy(gatherBuf_, s.c_str(), std::min((std::size_t)16, s.length()));
      result.push_back(asio::buffer(gatherBuf_));
    }

    sendingMessages_ = true;
  } else if (sending_ > 0) {
    formatResponse(result);
  }

  if (sending_ == 0) {
    while (sending_ == 0 && fetchMoreDataCallback_) {
      sending_ = 1;
      LOG_DEBUG("Invoking callback (nextContentBuffers)");
      Wt::WebRequest::WriteCallback f = fetchMoreDataCallback_;
      fetchMoreDataCallback_ = 0;
      f();
      sending_ = out_buf_.size();
    }
 
    if (sending_ > 0)
      formatResponse(result);
  }
}

  }
}
