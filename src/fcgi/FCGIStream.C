/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <algorithm>
#include <cctype>
#include <cstring>
#include <climits>
#include <errno.h>
#include <stdio.h>

#include "FCGIStream.h"
#include "WebController.h"
#include "Configuration.h"
#include "SslUtils.h"

#include "Wt/WSslInfo"
#include "Wt/WLogger"

#include "fcgio.h"
#include "fcgi_config.h"  // HAVE_IOSTREAM_WITHASSIGN_STREAMBUF

#include <boost/array.hpp>

#ifdef WT_WITH_SSL
#include <openssl/ssl.h>
#endif

#define SSL_CLIENT_CERT_CHAIN_PREFIX "SSL_CLIENT_CERT_CHAIN"

using std::memset;
using std::exit;

namespace Wt {
  LOGGER("wtfcgi");
}

namespace {
  using namespace Wt;

  inline std::string str(const char *v) {
    return v ? std::string(v) : std::string();
  }

  class FCGIRequest : public WebRequest
  {
    mutable std::string scriptName_, serverName_, queryString_,
      serverPort_, pathInfo_, remoteAddr_;

  public:
    FCGIRequest(FCGX_Request *request)
      : request_(request),
	headersCommitted_(false)
    { 
      in_streambuf_ = new fcgi_streambuf(request_->in, &buf_[0], buf_.size());
      out_streambuf_ = new fcgi_streambuf(request_->out);
      err_streambuf_ = new fcgi_streambuf(request_->err);

      in_ = new std::istream(in_streambuf_);
      out_ = new std::ostream(out_streambuf_);
      err_ = new std::ostream(err_streambuf_);
    }

    ~FCGIRequest() {
      delete err_;
      delete out_;
      delete in_;
      delete err_streambuf_;
      delete out_streambuf_;
      delete in_streambuf_;

      FCGX_Finish_r(request_);
      delete request_;
    }

    virtual void flush(ResponseState state, const WriteCallback& callback) {
      out().flush();

      if (state == ResponseFlush) {
        setAsyncCallback(callback);
      } else {
        setAsyncCallback(0);
      }

      emulateAsync(state);
    }

    virtual std::istream& in() { return *in_; }
    virtual std::ostream& out() {
      if (!headersCommitted_) {
	headersCommitted_ = true;
	*out_ << "\r\n";
      }
      return *out_; 
    }
    virtual std::ostream& err() { return *err_; }

    virtual void setStatus(int status)
    {
      *out_ << "Status: " << status << "\r\n";
    }

    virtual void setContentType(const std::string& value)
    {
      addHeader("Content-Type", value);
    }

    virtual void addHeader(const std::string& name, const std::string& value)
    {
      if (!headersCommitted_)
	*out_ << name << ": " << value << "\r\n";
      else
	LOG_WARN("addHeader(): " << name << ": " << value
		 << " ignored because headers already committed.");
    }

    virtual void setContentLength(::int64_t length)
    {
      addHeader("Content-Length", boost::lexical_cast<std::string>(length));
    }

    virtual void setRedirect(const std::string& url)
    {
      *out_ << "Location: " << url << "\r\n\r\n";
    }

    virtual const char *headerValue(const char *name) const {
      return envValue(cgiEnvName(name).c_str());
    }

    virtual const char *envValue(const char *name) const {
      char *result = FCGX_GetParam(name, request_->envp);
      if (result)
	return result;
      else
	return 0;
    }

    std::string cgiEnvName(const char *name) const {
      std::string result = name;
      std::string::size_type i;
      while ((i = result.find('-')) != std::string::npos)
	result[i] = '_';

      std::transform(result.begin(), result.end(), result.begin(), toupper);

      return "HTTP_" + result;
    }

    virtual const std::string& scriptName() const {
      if (scriptName_.empty()) {
	if (entryPoint_)
	  scriptName_ = str(envValue("SCRIPT_NAME")) + entryPoint_->path();
	else
	  scriptName_ = str(envValue("SCRIPT_NAME"));
      }

      return scriptName_;
    }

    virtual const std::string& serverName() const {
      if (serverName_.empty())
	serverName_ = str(envValue("SERVER_NAME"));

      return serverName_;
    }

    virtual const char *requestMethod() const {
      return envValue("REQUEST_METHOD");
    }

    virtual const std::string& queryString() const {
      if (queryString_.empty())
	queryString_ = str(envValue("QUERY_STRING"));

      return queryString_;
    }

    virtual const std::string& serverPort() const {
      if (serverPort_.empty())
	serverPort_ = str(envValue("SERVER_PORT"));

      return serverPort_;
    }

    virtual const std::string& pathInfo() const {
      if (pathInfo_.empty()) {
	pathInfo_ = str(envValue("PATH_INFO"));
	if (entryPoint_) {
	  /* Do we actually know what this is supposed to do? */
	  if (pathInfo_.length() >= entryPoint_->path().length()) {
	    pathInfo_ = pathInfo_.substr(entryPoint_->path().length());
	  }
	}
      }

      return pathInfo_;
    }

    virtual const std::string& remoteAddr() const {
      if (remoteAddr_.empty())
	remoteAddr_ = str(envValue("REMOTE_ADDR"));

      return remoteAddr_;
    }

    virtual const char *urlScheme() const {
      const char *https = envValue("HTTPS");
      if (https && strcasecmp(https, "ON") == 0)
	return "https";
      else
	return "http";
    }

    virtual bool isSynchronous() const {
      return true;
    }

    virtual WSslInfo *sslInfo() const {
#ifdef WT_WITH_SSL
      std::string clientCert = str(envValue("SSL_CLIENT_CERT"));
      if (!clientCert.empty()) {
	X509 *x509 = Wt::Ssl::readFromPem(clientCert);
	
	if (x509) {
          Wt::WSslCertificate clientCert = Wt::Ssl::x509ToWSslCertificate(x509);

	  X509_free(x509);

	  std::vector<Wt::WSslCertificate> clientCertChain;
	  unsigned depth = UINT_MAX;
	  for (unsigned i = 0; i < depth; i++) {
	    std::string name = SSL_CLIENT_CERT_CHAIN_PREFIX; 
	    name += boost::lexical_cast<std::string>(i);
	    char *cc = FCGX_GetParam(name.c_str(), request_->envp);
	    if (cc) {
              X509 *x509_i = Wt::Ssl::readFromPem(cc);
	      clientCertChain.push_back(Wt::Ssl::x509ToWSslCertificate(x509_i));
              X509_free(x509_i);
            } else 
	      break;
	  }
	  
	  
          Wt::WValidator::State state = Wt::WValidator::Invalid;
	  std::string verify = str(envValue("SSL_CLIENT_VERIFY"));
          std::string verifyInfo;
	  if (verify == "SUCCESS") {
	    state = Wt::WValidator::Valid;
	  } else if (verify.empty()) {
	    state = Wt::WValidator::Invalid;
	    verifyInfo = "SSL_CLIENT_VERIFY variable was empty";
	  } else {
	    state = Wt::WValidator::Invalid;
	    verifyInfo = verify;
	  }
	  Wt::WValidator::Result clientVerificationResult(state, verifyInfo);

	  return new Wt::WSslInfo(clientCert, 
                                  clientCertChain, 
				  clientVerificationResult);
	}
      }
#endif

      return 0;
    }

  private:
    FCGX_Request *request_;
    fcgi_streambuf *in_streambuf_, *out_streambuf_, *err_streambuf_;
    boost::array<char, 8> buf_; // Workaround for bug with fcgi_streambuf::underflow()
    std::istream *in_;
    std::ostream *out_, *err_;
    bool headersCommitted_;
  };
}

namespace Wt {

FCGIStream::FCGIStream()
{
  FCGX_Init();
}

FCGIStream::~FCGIStream()
{ }

WebRequest *FCGIStream::getNextRequest(int timeoutsec)
{
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(0, &rfds);
  struct timeval timeout;
  timeout.tv_sec = timeoutsec;
  timeout.tv_usec = 0;

  for(;;) {
    int result = select(FD_SETSIZE, &rfds, 0, 0, &timeout);

    if (result == 0)
      return 0; // timeout
    else if (result == -1) {
      if (errno != EINTR) {
	perror("select");
	exit(1); // FIXME: throw exception
      } else {
	// EINTR, try again
      }
    } else
      break;
  }

  FCGX_Request *request = new FCGX_Request();
  FCGX_InitRequest(request, 0, 0);

  if (FCGX_Accept_r(request) == 0) {
    return new FCGIRequest(request);
  } else {
    LOG_ERROR("could not FCGX_Accept ?");
    delete request;

    exit(1); // FIXME: throw exception
  }
}

}
