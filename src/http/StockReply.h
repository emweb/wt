// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
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

#include "Reply.h"
#include "WHttpDllDefs.h"

namespace http {
namespace server {

/// A stock reply to be sent to a client.
class WTHTTP_API StockReply final : public Reply
{
public:
  StockReply(Request& request, status_type status,
	     const Configuration& configuration);

  StockReply(Request& request, status_type status,
	     std::string extraContent,
	     const Configuration& configuration);

  virtual void reset(const Wt::EntryPoint *ep) override;

  virtual bool consumeData(const char *begin,
			   const char *end,
			   Request::State state) override;

protected:
  virtual std::string contentType() override;
  virtual ::int64_t contentLength() override;

  virtual bool nextContentBuffers(std::vector<asio::const_buffer>& result) override;

private:
  std::string content_;
  bool transmitted_;
};

} // namespace server
} // namespace http

#endif // HTTP_STOCK_REPLY_HPP
