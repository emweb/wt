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

#include "RequestParser.h"
#include "Request.h"
#include "Reply.h"
#undef min

/*
 * mongrel does this (http://mongrel.rubyforge.org/security.html):
 *
 * Any header over 112k.
 * Any query string over 10k.
 * Any header field value over 80k.
 * Any header field name over 256 bytes.
 * Any request URI greater than 512 bytes.
 */

static std::size_t MAX_REQUEST_HEADER_SIZE = 112*1024;
static int MAX_URI_SIZE = 512;
static int MAX_FIELD_VALUE_SIZE = 80*1024;
static int MAX_FIELD_NAME_SIZE = 256;
static int MAX_METHOD_SIZE = 16;

namespace http {
namespace server {

RequestParser::RequestParser()
  : state_(method_start)
{ 
  reset();
}

void RequestParser::reset()
{
  state_ = method_start;
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
  return (state_ == method_start);
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
  int thisSize = std::min((size_t)(end - begin), bodyRemainder_);

  Buffer::const_iterator thisBegin = begin;
  Buffer::const_iterator thisEnd = begin + thisSize;
  bodyRemainder_ -= thisSize;

  begin = thisEnd;

  reply->consumeRequestBody(thisBegin, thisEnd, bodyRemainder_ == 0);

  return bodyRemainder_ == 0;
}

boost::tribool& RequestParser::consume(Request& req, char input)
{
  static boost::tribool False(false);
  static boost::tribool True(true);
  static boost::tribool Indeterminate(boost::indeterminate);

  if (++requestSize_ > MAX_REQUEST_HEADER_SIZE)
	return False;

  switch (state_)
  {
  case method_start:
    if (input == '\r')
    {
      /*
       * allow a new line before a request -- this seems to be
       * accepted practice when dealing with multiple requests
       * in one connection, separated by a CRLF.
       */
      state_ = expecting_newline_0;
      return Indeterminate;
    } else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return False;
    }
    else
    {
      state_ = method;
      consumeToString(req.method, MAX_METHOD_SIZE);
      consumeChar(input);
      return Indeterminate;
    }
  case expecting_newline_0:
    if (input == '\n')
    {
      state_ = method_start;
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
      state_ = uri_start;
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
      state_ = uri;
      consumeToString(req.uri, MAX_URI_SIZE);
      consumeChar(input);
      return Indeterminate;
    }
  case uri:
    if (input == ' ')
    {
      consumeComplete();

      state_ = http_version_h;
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
      state_ = http_version_t_1;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case http_version_t_1:
    if (input == 'T')
    {
      state_ = http_version_t_2;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case http_version_t_2:
    if (input == 'T')
    {
      state_ = http_version_p;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case http_version_p:
    if (input == 'P')
    {
      state_ = http_version_slash;
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
      state_ = http_version_major_start;
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
      state_ = http_version_major;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case http_version_major:
    if (input == '.')
    {
      state_ = http_version_minor_start;
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
      state_ = http_version_minor;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case http_version_minor:
    if (input == '\r')
    {
      state_ = expecting_newline_1;
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
      state_ = header_line_start;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case header_line_start:
    if (input == '\r')
    {
      state_ = expecting_newline_3;
      return Indeterminate;
    }
    else if (!req.headerMap.empty() && (input == ' ' || input == '\t'))
    {
      // continuation of previous header
      state_ = header_lws;
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
      state_ = header_name;
      return Indeterminate;
    }
  case header_lws:
    if (input == '\r')
    {
      state_ = expecting_newline_2;
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
      state_ = header_value;
      headerValue_.push_back(input);
      return Indeterminate;
    }
  case header_name:
    if (input == ':')
    {
      consumeComplete();
      state_ = space_before_header_value;
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
      state_ = header_value;

      return Indeterminate;
    }
    else
    {
      consumeToString(headerValue_, MAX_FIELD_VALUE_SIZE);
      state_ = header_value;
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

      state_ = expecting_newline_2;
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
      state_ = header_line_start;
      return Indeterminate;
    }
    else
    {
      return False;
    }
  case expecting_newline_3:
    if (input == '\n')
	  return validate(req)?True:False;
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
  return c >= 0 && c <= 31 || c == 127;
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

bool RequestParser::validate(Request& req)
{
  Request::HeaderMap::const_iterator i = req.headerMap.find("Content-Length");

  if (i != req.headerMap.end()) {
    try {
      req.contentLength = boost::lexical_cast<unsigned int>(i->second);
    } catch (boost::bad_lexical_cast&) {
      return false;
    }
  }

  if (req.contentLength >= 0)
    bodyRemainder_ = req.contentLength;
  else
    bodyRemainder_ = 0;

  /*
   * HTTP 1.1 (RFC 2616) and HTTP 1.0 (RFC 1945) validation
   */
  return true;
}

} // namespace server
} // namespace http
