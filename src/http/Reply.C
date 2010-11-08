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

#include <time.h>
#include <string>
#include <boost/lexical_cast.hpp>

#ifdef WIN32
#ifdef WT_THREADED
#include <boost/thread/mutex.hpp>
#endif
#endif

#ifdef WIN32
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
#else
extern struct tm* gmtime_r(const time_t* t, struct tm* r);
#endif

namespace http {
namespace server {

namespace status_strings {

const std::string switching_protocols =
  "101 Switching Protocol\r\n";
const std::string ok =
  "200 OK\r\n";
const std::string created =
  "201 Created\r\n";
const std::string accepted =
  "202 Accepted\r\n";
const std::string no_content =
  "204 No Content\r\n";
const std::string partial_content =
  "206 Partial Content\r\n";
const std::string multiple_choices =
  "300 Multiple Choices\r\n";
const std::string moved_permanently =
  "301 Moved Permanently\r\n";
const std::string found =
  "302 Found\r\n";
const std::string see_other =
  "303 See Other\r\n";
const std::string not_modified =
  "304 Not Modified\r\n";
const std::string moved_temporarily =
  "307 Moved Temporarily\r\n";
const std::string bad_request =
  "400 Bad Request\r\n";
const std::string unauthorized =
  "401 Unauthorized\r\n";
const std::string forbidden =
  "403 Forbidden\r\n";
const std::string not_found =
  "404 Not Found\r\n";
const std::string request_entity_too_large =
  "413 Request Entity too Large\r\n";
const std::string requested_range_not_satisfiable =
  "416 Requested Range Not Satisfiable\r\n";
const std::string internal_server_error =
  "500 Internal Server Error\r\n";
const std::string not_implemented =
  "501 Not Implemented\r\n";
const std::string bad_gateway =
  "502 Bad Gateway\r\n";
const std::string service_unavailable =
  "503 Service Unavailable\r\n";

const std::string& toText(Reply::status_type status)
{
  switch (status)
  {
  case Reply::switching_protocols:
    return switching_protocols;
  case Reply::ok:
    return ok;
  case Reply::created:
    return created;
  case Reply::accepted:
    return accepted;
  case Reply::no_content:
    return no_content;
  case Reply::partial_content:
    return partial_content;
  case Reply::multiple_choices:
    return multiple_choices;
  case Reply::found:
    return found;
  case Reply::moved_permanently:
    return moved_permanently;
  case Reply::see_other:
    return see_other;
  case Reply::not_modified:
    return not_modified;
  case Reply::moved_temporarily:
    return moved_temporarily;
  case Reply::bad_request:
    return bad_request;
  case Reply::unauthorized:
    return unauthorized;
  case Reply::forbidden:
    return forbidden;
  case Reply::not_found:
    return not_found;
  case Reply::request_entity_too_large:
    return request_entity_too_large;
  case Reply::requested_range_not_satisfiable:
    return requested_range_not_satisfiable;
  case Reply::internal_server_error:
    return internal_server_error;
  case Reply::not_implemented:
    return not_implemented;
  case Reply::bad_gateway:
    return bad_gateway;
  case Reply::service_unavailable:
    return service_unavailable;
  default:
    return internal_server_error;
  }
}

} // namespace status_strings

namespace misc_strings {

const char name_value_separator[] = { ':', ' ' };
const char crlf[] = { '\r', '\n' };

} // namespace misc_strings

Reply::Reply(const Request& request)
  : request_(request),
    emptyBuffer((void *)0, 0),
    connection_(0),
    transmitting_(false),
    closeConnection_(false),
    chunkedEncoding_(false),
    gzipEncoding_(false),
    waitMoreData_(false),
    finishing_(false),
    contentSent_(0),
    contentOriginalSize_(0)
{
#ifndef WIN32
  gettimeofday(&startTime_, 0);
#endif
}

Reply::~Reply()
{ }

void Reply::release()
{ }

std::string Reply::location()
{
  return std::string();
}

void Reply::setWaitMoreData(bool how)
{
  waitMoreData_ = how;
}

void Reply::addHeader(const std::string name, const std::string value)
{
  headers_.push_back(std::make_pair(name, value));
}

bool Reply::nextBuffers(std::vector<asio::const_buffer>& result)
{
  bufs_.clear();

  if (relay_.get())
    return finishing_ = relay_->nextBuffers(result);
  else {
    if (!transmitting_) {
      transmitting_ = true;
      bool http10 = (request_.http_version_major == 1)
	&& (request_.http_version_minor == 0);

      closeConnection_
	= closeConnection_ || request_.closeConnection();

      /*
       * Status line.
       */
      result.push_back
	(buf("HTTP/"
	     + boost::lexical_cast<std::string>(request_.http_version_major)
	     + "."
	     + boost::lexical_cast<std::string>(request_.http_version_minor)
	     + " "));

      result.push_back(buf(status_strings::toText(responseStatus())));

      if (!http10 && responseStatus() != switching_protocols) {
	/*
	 * Date header (current time)
	 */
	result.push_back(buf(std::string("Date: ") + httpDate(time(0))));
	result.push_back(asio::buffer(misc_strings::crlf));
      }

      /*
       * Content type or location
       */

      std::string ct;
      if (responseStatus() >= 300 && responseStatus() < 400) {
	result.push_back(buf(std::string("Location: ") + location()));
	result.push_back(asio::buffer(misc_strings::crlf));
      } else if (responseStatus() != not_modified
		 && responseStatus() != switching_protocols) {
	ct = contentType();
	result.push_back(buf("Content-Type: " + ct));
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

      /*
       * Connection
       */
      //closeConnection_ = true;
      if (closeConnection_) {
	result.push_back(buf("Connection: close"));
	result.push_back(asio::buffer(misc_strings::crlf));
      } else {
	if (http10) {
	  result.push_back(buf("Connection: keep-alive"));
	  result.push_back(asio::buffer(misc_strings::crlf));
	}
      }

      if (responseStatus() != not_modified) {
	::int64_t cl = contentLength();

#ifdef WTHTTP_WITH_ZLIB
	/*
	 * Content-Encoding: gzip ?
	 */
	gzipEncoding_ = 
	     !haveContentEncoding
	  && Configuration::instance().compression()
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
	  result.push_back(buf("Content-Encoding: gzip"));
	  result.push_back(asio::buffer(misc_strings::crlf));
	  
	  initGzip();
	}
#endif

	if ((cl == -1) && http10 && !closeConnection_) {
	  /*
	   * We need to determine the length of the response...
	   */
	  contentSent_ = 0;
	  std::vector<asio::const_buffer> contentBuffers;
	  for (;;) {
	    int originalSize;
	    int encodedSize;
	    encodeNextContentBuffer(contentBuffers, originalSize, encodedSize);

	    contentSent_ += encodedSize;
	    contentOriginalSize_ += originalSize;

	    if (originalSize == 0)
	      break;
	  }

	  result.push_back 
	    (buf("Content-Length: "
		 + boost::lexical_cast<std::string>(contentSent_)));
	  result.push_back(asio::buffer(misc_strings::crlf));

	  result.push_back(asio::buffer(misc_strings::crlf));
      
	  result.insert(result.end(),
			contentBuffers.begin(), contentBuffers.end());

	  return finishing_ = true;
	} else {
	  /*
	   * We do not need to determine the length of the response...
	   * Transmit only header first.
	   */
	  if (cl != -1) {
	    result.push_back 
	      (buf("Content-Length: " + boost::lexical_cast<std::string>(cl)));
	    result.push_back(asio::buffer(misc_strings::crlf));

	    chunkedEncoding_ = false;
	  } else
	    if (closeConnection_)
	      chunkedEncoding_ = false; // should be false
	    else
	      if (!http10 && responseStatus() != switching_protocols)
		chunkedEncoding_ = true;

	  if (chunkedEncoding_) {
	    result.push_back(buf("Transfer-Encoding: chunked"));
	    result.push_back(asio::buffer(misc_strings::crlf));
	  }

	  result.push_back(asio::buffer(misc_strings::crlf));

	  return finishing_ = false;
	}
      } else { // responseStatus() == not-modified
	result.push_back(asio::buffer(misc_strings::crlf));

	return finishing_ = true;
      }
    } else { // transmitting (data)
      std::vector<asio::const_buffer> contentBuffers;
      int originalSize;
      int encodedSize;

      encodeNextContentBuffer(contentBuffers, originalSize, encodedSize);

      bool lastData = originalSize == 0;

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

	    if (!originalSize) {
	      result.push_back(buf("0"));
	      result.push_back(asio::buffer(misc_strings::crlf));
	      result.push_back(asio::buffer(misc_strings::crlf));
	    }
	  } else
	    result.push_back(asio::buffer(misc_strings::crlf));
	}

	return finishing_ = lastData;
      } else {
	result = contentBuffers;

	return finishing_ = lastData;
      }
    }
  }

  assert(false);

  return false;
}

void Reply::setConnection(Connection *connection)
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED
  connection_ = connection;

  if (relay_.get())
    relay_->setConnection(connection);

  if (connection) {
    try {
      remoteAddress_
	= connection_->socket().remote_endpoint().address().to_string();
    } catch (std::exception& e) {
      std::cerr << "remote_endpoint() threw: " << e.what() << std::endl;
    }
    requestMethod_ = request_.method;
    requestUri_ = request_.uri;
    requestMajor_ = request_.http_version_major;
    requestMinor_ = request_.http_version_minor;
  }
}

void Reply::send()
{
#ifdef WT_THREADED
  boost::recursive_mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED

  if (/* !finishing_ && */ connection_)
    connection_->startWriteResponse();
}

void Reply::setRelay(ReplyPtr reply)
{
  assert(!transmitting_);

  relay_ = reply;
  relay_->connection_ = connection_;

  relay_->remoteAddress_ = remoteAddress_;
  relay_->requestMethod_ = requestMethod_;
  relay_->requestUri_ = requestUri_;
  relay_->requestMajor_ = requestMajor_;
  relay_->requestMinor_ = requestMinor_;
}

void Reply::logReply(Wt::WLogger& logger)
{
  if (relay_.get())
    return relay_->logReply(logger);

  Wt::WLogEntry e = logger.entry();

  e << remoteAddress_ << Wt::WLogger::sep
    << /* rfc931 << */ Wt::WLogger::sep
    << /* authuser << */ Wt::WLogger::sep
    << Wt::WLogger::timestamp << Wt::WLogger::sep
    << requestMethod_ << ' ' << requestUri_ << " HTTP/"
    << requestMajor_ << '.' << requestMinor_ << Wt::WLogger::sep
    << responseStatus() << Wt::WLogger::sep
    << contentSent_;

  /*
  if (gzipEncoding_)
      std::cerr << " <" << contentOriginalSize_ << ">";
  */

#if 0
#ifndef WIN32
    struct timeval endTime;
    gettimeofday(&endTime, 0);
    
    std::cerr << " (" 
	      << (endTime.tv_sec - startTime_.tv_sec)*1000
                 + (endTime.tv_usec - startTime_.tv_usec)/1000
	      << "ms)";
#endif
#endif
}

asio::const_buffer Reply::buf(const std::string s)
{
  bufs_.push_back(s);
  return asio::buffer(bufs_.back());
}

std::string Reply::httpDate(time_t t)
{
  struct tm td;
  gmtime_r(&t, &td);
  char buffer[100];
  strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", &td);

  return buffer;
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
  assert(r == Z_OK);
}
#endif

void Reply::encodeNextContentBuffer(
       std::vector<asio::const_buffer>& result, int& originalSize,
       int& encodedSize)
{
  asio::const_buffer b = nextContentBuffer();
  originalSize = buffer_size(b);

#ifdef WTHTTP_WITH_ZLIB
  if (gzipEncoding_) {
    encodedSize = 0;

    gzipStrm_.avail_in = originalSize;
    gzipStrm_.next_in
      = (unsigned char *)asio::detail::buffer_cast_helper(b);

    unsigned char out[16*1024];
    do {
      gzipStrm_.next_out = out;
      gzipStrm_.avail_out = sizeof(out);

      // do not attempt to flush gzip deflate when we are still expecting data
      if (gzipStrm_.avail_in == 0 && originalSize != 0)
	return;

      int r = 0;
      r = deflate(&gzipStrm_, gzipStrm_.avail_in == 0 ? Z_FINISH : Z_NO_FLUSH);

      assert(r != Z_STREAM_ERROR);
    
      unsigned have = sizeof(out) - gzipStrm_.avail_out;

      if (have) {
	encodedSize += have;
	result.push_back(buf(std::string((char *)out, have)));
      }
    } while (gzipStrm_.avail_out == 0);

    if (originalSize == 0)
      deflateEnd(&gzipStrm_);

  } else {
#endif
    encodedSize = originalSize;

    if (encodedSize)
      result.push_back(b);
#ifdef WTHTTP_WITH_ZLIB
  }
#endif
}

} // namespace server
} // namespace http
