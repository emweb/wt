// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */

#ifndef HTTP_ACCESS_LOGGER_HPP
#define HTTP_ACCESS_LOGGER_HPP

#include <Wt/WLogger.h>

#include "WHttpDllDefs.h"

#include <mutex>
#include <shared_mutex>
#include <vector>

namespace http {
namespace server {

class AccessLogger : public Wt::WLogger
{
public:
  AccessLogger();
  ~AccessLogger();

  void setFormat(const std::string& format);
  std::string format() const { return format_; }

  std::string createMessage(
    const std::string& remoteIP,
    const std::string& method,
    const std::string& uri,
    const std::string& httpVersion,
    const std::string& status,
    const std::string& content) const;

private:
  bool beingDeleted_;
  std::string format_;
  mutable std::shared_timed_mutex formatLock_;

  enum class Field {
    Custom, // custom text, not a field
    IP, // remote IP address
    Method, // HTTP method (e.g. GET, POST)
    URI, // requested URI
    HttpVersion, // HTTP version
    Status, // HTTP status code
    Content // message body
  };

  struct Token {
    Token(Field type);
    Token(const std::string& text);

    Field type;
    std::string text;
  };

  std::vector<Token> tokens_;

  // must hold formatLock_ write mutex when calling this
  void parseFormat();
};


} // namespace server
} // namespace http

#endif // HTTP_ACCESS_LOGGER_HPP
