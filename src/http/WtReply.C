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
  const char char0xC1 = (char)0xC1;
}

#ifdef WTHTTP_WITH_ZLIB
static int SERVER_MAX_WINDOW_BITS = 15;
#endif

WtReply::WtReply(Request& request, const Wt::EntryPoint& entryPoint,
                 const Configuration &config)
  : Reply(request, config),
    entryPoint_(&entryPoint),
    in_(&in_mem_),
    out_(&out_buf_),
    urlScheme_(request.urlScheme),
    sending_(0),
    contentLength_(-1),
    bodyReceived_(0),
    sendingMessages_(false),
    httpRequest_(0)
#ifdef WTHTTP_WITH_ZLIB
    ,deflateInitialized_(false)
#endif
{
  reset(&entryPoint);
}

WtReply::~WtReply()
{
  delete httpRequest_;

  if (&in_mem_ != in_) {
    dynamic_cast<std::fstream *>(in_)->close();
    delete in_;
  }

  if (!requestFileName_.empty())
    unlink(requestFileName_.c_str());

#ifdef WTHTTP_WITH_ZLIB
  if(deflateInitialized_)
	deflateEnd(&zOutState_);
#endif
}

void WtReply::reset(const Wt::EntryPoint *ep)
{
  Reply::reset(ep);

  entryPoint_ = ep;

  in_mem_.str("");
  in_mem_.clear();

  out_buf_.consume(sending_);
  sending_ = 0;
  contentType_.clear();
  location_.clear();
  contentLength_ = -1;
  bodyReceived_ = 0;
  sendingMessages_ = false;

  fetchMoreDataCallback_ = 0;
  readMessageCallback_ = 0;

  if (httpRequest_)
    httpRequest_->reset(boost::dynamic_pointer_cast<WtReply>
			(shared_from_this()), ep);

  if (&in_mem_ != in_) {
    dynamic_cast<std::fstream *>(in_)->close();
    delete in_;
  }

  if (!requestFileName_.empty())
    unlink(requestFileName_.c_str());

  if (request_.contentLength > configuration().maxMemoryRequestSize()) {
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
#ifdef WTHTTP_WITH_ZLIB
  if(deflateInitialized_)
	deflateReset(&zOutState_);
#endif
}

void WtReply::logReply(Wt::WLogger& logger)
{
  Reply::logReply(logger);

  if (httpRequest_)
    httpRequest_->log();
}

bool WtReply::consumeData(Buffer::const_iterator begin,
			  Buffer::const_iterator end,
			  Request::State state)
{
  consumeRequestBody(begin, end, state);
  return true;
}

void WtReply::consumeRequestBody(Buffer::const_iterator begin,
				 Buffer::const_iterator end,
				 Request::State state)
{
  if (request().type != Request::WebSocket) {
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
            std::ios::out | std::ios::binary | std::ios::app);
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
				       (shared_from_this()), entryPoint_);

      if (end - begin > 0) {
	bodyReceived_ += (end - begin);

	if (!connection()->server()->controller()->requestDataReceived
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
            std::ios::in | std::ios::binary );
          if (!*in_) {
            LOG_ERROR("error opening spooled request " << requestFileName_);
            setStatus(internal_server_error);
            setCloseConnection();
            state = Request::Error;
          }
        }

	in_->seekg(0); // rewind

	// Note: this is being posted because we want to release the strand
	// we currently hold; we need to do that because otherwise the strand
	// could be locked during a recursive event loop
	// 
	// Are we sure that the reply object isn't deleted before it's used?
	// the httpRequest_ has a WtReplyPtr and httpRequest_ is only deleted
	// from the destructor, so that's okay.
	//
	// The WtReplyPtr is reset in HTTPRequest::flush(Done), could that
	// be called already? No because nobody is aware yet of this request
	// object.

	// But (for benchmark's sake), there's no need to post for a static
	// resource
	if (entryPoint_->resource())
	  connection()->server()->controller()->handleRequest(httpRequest_);
	else
	  connection()->server()->service().post
	    (boost::bind(&Wt::WebController::handleRequest,
			 connection()->server()->controller(),
			 httpRequest_));
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
	connection()->close();
      } else {
	if (status() < 300)
	  setStatus(bad_request);

	setRelay
	  (ReplyPtr(new StockReply(request(), status(), configuration())));

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
                                     (shared_from_this()), entryPoint_);
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
				       (shared_from_this()), entryPoint_);
	httpRequest_->setWebSocketRequest(true);
      }

      LOG_DEBUG("ws: accepting connection");
      connection()->server()->controller()->handleRequest(httpRequest_);
    }
  }
}

void WtReply::readRestWebSocketHandshake()
{
  connection()->handleReadBody(shared_from_this());
}

void WtReply::consumeWebSocketMessage(ws_opcode opcode,
				      const char* begin,
				      const char* end,
				      Request::State state)
{
  in_mem_.write(begin, static_cast<std::streamsize>(end - begin));

  if (state != Request::Partial) {
    if (state == Request::Error) {
      LOG_DEBUG("WtReply::consumeWebSocketMessage(): rx error");
      in_mem_.str("");
      in_mem_.clear();

      Wt::WebRequest::ReadCallback cb = readMessageCallback_;
      readMessageCallback_ = 0;

      // We need to post since in Wt we may be entering a recursive event
      // loop and we need to release the strand
      connection()->server()->service().post
	(boost::bind(cb, Wt::ReadError));
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

	// We need to post since in Wt we may be entering a recursive event
	// loop and we need to release the strand
	connection()->server()->service().post
	  (boost::bind(cb, Wt::ReadMessage));

	break;
      }
    case ping:
      {
	LOG_DEBUG("WtReply::consumeWebSocketMessage(): rx ping");

	Wt::WebRequest::ReadCallback cb = readMessageCallback_;
	readMessageCallback_ = 0;

	// We need to post since in Wt we may be entering a recursive event
	// loop and we need to release the strand
	connection()->server()->service().post
	  (boost::bind(cb, Wt::ReadPing));

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

void WtReply::writeDone(bool success)
{
  if (relay()) {
    relay()->writeDone(success);
    return;
  }

  LOG_DEBUG("writeDone() success:" << success << ", sent: " << sending_);
  out_buf_.consume(sending_);
  sending_ = 0;

  if (fetchMoreDataCallback_) {
    Wt::WebRequest::WriteCallback f = fetchMoreDataCallback_;
    fetchMoreDataCallback_ = 0;
    f(success ? Wt::WriteCompleted : Wt::WriteError);
  }
}

void WtReply::send(const Wt::WebRequest::WriteCallback& callBack,
		   bool responseComplete)
{
  LOG_DEBUG("WtReply::send(): " << sending_);

  fetchMoreDataCallback_ = callBack;

  if (sending_ != 0) {
    LOG_DEBUG("WtReply::send(): still busy sending... ignoring");
    return;
  }

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
      f(Wt::WriteCompleted);

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

void WtReply::readWebSocketMessage(const Wt::WebRequest::ReadCallback& callBack)
{
  LOG_DEBUG("readWebSocketMessage(): " << readMessageCallback_
	    << ", " << callBack);

  assert(request().type == Request::WebSocket);

  if (readMessageCallback_)
    return;

  readMessageCallback_ = callBack;

  if (&in_mem_ != in_) {
    dynamic_cast<std::fstream *>(in_)->close();
    delete in_;
    in_ = &in_mem_;
  }

  in_mem_.str("");
  in_mem_.clear();

  connection()->strand().post(boost::bind(&Connection::handleReadBody,
					  connection(),
					  shared_from_this()));
}

bool WtReply::readAvailable()
{
  return connection()->readAvailable();
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

  bool webSocket = request().type == Request::WebSocket;
  if (webSocket) {
    std::size_t size = sending_;
	std::vector<boost::asio::const_buffer> buffers;

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
	std::size_t payloadLength;
#ifdef WTHTTP_WITH_ZLIB
	if(!request_.pmdState_.enabled ) {
#endif
	  result.push_back(asio::buffer(&misc_strings::char0x81, 1)); // RSV1 = 0
	  payloadLength = size;
#ifdef WTHTTP_WITH_ZLIB
	} else  {
	  result.push_back(asio::buffer(&misc_strings::char0xC1, 1)); // RSV1 = 1
	  const unsigned char* data = boost::asio::buffer_cast<const unsigned char*>(out_buf_.data());
	  int size = boost::asio::buffer_size(out_buf_.data());
	  bool hasMore = false;
	  payloadLength = 0;
	  do {
		unsigned char buffer[16 * 1024];
		int bs = deflate(data, size, buffer, hasMore);

		if (!hasMore) {
		  // strip trailing for 0x0 0x0 0xff 0xff bytes
		  bs = bs - 4;
		}
		
		buffers.push_back(buf(std::string((char*)buffer, bs)));
		payloadLength+=bs;
	  } while (hasMore);

	  assert(zOutState_.avail_in == 0);

	  //TODO need to free out_buf
	  if (request_.pmdState_.server_max_window_bits < 0) // context_takeover
		deflateReset(&zOutState_);

	  if(payloadLength <= 0) {
		LOG_ERROR("ws: deflate failed");
		sending_ = 0;
		return;
	  }
	}
#endif

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

#ifdef WTHTTP_WITH_ZLIB
	// Compress frame if compression is enabled
	if(request_.pmdState_.enabled) 
	  for(unsigned i = 0; i < buffers.size() ; ++i) 
		result.push_back(buffers[i]);
	else 
#endif
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

bool WtReply::nextContentBuffers(std::vector<asio::const_buffer>& result)
{
  sending_ = out_buf_.size();

  LOG_DEBUG("avail now: " << sending_);

  bool webSocket = request().type == Request::WebSocket;

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

  return httpRequest_ ? httpRequest_->done() : true;
}


#ifdef WTHTTP_WITH_ZLIB

bool WtReply::initDeflate() 
{
  zOutState_.zalloc = Z_NULL;
  zOutState_.zfree = Z_NULL;
  zOutState_.opaque = Z_NULL;

  int wsize = request_.pmdState_.server_max_window_bits != -1 ?
	request_.pmdState_.server_max_window_bits : SERVER_MAX_WINDOW_BITS;

  int ret = deflateInit2(
	  &zOutState_,
	  Z_DEFAULT_COMPRESSION,
	  Z_DEFLATED,
	  -1* wsize,
	  8, // memory level 1-9
	  Z_FIXED
	  );
  if(ret != Z_OK) return false;
  deflateInitialized_ = true;
  return true;
}
  
int WtReply::deflate(const unsigned char* in, size_t size, unsigned char out[], bool& hasMore)
{
  size_t output = 0;
  const int bufferSize = 16 * 1024;

  LOG_DEBUG("wthttp: wt: deflate frame");

  if(!deflateInitialized_) {
	if(!initDeflate())
	  return -1;
  }
  
  // If it's the first iteration init the data
  if(!hasMore) {
    // Set only at the first iteration
    zOutState_.avail_in = size;
    zOutState_.next_in = const_cast<unsigned char *>(in);
  }

  // Output to local buffer
  zOutState_.avail_out = bufferSize;
  zOutState_.next_out = out;

  hasMore = true;

  int ret = ::deflate(&zOutState_, 
      request_.pmdState_.server_max_window_bits < 0 
      ? Z_FULL_FLUSH : Z_SYNC_FLUSH);
  
  assert(ret != Z_STREAM_ERROR);

  output = bufferSize - zOutState_.avail_out;

  if (zOutState_.avail_out != 0) 
    hasMore = false;

  return output; 
}
#endif
  
}
}
