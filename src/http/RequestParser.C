/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */
//
// request_parser.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/lexical_cast.hpp>

#include "md5.h"

#include "RequestParser.h"
#include "Request.h"
#include "Reply.h"
#include "Server.h"
#include "WebController.h"

#undef min

/*
 * mongrel does this (http://mongrel.rubyforge.org/security.html):
 *
 * Any header over 112k.
 * Any query string over 10k.
 * Any header field value over 80k.
 * Any header field name over 256 bytes.
 * Any request URI greater than 10k bytes.
 */

static std::size_t MAX_REQUEST_HEADER_SIZE = 112*1024;
static int MAX_URI_SIZE = 10*1024;
static int MAX_FIELD_VALUE_SIZE = 80*1024;
static int MAX_FIELD_NAME_SIZE = 256;
static int MAX_METHOD_SIZE = 16;

static int MAX_WEBSOCKET_MESSAGE_LENGTH = 112*1024;

namespace http {
namespace server {

RequestParser::RequestParser(Server *server)
  : server_(server)
{
  reset();
}

void RequestParser::reset()
{
  httpState_ = method_start;
  wsState_ = ws_start;
  frameType_ = 0x00;
  requestSize_ = 0;
  buf_ptr_ = 0;
}

bool RequestParser::consumeChar(char c)
{
  if (buf_ptr_ + dest_->length() > maxSize_)
    return false;

  buf_[buf_ptr_++] = c;
  
  if (buf_ptr_ == sizeof(buf_)) {
    dest_->append(buf_, sizeof(buf_));
    buf_ptr_ = 0;
  }

  return true;
}

void RequestParser::consumeToString(std::string& result, int maxSize)
{
  buf_ptr_ = 0;
  dest_ = &result;
  maxSize_ = maxSize;
  dest_->clear();
}

void RequestParser::consumeComplete()
{
  if (buf_ptr_)
    dest_->append(buf_, buf_ptr_);
  buf_ptr_ = 0;
}

bool RequestParser::initialState() const
{
  return (httpState_ == method_start);
}

boost::tuple<boost::tribool, Buffer::const_iterator>
RequestParser::parse(Request& req, Buffer::const_iterator begin,
		     Buffer::const_iterator end)
{
  boost::tribool Indeterminate = boost::indeterminate;
  boost::tribool& result(Indeterminate);

  while (boost::indeterminate(result) && (begin != end))
    result = consume(req, *begin++);

  return boost::make_tuple(result, begin);
}

bool RequestParser::parseBody(Request& req, ReplyPtr reply,
			      Buffer::const_iterator& begin,
			      Buffer::const_iterator end)
{
  static bool doWebSockets = server_->controller()->configuration().webSockets();

  if (doWebSockets && req.isWebSocketRequest()) {
    Request::State state = parseWebSocketMessage(req, reply, begin, end);

    if (state == Request::Error)
      reply->consumeData(begin, begin, Request::Error);

    return state != Request::Partial;
  } else {
    ::int64_t thisSize = std::min((::int64_t)(end - begin), remainder_);

    Buffer::const_iterator thisBegin = begin;
    Buffer::const_iterator thisEnd = begin + thisSize;
    remainder_ -= thisSize;

    begin = thisEnd;

    bool endOfRequest = remainder_ == 0;

    reply->consumeData(thisBegin, thisEnd,
		       endOfRequest ? Request::Complete : Request::Partial);

    if (reply->responseStatus() == Reply::request_entity_too_large)
      return true;
    else
      return endOfRequest;
  }
}

bool RequestParser::doWebSocketHandShake(const Request& req)
{
  Request::HeaderMap::const_iterator k1, k2, origin;

  k1 = req.headerMap.find("Sec-WebSocket-Key1");
  k2 = req.headerMap.find("Sec-WebSocket-Key2");
  origin = req.headerMap.find("Origin");

  if (k1 != req.headerMap.end() && k2 != req.headerMap.end()
      && origin != req.headerMap.end()) {
    ::uint32_t n1, n2;

    if (parseCrazyWebSocketKey(k1->second, n1)
	&& parseCrazyWebSocketKey(k2->second, n2)) {
      unsigned char key3[8];
      memcpy(key3, buf_, 8);

      ::uint32_t v;

      v = htonl(n1);
      memcpy(buf_, &v, 4);

      v = htonl(n2);
      memcpy(buf_ + 4, &v, 4);

      memcpy(buf_ + 8, key3, 8);

      md5_state_t c;
      md5_init(&c);
      md5_append(&c, (unsigned char *)buf_, 16);
      md5_finish(&c, (unsigned char *)buf_);

      return true;
    } else
      return false;
  } else
    return false;
}

bool RequestParser::parseCrazyWebSocketKey(const std::string& key,
					   ::uint32_t& result)
{
  std::string number;
  int spaces = 0;

  for (unsigned i = 0; i < key.length(); ++i)
    if (key[i] >= '0' && key[i] <= '9')
      number += key[i];
    else if (key[i] == ' ')
      ++spaces;

  ::uint64_t n = boost::lexical_cast< ::uint64_t >(number);

  if (!spaces)
    return false;
  
  if (n % spaces == 0) {
    result = n / spaces;
    return true;
  } else
    return false;
}


Request::State
RequestParser::parseWebSocketMessage(Request& req,
				     ReplyPtr reply,
				     Buffer::const_iterator& begin,
				     Buffer::const_iterator end)
{
  if (wsState_ == ws_start) {
    wsState_ = ws_hand_shake;

    reply->consumeData(begin, begin, Request::Complete);

    return Request::Complete;
  } else if (wsState_ == ws_hand_shake) {
    ::int64_t thisSize = std::min((::int64_t)(end - begin), 8 - remainder_);

    memcpy(buf_ + remainder_, begin, thisSize);
    remainder_ += thisSize;
    begin += thisSize;

    if (remainder_ == 8) {
      bool okay = doWebSocketHandShake(req);

      if (okay) {
	wsState_ = frame_start;
	
	reply->consumeData(buf_, buf_ + 16, Request::Complete);

	return Request::Complete;
      } else
	return Request::Error;
    } else
      return Request::Partial;
  }

  Buffer::const_iterator dataBegin = begin;
  Buffer::const_iterator dataEnd = end;

  Request::State state = Request::Partial;

  while (begin < end && state == Request::Partial) {
    switch (wsState_) {
    case frame_start:
      frameType_ = *begin;

      if (frameType_ & 0x80) {
	wsState_ = binary_length;
	remainder_ = 0;
      } else {
	wsState_ = text_data;
	dataBegin = begin;
        ++dataBegin;
	remainder_ = 0;
      }

      ++begin;
      break;

    case binary_length:
      remainder_ = remainder_ << 7 | (*begin & 0x7F);
      if ((*begin & 0x80) == 0) {
	if (remainder_ == 0 || remainder_ >= MAX_WEBSOCKET_MESSAGE_LENGTH)
	  return Request::Error;
	wsState_ = binary_data;
      }

      ++begin;
      break;

    case text_data:
      if (static_cast<unsigned char>(*begin) == 0xFF) {
	state = Request::Complete;
	dataEnd = begin;
      } else {
	++remainder_;

	if (remainder_ >= MAX_WEBSOCKET_MESSAGE_LENGTH)
	  return Request::Error;
      }

      ++begin;
      break;

    case binary_data:
      {
	::int64_t thisSize = std::min((::int64_t)(end - begin), remainder_);

	dataBegin = begin;
	begin = begin + thisSize;
	dataEnd = begin;
	remainder_ -= thisSize;

	if (remainder_ == 0)
	  state = Request::Complete;
	break;
      }

    default:
      assert(false);
    }
  }

  if (state == Request::Complete)
    wsState_ = frame_start;

  if (frameType_ == 0x00) {
    if (dataBegin < dataEnd || state == Request::Complete) {
      assert(*dataBegin != 0);

      reply->consumeData(dataBegin, dataEnd, state);
    }
  } else
    return Request::Error;

  return state;
}

boost::tribool& RequestParser::consume(Request& req, char input)
{
  static boost::tribool False(false);
  static boost::tribool True(true);
  static boost::tribool Indeterminate(boost::indeterminate);

  if (++requestSize_ > MAX_REQUEST_HEADER_SIZE)
    return False;

  switch (httpState_)
  {
  case method_start:
    if (input == '\r')
    {
      /*
       * allow a new line before a request -- this seems to be
       * accepted practice when dealing with multiple requests
       * in one connection, separated by a CRLF.
       */
      httpState_ = expecting_newline_0;
      return Indeterminate;
    } else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return False;
    }
    else
    {
      httpState_ = method;
      consumeToString(req.method, MAX_METHOD_SIZE);
      consumeChar(input);
      return Indeterminate;
    }
  case expecting_newline_0:
    if (input == '\n')
    {
      httpState_ = method_start;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case method:
    if (input == ' ')
    {
      consumeComplete();
      httpState_ = uri_start;
      return Indeterminate;
    }
    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return False;
    }
    else
    {
      if (consumeChar(input))
	return Indeterminate;
      else
	return False;
    }
  case uri_start:
    if (is_ctl(input))
    {
      return False;
    }
    else
    {
      httpState_ = uri;
      consumeToString(req.uri, MAX_URI_SIZE);
      consumeChar(input);
      return Indeterminate;
    }
  case uri:
    if (input == ' ')
    {
      consumeComplete();

      httpState_ = http_version_h;
      return Indeterminate;
    }
    else if (is_ctl(input))
    {
      return False;
    }
    else
    {
      if (consumeChar(input))
	return Indeterminate;
      else
	return False;
    }
  case http_version_h:
    if (input == 'H')
    {
      httpState_ = http_version_t_1;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case http_version_t_1:
    if (input == 'T')
    {
      httpState_ = http_version_t_2;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case http_version_t_2:
    if (input == 'T')
    {
      httpState_ = http_version_p;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case http_version_p:
    if (input == 'P')
    {
      httpState_ = http_version_slash;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case http_version_slash:
    if (input == '/')
    {
      req.http_version_major = 0;
      req.http_version_minor = 0;
      httpState_ = http_version_major_start;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case http_version_major_start:
    if (is_digit(input))
    {
      req.http_version_major = req.http_version_major * 10 + input - '0';
      httpState_ = http_version_major;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case http_version_major:
    if (input == '.')
    {
      httpState_ = http_version_minor_start;
      return Indeterminate;
    }
    else if (is_digit(input))
    {
      req.http_version_major = req.http_version_major * 10 + input - '0';
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case http_version_minor_start:
    if (is_digit(input))
    {
      req.http_version_minor = req.http_version_minor * 10 + input - '0';
      httpState_ = http_version_minor;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case http_version_minor:
    if (input == '\r')
    {
      httpState_ = expecting_newline_1;
      return Indeterminate;
    }
    else if (is_digit(input))
    {
      req.http_version_minor = req.http_version_minor * 10 + input - '0';
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case expecting_newline_1:
    if (input == '\n')
    {
      httpState_ = header_line_start;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case header_line_start:
    if (input == '\r')
    {
      httpState_ = expecting_newline_3;
      return Indeterminate;
    }
    else if (!req.headerMap.empty() && (input == ' ' || input == '\t'))
    {
      // continuation of previous header
      httpState_ = header_lws;
      return Indeterminate;
    }
    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return False;
    }
    else
    {
      consumeToString(headerName_, MAX_FIELD_NAME_SIZE);
      consumeChar(input);
      httpState_ = header_name;
      return Indeterminate;
    }
  case header_lws:
    if (input == '\r')
    {
      httpState_ = expecting_newline_2;
      return Indeterminate;
    }
    else if (input == ' ' || input == '\t')
    {
      return Indeterminate;
    }
    else if (is_ctl(input))
    {
      return False;
    }
    else
    {
      httpState_ = header_value;
      headerValue_.push_back(input);
      return Indeterminate;
    }
  case header_name:
    if (input == ':')
    {
      consumeComplete();
      httpState_ = space_before_header_value;
      return Indeterminate;
    }
    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return False;
    }
    else
    {
      if (consumeChar(input))
	return Indeterminate;
      else
	return False;
    }
  case space_before_header_value:
    if (input == ' ')
    {
      consumeToString(headerValue_, MAX_FIELD_VALUE_SIZE);
      httpState_ = header_value;

      return Indeterminate;
    }
    else
    {
      consumeToString(headerValue_, MAX_FIELD_VALUE_SIZE);
      httpState_ = header_value;
    }
  case header_value:
    if (input == '\r')
    {
      consumeComplete();

      if (req.headerMap.find(headerName_) != req.headerMap.end()) {
	req.headerMap[headerName_] += ',' + headerValue_;
      } else {
	Request::HeaderMap::iterator i
	  = req.headerMap.insert(std::make_pair(headerName_, headerValue_))
	    .first;
	req.headerOrder.push_back(i);
      }

      httpState_ = expecting_newline_2;
      return Indeterminate;
    }
    else if (is_ctl(input))
    {
      return False;
    }
    else
    {
      if (consumeChar(input))
	return Indeterminate;
      else
	return False;
    }
  case expecting_newline_2:
    if (input == '\n')
    {
      httpState_ = header_line_start;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case expecting_newline_3:
    if (input == '\n')
      return True;
    else
      return False;
  default:
    return False;
  }
}

bool RequestParser::is_char(int c)
{
  return c >= 0 && c <= 127;
}

bool RequestParser::is_ctl(int c)
{
  return (c >= 0 && c <= 31) || c == 127;
}

bool RequestParser::is_tspecial(int c)
{
  switch (c)
  {
  case '(': case ')': case '<': case '>': case '@':
  case ',': case ';': case ':': case '\\': case '"':
  case '/': case '[': case ']': case '?': case '=':
  case '{': case '}': case ' ': case '\t':
    return true;
  default:
    return false;
  }
}

bool RequestParser::is_digit(int c)
{
  return c >= '0' && c <= '9';
}

Reply::status_type RequestParser::validate(Request& req)
{
  req.contentLength = 0;

  Request::HeaderMap::const_iterator i = req.headerMap.find("Content-Length");
  if (i != req.headerMap.end()) {
    try {
      req.contentLength = boost::lexical_cast< ::int64_t >(i->second);
    } catch (boost::bad_lexical_cast&) {
      return Reply::bad_request;
    }
  }

  /*
   * We could probably be more pedantic here.
   */
  remainder_ = req.contentLength;

  /*
   * Other things that we could check: if there is a body expected (like
   * POST, but there is no content-length: return 411 Length Required
   *
   * HTTP 1.1 (RFC 2616) and HTTP 1.0 (RFC 1945) validation
   */

  return Reply::ok;
}

} // namespace server
} // namespace http
