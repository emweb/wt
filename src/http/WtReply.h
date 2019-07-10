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

#ifdef WTHTTP_WITH_ZLIB
#include <zlib.h>
#endif

namespace http {
namespace server {

class StockReply;
class HTTPRequest;
class WtReply;
class Configuration;

typedef std::shared_ptr<WtReply> WtReplyPtr;

/// A Wt application reply to be sent to a client.
class WtReply final : public Reply
{
public:
  WtReply(Request& request, const Wt::EntryPoint& ep,
	  const Configuration &config);

  virtual void reset(const Wt::EntryPoint *ep) override;
  virtual void writeDone(bool success) override;
  virtual void logReply(Wt::WLogger& logger) override;

  ~WtReply();

  virtual bool consumeData(const char *begin,
			   const char *end,
			   Request::State state) override;

  virtual bool consumeWebSocketMessage(ws_opcode opcode,
				       const char* begin,
				       const char* end,
				       Request::State state) override;

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
  asio::streambuf out_buf_;
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
#ifdef WTHTTP_WITH_ZLIB
  std::vector<asio::const_buffer> compressedBuffers_;
  bool deflateInitialized_;
#endif

  virtual std::string contentType() override;
  virtual std::string location() override;
  virtual ::int64_t contentLength() override;

  virtual bool nextContentBuffers(std::vector<asio::const_buffer>& result) override;

private:
  void readRestWebSocketHandshake();

  void consumeRequestBody(const char *begin,
			  const char *end,
			  Request::State state);
  void formatResponse(std::vector<asio::const_buffer>& result);
#ifdef WTHTTP_WITH_ZLIB
  int deflate(const unsigned char* in, size_t in_size, unsigned char out[], bool& hasMore);
  bool initDeflate();

  z_stream zOutState_;
#endif
};

} // namespace server
} // namespace http

#endif // WT_REPLY_HPP
