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

#ifndef HTTP_STOCK_REPLY_HPP
#define HTTP_STOCK_REPLY_HPP

#include <string>
#include <vector>
#include <boost/asio.hpp>
namespace asio = boost::asio;

#include "Reply.h"
#include "WHttpDllDefs.h"

namespace http {
namespace server {

/// A stock reply to be sent to a client.
class WTHTTP_API StockReply : public Reply
{
public:
  StockReply(const Request& request, status_type status);
  StockReply(const Request& request, status_type status,
	     std::string extraContent);
  StockReply(const Request& request, status_type status,
	     const std::string &extraContent, const std::string &err_root);

  virtual void consumeData(Buffer::const_iterator begin,
			   Buffer::const_iterator end,
			   Request::State state);

protected:
  virtual status_type responseStatus();
  virtual std::string contentType();
  virtual ::int64_t contentLength();

  virtual asio::const_buffer nextContentBuffer();  

private:
  status_type status_;
  std::string err_root_;
  std::string content_;
  bool transmitted_;
};

} // namespace server
} // namespace http

#endif // HTTP_STOCK_REPLY_HPP
