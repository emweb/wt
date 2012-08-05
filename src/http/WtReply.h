// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */
//
#ifndef HTTP_WT_REPLY_HPP
#define HTTP_WT_REPLY_HPP

#include <string>
#include <sstream>
#include <vector>

#include "Reply.h"
#include "../web/Configuration.h"

namespace http {
namespace server {

class StockReply;
class HTTPRequest;
class WtReply;
class Configuration;

typedef boost::shared_ptr<WtReply> WtReplyPtr;

/// A Wt application reply to be sent to a client.
class WtReply : public Reply
{
public:
  typedef boost::function<void(void)> CallbackFunction;

  WtReply(const Request& request, const Wt::EntryPoint& ep,
	  const Configuration &config);
  ~WtReply();

  virtual void consumeData(Buffer::const_iterator begin,
			   Buffer::const_iterator end,
			   Request::State state);

  virtual void consumeWebSocketMessage(ws_opcode opcode,
				       Buffer::const_iterator begin,
				       Buffer::const_iterator end,
				       Request::State state);

  void setContentLength(::int64_t length);
  void setContentType(const std::string& type);
  void setLocation(const std::string& location);
  void send(const std::string& text, CallbackFunction callBack,
	    bool responseComplete);
  void readWebSocketMessage(CallbackFunction callBack);
  bool readAvailable();

  virtual bool waitMoreData() const;

  std::istream& cin() { return *cin_; }
  const Request& request() const { return request_; }
  std::string urlScheme() const { return urlScheme_; }

protected:
  const Wt::EntryPoint& entryPoint_;
  std::iostream    *cin_;
  std::stringstream cin_mem_;
  std::string       requestFileName_;
  std::string       cout_, nextCout_;
  std::string       contentType_;
  std::string       location_;
  std::string       urlScheme_;
  bool              responseSent_;
  ::int64_t         contentLength_, bodyReceived_;
  bool              sendingMessages_;
  CallbackFunction  fetchMoreDataCallback_, readMessageCallback_;
  HTTPRequest      *httpRequest_;
  bool              sending_;

  virtual std::string     contentType();
  virtual std::string     location();
  virtual ::int64_t       contentLength();

  virtual asio::const_buffer nextContentBuffer();  

private:
  void readRestWebSocketHandshake();

  void consumeRequestBody(Buffer::const_iterator begin,
			  Buffer::const_iterator end,
			  Request::State state);
};

} // namespace server
} // namespace http

#endif // WT_REPLY_HPP
