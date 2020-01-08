/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */

#include "Request.h"

#include <ostream>
#include <boost/algorithm/string.hpp>

#include "SslUtils.h"
#include "WebUtils.h"

#include <Wt/WLogger.h>
#include <Wt/WSslInfo.h>
#include <Wt/WTime.h>

#ifdef HTTP_WITH_SSL
#include <openssl/ssl.h>
#endif

#ifdef WT_WIN32
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define HAVE_STRCASECMP
#define HAVE_STRNCASECMP
#endif

#ifdef __QNXNTO__
#define strcasecmp stricmp
#define strncasecmp strnicmp
#define HAVE_STRCASECMP
#define HAVE_STRNCASECMP
#endif

namespace Wt {
  LOGGER("wthttp");
}

namespace http {
namespace server {

std::string buffer_string::str() const
{
  std::string result;
  result.reserve(length());

  for (const buffer_string *s = this; s; s = s->next)
    if (s->data)
      result.append(s->data, s->len);

  return result;
}

unsigned buffer_string::length() const
{
  unsigned result = 0;

  for (const buffer_string *s = this; s; s = s->next)
    result += s->len;
  
  return result;
}

bool buffer_string::contains(const char *s) const
{
  if (next)
    return strstr(str().c_str(), s) != nullptr;
  else if (data)
    return strstr(data, s) != nullptr;
  else
    return false;
}

bool buffer_string::icontains(const char *s) const
{
#ifdef HAVE_STRCASESTR
  if (next)
    return strcasestr(str().c_str(), s) != nullptr;
  else if (data)
    return strcasestr(data, s) != nullptr;
  else
    return false;
#else
  if (next)
    return boost::icontains(str().c_str(), s);
  else if (data)
    return boost::icontains(data, s);
  else
    return false;
#endif
}

bool buffer_string::iequals(const char *s) const
{
#ifdef HAVE_STRCASECMP
  if (next)
    return strcasecmp(s, str().c_str()) == 0;
  else if (data)
    return strcasecmp(s, data) == 0;
  else
    return false;
#else
  if (next)
    return boost::iequals(s, str().c_str());
  else if (data)
    return boost::iequals(s, data);
  else
    return false;
#endif
}

bool buffer_string::istarts_with(const char *s, unsigned int len) const
{
#ifdef HAVE_STRNCASECMP
  if (next)
    return strncasecmp(s, str().c_str(), len) == 0;
  else if (data)
    return strncasecmp(s, data, len) == 0;
  else
    return false;
#else
  if (next)
    return boost::istarts_with(str().c_str(), s);
  else if (data)
    return boost::istarts_with(data, s);
  else
    return false;
#endif
}

void buffer_string::write(std::ostream &os) const
{
  for (const buffer_string *s = this; s; s = s->next) {
    os.write(s->data, s->len);
  }
}

std::ostream& operator<< (std::ostream &os, const buffer_string &str)
{
  str.write(os);
  return os;
}

bool buffer_string::operator==(const buffer_string& other) const
{
  if (next || other.next)
    return str() == other.str();
  else
    if (data && other.data)
      return strcmp(data, other.data) == 0;
    else
      return data == other.data;
}

bool buffer_string::operator==(const std::string& other) const
{
  if (next)
    return str() == other;
  else if (data)
    return data == other;
  else
    return false;
}

bool buffer_string::operator==(const char *other) const
{
  if (next)
    return str() == other;
  else if (data)
    return strcmp(data, other) == 0;
  else
    return false;
}

bool buffer_string::operator!=(const char *other) const
{
  return !(*this == other);
}

void Request::reset()
{
  method.clear();
  uri.clear();
  urlScheme[0] = 0;
  headers.clear();
  request_path.clear();
  request_query.clear();

  contentLength = -1;
  webSocketVersion = -1;
  type = HTTP;
}

void Request::process()
{
  // Concatenate header values of same header with ',' separator
  for (HeaderList::iterator i = headers.begin(); i != headers.end(); ++i) {
    if (!i->name.empty()) {
      HeaderList::iterator j = i;
      for (++j; j != headers.end(); ++j) {
	if (j->name == i->name) {
	  buffer_string *s = &i->value;
	  while (s->next)
	    s = s->next;
	  s->data[s->len++] = ','; // replace '\0' with a ','
	  s->next = &j->value;

	  j->name.clear();
	}
      }
    }
  }
}

void Request::enableWebSocket()
{
  webSocketVersion = -1;

  const Header *i = getHeader("Connection");
  if (i && i->value.icontains("Upgrade")) {
    const Header *j = getHeader("Upgrade");
    if (j && j->value.iequals("WebSocket")) {
      webSocketVersion = 0;
      type = WebSocket;

      const Header *k = getHeader("Sec-WebSocket-Version");
      if (k) {
	try {
	  webSocketVersion = Wt::Utils::stoi(k->value.str());
	} catch (std::exception& e) {
	  LOG_ERROR("could not parse Sec-WebSocket-Version: "
		    << k->value.str());
	}
      }
    }
  }
}

bool Request::closeConnection() const 
{
  if ((http_version_major == 1) && (http_version_minor == 0)) {
    const Header *i = getHeader("Connection");

    if (i && i->value.iequals("Keep-Alive"))
      return false;

    return true;
  }

  if ((http_version_major == 1) && (http_version_minor == 1)) {
    const Header *i = getHeader("Connection");
    
    if (i && i->value.icontains("close"))
      return true;

    return false;
  }

  return true;
}

bool Request::acceptGzipEncoding() const
{
  const Header *i = getHeader("Accept-Encoding");

  if (i)
    return i->value.contains("gzip");
  else
    return false;
}

std::unique_ptr<Wt::WSslInfo> Request::sslInfo() const
{
#ifdef HTTP_WITH_SSL
  if (!ssl)
    return nullptr;

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
    
    Wt::ValidationState state = Wt::ValidationState::Invalid;
    std::string info;

    long SSL_state = SSL_get_verify_result(ssl);
    if (SSL_state == X509_V_OK) {
      state = Wt::ValidationState::Valid;
    } else {
      state = Wt::ValidationState::Invalid;
      info = X509_verify_cert_error_string(SSL_state);
    }
    Wt::WValidator::Result clientVerificationResult(state, info);
    
    return Wt::cpp14::make_unique<Wt::WSslInfo>(clientCert,
                                                clientCertChain,
                                                clientVerificationResult);
  }
#endif
  return nullptr;
}

const Request::Header *Request::getHeader(const std::string& name) const
{
  for (HeaderList::const_iterator i = headers.begin(); i != headers.end();
       ++i) {
    if (i->name.iequals(name.c_str()))
      return &(*i);
  }

  return nullptr;
}

const Request::Header *Request::getHeader(const char *name) const
{
  for (HeaderList::const_iterator i = headers.begin(); i != headers.end();
       ++i) {
    if (i->name.iequals(name))
      return &(*i);
  }

  return nullptr;
}

} // namespace server
} // namespace http
