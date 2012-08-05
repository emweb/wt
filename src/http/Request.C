/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * All rights reserved.
 */

#include "Request.h"

#include <ostream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "SslUtils.h"

#include <Wt/WLogger>
#include <Wt/WSslInfo>
#include <Wt/WTime>

#ifdef HTTP_WITH_SSL
#include <openssl/ssl.h>
#endif

namespace Wt {
  LOGGER("wthttp");
}

namespace http {
namespace server {

void Request::reset()
{
  method.clear();
  uri.clear();
  urlScheme.clear();
  headerMap.clear();
  headerOrder.clear();
  request_path.clear();
  request_query.clear();

  contentLength = -1;

#ifdef HTTP_WITH_SSL
  ssl = 0;
#endif
  webSocketVersion = -1;
}

void Request::transmitHeaders(std::ostream& out) const
{
  static const char *CRLF = "\r\n";

  out << method << " " << uri << " HTTP/"
      << http_version_major << "."
      << http_version_minor << CRLF;

  for (std::size_t i = 0; i < headerOrder.size(); ++i) {
    HeaderMap::const_iterator it = headerOrder[i];
    out << it->first << ": " << it->second << CRLF;
  }
}

void Request::enableWebSocket()
{
  webSocketVersion = -1;

  HeaderMap::const_iterator i = headerMap.find("Connection");
  if (i != headerMap.end() && boost::icontains(i->second, "Upgrade")) {
    HeaderMap::const_iterator j = headerMap.find("Upgrade");
    if (j != headerMap.end() && boost::iequals(j->second, "WebSocket")) {
      webSocketVersion = 0;

      HeaderMap::const_iterator k = headerMap.find("Sec-WebSocket-Version");
      if (k != headerMap.end()) {
	try {
	  webSocketVersion = boost::lexical_cast<int>(k->second);
	} catch (std::exception& e) {
	  LOG_ERROR("could not parse Sec-WebSocket-Version: " << k->second);
	}
      }
    }
  }
}

bool Request::closeConnection() const 
{
  if ((http_version_major == 1)
      && (http_version_minor == 0)) {
    HeaderMap::const_iterator i = headerMap.find("Connection");

    if (i != headerMap.end()) {
      if (boost::iequals(i->second, "Keep-Alive"))
	return false;
    }

    return true;
  }

  if ((http_version_major == 1)
      && (http_version_minor == 1)) {
    HeaderMap::const_iterator i = headerMap.find("Connection");
    
    if (i != headerMap.end()) {
      if (boost::icontains(i->second, "close"))
	return true;
    }

    return false;
  }

  return true;
}

bool Request::acceptGzipEncoding() const
{
  HeaderMap::const_iterator i = headerMap.find("Accept-Encoding");

  if (i != headerMap.end())
    return i->second.find("gzip") != std::string::npos;
  else
    return false;
}

Wt::WSslInfo *Request::sslInfo() const
{
#ifdef HTTP_WITH_SSL
  if (!ssl)
    return 0;

  X509 *x509 = SSL_get_peer_certificate(ssl);
  
  if (x509) {
    Wt::WSslCertificate clientCert = Wt::Ssl::x509ToWSslCertificate(x509);
    
    X509_free(x509);

    std::vector<Wt::WSslCertificate> clientCertChain;
    STACK_OF(X509) *certChain = SSL_get_peer_cert_chain(ssl);
    if (certChain) {
      for (int i = 0; i < sk_X509_num(certChain); ++i) {
	X509 *x509_i = sk_X509_value(certChain, i);
	clientCertChain.push_back(Wt::Ssl::x509ToWSslCertificate(x509_i));
      }
    }
    
    Wt::WValidator::State state = Wt::WValidator::Invalid;
    std::string info;

    long SSL_state = SSL_get_verify_result(ssl);
    if (SSL_state == X509_V_OK) {
      state = Wt::WValidator::Valid;
    } else {
      state = Wt::WValidator::Invalid;
      info = X509_verify_cert_error_string(SSL_state);
    }
    Wt::WValidator::Result clientVerificationResult(state, info);
    
    return new Wt::WSslInfo(clientCert, 
			    clientCertChain, 
			    clientVerificationResult);
  }
#endif
  return 0;
}

std::string Request::getHeader(const std::string& name) const
{
  HeaderMap::const_iterator i = headerMap.find(name);

  if (i != headerMap.end())
    return i->second;
  else
    return std::string();
}

} // namespace server
} // namespace http
