/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */
//
// reply.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "Configuration.h"
#include "Connection.h"
#include "Reply.h"
#include "Request.h"
#include "Server.h"

#include <time.h>
#include <string>
#include <boost/lexical_cast.hpp>

#ifdef WIN32
// gmtime_r can be defined by mingw
#ifndef gmtime_r
static struct tm* gmtime_r(const time_t* t, struct tm* r)
{
  // gmtime is threadsafe in windows because it uses TLS
  struct tm *theTm = gmtime(t);
  if (theTm) {
    *r = *theTm;
    return r;
  } else {
    return 0;
  }
}
#endif // gmtime_r
#else
extern struct tm* gmtime_r(const time_t* t, struct tm* r);
#endif

namespace Wt {
  LOGGER("wthttp");
}

namespace http {
namespace server {

template <typename T>
inline asio::const_buffer asio_cstring_buf(T str);

template <std::size_t N>
inline asio::const_buffer asio_cstring_buf(const char (&s) [N])
{
  return asio::const_buffer(s, N-1);
}

unsigned httpDateBuf(time_t t, char *buf)
{
  struct tm td;
  gmtime_r(&t, &td);
  return strftime(buf, 100, "%a, %d %b %Y %H:%M:%S GMT", &td);
}

namespace status_strings {

asio::const_buffer toText(Reply::status_type status)
{
  switch (status)
  {
  case Reply::switching_protocols:
    return asio_cstring_buf("101 Switching Protocol\r\n");
  case Reply::ok:
    return asio_cstring_buf("200 OK\r\n");
  case Reply::created:
    return asio_cstring_buf("201 Created\r\n");
  case Reply::accepted:
    return asio_cstring_buf("202 Accepted\r\n");
  case Reply::no_content:
    return asio_cstring_buf("204 No Content\r\n");
  case Reply::partial_content:
    return asio_cstring_buf("206 Partial Content\r\n");
  case Reply::multiple_choices:
    return asio_cstring_buf("300 Multiple Choices\r\n");
  case Reply::moved_permanently:
    return asio_cstring_buf("301 Moved Permanently\r\n");
  case Reply::found:
    return asio_cstring_buf("302 Found\r\n");
  case Reply::see_other:
    return asio_cstring_buf("303 See Other\r\n");
  case Reply::not_modified:
    return asio_cstring_buf("304 Not Modified\r\n");
  case Reply::moved_temporarily:
    return asio_cstring_buf("307 Moved Temporarily\r\n");
  case Reply::bad_request:
    return asio_cstring_buf("400 Bad Request\r\n");
  case Reply::unauthorized:
    return asio_cstring_buf("401 Unauthorized\r\n");
  case Reply::forbidden:
    return asio_cstring_buf("403 Forbidden\r\n");
  case Reply::not_found:
    return asio_cstring_buf("404 Not Found\r\n");
  case Reply::request_entity_too_large:
    return asio_cstring_buf("413 Request Entity too Large\r\n");
  case Reply::requested_range_not_satisfiable:
    return asio_cstring_buf("416 Requested Range Not Satisfiable\r\n");
  case Reply::not_implemented:
    return asio_cstring_buf("501 Not Implemented\r\n");
  case Reply::bad_gateway:
    return asio_cstring_buf("502 Bad Gateway\r\n");
  case Reply::service_unavailable:
    return asio_cstring_buf("503 Service Unavailable\r\n");
  case Reply::no_status:
  case Reply::internal_server_error:
  default:
    return asio_cstring_buf("500 Internal Server Error\r\n");
  }
}

} // namespace status_strings

namespace misc_strings {

const char name_value_separator[] = { ':', ' ' };
const char crlf[] = { '\r', '\n' };

} // namespace misc_strings

Reply::Reply(const Request& request, const Configuration& config)
  : request_(request),
    configuration_(config),
    status_(no_status),
    transmitting_(false),
    closeConnection_(false),
    chunkedEncoding_(false),
    gzipEncoding_(false),
    contentSent_(0),
    contentOriginalSize_(0)
#ifdef WTHTTP_WITH_ZLIB
    , gzipBusy_(false)
#endif // WTHTTP_WITH_ZLIB
{ }

Reply::~Reply()
{ 
#ifdef WTHTTP_WITH_ZLIB
  if (gzipBusy_)
    deflateEnd(&gzipStrm_);
#endif // WTHTTP_WITH_ZLIB
}

void Reply::setStatus(status_type status)
{
  status_ = status;
}

void Reply::consumeWebSocketMessage(ws_opcode opcode,
				    Buffer::const_iterator begin,
				    Buffer::const_iterator end,
				    Request::State state)
{
  LOG_ERROR("Reply::consumeWebSocketMessage() is pure virtual");
}

std::string Reply::location()
{
  return std::string();
}

void Reply::addHeader(const std::string name, const std::string value)
{
  headers_.push_back(std::make_pair(name, value));
}

bool Reply::nextBuffers(std::vector<asio::const_buffer>& result)
{
  bufs_.clear();

  if (relay_.get())
    return relay_->nextBuffers(result);
  else {
    if (!transmitting_) {
      transmitting_ = true;
      bool http10 = (request_.http_version_major == 1)
	&& (request_.http_version_minor == 0);

      closeConnection_ = closeConnection_ || request_.closeConnection();

      unsigned gather_i = 0;
      /*
       * Status line.
       */

      result.push_back(asio_cstring_buf("HTTP/"));

      gather_buf_[gather_i] = '0' + request_.http_version_major;
      result.push_back(asio::buffer(gather_buf_ + gather_i++, 1));

      result.push_back(asio_cstring_buf("."));

      gather_buf_[gather_i] = '0' + request_.http_version_minor;
      result.push_back(asio::buffer(gather_buf_ + gather_i++, 1));

      result.push_back(asio_cstring_buf(" "));

      result.push_back(status_strings::toText(status_));

      if (!http10 && status_ != switching_protocols) {
	/*
	 * Date header (current time)
	 */
	result.push_back(asio_cstring_buf("Date: "));
	unsigned length = httpDateBuf(time(0), gather_buf_ + gather_i);
	result.push_back(asio::buffer(gather_buf_ + gather_i, length));
	gather_i += length;
	result.push_back(asio::buffer(misc_strings::crlf));
      }

      /*
       * Content type or location
       */

      std::string ct;
      if (status_ >= 300 && status_ < 400) {
	if (!location().empty()) {
	  result.push_back(asio_cstring_buf("Location: "));
	  result.push_back(buf(location()));
	  result.push_back(asio::buffer(misc_strings::crlf));
	}
      } else if (status_ != not_modified && status_ != switching_protocols) {
	ct = contentType();
	result.push_back(asio_cstring_buf("Content-Type: "));
	result.push_back(buf(ct));
	result.push_back(asio::buffer(misc_strings::crlf));
      }

      /*
       * Other provided headers
       */
      bool haveContentEncoding = false;
      for (unsigned i = 0; i < headers_.size(); ++i) {
	if (headers_[i].first == "Content-Encoding")
	  haveContentEncoding = true;
	result.push_back(asio::buffer(headers_[i].first));
	result.push_back(asio::buffer(misc_strings::name_value_separator));
	result.push_back(asio::buffer(headers_[i].second));
	result.push_back(asio::buffer(misc_strings::crlf));
      }

      ::int64_t cl = -1;

      if (status_ != not_modified)
	cl = contentLength();
      else
	cl = 0;

      /*
       * We would need to figure out the content length based on the
       * response data, but this doesn't work: WtReply reuses the
       * same buffers over and over, expecting them to be sent
       * inbetween each call to nextContentBuffer()
       */
      if ((cl == -1) && http10)
	closeConnection_ = true;

      /*
       * Connection
       */
      if (closeConnection_) {
	result.push_back(asio_cstring_buf("Connection: close"));
	result.push_back(asio::buffer(misc_strings::crlf));
      } else {
	if (http10) {
	  result.push_back(asio_cstring_buf("Connection: keep-alive"));
	  result.push_back(asio::buffer(misc_strings::crlf));
	}
      }

      if (status_ != not_modified) {
#ifdef WTHTTP_WITH_ZLIB
	/*
	 * Content-Encoding: gzip ?
	 */
	gzipEncoding_ = 
	     !haveContentEncoding
	  && configuration_.compression()
	  && request_.acceptGzipEncoding()
	  && (cl == -1)
	  && (ct.find("text/html") != std::string::npos
	      || ct.find("text/plain") != std::string::npos
	      || ct.find("text/javascript") != std::string::npos
	      || ct.find("text/css") != std::string::npos
	      || ct.find("application/xhtml+xml")!= std::string::npos
	      || ct.find("image/svg+xml")!= std::string::npos
	      || ct.find("text/x-json") != std::string::npos);

	if (gzipEncoding_) {
	  result.push_back(asio_cstring_buf("Content-Encoding: gzip"));
	  result.push_back(asio::buffer(misc_strings::crlf));
	  
	  initGzip();
	}
#endif

	/*
	 * We do not need to determine the length of the response...
	 * Transmit only header first.
	 */
	if (cl != -1) {
	  result.push_back(asio_cstring_buf("Content-Length: "));
	  result.push_back(buf(boost::lexical_cast<std::string>(cl)));
	  result.push_back(asio::buffer(misc_strings::crlf));

	  chunkedEncoding_ = false;
	} else
	  if (closeConnection_)
	    chunkedEncoding_ = false; // should be false
	  else
	    if (!http10 && status_ != switching_protocols)
	      chunkedEncoding_ = true;

	if (chunkedEncoding_) {
	  result.push_back(asio_cstring_buf("Transfer-Encoding: chunked"));
	  result.push_back(asio::buffer(misc_strings::crlf));
	}

	result.push_back(asio::buffer(misc_strings::crlf));

	return false;
      } else { // status_ == not-modified
	result.push_back(asio::buffer(misc_strings::crlf));

	return true;
      }
    } else { // transmitting (data)
      std::vector<asio::const_buffer> contentBuffers;
      int originalSize;
      int encodedSize;

      encodeNextContentBuffer(contentBuffers, originalSize, encodedSize);

      bool lastData = (originalSize == 0 && !waitMoreData());

      contentSent_ += encodedSize;
      contentOriginalSize_ += originalSize;

      if (chunkedEncoding_) {
	if (encodedSize || lastData) {
	  std::ostringstream length;
	  length << std::hex << encodedSize;
	  result.push_back(buf(length.str()));
	  result.push_back(asio::buffer(misc_strings::crlf));

	  if (encodedSize) {
	    result.insert(result.end(),
			  contentBuffers.begin(), contentBuffers.end());
	    result.push_back(asio::buffer(misc_strings::crlf));

	    if (lastData) {
	      result.push_back(asio_cstring_buf("0"));
	      result.push_back(asio::buffer(misc_strings::crlf));
	      result.push_back(asio::buffer(misc_strings::crlf));
	    }
	  } else
	    result.push_back(asio::buffer(misc_strings::crlf));
	}

	return originalSize == 0;
      } else {
	result = contentBuffers;

	return originalSize == 0;
      }
    }
  }

  assert(false);

  return false;
}

bool Reply::closeConnection() const
{
  if (closeConnection_)
    return true;

  if (relay_.get())
    return relay_->closeConnection();
  else
    return false;
}

void Reply::setConnection(ConnectionPtr connection)
{
  connection_ = connection;

  if (relay_.get())
    relay_->setConnection(connection);

  try {
    remoteAddress_
      = connection->socket().remote_endpoint().address().to_string();
  } catch (std::exception& e) {
    LOG_ERROR("remote_endpoint() threw: " << e.what());
  }

  requestMethod_ = request_.method;
  requestUri_ = request_.uri;
  requestMajor_ = request_.http_version_major;
  requestMinor_ = request_.http_version_minor;
}

void Reply::send()
{
  ConnectionPtr connection = getConnection();

  if (connection) {
    LOG_DEBUG(this << ": Reply: send(): scheduling write response.");

    connection->server()->service().post
      (connection->strand().wrap
       (boost::bind(&Connection::startWriteResponse, connection)));
  }
}

void Reply::setRelay(ReplyPtr reply)
{
  if (!transmitting_) {
    relay_ = reply;
    relay_->connection_ = connection_;

    relay_->remoteAddress_ = remoteAddress_;
    relay_->requestMethod_ = requestMethod_;
    relay_->requestUri_ = requestUri_;
    relay_->requestMajor_ = requestMajor_;
    relay_->requestMinor_ = requestMinor_;
  }
}

void Reply::logReply(Wt::WLogger& logger)
{
  if (relay_.get())
    return relay_->logReply(logger);

  Wt::WLogEntry e = logger.entry("");

  e << remoteAddress_ << Wt::WLogger::sep
    << /* rfc931 << */ Wt::WLogger::sep
    << /* authuser << */ Wt::WLogger::sep
    << Wt::WLogger::timestamp << Wt::WLogger::sep
    << requestMethod_ << ' ' << requestUri_ << " HTTP/"
    << requestMajor_ << '.' << requestMinor_ << Wt::WLogger::sep
    << status_ << Wt::WLogger::sep
    << contentSent_;

  /*
  if (gzipEncoding_)
      std::cerr << " <" << contentOriginalSize_ << ">";
  */
}

asio::const_buffer Reply::buf(const std::string s)
{
  bufs_.push_back(s);
  return asio::buffer(bufs_.back());
}

std::string Reply::httpDate(time_t t)
{
  char buf[100];
  httpDateBuf(t, buf);
  return buf;
}

#ifdef WTHTTP_WITH_ZLIB
void Reply::initGzip()
{
  gzipStrm_.zalloc = Z_NULL;
  gzipStrm_.zfree = Z_NULL;
  gzipStrm_.opaque = Z_NULL;
  gzipStrm_.next_in = Z_NULL;
  int r = 0;
  r = deflateInit2(&gzipStrm_, Z_DEFAULT_COMPRESSION,
		   Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY);
  gzipBusy_ = true;
  assert(r == Z_OK);
}
#endif

void Reply::encodeNextContentBuffer(
       std::vector<asio::const_buffer>& result, int& originalSize,
       int& encodedSize)
{
  std::vector<asio::const_buffer> buffers;
  nextContentBuffers(buffers);

  originalSize = 0;

  bool lastData = buffers.empty() && !waitMoreData();

#ifdef WTHTTP_WITH_ZLIB
  if (gzipEncoding_) {
    encodedSize = 0;

    if (!lastData) {
      for (unsigned i = 0; i < buffers.size(); ++i) {
	const asio::const_buffer& b = buffers[i];
	int bs = buffer_size(b); // std::size_t ?
	originalSize += bs;

	gzipStrm_.avail_in = bs;
	gzipStrm_.next_in = (unsigned char *)asio::detail::buffer_cast_helper(b);

	unsigned char out[16*1024];
	do {
	  gzipStrm_.next_out = out;
	  gzipStrm_.avail_out = sizeof(out);

	  int r = 0;
	  r = deflate(&gzipStrm_, Z_NO_FLUSH);

	  assert(r != Z_STREAM_ERROR);

	  unsigned have = sizeof(out) - gzipStrm_.avail_out;

	  if (have) {
	    encodedSize += have;
	    result.push_back(buf(std::string((char *)out, have)));
	  }
	} while (gzipStrm_.avail_out == 0);
      }
    } else {
      unsigned char out[16*1024], in[1];

      /*
       * Could be for a 0 length response, still needs to be properly
       * encoded.
       */
      if (!gzipStrm_.next_in) {
	gzipStrm_.next_in = in;
	gzipStrm_.avail_in = 0;
      }

      do {
	gzipStrm_.next_out = out;
	gzipStrm_.avail_out = sizeof(out);

	int r = 0;
	r = deflate(&gzipStrm_, Z_FINISH);

	assert(r != Z_STREAM_ERROR);

	unsigned have = sizeof(out) - gzipStrm_.avail_out;

	if (have) {
	  encodedSize += have;
	  result.push_back(buf(std::string((char *)out, have)));
	}
      } while (gzipStrm_.avail_out == 0);

      deflateEnd(&gzipStrm_);
      gzipBusy_ = false;
    }
  } else {
#endif
    for (unsigned i = 0; i < buffers.size(); ++i) {
      const asio::const_buffer& b = buffers[i];
      int bs = buffer_size(b); // std::size_t ?
      originalSize += bs;

      if (bs)
	result.push_back(b);
    }

    encodedSize = originalSize;
#ifdef WTHTTP_WITH_ZLIB
  }
#endif
}

} // namespace server
} // namespace http
