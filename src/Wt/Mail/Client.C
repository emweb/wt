/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

// bugfix for https://svn.boost.org/trac/boost/ticket/5722
#include <boost/asio.hpp>

#include "Client"
#include "Message"
#include "Wt/WApplication"
#include "Wt/WException"

#include <boost/lexical_cast.hpp>

namespace Wt {

LOGGER("Mail.Client");

namespace {
  bool logged = false;
}
		
  namespace Mail {

using boost::asio::ip::tcp;

class Client::Impl
{
public:
  enum ReplyCode {
    Ready = 220,
    Bye = 221,
    Ok = 250,
    StartMailInput = 354
  };

  Impl(const std::string& host, const std::string& selfFQDN, int port)
    : socket_(io_service_)
  {
    // Get a list of endpoints corresponding to the server name.
    tcp::resolver resolver(io_service_);

    tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    // Try each endpoint until we successfully establish a connection.
    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endpoint_iterator != end) {
      socket_.close();
      socket_.connect(*endpoint_iterator++, error);
    }

    if (error) {
      socket_.close();
      LOG_ERROR("could not connect to: " << host << ":" << port);
      return;
    }

    try {
      failIfReplyCodeNot(Ready);

      send("EHLO " + selfFQDN + "\r\n");

      failIfReplyCodeNot(Ok);
    } catch (std::exception& e) {
      socket_.close();
      LOG_ERROR(e.what());
      return;
    }
  }

  bool good() const {
    return socket_.is_open();
  }

  ~Impl()
  {
    if (good()) {
      try {
	send("QUIT\r\n");
	failIfReplyCodeNot(Bye);
      } catch (std::exception& e) {
	socket_.close();
	LOG_ERROR(e.what());
	return;
      }
    }
  }

  bool send(const Message& message)
  {
    try {
      send("MAIL FROM:<" + message.from().address() + ">\r\n");
      failIfReplyCodeNot(Ok);
      for (unsigned i = 0; i < message.recipients().size(); ++i) {
	const Mailbox& m = message.recipients()[i].mailbox;

	send("RCPT TO:<" + m.address() + ">\r\n");
	failIfReplyCodeNot(Ok);
      }

      send("DATA\r\n");
      failIfReplyCodeNot(StartMailInput);

      boost::asio::streambuf buf;
      std::ostream data(&buf);
      message.write(data);
      data << ".\r\n";

      boost::asio::write(socket_, buf);

      failIfReplyCodeNot(Ok);
    } catch (std::exception& e) {
      socket_.close();
      LOG_ERROR(e.what());
      return false;
    }
    return true;
  }

private:
  void send(const std::string& s)
  {
    LOG_DEBUG("C " << s);
    // boost::asio::const_buffer request = boost::asio::buffer(s);
    boost::asio::write(socket_, boost::asio::buffer(s));

    // FIXME error handling ?
  }

  void failIfReplyCodeNot(ReplyCode expected)
  {
    int r = readResponse();

    if (r != expected)
      throw WException("Unexpected response "
		       + boost::lexical_cast<std::string>(r));
  }

  int readResponse() {
    int replyCode = -1;

    boost::asio::streambuf response;
    for (;;) {
      boost::asio::read_until(socket_, response, "\r\n");

      std::istream in(&response);
      int code;
      in >> code;

      if (!in)
	throw WException("Invalid response");

      std::string msg;
      std::getline(in, msg);

      LOG_DEBUG("S " << code << msg);

      if (replyCode == -1)
	replyCode = code;
      else if (code != replyCode)
	throw WException("Inconsistent multi-line response");

      if (msg.length() > 0 && msg[0] == '-')
	continue;
      else
	return replyCode;
    }

    return -1; // Not reachable
  }

private:
  boost::asio::io_service io_service_;
  tcp::socket socket_;
};

Client::Client(const std::string& selfHost)
  : impl_(0),
    selfHost_(selfHost)
{ 
  if (selfHost_.empty()) {
    selfHost_ = "localhost";
    WApplication::readConfigurationProperty("smtp-self-host", selfHost_);

    if (!logged)
      LOG_INFO("Smtp::Client: using '" << selfHost_ << "' (from smtp-self-host property) "
	       "as self host");
  } else
    if (!logged)
      LOG_INFO("Smtp::Client: using '" << selfHost_ << "' as self host");
}

Client::~Client()
{
  disconnect();
}

bool Client::connect()
{
  std::string smtpHost = "localhost";
  std::string smtpPortStr = "25";
  
  WApplication::readConfigurationProperty("smtp-host", smtpHost);
  WApplication::readConfigurationProperty("smtp-port", smtpPortStr);

  int smtpPort = boost::lexical_cast<int>(smtpPortStr);

  if (!logged)
    LOG_INFO("Smtp::Client: using '" << smtpHost << ":" << smtpPortStr
	     << "' (from smtp-host and smtp-port properties) as SMTP host");

  return connect(smtpHost, smtpPort);
}

bool Client::connect(const std::string& smtpHost, int smtpPort)
{
  if (!logged) {
    LOG_INFO("Smtp::Client: connecting to '" << smtpHost << ":" << smtpPort);
    logged = true;
  }

  delete impl_;
  impl_ = new Impl(smtpHost, selfHost_, smtpPort);

  return impl_->good();
}

void Client::disconnect()
{
  delete impl_;
  impl_ = 0;
}

bool Client::send(const Message& message)
{
  return impl_->send(message);
}

  }
}
