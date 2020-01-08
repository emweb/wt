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

#ifndef HTTP_STATIC_REPLY_HPP
#define HTTP_STATIC_REPLY_HPP

#include <string>
#include <vector>
#include <fstream>

#include "Reply.h"

namespace http {
namespace server {

class StockReply;
class Request;

class StaticReply final : public Reply
{
public:
  StaticReply(Request& request, const Configuration& config);

  virtual void reset(const Wt::EntryPoint *ep) override;
  virtual void writeDone(bool success) override;

  virtual bool consumeData(const char *begin,
			   const char *end,
			   Request::State state) override;

protected:
  virtual std::string contentType() override;
  virtual ::int64_t contentLength() override;

  virtual bool nextContentBuffers(std::vector<asio::const_buffer>& result) override;

private:
  std::string path_;
  std::string extension_;
  std::ifstream stream_;
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
