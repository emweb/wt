/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <algorithm>
#include <cctype>
#include <cstring>
#include <errno.h>
#include <stdio.h>

#include "FCGIStream.h"
#include "WebController.h"
#include "Configuration.h"

#include "fcgio.h"
#include "fcgi_config.h"  // HAVE_IOSTREAM_WITHASSIGN_STREAMBUF

using std::memset;
using std::exit;

namespace {
  using namespace Wt;

  class FCGIRequest : public WebRequest
  {
  public:
    FCGIRequest(FCGX_Request *request)
      : request_(request)
    { 
      in_streambuf_ = new fcgi_streambuf(request_->in);
      out_streambuf_ = new fcgi_streambuf(request_->out);
      err_streambuf_ = new fcgi_streambuf(request_->err);
      in_ = new std::istream(in_streambuf_);
      out_ = new std::ostream(out_streambuf_);
      err_ = new std::ostream(err_streambuf_);

      //std::cerr.rdbuf(err_->rdbuf());
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

    virtual void flush(ResponseState state, CallbackFunction callback) {
      out_->flush();

      if (state == ResponseFlush) {
        setAsyncCallback(callback);
      } else {
        setAsyncCallback(0);
      }
      emulateAsync(state);
    }

    virtual std::istream& in() { return *in_; }
    virtual std::ostream& out() { return *out_; }
    virtual std::ostream& err() { return *err_; }

    virtual void setStatus(int status)
    {
      out() << "Status: " << status << "\r\n";
    }

    virtual void setContentType(const std::string& value)
    {
      out() << "Content-Type: " << value << "\r\n\r\n";
    }

    virtual void addHeader(const std::string& name, const std::string& value)
    {
      out() << name << ": " << value << "\r\n";
    }

    virtual void setContentLength(::int64_t length)
    {
      addHeader("Content-Length", boost::lexical_cast<std::string>(length));
    }

    virtual void setRedirect(const std::string& url)
    {
      out() << "Location: " << url << "\r\n\r\n";
    }

    virtual std::string headerValue(const std::string& name) const {
      return envValue(cgiEnvName(name));
    }

    virtual std::string envValue(const std::string& name) const {
      char *result = FCGX_GetParam(name.c_str(), request_->envp);
      if (result)
	return result;
      else
	return "";
    }

    std::string cgiEnvName(const std::string& name) const {
      std::string result = name;
      std::string::size_type i;
      while ((i = result.find('-')) != std::string::npos)
	result[i] = '_';

      std::transform(result.begin(), result.end(), result.begin(), toupper);

      return "HTTP_" + result;
    }

    virtual std::string scriptName() const {
      if (entryPoint_) {
        return envValue("SCRIPT_NAME") + entryPoint_->path();
      } else {
        return envValue("SCRIPT_NAME");
      }
    }

    virtual std::string serverName() const {
      return envValue("SERVER_NAME");
    }

    virtual std::string requestMethod() const {
      return envValue("REQUEST_METHOD");
    }

    virtual std::string queryString() const {
      return envValue("QUERY_STRING");
    }

    virtual std::string serverPort() const {
      return envValue("SERVER_PORT");
    }

    virtual std::string pathInfo() const {
      if (entryPoint_) {
        std::string pi = envValue("PATH_INFO");
        if (pi.size() >= entryPoint_->path().size()) {
          return pi.substr(entryPoint_->path().size());
        } else {
          return pi;
        }
      } else {
        return envValue("PATH_INFO");
      }
    }

    virtual std::string remoteAddr() const {
      return envValue("REMOTE_ADDR");
    }

    virtual std::string urlScheme() const {
      std::string https = envValue("HTTPS");
      if (https == "ON" || https == "on")
	return "https";
      else
	return "http";
    }

    virtual bool isSynchronous() const {
      return true;
    }

  private:
    FCGX_Request *request_;
    fcgi_streambuf *in_streambuf_, *out_streambuf_, *err_streambuf_;
    std::istream *in_;
    std::ostream *out_, *err_;
  };
}

namespace Wt {

FCGIStream::FCGIStream()
  : WebStream(true)
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
      } else
	; // EINTR, try again
    } else
      break;
  }

  FCGX_Request *request = new FCGX_Request();
  FCGX_InitRequest(request, 0, 0);

  if (FCGX_Accept_r(request) == 0) {
    return new FCGIRequest(request);
  } else {
    std::cerr << "Could not FCGX_Accept ?" << std::endl;
    delete request;

    exit(1); // FIXME: throw exception
  }
}

}
