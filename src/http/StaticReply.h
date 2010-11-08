// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */
//
// reply.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_STATIC_REPLY_HPP
#define HTTP_STATIC_REPLY_HPP

#include <string>
#include <vector>
#include <fstream>
#include <boost/asio.hpp>
namespace asio = boost::asio;

#include "Reply.h"

namespace http {
namespace server {

class StockReply;
class Request;

class StaticReply : public Reply
{
public:
  StaticReply(const std::string &full_path, const std::string &extension,
	      const Request& request, const std::string &err_root);

  virtual void consumeData(Buffer::const_iterator begin,
			   Buffer::const_iterator end,
			   Request::State state);

protected:
  virtual status_type responseStatus();
  virtual std::string contentType();
  virtual ::int64_t contentLength();

  virtual asio::const_buffer nextContentBuffer();

private:
  std::string     path_;
  std::string     extension_;
  std::ifstream   stream_;
  ::int64_t fileSize_;

  char buf_[64 * 1024];

  std::string computeModifiedDate() const;
  std::string computeETag() const;
  static std::string computeExpires();

  void parseRangeHeader();
  bool hasRange_;
  ::int64_t rangeBegin_;
  ::int64_t rangeEnd_;

};

} // namespace server
} // namespace http

#endif // HTTP_STATIC_REPLY_HPP
