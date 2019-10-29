/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "HTTPRequest.h"
#include "Configuration.h"
#include "WtReply.h"

#include "Wt/Utils.h"
#include "Wt/WConfig.h"
#include "Wt/WLogger.h"
#include "Wt/WSslInfo.h"
#include "Wt/Http/Message.h"
#include "Wt/Json/Array.h"
#include "Wt/Json/Object.h"
#include "Wt/Json/Parser.h"

#include "web/SslUtils.h"

#define PEM_HEADER "-----BEGIN CERTIFICATE-----"
#define PEM_FOOTER "-----END CERTIFICATE-----"

#define PEM_ESCAPED_HEADER "-----BEGIN%20CERTIFICATE-----"
#define PEM_ESCAPED_FOOTER "-----END%20CERTIFICATE-----%0A"

namespace Wt {
  LOGGER("wthttp");
}

namespace http {
namespace server {

const std::string HTTPRequest::empty_;

HTTPRequest::HTTPRequest(WtReplyPtr reply, const Wt::EntryPoint *entryPoint)
  : reply_(reply)
{
  entryPoint_ = entryPoint;
}

void HTTPRequest::reset(WtReplyPtr reply, const Wt::EntryPoint *entryPoint)
{
  WebRequest::reset();

  reply_ = reply;
  entryPoint_ = entryPoint;
}

bool HTTPRequest::done() const
{
  return !reply_.get();
}

void HTTPRequest::flush(ResponseState state, const WriteCallback& callback)
{
  WtReplyPtr ptr = reply_;

  if (done())
    return;

  if (state == ResponseState::ResponseDone)
    reply_.reset();

  ptr->send(callback, state == ResponseState::ResponseDone);
}

void HTTPRequest::readWebSocketMessage(const ReadCallback& callback)
{
  reply_->readWebSocketMessage(callback);
}

bool HTTPRequest::webSocketMessagePending() const
{
  return reply_->readAvailable();
}

bool HTTPRequest::detectDisconnect(const DisconnectCallback& callback)
{
  reply_->detectDisconnect(callback);
  return true;
}

void HTTPRequest::setStatus(int status)
{
  reply_->setStatus((Reply::status_type) status);
}

void HTTPRequest::setContentLength(::int64_t length)
{
  reply_->setContentLength(length);
}

void HTTPRequest::addHeader(const std::string& name, const std::string& value)
{
  reply_->addHeader(name, value);
}

void HTTPRequest::setContentType(const std::string& value)
{
  reply_->setContentType(value);
}

void HTTPRequest::setRedirect(const std::string& url)
{
  reply_->setLocation(url);
}

const char *HTTPRequest::headerValue(const char *name) const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return nullptr;

  const Request::Header *i = p->request().getHeader(name);
  if (i)
    return cstr(i->value);
  else
    return nullptr;
}

std::vector<Wt::Http::Message::Header> HTTPRequest::headers() const
{
  std::vector<Wt::Http::Message::Header> headerVector;
  WtReplyPtr p = reply_;
  if (!p.get())
    return headerVector;

  const std::list<Request::Header> &headers = p->request().headers;

  for (std::list<Request::Header>::const_iterator it=headers.begin(); it != headers.end(); ++it){
    if (cstr(it->name)) {
      headerVector.push_back(Wt::Http::Message::Header(it->name.str(), it->value.str()));
    }
  }

  return headerVector;
}

const char *HTTPRequest::cstr(const buffer_string& bs) const {
  if (!bs.next)
    return bs.data;
  else {
    s_.push_back(bs.str());
    return s_.back().c_str();
  }
}

::int64_t HTTPRequest::contentLength() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return 0;

  return p->request().contentLength;
}

const char *HTTPRequest::contentType() const
{
  return headerValue("Content-Type");
}

const char *HTTPRequest::envValue(const char *name) const
{
  if (strcmp(name, "CONTENT_TYPE") == 0) {
    return headerValue("Content-Type");
  } else if (strcmp(name, "CONTENT_LENGTH") == 0) {
    return headerValue("Content-Length");
  } else if (strcmp(name, "SERVER_SIGNATURE") == 0) {
    return "<address>Wt httpd server</address>";
  } else if (strcmp(name, "SERVER_SOFTWARE") == 0) {
    return "Wthttpd/" WT_VERSION_STR ;
  } else if (strcmp(name, "SERVER_ADMIN") == 0) {
    return "webmaster@localhost";
  } else if (strcmp(name, "REMOTE_ADDR") == 0) {
    return remoteAddr().c_str();
  } else if (strcmp(name, "DOCUMENT_ROOT") == 0) {
    return reply_->configuration().docRoot().c_str();
  } else
    return nullptr;
}

const std::string& HTTPRequest::serverName() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return empty_;

  return p->configuration().serverName();
}

const std::string& HTTPRequest::serverPort() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return empty_;

  if (serverPort_.empty())
    serverPort_ = std::to_string(p->request().port);

  return serverPort_;
}

const std::string& HTTPRequest::scriptName() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return empty_;

  return p->request().request_path;
}

const char * HTTPRequest::requestMethod() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return nullptr;

  return cstr(p->request().method);
}

const std::string& HTTPRequest::queryString() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return empty_;

  return p->request().request_query;
}

const std::string& HTTPRequest::pathInfo() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return empty_;

  return p->request().request_extra_path;
}

const std::string& HTTPRequest::remoteAddr() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return empty_;

  return p->request().remoteIP;
}

const char *HTTPRequest::urlScheme() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return "http";

  return p->request().urlScheme;
}

bool HTTPRequest::isSynchronous() const
{
  return false;
}

std::unique_ptr<Wt::WSslInfo> HTTPRequest::sslInfo(bool behindReverseProxy) const
{
  auto result = reply_->request().sslInfo();
  if (behindReverseProxy) {
#ifdef HTTP_WITH_SSL
    if (!result)
      result = sslInfoFromJson();
#endif // HTTP_WITH_SSL
    if (!result)
      result = sslInfoFromHeaders();
  }
  return result;
}

#ifdef HTTP_WITH_SSL
std::unique_ptr<Wt::WSslInfo> HTTPRequest::sslInfoFromJson() const
{
  const char * const ssl_client_certificates = headerValue("X-Wt-Ssl-Client-Certificates");

  if (!ssl_client_certificates)
    return nullptr;

  Wt::Json::Object obj;
  Wt::Json::ParseError error;
  if (!Wt::Json::parse(Wt::Utils::base64Decode(ssl_client_certificates), obj, error)) {
    LOG_ERROR("error while parsing client certificates");
    return nullptr;
  }

  std::string clientCertificatePem = obj["client-certificate"];

  X509 *cert = Wt::Ssl::readFromPem(clientCertificatePem);

  if (cert) {
    Wt::WSslCertificate clientCert = Wt::Ssl::x509ToWSslCertificate(cert);
    X509_free(cert);

    const Wt::Json::Array &arr = obj["client-pem-certification-chain"];

    std::vector<Wt::WSslCertificate> clientCertChain;

    for (const auto &cert : arr) {
      clientCertChain.push_back(Wt::Ssl::x509ToWSslCertificate(Wt::Ssl::readFromPem(cert)));
    }

    Wt::ValidationState state = static_cast<Wt::ValidationState>(static_cast<int>(obj["client-verification-result-state"]));
    Wt::WString message = obj["client-verification-result-message"];

    return Wt::cpp14::make_unique<Wt::WSslInfo>(clientCert,
                                                clientCertChain,
                                                Wt::WValidator::Result(state, message));
  }

  return nullptr;
}
#endif // HTTP_WITH_SSL

std::unique_ptr<Wt::WSslInfo> HTTPRequest::sslInfoFromHeaders() const
{
  const char * const client_verify = headerValue("X-SSL-Client-Verify");
  const char * const client_s_dn = headerValue("X-SSL-Client-S-DN");
  const char * const client_i_dn = headerValue("X-SSL-Client-I-DN");
  const char * const validity_start = headerValue("X-SSL-Client-V-Start");
  const char * const validity_end = headerValue("X-SSL-Client-V-End");
  const char * const client_cert = headerValue("X-SSL-Client-Cert");

  if (!client_verify)
    return nullptr;

  if (boost::iequals(client_verify, "NONE"))
    return nullptr;

  enum Verify {
    SUCCESS,
    FAILED,
    GENEROUS
  };

  const char *failedReason = nullptr;

  Verify v = FAILED;
  if (boost::iequals(client_verify, "SUCCESS"))
    v = SUCCESS;
  else if (boost::iequals(client_verify, "GENEROUS"))
    v = GENEROUS;
  else if (boost::istarts_with(client_verify, "FAILED:")) {
    v = FAILED;
    failedReason = client_verify + sizeof("FAILED");
  } else
    return nullptr;

  std::string clientCertStr;
  if (client_cert) {
    clientCertStr = client_cert;
    boost::trim(clientCertStr);
    if (boost::starts_with(clientCertStr, PEM_HEADER)) {
      const std::size_t start = sizeof(PEM_HEADER) - 1;
      const std::size_t end = clientCertStr.find(PEM_FOOTER);
      if (end != std::string::npos) {
        for (std::size_t i = start; i < end; ++i) {
          if (clientCertStr[i] == ' ')
            clientCertStr[i] = '\n';
        }
      } else {
        clientCertStr.clear();
      }
    } else if (boost::starts_with(clientCertStr, PEM_ESCAPED_HEADER) &&
               boost::ends_with(clientCertStr, PEM_ESCAPED_FOOTER)) {
      clientCertStr = Wt::Utils::urlDecode(clientCertStr);
    } else {
      clientCertStr.clear();
    }
  }

#ifdef WT_WITH_SSL
  if (!clientCertStr.empty()) {
    // try parse cert, use cert for all other info
    X509 *cert = Wt::Ssl::readFromPem(clientCertStr);

    if (cert) {
      Wt::WSslCertificate clientCert = Wt::Ssl::x509ToWSslCertificate(cert);
      return Wt::cpp14::make_unique<Wt::WSslInfo>(clientCert,
                                                  std::vector<Wt::WSslCertificate>(),
                                                  Wt::WValidator::Result(v == SUCCESS ? Wt::ValidationState::Valid : Wt::ValidationState::Invalid,
                                                                         failedReason ? Wt::utf8(failedReason) : Wt::WString::Empty));
    }
  }
#endif // WT_WITH_SSL

  if (client_s_dn &&
      client_i_dn &&
      validity_start &&
      validity_end) {
    std::vector<Wt::WSslCertificate::DnAttribute> subjectDn =
        Wt::WSslCertificate::dnFromString(client_s_dn);
    std::vector<Wt::WSslCertificate::DnAttribute> issuerDn =
        Wt::WSslCertificate::dnFromString(client_i_dn);

    const Wt::WString validityFormat = Wt::utf8("MMM dd hh:mm:ss yyyy 'GMT'");
    Wt::WDateTime validityStart = Wt::WDateTime::fromString(
          Wt::utf8(validity_start), validityFormat);
    Wt::WDateTime validityEnd = Wt::WDateTime::fromString(
          Wt::utf8(validity_end), validityFormat);

    Wt::WSslCertificate clientCert(subjectDn,
                                   issuerDn,
                                   validityStart,
                                   validityEnd,
                                   clientCertStr);
    return Wt::cpp14::make_unique<Wt::WSslInfo>(clientCert,
                                                std::vector<Wt::WSslCertificate>(),
                                                Wt::WValidator::Result(v == SUCCESS ? Wt::ValidationState::Valid : Wt::ValidationState::Invalid,
                                                                       failedReason ? Wt::utf8(failedReason) : Wt::WString::Empty));
  }

  return nullptr;
}

const std::vector<std::pair<std::string, std::string> > &HTTPRequest::urlParams() const
{
  WtReplyPtr p = reply_;
  if (!p.get())
    return WebRequest::urlParams();

  return p->request().url_params;
}

} // namespace server
} // namespace http
