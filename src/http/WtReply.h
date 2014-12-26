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
#include "../web/WebRequest.h"

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
  WtReply(Request& request, const Wt::EntryPoint& ep,
	  const Configuration &config);

  virtual void reset(const Wt::EntryPoint *ep);
  virtual void writeDone(bool success);
  virtual void logReply(Wt::WLogger& logger);

  ~WtReply();

  virtual bool consumeData(Buffer::const_iterator begin,
			   Buffer::const_iterator end,
			   Request::State state);

  virtual void consumeWebSocketMessage(ws_opcode opcode,
				       Buffer::const_iterator begin,
				       Buffer::const_iterator end,
				       Request::State state);

  void setContentLength(::int64_t length);
  void setContentType(const std::string& type);
  void setLocation(const std::string& location);
  void send(const Wt::WebRequest::WriteCallback& callBack,
	    bool responseComplete);
  void readWebSocketMessage(const Wt::WebRequest::ReadCallback& callBack);
  bool readAvailable();

  std::istream& in() { return *in_; }
  std::ostream& out() { return out_; }
  Request& request() { return request_; }
  std::string urlScheme() const { return urlScheme_; }

protected:
  const Wt::EntryPoint *entryPoint_;
  std::stringstream in_mem_;
  std::iostream *in_;
  std::string requestFileName_;
  boost::asio::streambuf out_buf_;
  std::ostream out_;
  std::string contentType_;
  std::string location_;
  std::string urlScheme_;
  std::size_t sending_;
  ::int64_t contentLength_, bodyReceived_;
  bool sendingMessages_;
  Wt::WebRequest::WriteCallback fetchMoreDataCallback_;
  Wt::WebRequest::ReadCallback readMessageCallback_;
  HTTPRequest *httpRequest_;

  char gatherBuf_[16];

  virtual std::string contentType();
  virtual std::string location();
  virtual ::int64_t contentLength();

  virtual bool nextContentBuffers(std::vector<asio::const_buffer>& result);

private:
  void readRestWebSocketHandshake();

  void consumeRequestBody(Buffer::const_iterator begin,
			  Buffer::const_iterator end,
			  Request::State state);
  void formatResponse(std::vector<asio::const_buffer>& result);
};

} // namespace server
} // namespace http

#endif // WT_REPLY_HPP
