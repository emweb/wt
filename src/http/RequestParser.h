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

#ifdef WTHTTP_WITH_ZLIB
#include <zlib.h>
#endif

namespace http {
namespace server {

class Request;
class Server;

/// Parser for incoming requests.
class RequestParser
{
public:
  enum ParseResult {
    ReadMore, // The parser is ready to read more
    Done,     // The parser is done
    NotReady  // The parser is not ready to read more
  };

  /// Construct ready to parse the request method.
  RequestParser(Server *server);
  ~RequestParser();

  /// Reset to initial parser state.
  void reset();

  /// Parse some data. The tribool return value is true when a complete request
  /// has been parsed, false if the data is invalid, indeterminate when more
  /// data is required. The iterator return value indicates how much
  /// of the input has been consumed.
  boost::tuple<boost::tribool, Buffer::iterator>
    parse(Request& req, Buffer::iterator begin, Buffer::iterator end);

  /// Validate
  Reply::status_type validate(Request& req);

  ParseResult parseBody(Request& req, ReplyPtr reply,
		 Buffer::iterator& begin, Buffer::iterator end);

  bool initialState() const;
  

#ifdef WTHTTP_WITH_ZLIB
  bool frameCompressed_;
#endif


private:
  /// Handle the next character of input.
  boost::tribool& consume(Request& req, Buffer::iterator input);

  /// Check if a byte is an HTTP character.
  static bool is_char(int c);

  /// Check if a byte is an HTTP control character.
  static bool is_ctl(int c);

  /// Check if a byte is defined as an HTTP tspecial character.
  static bool is_tspecial(int c);

  /// Check if a byte is a digit.
  static bool is_digit(int c);

  bool consumeChar(Buffer::iterator d);
  void consumeToString(buffer_string& result, int maxSize);
  void consumeComplete(Buffer::iterator d);

  Request::State parseWebSocketMessage(Request& req, ReplyPtr reply,
				       Buffer::iterator& begin,
				       Buffer::iterator end);

  bool doWebSocketHandshake00(const Request& req);
  std::string doWebSocketHandshake13(const Request& req);
  bool parseCrazyWebSocketKey(const buffer_string& key, ::uint32_t& number);

#ifdef WTHTTP_WITH_ZLIB
  bool doWebSocketPerMessageDeflateNegotiation(const Request& req, std::string& compressHeader);
  bool inflate(unsigned char* in, size_t size, unsigned char out[], bool& hasMore);
  bool initInflate();
#endif

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
    ws00_hand_shake,
    ws00_frame_start,
    ws00_text_data,
    ws00_binary_length,
    ws00_binary_data,
    ws13_frame_start,
    ws13_payload_length,
    ws13_extended_payload_length,
    ws13_mask,
    ws13_payload
  } wsState_;


#ifdef WTHTTP_WITH_ZLIB
  z_stream zInState_;
  bool inflateInitialized_;
#endif
  uint64_t read_;

  // used for ws00 frameType or ws13 opcode byte
  unsigned char wsFrameType_;
  unsigned char wsCount_;
  unsigned wsMask_;

  // used for ws00 handshake
  char ws00_buf_[16];

  ::uint64_t requestSize_;

  // used for HTTP POST body and ws frame/payload length
  ::int64_t remainder_;

  // used for HTTP preamble + header parsing
  buffer_string *currentString_;
  unsigned maxSize_;
  bool haveHeader_;

  Server *server_;
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_PARSER_HPP
