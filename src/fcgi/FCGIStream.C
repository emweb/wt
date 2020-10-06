/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <climits>
#include <errno.h>
#include <stdio.h>

#include "FCGIStream.h"
#include "WebController.h"
#include "Configuration.h"
#include "SslUtils.h"

#include "Wt/WSslInfo.h"
#include "Wt/WLogger.h"

#include "fcgio.h"
#include "fcgi_config.h"  // HAVE_IOSTREAM_WITHASSIGN_STREAMBUF

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

  class FCGIRequest final : public WebResponse
  {
    mutable std::string scriptName_, serverName_, queryString_,
      serverPort_, pathInfo_, remoteAddr_;

  public:
    FCGIRequest(FCGX_Request *request)
      : request_(request),
	headersCommitted_(false),
	status_(-1)
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

    virtual void flush(ResponseState state, const WriteCallback& callback) override
    {
      out().flush();

      if (state == ResponseState::ResponseFlush) {
        setAsyncCallback(callback);
      } else {
        setAsyncCallback(nullptr);
      }

      emulateAsync(state);
    }

    virtual std::istream& in() override { return *in_; }
    virtual std::ostream& out() override {
      if (!headersCommitted_) {
	headersCommitted_ = true;
	if(status_ > -1 ) 
	  *out_ << "Status: " << status_ << "\r\n";
	*out_ << "\r\n";
      }
      return *out_; 
    }
    virtual std::ostream& err() override { return *err_; }

    virtual void setStatus(int status) override
    {
	  status_ = status;
    }

    virtual void setContentType(const std::string& value) override
    {
      addHeader("Content-Type", value);
    }

    virtual void addHeader(const std::string& name, const std::string& value) override
    {
      if (!headersCommitted_)
	*out_ << name << ": " << value << "\r\n";
      else
	LOG_WARN("addHeader(): " << name << ": " << value
		 << " ignored because headers already committed.");
    }

    virtual void setContentLength(::int64_t length) override
    {
      addHeader("Content-Length", std::to_string(length));
    }

    virtual void setRedirect(const std::string& url) override
    {
      *out_ << "Location: " << url << "\r\n\r\n";
    }

    virtual const char *headerValue(const char *name) const override 
    {
      return envValue(cgiEnvName(name).c_str());
    }

    std::vector<Wt::Http::Message::Header> headers() const override
    {
      std::vector<Wt::Http::Message::Header> headerVector;
      std::string header_prefix("HTTP_");
      int prefix_length = header_prefix.length();

      for(int i=0; request_->envp[i];++i){
	std::string env_string(request_->envp[i]);
	if (env_string.compare(0,prefix_length, header_prefix) == 0){
	  std::string name = env_string.substr(prefix_length, env_string.find("=") - prefix_length);
	  std::string value = env_string.substr(env_string.find("=") + 1);
          headerVector.push_back(Wt::Http::Message::Header(name, value));
        }
      }
      const char *contentLength = envValue("CONTENT_LENGTH");
      if (contentLength) {
	headerVector.push_back(Wt::Http::Message::Header("CONTENT_LENGTH", contentLength));
      }
      const char *contentType = envValue("CONTENT_TYPE");
      if (contentType){
	headerVector.push_back(Wt::Http::Message::Header("CONTENT_TYPE", contentType));
      }
      return headerVector;
    }


    virtual const char *envValue(const char *name) const override
    {
      char *result = FCGX_GetParam(name, request_->envp);
      if (result)
	return result;
      else
	return nullptr;
    }

    std::string cgiEnvName(const char *name) const {
      std::string result = name;
      std::string::size_type i;
      while ((i = result.find('-')) != std::string::npos)
	result[i] = '_';

      std::transform(result.begin(), result.end(), result.begin(), toupper);

      return "HTTP_" + result;
    }

    virtual const std::string& scriptName() const override
    {
      if (scriptName_.empty()) {
	if (entryPoint_)
	  scriptName_ = str(envValue("SCRIPT_NAME")) + entryPoint_->path();
	else
	  scriptName_ = str(envValue("SCRIPT_NAME"));
      }

      return scriptName_;
    }

    virtual const std::string& serverName() const override
    {
      if (serverName_.empty())
	serverName_ = str(envValue("SERVER_NAME"));

      return serverName_;
    }

    virtual const char *requestMethod() const override
    {
      return envValue("REQUEST_METHOD");
    }

    virtual const std::string& queryString() const override
    {
      if (queryString_.empty())
	queryString_ = str(envValue("QUERY_STRING"));

      return queryString_;
    }

    virtual const std::string& serverPort() const override
    {
      if (serverPort_.empty())
	serverPort_ = str(envValue("SERVER_PORT"));

      return serverPort_;
    }

    virtual const std::string& pathInfo() const override
    {
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

    virtual const std::string& remoteAddr() const override
    {
      if (remoteAddr_.empty())
	remoteAddr_ = str(envValue("REMOTE_ADDR"));

      return remoteAddr_;
    }

    virtual const char *urlScheme() const override
    {
      const char *https = envValue("HTTPS");
      if (https && strcasecmp(https, "ON") == 0)
	return "https";
      else
	return "http";
    }

    virtual bool isSynchronous() const {
      return true;
    }

    virtual std::unique_ptr<WSslInfo> sslInfo(const Wt::Configuration &) const override
    {
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
	    name += std::to_string(i);
	    char *cc = FCGX_GetParam(name.c_str(), request_->envp);
	    if (cc) {
              X509 *x509_i = Wt::Ssl::readFromPem(cc);
	      clientCertChain.push_back(Wt::Ssl::x509ToWSslCertificate(x509_i));
              X509_free(x509_i);
            } else 
	      break;
	  }
	  
	  
          Wt::ValidationState state = Wt::ValidationState::Invalid;
	  std::string verify = str(envValue("SSL_CLIENT_VERIFY"));
          std::string verifyInfo;
	  if (verify == "SUCCESS") {
	    state = Wt::ValidationState::Valid;
	  } else if (verify.empty()) {
	    state = Wt::ValidationState::Invalid;
	    verifyInfo = "SSL_CLIENT_VERIFY variable was empty";
	  } else {
	    state = Wt::ValidationState::Invalid;
	    verifyInfo = verify;
	  }
	  Wt::WValidator::Result clientVerificationResult(state, verifyInfo);

          return std::make_unique<Wt::WSslInfo>(clientCert,
                                                clientCertChain,
                                                clientVerificationResult);
	}
      }
#endif

      return nullptr;
    }

  private:
    FCGX_Request *request_;
    fcgi_streambuf *in_streambuf_, *out_streambuf_, *err_streambuf_;
    std::array<char, 8> buf_; // Workaround for bug with fcgi_streambuf::underflow()
    std::istream *in_;
    std::ostream *out_, *err_;
    bool headersCommitted_;
	int status_;
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
    int result = select(FD_SETSIZE, &rfds, nullptr, nullptr, &timeout);

    if (result == 0)
      return nullptr; // timeout
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
