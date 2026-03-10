/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */

#include "AccessLogger.h"

#include <regex>

namespace Wt {
  LOGGER("wthttp");
}

namespace http {
namespace server {

AccessLogger::Token::Token(Field type)
  : type(type)
{ }

AccessLogger::Token::Token(const std::string& text)
  : type(Field::Custom),
    text(text)
{ }

AccessLogger::AccessLogger()
  : beingDeleted_(false),
    redirect_(false)
{ }

AccessLogger::~AccessLogger()
{
  std::unique_lock<std::shared_timed_mutex> l(formatLock_);
  beingDeleted_ = true;
}

void AccessLogger::setFormat(const std::string& format)
{
  std::unique_lock<std::shared_timed_mutex> l(formatLock_);
  if (beingDeleted_) {
    LOG_ERROR("AccessLogger: cannot set format while being deleted");
    return;
  }

  format_ = format;
  parseFormat();
}

void AccessLogger::setRedirect(bool redirect)
{
  redirect_ = redirect;
}

std::string AccessLogger::createMessage(
  const std::string& remoteIP,
  const std::string& method,
  const std::string& uri,
  const std::string& httpVersion,
  const std::string& status,
  const std::string& content) const
{
  std::shared_lock<std::shared_timed_mutex> l(formatLock_);

  if (beingDeleted_) {
    LOG_ERROR("AccessLogger: cannot create message while being deleted");
    return "";
  }

  Wt::WStringStream res;
  for (const auto& token : tokens_) {
    switch (token.type) {
    case Field::Custom:
      res << token.text;
      break;
    case Field::IP:
      res << remoteIP;
      break;
    case Field::Method:
      res << method;
      break;
    case Field::URI:
      res << uri;
      break;
    case Field::HttpVersion:
      res << httpVersion;
      break;
    case Field::Status:
      res << status;
      break;
    case Field::Content:
      res << content;
      break;
    default:
      LOG_ERROR("AccessLogger: unknown field type " << static_cast<int>(token.type));
      break;
    }
  }
  return res.str();
}

void AccessLogger::parseFormat()
{
  tokens_.clear();
  const std::regex fieldSeparatorsRegex(
    "\\$\\{IP\\}|"
    "\\$\\{METHOD\\}|"
    "\\$\\{URI\\}|"
    "\\$\\{HTTP_VERSION\\}|"
    "\\$\\{STATUS\\}|"
    "\\$\\{CONTENT\\}");

  auto begin = std::sregex_token_iterator(format_.begin(), format_.end(), fieldSeparatorsRegex, {-1, 0});
  auto end = std::sregex_token_iterator();

  for (std::sregex_token_iterator i = begin; i != end; ++i) {

    std::string t = i->str();
    if (t == "${IP}") {
      tokens_.emplace_back(Field::IP);
    } else if (t == "${METHOD}") {
      tokens_.emplace_back(Field::Method);
    } else if (t == "${URI}") {
      tokens_.emplace_back(Field::URI);
    } else if (t == "${HTTP_VERSION}") {
      tokens_.emplace_back(Field::HttpVersion);
    } else if (t == "${STATUS}") {
      tokens_.emplace_back(Field::Status);
    } else if (t == "${CONTENT}") {
      tokens_.emplace_back(Field::Content);
    } else if (!t.empty()) {
      tokens_.emplace_back(t);
    }
  }
}



} // namespace server
} // namespace http
