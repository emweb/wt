// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */
//
// request_parser.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_REQUEST_PARSER_HPP
#define HTTP_REQUEST_PARSER_HPP

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>

#include "Buffer.h"
#include "Reply.h"

namespace http {
namespace server {

class Request;
class Server;

/// Parser for incoming requests.
class RequestParser
{
public:
  /// Construct ready to parse the request method.
  RequestParser(Server *server);

  /// Reset to initial parser state.
  void reset();

  /// Parse some data. The tribool return value is true when a complete request
  /// has been parsed, false if the data is invalid, indeterminate when more
  /// data is required. The iterator return value indicates how much
  /// of the input has been consumed.
  boost::tuple<boost::tribool, Buffer::const_iterator>
    parse(Request& req, Buffer::const_iterator begin,
	  Buffer::const_iterator end);

  /// Validate
  Reply::status_type validate(Request& req);

  bool parseBody(Request& req, ReplyPtr reply,
		 Buffer::const_iterator& begin, Buffer::const_iterator end);

  bool initialState() const;


private:
  /// Handle the next character of input.
  boost::tribool& consume(Request& req, char input);

  /// Check if a byte is an HTTP character.
  static bool is_char(int c);

  /// Check if a byte is an HTTP control character.
  static bool is_ctl(int c);

  /// Check if a byte is defined as an HTTP tspecial character.
  static bool is_tspecial(int c);

  /// Check if a byte is a digit.
  static bool is_digit(int c);

  bool consumeChar(char input);
  void consumeToString(std::string& result, int maxSize);
  void consumeComplete();

  Request::State parseWebSocketMessage(Request& req, ReplyPtr reply,
				       Buffer::const_iterator& begin,
				       Buffer::const_iterator end);

  bool doWebSocketHandShake(const Request& req);
  bool parseCrazyWebSocketKey(const std::string& key, ::uint32_t& number);

  Server *server_;

  /// The current state of the request parser.
  enum http_state
  {
    method_start,
    expecting_newline_0,
    method,
    uri_start,
    uri,
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3
  } httpState_;

  enum ws_state {
    ws_start,
    ws_hand_shake,
    frame_start,
    text_data,
    binary_length,
    binary_data
  } wsState_;

  unsigned char frameType_;

  std::string  headerName_;
  std::string  headerValue_;

  ::uint64_t   requestSize_;

  // used for HTTP POST body, WS hand-shake, and WS binary frame length
  ::int64_t    remainder_;

  char         buf_[4096];
  unsigned     buf_ptr_;
  std::string *dest_;
  unsigned     maxSize_;
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_PARSER_HPP
