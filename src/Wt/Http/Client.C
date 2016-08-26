/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

// bugfix for https://svn.boost.org/trac/boost/ticket/5722
#include <boost/asio.hpp>

#include "Wt/Http/Client"
#include "Wt/WApplication"
#include "Wt/WIOService"
#include "Wt/WEnvironment"
#include "Wt/WLogger"
#include "Wt/WServer"
#include "Wt/Utils"

#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/system/error_code.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/enable_shared_from_this.hpp>

#ifdef WT_WITH_SSL
#include <boost/asio/ssl.hpp>

#if BOOST_VERSION >= 104700
#define VERIFY_CERTIFICATE
#endif

#endif // WT_WITH_SSL

#ifdef WT_WIN32
#define strcasecmp _stricmp
#endif

using boost::asio::ip::tcp;

namespace Wt {

LOGGER("Http.Client");

  namespace Http {

class Client::Impl : public boost::enable_shared_from_this<Client::Impl>
{
public:
  struct ChunkState {
    enum State { Size, Data, Complete, Error } state;
    std::size_t size;
    int parsePos;
  };

  Impl(WIOService& ioService, WServer *server, const std::string& sessionId)
    : ioService_(ioService),
      strand_(ioService),
      resolver_(ioService_),
      timer_(ioService_),
      server_(server),
      sessionId_(sessionId),
      timeout_(0),
      maximumResponseSize_(0),
      responseSize_(0),
      aborted_(false)
  { }

  virtual ~Impl() { }

  void setTimeout(int timeout) { 
    timeout_ = timeout; 
  }

  void setMaximumResponseSize(std::size_t bytes) {
    maximumResponseSize_ = bytes;
  }

  void request(const std::string& method, const std::string& auth, 
	       const std::string& server, int port, const std::string& path,
	       const Message& message)
  {
    std::ostream request_stream(&requestBuf_);
    request_stream << method << " " << path << " HTTP/1.1\r\n";
    request_stream << "Host: " << server << ":" 
		   << boost::lexical_cast<std::string>(port) << "\r\n";

    if (!auth.empty())
      request_stream << "Authorization: Basic " 
		     << Wt::Utils::base64Encode(auth) << "\r\n";

    bool haveContentLength = false;
    for (unsigned i = 0; i < message.headers().size(); ++i) {
      const Message::Header& h = message.headers()[i];
      if (strcasecmp(h.name().c_str(), "Content-Length") == 0)
	haveContentLength = true;
      request_stream << h.name() << ": " << h.value() << "\r\n";
    }

    if ((method == "POST" || method == "PUT" || method == "DELETE") &&
	!haveContentLength)
      request_stream << "Content-Length: " << message.body().length() 
		     << "\r\n";

    request_stream << "Connection: close\r\n\r\n";

    if (method == "POST" || method == "PUT" || method == "DELETE")
      request_stream << message.body();

    tcp::resolver::query query(server, boost::lexical_cast<std::string>(port));

    startTimer();
    resolver_.async_resolve
      (query,
       strand_.wrap(boost::bind(&Impl::handleResolve,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::iterator)));
  }

  void asyncStop(boost::shared_ptr<Impl> *impl)
  {
    ioService_.post
      (strand_.wrap(boost::bind(&Impl::stop, shared_from_this(), impl)));
  }

  Signal<boost::system::error_code, Message>& done() { return done_; }
  Signal<Message>& headersReceived() { return headersReceived_; }
  Signal<std::string>& bodyDataReceived() { return bodyDataReceived_; }

  bool hasServer() { return server_ != 0; }

protected:
  typedef boost::function<void(const boost::system::error_code&)>
    ConnectHandler;
  typedef boost::function<void(const boost::system::error_code&,
			       const std::size_t&)> IOHandler;

  virtual tcp::socket& socket() = 0;
  virtual void asyncConnect(tcp::endpoint& endpoint,
			    const ConnectHandler& handler) = 0;
  virtual void asyncHandshake(const ConnectHandler& handler) = 0;
  virtual void asyncWriteRequest(const IOHandler& handler) = 0;
  virtual void asyncReadUntil(const std::string& s,
			      const IOHandler& handler) = 0;
  virtual void asyncRead(const IOHandler& handler) = 0;

private:
  void stop(boost::shared_ptr<Impl> *impl)
  {
    /* Within strand */

    aborted_ = true;

    try {
      if (socket().is_open()) {
	boost::system::error_code ignored_ec;
	socket().shutdown(tcp::socket::shutdown_both, ignored_ec);
	socket().close();
      }
    } catch (std::exception& e) {
      LOG_INFO("Client::abort(), stop(), ignoring error: " << e.what());
    }

    if (impl)
      impl->reset();
  }

  void startTimer()
  {
    timer_.expires_from_now(boost::posix_time::seconds(timeout_));
    timer_.async_wait
      (strand_.wrap(boost::bind(&Impl::timeout, shared_from_this(),
				boost::asio::placeholders::error)));
  }

  void cancelTimer()
  {
    /* Within strand */

    timer_.cancel();
  }

  void timeout(const boost::system::error_code& e)
  {
    /* Within strand */

    if (e != boost::asio::error::operation_aborted) {
      boost::system::error_code ignored_ec;
      socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both,
			ignored_ec);

      err_ = boost::asio::error::timed_out;
    }
  }

  void handleResolve(const boost::system::error_code& err,
		     tcp::resolver::iterator endpoint_iterator)
  {
    /* Within strand */

    cancelTimer();

    if (!err && !aborted_) {
      // Attempt a connection to the first endpoint in the list.
      // Each endpoint will be tried until we successfully establish
      // a connection.
      tcp::endpoint endpoint = *endpoint_iterator;

      startTimer();
      asyncConnect(endpoint,
		   strand_.wrap(boost::bind(&Impl::handleConnect,
					    shared_from_this(),
					    boost::asio::placeholders::error,
					    ++endpoint_iterator)));
    } else {
      err_ = err;
      complete();
    }
  }
 
  void handleConnect(const boost::system::error_code& err,
		     tcp::resolver::iterator endpoint_iterator)
  {
    /* Within strand */

    cancelTimer();

    if (!err && !aborted_) {
      // The connection was successful. Do the handshake (SSL only)
      startTimer();
      asyncHandshake
	(strand_.wrap(boost::bind(&Impl::handleHandshake,
				  shared_from_this(),
				  boost::asio::placeholders::error)));
    } else if (endpoint_iterator != tcp::resolver::iterator()) {
      // The connection failed. Try the next endpoint in the list.
      socket().close();

      handleResolve(boost::system::error_code(), endpoint_iterator);
    } else {
      err_ = err;
      complete();
    }
  }

  void handleHandshake(const boost::system::error_code& err)
  {
    /* Within strand */

    cancelTimer();

    if (!err && !aborted_) {
      // The handshake was successful. Send the request.
      startTimer();
      asyncWriteRequest
	(strand_.wrap
	 (boost::bind(&Impl::handleWriteRequest,
		      shared_from_this(),
		      boost::asio::placeholders::error,
		      boost::asio::placeholders::bytes_transferred)));
    } else {
      err_ = err;
      complete();
    }
  }

  void handleWriteRequest(const boost::system::error_code& err,
			  const std::size_t&)
  {
    /* Within strand */

    cancelTimer();

    if (!err) {
      // Read the response status line.
      startTimer();
      asyncReadUntil
	("\r\n",
	 strand_.wrap
	 (boost::bind(&Impl::handleReadStatusLine,
		      shared_from_this(),
		      boost::asio::placeholders::error,
		      boost::asio::placeholders::bytes_transferred)));
    } else {
      err_ = err;
      complete();
    }
  }

  bool addResponseSize(std::size_t s)
  {
    responseSize_ += s;

    if (maximumResponseSize_ && responseSize_ > maximumResponseSize_) {
      err_ = boost::asio::error::message_size;
      complete();
      return false;
    }

    return true;
  }

  void handleReadStatusLine(const boost::system::error_code& err,
			    const std::size_t& s)
  {
    /* Within strand */

    cancelTimer();

    if (!err) {
      if (!addResponseSize(s))
	return;

      // Check that response is OK.
      std::istream response_stream(&responseBuf_);
      std::string http_version;
      response_stream >> http_version;
      unsigned int status_code;
      response_stream >> status_code;
      std::string status_message;
      std::getline(response_stream, status_message);
      if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
	err_ = boost::system::errc::make_error_code
	  (boost::system::errc::protocol_error);
	complete();
	return;
      }

      LOG_DEBUG(status_code << " " << status_message);

      response_.setStatus(status_code);

      // Read the response headers, which are terminated by a blank line.
      startTimer();
      asyncReadUntil
	("\r\n\r\n",
	 strand_.wrap
	 (boost::bind(&Impl::handleReadHeaders,
		      shared_from_this(),
		      boost::asio::placeholders::error,
		      boost::asio::placeholders::bytes_transferred)));
    } else {
      err_ = err;
      complete();
    }
  }

  void handleReadHeaders(const boost::system::error_code& err,
			 const std::size_t& s)
  {
    /* Within strand */

    cancelTimer();

    if (!err) {
      if (!addResponseSize(s))
	return;

      chunkedResponse_ = false;

      // Process the response headers.
      std::istream response_stream(&responseBuf_);
      std::string header;
      while (std::getline(response_stream, header) && header != "\r") {
	std::size_t i = header.find(':');
	if (i != std::string::npos) {
	  std::string name = boost::trim_copy(header.substr(0, i));
	  std::string value = boost::trim_copy(header.substr(i+1));
	  response_.addHeader(name, value);

	  if (boost::iequals(name, "Transfer-Encoding") &&
	      boost::iequals(value, "chunked")) {
	    chunkedResponse_ = true;
	    chunkState_.size = 0;
	    chunkState_.parsePos = 0;
	    chunkState_.state = ChunkState::Size;
	  }
	}
      }
      
      if (headersReceived_.isConnected()) {
	if (server_)
	  server_->post(sessionId_,
			boost::bind(&Impl::emitHeadersReceived,
				    shared_from_this()));
	else
	  emitHeadersReceived();
      }

      // Write whatever content we already have to output.
      if (responseBuf_.size() > 0) {
	std::stringstream ss;
	ss << &responseBuf_;
	addBodyText(ss.str());
      }

      // Start reading remaining data until EOF.
      startTimer();
      asyncRead(strand_.wrap
		(boost::bind(&Impl::handleReadContent,
			     shared_from_this(),
			     boost::asio::placeholders::error,
			     boost::asio::placeholders::bytes_transferred)));
    } else {
      err_ = err;
      complete();
    }
  }

  void handleReadContent(const boost::system::error_code& err,
			 const std::size_t& s)
  {
    /* Within strand */

    cancelTimer();

    if (!err) {
      if (!addResponseSize(s))
	return;

      std::stringstream ss;
      ss << &responseBuf_;

      addBodyText(ss.str());

      if (!aborted_) {
	// Continue reading remaining data until EOF.
	startTimer();
	asyncRead
	  (strand_.wrap
	   (boost::bind(&Impl::handleReadContent,
			shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)));
      }
    } else if (err != boost::asio::error::eof
	       && err != boost::asio::error::shut_down
	       && err != boost::asio::error::bad_descriptor
	       && err != boost::asio::error::operation_aborted
	       && err.value() != 335544539) {
      err_ = err;
      complete();
    } else {
      complete();
    }
  }

  void addBodyText(const std::string& text)
  {
    if (chunkedResponse_) {
      chunkedDecode(text);
      if (chunkState_.state == ChunkState::Error) {
	protocolError(); return;
      } else if (chunkState_.state == ChunkState::Complete) {
	complete(); return;
      }
    } else {
      if (maximumResponseSize_)
	response_.addBodyText(text);

      LOG_DEBUG("Data: " << text);
      haveBodyData(text);
    }
  }

  void chunkedDecode(const std::string& text)
  {
    std::string::const_iterator pos = text.begin();
    while (pos != text.end()) {
      switch (chunkState_.state) {
      case ChunkState::Size: {
	unsigned char ch = *(pos++);

	switch (chunkState_.parsePos) {
	case -2:
	  if (ch != '\r') {
	    chunkState_.state = ChunkState::Error; return;
	  }

	  chunkState_.parsePos = -1;

	  break;
	case -1:
	  if (ch != '\n') {
	    chunkState_.state = ChunkState::Error; return;
	  }

	  chunkState_.parsePos = 0;
	  
	  break;
	case 0:
	  if (ch >= '0' && ch <= '9') {
	    chunkState_.size <<= 4;
	    chunkState_.size |= (ch - '0');
	  } else if (ch >= 'a' && ch <= 'f') {
	    chunkState_.size <<= 4;
	    chunkState_.size |= (10 + ch - 'a');
	  } else if (ch >= 'A' && ch <= 'F') {
	    chunkState_.size <<= 4;
	    chunkState_.size |= (10 + ch - 'A');
	  } else if (ch == '\r') {
	    chunkState_.parsePos = 2;
	  } else if (ch == ';') {
	    chunkState_.parsePos = 1;
	  } else {
	     chunkState_.state = ChunkState::Error; return;
	  }

	  break;
	case 1:
	  /* Ignoring extensions and syntax for now */
	  if (ch == '\r')
	    chunkState_.parsePos = 2;

	  break;
	case 2:
	  if (ch != '\n') {
	    chunkState_.state = ChunkState::Error; return;
	  }

	  if (chunkState_.size == 0) {
	    chunkState_.state = ChunkState::Complete; return;
	  }
	    
	  chunkState_.state = ChunkState::Data;
	}

	break;
      }
      case ChunkState::Data: {
	std::size_t thisChunk
	  = std::min(std::size_t(text.end() - pos), chunkState_.size);
	std::string text = std::string(pos, pos + thisChunk);
	if (maximumResponseSize_)
	  response_.addBodyText(text);

	LOG_DEBUG("Chunked data: " << text);
	haveBodyData(text);
	chunkState_.size -= thisChunk;
	pos += thisChunk;

	if (chunkState_.size == 0) {
	  chunkState_.parsePos = -2;
	  chunkState_.state = ChunkState::Size;
	}
	break;
      }
      default:
	assert(false); // Illegal state
      }
    }
  }

  void protocolError()
  {
    err_ = boost::system::errc::make_error_code
      (boost::system::errc::protocol_error);
    complete();
  } 

  void complete()
  {
    stop(0);
    if (server_)
      server_->post(sessionId_,
		    boost::bind(&Impl::emitDone, shared_from_this()));
    else
      emitDone();
  }

  void haveBodyData(std::string text)
  {
    if (bodyDataReceived_.isConnected()) {
      if (server_)
	server_->post(sessionId_,
		      boost::bind(&Impl::emitBodyReceived, shared_from_this(),
				  text));
      else
	emitBodyReceived(text);
    }
  }

  void emitDone()
  {
    done_.emit(err_, response_);
  }

  void emitHeadersReceived() {
    headersReceived_.emit(response_);
  }

  void emitBodyReceived(std::string text) {
    bodyDataReceived_.emit(text);
  }

protected:
  WIOService& ioService_;
  boost::asio::strand strand_;
  tcp::resolver resolver_;
  boost::asio::streambuf requestBuf_;
  boost::asio::streambuf responseBuf_;

private:
  boost::asio::deadline_timer timer_;
  WServer *server_;
  std::string sessionId_;
  int timeout_;
  std::size_t maximumResponseSize_, responseSize_;
  bool chunkedResponse_;
  ChunkState chunkState_;
  boost::system::error_code err_;
  Message response_;
  Signal<boost::system::error_code, Message> done_;
  Signal<Message> headersReceived_;
  Signal<std::string> bodyDataReceived_;
  bool aborted_;
};

class Client::TcpImpl : public Client::Impl
{
public:
  TcpImpl(WIOService& ioService, WServer *server, const std::string& sessionId)
    : Impl(ioService, server, sessionId),
      socket_(ioService_)
  { }

protected:
  virtual tcp::socket& socket()
  {
    return socket_;
  }

  virtual void asyncConnect(tcp::endpoint& endpoint,
			    const ConnectHandler& handler)
  {
    socket_.async_connect(endpoint, handler);
  }

  virtual void asyncHandshake(const ConnectHandler& handler)
  {
    handler(boost::system::error_code());
  }

  virtual void asyncWriteRequest(const IOHandler& handler)
  {
    boost::asio::async_write(socket_, requestBuf_, handler);
  }

  virtual void asyncReadUntil(const std::string& s,
			      const IOHandler& handler)
  {
    boost::asio::async_read_until(socket_, responseBuf_, s, handler);
  }

  virtual void asyncRead(const IOHandler& handler)
  {
    boost::asio::async_read(socket_, responseBuf_,
			    boost::asio::transfer_at_least(1), handler);
  }

private:
  tcp::socket socket_;
};

#ifdef WT_WITH_SSL

class Client::SslImpl : public Client::Impl
{
public:
  SslImpl(WIOService& ioService, bool verifyEnabled, WServer *server,
	  boost::asio::ssl::context& context, const std::string& sessionId,
	  const std::string& hostName)
    : Impl(ioService, server, sessionId),
      socket_(ioService_, context),
      verifyEnabled_(verifyEnabled),
      hostName_(hostName)
  {
#ifndef OPENSSL_NO_TLSEXT
    if (!SSL_set_tlsext_host_name(socket_.native_handle(), hostName.c_str())) {
      LOG_ERROR("could not set tlsext host.");
    }
#endif
  }

protected:
  virtual tcp::socket& socket()
  {
    return socket_.next_layer();
  }

  virtual void asyncConnect(tcp::endpoint& endpoint,
			    const ConnectHandler& handler)
  {
    socket_.lowest_layer().async_connect(endpoint, handler);
  }

  virtual void asyncHandshake(const ConnectHandler& handler)
  {
#ifdef VERIFY_CERTIFICATE
    if (verifyEnabled_) {
      socket_.set_verify_mode(boost::asio::ssl::verify_peer);
      LOG_DEBUG("verifying that peer is " << hostName_);
      socket_.set_verify_callback
	(boost::asio::ssl::rfc2818_verification(hostName_));
    }
#endif // VERIFY_CERTIFICATE

    socket_.async_handshake(boost::asio::ssl::stream_base::client, handler);
  }

  virtual void asyncWriteRequest(const IOHandler& handler)
  {
    boost::asio::async_write(socket_, requestBuf_, handler);
  }

  virtual void asyncReadUntil(const std::string& s,
			      const IOHandler& handler)
  {
    boost::asio::async_read_until(socket_, responseBuf_, s, handler);
  }

  virtual void asyncRead(const IOHandler& handler)
  {
    boost::asio::async_read(socket_, responseBuf_,
			    boost::asio::transfer_at_least(1), handler);
  }

private:
  typedef boost::asio::ssl::stream<tcp::socket> ssl_socket;

  ssl_socket socket_;
  bool verifyEnabled_;
  std::string hostName_;
};
#endif // WT_WITH_SSL

Client::Client(WObject *parent)
  : WObject(parent),
    ioService_(0),
    timeout_(10),
    maximumResponseSize_(64*1024),
#ifdef VERIFY_CERTIFICATE
    verifyEnabled_(true),
#else
    verifyEnabled_(false),
#endif
    followRedirect_(false),
    redirectCount_(0),
    maxRedirects_(20)
{ }

Client::Client(WIOService& ioService, WObject *parent)
  : WObject(parent),
    ioService_(&ioService),
    timeout_(10),
    maximumResponseSize_(64*1024),
#ifdef VERIFY_CERTIFICATE
    verifyEnabled_(true),
#else
    verifyEnabled_(false),
#endif
    followRedirect_(false),
    redirectCount_(0),
    maxRedirects_(20)
{ }

Client::~Client()
{
  abort();
}

void Client::setSslCertificateVerificationEnabled(bool enabled)
{
  verifyEnabled_ = enabled;
}

void Client::abort()
{
  boost::shared_ptr<Impl> impl = impl_;
  if (impl) {
    if (impl->hasServer()) {
      // handling of redirect happens in the WApplication
      impl->asyncStop(0);
      impl_.reset();
    } else {
      // handling of redirect happens in the strand of impl
      impl->asyncStop(&impl_);
    }
  }
}

void Client::setTimeout(int seconds)
{
  timeout_ = seconds;
}

void Client::setMaximumResponseSize(std::size_t bytes)
{
  maximumResponseSize_ = bytes;
}

void Client::setSslVerifyFile(const std::string& file)
{
  verifyFile_ = file;
}

void Client::setSslVerifyPath(const std::string& path)
{
  verifyPath_ = path;
}

bool Client::get(const std::string& url)
{
  return request(Get, url, Message());
}

bool Client::get(const std::string& url, 
		 const std::vector<Message::Header> headers)
{
  Message m(headers);
  return request(Get, url, m);
}

bool Client::post(const std::string& url, const Message& message)
{
  return request(Post, url, message);
}

bool Client::put(const std::string& url, const Message& message)
{
  return request(Put, url, message);
}

bool Client::deleteRequest(const std::string& url, const Message& message)
{
  return request(Delete, url, message);
}

bool Client::request(Http::Method method, const std::string& url,
		     const Message& message)
{
  std::string sessionId;

  WIOService *ioService = ioService_;
  WServer *server = 0;

  WApplication *app = WApplication::instance();

  if(impl_.get()) {
    LOG_ERROR("another request is in progress");
    return false;
  }

  if (app) {
    sessionId = app->sessionId();
    server = app->environment().server();
    ioService = &server->ioService();
  } else if (!ioService) {
    server = WServer::instance();

    if (server)
      ioService = &server->ioService();
    else {
      LOG_ERROR("requires a WIOService for async I/O");
      return false;
    }

    server = 0;
  }

  URL parsedUrl;

  if (!parseUrl(url, parsedUrl))
    return false;

  if (parsedUrl.protocol == "http") {
    impl_.reset(new TcpImpl(*ioService, server, sessionId));

#ifdef WT_WITH_SSL
  } else if (parsedUrl.protocol == "https") {
    boost::asio::ssl::context context
      (*ioService, boost::asio::ssl::context::tlsv1);

#ifdef VERIFY_CERTIFICATE
    if (verifyEnabled_)
      context.set_default_verify_paths();

    if (!verifyFile_.empty() || !verifyPath_.empty()) {
      if (!verifyFile_.empty())
	context.load_verify_file(verifyFile_);
      if (!verifyPath_.empty())
	context.add_verify_path(verifyPath_);
    }
#endif // VERIFY_CERTIFICATE

    impl_.reset(new SslImpl(*ioService, verifyEnabled_,
			    server, 
			    context, 
			    sessionId, 
			    parsedUrl.host));
#endif // WT_WITH_SSL

  } else {
    LOG_ERROR("unsupported protocol: " << parsedUrl.protocol);
    return false;
  }

  if (followRedirect()) {
    impl_->done().connect(boost::bind(&Client::handleRedirect,
				      this, method, _1, _2, message));
  } else {
    impl_->done().connect(this, &Client::emitDone);
  }

  if (headersReceived_.isConnected())
    impl_->headersReceived().connect(this, &Client::emitHeadersReceived);

  if (bodyDataReceived_.isConnected())
    impl_->bodyDataReceived().connect(this, &Client::emitBodyReceived);

  impl_->setTimeout(timeout_);
  impl_->setMaximumResponseSize(maximumResponseSize_);

  const char *methodNames_[] = { "GET", "POST", "PUT", "DELETE" };

  LOG_DEBUG(methodNames_[method] << " " << url);

  impl_->request(methodNames_[method], 
		 parsedUrl.auth,
		 parsedUrl.host, 
		 parsedUrl.port, 
		 parsedUrl.path, 
		 message);

  return true;
}

bool Client::followRedirect() const
{
  return followRedirect_;
}

void Client::setFollowRedirect(bool followRedirect)
{
  followRedirect_ = followRedirect;
}

int Client::maxRedirects() const
{
  return maxRedirects_;
}

void Client::setMaxRedirects(int maxRedirects)
{
  maxRedirects_ = maxRedirects;
}

void Client::handleRedirect(Http::Method method, boost::system::error_code err, const Message& response, const Message& request)
{
  if (!impl_) {
    emitDone(err, response);
    return;
  }
  impl_.reset();
  int status = response.status();
  if (!err && (((status == 301 || status == 302 || status == 307) && method == Get) || status == 303)) {
    const std::string *newUrl = response.getHeader("Location");
    ++ redirectCount_;
    if (newUrl) {
      if (redirectCount_ <= maxRedirects_) {
	get(*newUrl, request.headers());
	return;
      } else {
	LOG_WARN("Redirect count of " << maxRedirects_ << " exceeded! Redirect URL: " << *newUrl);
      }
    }
  }
  emitDone(err, response);
}

void Client::emitDone(boost::system::error_code err, const Message& response)
{
  impl_.reset();
  redirectCount_ = 0;
  done_.emit(err, response);
}

void Client::emitHeadersReceived(const Message& response)
{
  headersReceived_.emit(response);
}

void Client::emitBodyReceived(const std::string& data)
{
  bodyDataReceived_.emit(data);
}

bool Client::parseUrl(const std::string &url, URL &parsedUrl)
{
  std::size_t i = url.find("://");
  if (i == std::string::npos) {
    LOG_ERROR("ill-formed URL: " << url);
    return false;
  }

  parsedUrl.protocol = url.substr(0, i);
  std::string rest = url.substr(i + 3);
  // find auth
  std::size_t l = rest.find('@');
  if (l != std::string::npos) {
    parsedUrl.auth = rest.substr(0, l);
    parsedUrl.auth = Wt::Utils::urlDecode(parsedUrl.auth);
    rest = rest.substr(l+1);
  }

  // find host
  std::size_t j = rest.find('/');

  if (j == std::string::npos) {
    parsedUrl.host = rest;
    parsedUrl.path = "/";
  } else {
    parsedUrl.host = rest.substr(0, j);
    parsedUrl.path = rest.substr(j);
  }

  std::size_t k = parsedUrl.host.find(':');
  if (k != std::string::npos) {
    try {
      parsedUrl.port = boost::lexical_cast<int>(parsedUrl.host.substr(k + 1));
    } catch (boost::bad_lexical_cast& e) {
      LOG_ERROR("invalid port: " << parsedUrl.host.substr(k + 1));
      return false;
    }
    parsedUrl.host = parsedUrl.host.substr(0, k);
  } else {
    if (parsedUrl.protocol == "http")
      parsedUrl.port = 80;
    else if (parsedUrl.protocol == "https")
      parsedUrl.port = 443;
    else
      parsedUrl.port = 80; // protocol will not be handled anyway
  }

  return true;
}

  }
}
