/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <stdint.h>
#include <stddef.h>
#include <sstream>
#include <string>
#include <vector>

#include "Wt/WSslInfo.h"
#include "web/CgiParser.h"
#include "web/WebRequest.h"

#define kMaxInputLength 65536

// Minimal WebRequest used to drive CgiParser without a server/socket; modelled
// on the MockRequest in test/private/EventDecodeTest.C. The fuzz bytes are the
// request body fed through in().
class FuzzRequest : public Wt::WebRequest {
public:
  bool supportsTransferWebSocketResourceSocket() override { return false; }
  void flush(WT_MAYBE_UNUSED ResponseState state, WT_MAYBE_UNUSED const WriteCallback& callback) override { }
  std::istream &in() override { return in_; }
  std::ostream &out() override { return out_; }
  std::ostream &err() override { return err_; }
  void setRedirect(WT_MAYBE_UNUSED const std::string& url) override { }
  void setStatus(WT_MAYBE_UNUSED int status) override { }
  int status() override { return 0; }
  void setContentType(const std::string &value) override { contentType_ = value; }
  const char *contentType() const override { return contentType_.c_str(); }
  void setContentLength(int64_t length) override { contentLength_ = length; }
  int64_t contentLength() const override { return contentLength_; }
  void addHeader(WT_MAYBE_UNUSED const std::string& name, WT_MAYBE_UNUSED const std::string& value) override { }
  void insertHeader(WT_MAYBE_UNUSED const std::string &name, WT_MAYBE_UNUSED const std::string &value) override { }
  const char *envValue(WT_MAYBE_UNUSED const char *name) const override { return nullptr; }
  const std::string &serverName() const override { return s_; }
  const std::string &serverPort() const override { return s_; }
  const std::string &scriptName() const override { return s_; }
  const char *requestMethod() const override { return requestMethod_.c_str(); }
  const std::string &queryString() const override { return s_; }
  const std::string &pathInfo() const override { return s_; }
  const std::string &remoteAddr() const override { return s_; }
  const char *urlScheme() const override { return s_.c_str(); }
  const char *headerValue(WT_MAYBE_UNUSED const char* name) const override { return nullptr; }
  std::vector<Wt::Http::Message::Header> headers() const override { return {}; }
  std::unique_ptr<Wt::WSslInfo> sslInfo(const Wt::Configuration &) const override { return nullptr; }

  std::string contentType_;
  int64_t contentLength_ = 0;
  std::stringstream in_;
  std::stringstream out_;
  std::stringstream err_;
  std::string requestMethod_ = "POST";
  std::string s_;
};

// Fuzzes CgiParser (src/web/CgiParser.C): the multipart/form-data and
// x-www-form-urlencoded request-body parser.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size < 1 || Size > kMaxInputLength) {
    return 0;
  }

  const uint8_t selector = Data[0];
  const char *body = reinterpret_cast<const char *>(Data + 1);
  const size_t bodyLen = Size - 1;

  FuzzRequest request;
  request.in_.write(body, bodyLen);
  request.contentLength_ = static_cast<int64_t>(bodyLen);

  if ((selector & 1) == 0) {
    request.contentType_ = "multipart/form-data; boundary=AaB03x";
  } else {
    request.contentType_ = "application/x-www-form-urlencoded";
  }

  try {
    Wt::CgiParser parser(/*maxRequestSize=*/1 << 20, /*maxFormData=*/1 << 20);
    parser.parse(request, Wt::CgiParser::ReadDefault);
  } catch (...) {
  }

  return 0;
}
