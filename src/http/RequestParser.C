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

#include "../Wt/WLogger"
#include "../Wt/Utils"

#include "../web/base64.h"

#include "RequestParser.h"
#include "Request.h"
#include "Reply.h"
#include "Server.h"
#include "WebController.h"

#undef min
#if defined(_MSC_VER)
#define strtoll _strtoi64
#endif

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

namespace Wt {
  LOGGER("wthttp");
}

namespace http {
namespace server {

RequestParser::RequestParser(Server *)
{
  reset();
}

void RequestParser::reset()
{
  httpState_ = method_start;
  wsState_ = ws_start;
  wsFrameType_ = 0x00;
  wsCount_ = 0;
  requestSize_ = 0;
  remainder_ = 0;
  currentString_ = 0;
  maxSize_ = 0;
  haveHeader_ = false;
}

bool RequestParser::consumeChar(Buffer::iterator d)
{
  if (currentString_->data == 0)
    currentString_->data = &(*d);

  if (++currentString_->len > maxSize_)
    return false;

  return true;
}

void RequestParser::consumeToString(buffer_string& result, int maxSize)
{
  currentString_ = &result;
  maxSize_ = maxSize;
}

void RequestParser::consumeComplete(Buffer::iterator d)
{
  // We 0-terminate for convenient C-string manipulations
  // Note that for headers, we may change this in a ',' when concatenating
  // values.
  *d = 0;
  currentString_ = 0;
}

bool RequestParser::initialState() const
{
  return (httpState_ == method_start);
}

boost::tuple<boost::tribool, Buffer::iterator>
RequestParser::parse(Request& req, Buffer::iterator begin,
		     Buffer::iterator end)
{
  boost::tribool result = boost::indeterminate;

  while (boost::indeterminate(result) && (begin != end))
    result = consume(req, begin++);

  if (boost::indeterminate(result) && currentString_) {
    req.headers.push_back(Request::Header());
    currentString_->next = &req.headers.back().value;
    currentString_ = currentString_->next;
  }

  return boost::make_tuple(result, begin);
}

RequestParser::ParseResult RequestParser::parseBody(Request& req, ReplyPtr reply,
			      Buffer::iterator& begin, Buffer::iterator end)
{
  if (req.type == Request::WebSocket) {
    Request::State state = parseWebSocketMessage(req, reply, begin, end);

    if (state == Request::Error)
      reply->consumeData(begin, begin, Request::Error);

    return state == Request::Partial ? ReadMore : Done;
  } else if (req.type == Request::TCP) {
    ::int64_t thisSize = (::int64_t)(end - begin);

    Buffer::iterator thisBegin = begin;
    Buffer::iterator thisEnd = begin + thisSize;

    begin = thisEnd;

    bool canReadMore = reply->consumeData(thisBegin, thisEnd,
	Request::Partial);

    if (reply->status() == Reply::request_entity_too_large)
      return Done;
    else if (canReadMore)
      return ReadMore;
    else
      return NotReady;
  } else {
    ::int64_t thisSize = std::min((::int64_t)(end - begin), remainder_);

    Buffer::iterator thisBegin = begin;
    Buffer::iterator thisEnd = begin + thisSize;
    remainder_ -= thisSize;

    begin = thisEnd;

    bool endOfRequest = remainder_ == 0;

    bool canReadMore = reply->consumeData(thisBegin, thisEnd,
			 endOfRequest ? Request::Complete : Request::Partial);

    if (reply->status() == Reply::request_entity_too_large)
      return Done;
    else if (endOfRequest)
      return Done;
    else if (canReadMore)
      return ReadMore;
    else
      return NotReady;
  }
}

bool RequestParser::doWebSocketHandshake00(const Request& req)
{
  const Request::Header *k1 = req.getHeader("Sec-WebSocket-Key1");
  const Request::Header *k2 = req.getHeader("Sec-WebSocket-Key2");
  const Request::Header *origin = req.getHeader("Origin");

  if (k1 && k2 && origin) {
    ::uint32_t n1, n2;

    if (parseCrazyWebSocketKey(k1->value, n1)
	&& parseCrazyWebSocketKey(k2->value, n2)) {
      unsigned char key3[8];
      memcpy(key3, ws00_buf_, 8);

      ::uint32_t v;

      v = htonl(n1);
      memcpy(ws00_buf_, &v, 4);

      v = htonl(n2);
      memcpy(ws00_buf_ + 4, &v, 4);

      memcpy(ws00_buf_ + 8, key3, 8);

      std::string md5 = Wt::Utils::md5(std::string(ws00_buf_, 16));
      memcpy(ws00_buf_, md5.c_str(), 16);

      return true;
    } else
      return false;
  } else
    return false;
}

bool RequestParser::parseCrazyWebSocketKey(const buffer_string& k,
					   ::uint32_t& result)
{
  std::string key = k.str();

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

std::string RequestParser::doWebSocketHandshake13(const Request& req)
{
  const Request::Header *k = req.getHeader("Sec-WebSocket-Key");

  if (k) {
    std::string key = k->value.str();
    static const std::string guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    std::string hash = Wt::Utils::sha1(key + guid);

    std::vector<char> v;
    base64::encode(hash.begin(), hash.end(), std::back_inserter(v));

    return std::string(v.begin(), v.end());
  } else
    return std::string();
}

Request::State
RequestParser::parseWebSocketMessage(Request& req, ReplyPtr reply,
				     Buffer::iterator& begin,
				     Buffer::iterator end)
{
  switch (wsState_) {
  case ws_start:
    {
      if (req.webSocketVersion != 0
	  && req.webSocketVersion != 7
	  && req.webSocketVersion != 8
	  && req.webSocketVersion != 13) {
	LOG_ERROR("ws: unsupported protocol version " << req.webSocketVersion);
	// FIXME add Sec-WebSocket-Version fields
	return Request::Error;
      }

      if (req.webSocketVersion == 0) {
	LOG_INFO("ws: connect with protocol version 0");
	/*
	 * In version 00, the hand shake is two-staged, we first need to
	 * send the 101 to be able to access the part of the handshake
	 * that is sent after the GET
	 */
	const Request::Header *host = req.getHeader("Host");
	if (!host || host->value.empty()) {
	  LOG_ERROR("ws: missing Host field");
	  return Request::Error;
	}

	wsState_ = ws00_hand_shake;
	wsCount_ = 0;

	reply->setStatus(Reply::switching_protocols);
	reply->addHeader("Connection", "Upgrade");
	reply->addHeader("Upgrade", "WebSocket");

	const Request::Header *origin = req.getHeader("Origin");
	if (origin && !origin->value.empty())
	  reply->addHeader("Sec-WebSocket-Origin", origin->value.str());

	std::string location = std::string(req.urlScheme)
	  + "://" + host->value.str() + req.request_path
	  + "?" + req.request_query;
	reply->addHeader("Sec-WebSocket-Location", location);

	reply->consumeData(begin, begin, Request::Partial);

	return Request::Complete;
      } else {
	LOG_INFO("ws: connect with protocol version " << req.webSocketVersion);
	std::string accept = doWebSocketHandshake13(req);

	if (accept.empty()) {
	  LOG_ERROR("ws: error computing handshake result");
	  return Request::Error;
	} else {
	  wsState_ = ws13_frame_start;
	  reply->setStatus(Reply::switching_protocols);
	  reply->addHeader("Connection", "Upgrade");
	  reply->addHeader("Upgrade", "WebSocket");
	  reply->addHeader("Sec-WebSocket-Accept", accept);
	  reply->consumeData(begin, begin, Request::Complete);

	  return Request::Complete;
	}
      }

      break;
    }
  case ws00_hand_shake:
    {
      unsigned thisSize = std::min((::int64_t)(end - begin),
				   (::int64_t)(8 - wsCount_));

      memcpy(ws00_buf_ + wsCount_, begin, thisSize);
      wsCount_ += thisSize;
      begin += thisSize;

      if (wsCount_ == 8) {
	bool okay = doWebSocketHandshake00(req);

	if (okay) {
	  wsState_ = ws00_frame_start;
	  reply->consumeData(ws00_buf_, ws00_buf_ + 16, Request::Complete);

	  return Request::Complete;
	} else {
	  LOG_ERROR("ws: invalid client hand-shake");
	  return Request::Error;
	}
      } else
	return Request::Partial;
    }
  default:
    break;
  }

  Buffer::iterator dataBegin = begin;
  Buffer::iterator dataEnd = end;

  Request::State state = Request::Partial;

  while (begin < end && state == Request::Partial) {
    switch (wsState_) {
    case ws00_frame_start:
      wsFrameType_ = *begin;

      if (wsFrameType_ & 0x80) {
	wsState_ = ws00_binary_length;
	remainder_ = 0;
      } else {
	wsState_ = ws00_text_data;
	dataBegin = begin;
        ++dataBegin;
	remainder_ = 0;
      }

      ++begin;

      break;
    case ws00_binary_length:
      remainder_ = remainder_ << 7 | (*begin & 0x7F);
      if ((*begin & 0x80) == 0) {
	if (remainder_ == 0 || remainder_ >= MAX_WEBSOCKET_MESSAGE_LENGTH) {
	  LOG_ERROR("ws: oversized binary frame of length " << remainder_);
	  return Request::Error;
	}
	wsState_ = ws00_binary_data;
      }

      ++begin;

      break;
    case ws00_text_data:
      if (static_cast<unsigned char>(*begin) == 0xFF) {
	state = Request::Complete;
	wsState_ = ws00_frame_start;
	dataEnd = begin;
      } else {
	++remainder_;

	if (remainder_ >= MAX_WEBSOCKET_MESSAGE_LENGTH) {
	  LOG_ERROR("ws: oversized text frame of length " << remainder_);
	  return Request::Error;
	}
      }

      ++begin;

      break;
    case ws00_binary_data:
      {
	::int64_t thisSize = std::min((::int64_t)(end - begin), remainder_);

	dataBegin = begin;
	begin = begin + thisSize;
	dataEnd = begin;
	remainder_ -= thisSize;

	if (remainder_ == 0) {
	  state = Request::Complete;
	  wsState_ = ws00_frame_start;
	}

	break;
      }
    case ws13_frame_start:
      {
	unsigned char frameType = *begin;

	LOG_DEBUG("ws: new frame, opcode byte=" << (int)frameType);

	/* RSV1-3 must be 0 */
	if (frameType & 0x70)
	  return Request::Error;

	switch (frameType & 0x0F) {
	case 0x0: // Continuation frame of a fragmented message
	  if (frameType & 0x80)
	    wsFrameType_ |= 0x80; // mark the end-of-frame

	  break;
	case 0x1: // Text frame
	case 0x2: // Binary frame
	case 0x8: // Close
	case 0x9: // Ping
	case 0xA: // Pong
	  wsFrameType_ = frameType;

	  break;
	default:
	  LOG_ERROR("ws: unknown opcode");
	  return Request::Error;
	}

	wsState_ = ws13_payload_length;
	wsCount_ = 0;

	++begin;

	break;
      }
    case ws13_payload_length:
      /* Client frame must be masked */
      if ((*begin & 0x80) == 0) {
	LOG_ERROR("ws: client frame not masked");
	return Request::Error;
      }

      remainder_ = *begin & 0x7F;

      if (remainder_ < 126) {
	LOG_DEBUG("ws: new frame length " << remainder_);
	wsMask_ = 0;
	wsState_ = ws13_mask;
	wsCount_ = 4;
      } else if (remainder_ == 126) {
	wsState_ = ws13_extended_payload_length;
	wsCount_ = 2;
	remainder_ = 0;
      } else if (remainder_ == 127) {
	wsState_ = ws13_extended_payload_length;
	wsCount_ = 8;
	remainder_ = 0;
      }

      ++begin;

      break;
    case ws13_extended_payload_length:
      remainder_ <<= 8;
      remainder_ |= (unsigned char)(*begin);
      --wsCount_;

      if (wsCount_ == 0) {
	LOG_DEBUG("ws: new frame length " << remainder_);
	if (remainder_ >= MAX_WEBSOCKET_MESSAGE_LENGTH) {
	  LOG_ERROR("ws: oversized frame of length " << remainder_);
	  return Request::Error;
	}
	wsMask_ = 0;
	wsState_ = ws13_mask;
	wsCount_ = 4;
      }

      ++begin;

      break;
    case ws13_mask:
      wsMask_ <<= 8;
      wsMask_ |= (unsigned char)(*begin);
      --wsCount_;

      if (wsCount_ == 0) {
	LOG_DEBUG("ws: new frame read mask");
	if (remainder_ != 0) {
	  wsState_ = ws13_payload;
	} else {
	  // Frame without data (like pong)
	  if (wsFrameType_ & 0x80)
	    state = Request::Complete;
	  wsState_ = ws13_frame_start;
	}
      }
      
      ++begin;

      break;
    case ws13_payload:
      {
	::int64_t thisSize = std::min((::int64_t)(end - begin), remainder_);

	dataBegin = begin;
	begin = begin + thisSize;
	dataEnd = begin;
	remainder_ -= thisSize;

	/* Unmask dataBegin to dataEnd, mask offset in wsCount_ */
	for (Buffer::iterator i = dataBegin; i != dataEnd; ++i) {
	  unsigned char d = *i;
	  unsigned char m = (unsigned char)(wsMask_ >> ((3 - wsCount_) * 8));
	  d = d ^ m;
	  *i = d;
	  wsCount_ = (wsCount_ + 1) % 4;
	}

	LOG_DEBUG("ws: reading payload, remains = " << remainder_);

	if (remainder_ == 0) {
	  if (wsFrameType_ & 0x80)
	    state = Request::Complete;

	  wsState_ = ws13_frame_start;
	}

	break;
      }
    default:
      assert(false);
    }
  }

  LOG_DEBUG("ws: " << (dataEnd - dataBegin) << "," << state);

  if (dataBegin < dataEnd || state == Request::Complete) {
    if (wsState_ < ws13_frame_start) {
      if (wsFrameType_ == 0x00)
	reply->consumeWebSocketMessage(Reply::text_frame,
				       dataBegin, dataEnd, state);
    } else {
      Reply::ws_opcode opcode = (Reply::ws_opcode)(wsFrameType_ & 0x0F);
      reply->consumeWebSocketMessage(opcode, dataBegin, dataEnd, state);
    }
  }

  return state;
}

boost::tribool& RequestParser::consume(Request& req, Buffer::iterator it)
{
  static boost::tribool False(false);
  static boost::tribool True(true);
  static boost::tribool Indeterminate(boost::indeterminate);

  const char input = *it;

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
      consumeChar(it);
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
      consumeComplete(it);
      httpState_ = uri_start;
      return Indeterminate;
    }
    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return False;
    }
    else
    {
      if (consumeChar(it))
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
      consumeChar(it);
      return Indeterminate;
    }
  case uri:
    if (input == ' ')
    {
      consumeComplete(it);

      httpState_ = http_version_h;
      return Indeterminate;
    }
    else if (is_ctl(input))
    {
      return False;
    }
    else
    {
      if (consumeChar(it))
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
    else if ((input == ' ' || input == '\t') && haveHeader_)
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
      req.headers.push_back(Request::Header());
      haveHeader_ = true;
      consumeToString(req.headers.back().name, MAX_FIELD_NAME_SIZE);
      consumeChar(it);
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
      consumeChar(it);
      return Indeterminate;
    }
  case header_name:
    if (input == ':')
    {
      consumeComplete(it);
      httpState_ = space_before_header_value;
      return Indeterminate;
    }
    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return False;
    }
    else
    {
      if (consumeChar(it))
	return Indeterminate;
      else
	return False;
    }
  case space_before_header_value:
    if (input == ' ')
    {
      consumeToString(req.headers.back().value, MAX_FIELD_VALUE_SIZE);
      httpState_ = header_value;

      return Indeterminate;
    }
    else
    {
      consumeToString(req.headers.back().value, MAX_FIELD_VALUE_SIZE);
      httpState_ = header_value;

      /* fall through */
    }
  case header_value:
    if (input == '\r')
    {
      consumeComplete(it);

      httpState_ = expecting_newline_2;
      return Indeterminate;
    }
    else if (is_ctl(input))
    {
      return False;
    }
    else
    {
      if (consumeChar(it))
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
  req.process();

  req.contentLength = 0;

  const Request::Header *h = req.getHeader("Content-Length");

  if (h) {
    if (!h->value.next) {
      char *endptr;
      const char *cl = h->value.data;
      req.contentLength = strtoll(cl, &endptr, 10);
      if (*endptr != 0)
	return Reply::bad_request;
    } else {
      try {
	std::string cl = h->value.str();
	req.contentLength = boost::lexical_cast< ::int64_t >(cl);
      } catch (boost::bad_lexical_cast&) {
	return Reply::bad_request;
      }
    }

    if (req.contentLength < 0)
      return Reply::bad_request;
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
