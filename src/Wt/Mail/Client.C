/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

// bugfix for https://svn.boost.org/trac/boost/ticket/5722
#include <Wt/AsioWrapper/asio.hpp>
#include <Wt/AsioWrapper/system_error.hpp>

#ifdef WT_WITH_SSL
#include <Wt/AsioWrapper/ssl.hpp>

#include "web/SslUtils.h"
#endif // WT_WITH_SSL

#include "Client.h"
#include "Message.h"
#include "Wt/WApplication.h"
#include "Wt/WException.h"
#include "Wt/Utils.h"

#include "WebUtils.h"

#include <boost/algorithm/string/predicate.hpp>

namespace Wt {

LOGGER("Mail.Client");

namespace {
  bool logged = false;
}

namespace asio = AsioWrapper::asio;
		
  namespace Mail {

using asio::ip::tcp;
#ifdef WT_WITH_SSL
namespace ssl = asio::ssl;
#endif // WT_WITH_SSL

class Client::BaseImpl
{
public:
  enum class ReplyCode : int {
    Ready = 220,
    Bye = 221,
    AuthSuccessful = 235,
    Ok = 250,
    AuthContinue = 334,
    StartMailInput = 354
  };

  BaseImpl()
  { }

  virtual ~BaseImpl()
  { }

  virtual bool good() const = 0;
  virtual bool send(const Message &message) = 0;
};

namespace {

template<bool ssl>
struct SocketData;

template<>
struct SocketData<false> {
  SocketData(asio::io_service &io_service, bool verifyCerts)
    : socket_(io_service)
  {
    (void)verifyCerts;
  }

  tcp::socket socket_;
};
#ifdef WT_WITH_SSL
template<>
struct SocketData<true> {
  SocketData(asio::io_service &io_service, bool verifyCerts)
    : ssl_ctx_(Ssl::createSslContext(io_service, verifyCerts)),
      stream_(io_service, ssl_ctx_),
      encrypted_(false)
  { }

  ssl::context ssl_ctx_;
  ssl::stream<tcp::socket> stream_;
  bool encrypted_;
};
#endif // WT_WITH_SSL

}

template<bool ssl>
class Client::Impl : public Client::BaseImpl {
public:
  Impl(const Client::Configuration &config,
       const std::string& host,
       int port)
    : data_(io_service_, config.certificateVerificationEnabled_)
  {
    // Get a list of endpoints corresponding to the server name.
    tcp::resolver resolver(io_service_);

    tcp::resolver::query query(host, std::to_string(port));
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    tcp::resolver::iterator end;

    // Try each endpoint until we successfully establish a connection.
    AsioWrapper::error_code error = asio::error::host_not_found;
    while (error && endpoint_iterator != end) {
      close();
      socket().connect(*endpoint_iterator++, error);
    }

    if (error) {
      close();
      LOG_ERROR("could not connect to: " << host << ":" << port);
      return;
    }

    try {
      doInitTLS(config, host);

      failIfReplyCodeNot(ReplyCode::Ready);

      send("EHLO " + config.selfHost_ + "\r\n");

      failIfReplyCodeNot(ReplyCode::Ok);

      doStartTLS(config, host);

      doAuth(config);
    } catch (std::exception& e) {
      close();
      LOG_ERROR(e.what());
      return;
    }
  }

  virtual ~Impl() override
  {
    if (good()) {
      try {
        send("QUIT\r\n");
        failIfReplyCodeNot(ReplyCode::Bye);
        close();
      } catch (std::exception& e) {
        close();
        LOG_ERROR(e.what());
        return;
      }
    }
  }

  virtual bool good() const override
  {
    return socket().is_open();
  }

  virtual bool send(const Message& message) override
  {
    try {
      send("MAIL FROM:<" + message.from().address() + ">\r\n");
      failIfReplyCodeNot(ReplyCode::Ok);
      for (unsigned i = 0; i < message.recipients().size(); ++i) {
	const Mailbox& m = message.recipients()[i].mailbox;

	send("RCPT TO:<" + m.address() + ">\r\n");
	failIfReplyCodeNot(ReplyCode::Ok);
      }

      send("DATA\r\n");
      failIfReplyCodeNot(ReplyCode::StartMailInput);

      asio::streambuf buf;
      std::ostream data(&buf);
      message.write(data);
      data << ".\r\n";

      write(buf);

      failIfReplyCodeNot(ReplyCode::Ok);
    } catch (std::exception& e) {
      close();
      LOG_ERROR(e.what());
      return false;
    }
    return true;
  }

private:
  void doAuth(const Client::Configuration &config)
  {
    if (config.username_.empty() ||
        config.password_.empty())
      return;

    if (config.authenticationMethod_ == AuthenticationMethod::Plain)
      doPlainAuth(config);
    else if (config.authenticationMethod_ == AuthenticationMethod::Login)
      doLoginAuth(config);
    else
      assert(false);
  }

  void doPlainAuth(const Client::Configuration &config)
  {
    assert(config.authenticationMethod_ == AuthenticationMethod::Plain);

    const std::string authStr = '\0' + config.username_ + '\0' + config.password_;
    const std::string authStrBase64 = Utils::base64Encode(authStr, false) + "\r\n";

    send("AUTH PLAIN\r\n");

    failIfReplyCodeNot(ReplyCode::AuthContinue);

    send(authStrBase64);

    failIfReplyCodeNot(ReplyCode::AuthSuccessful);
  }

  void doLoginAuth(const Client::Configuration &config)
  {
    assert(config.authenticationMethod_ == AuthenticationMethod::Login);

    const std::string usernameBase64 = Utils::base64Encode(config.username_, false) + "\r\n";
    const std::string passwordBase64 = Utils::base64Encode(config.password_, false) + "\r\n";

    send("AUTH LOGIN\r\n");

    failIfReplyCodeNot(ReplyCode::AuthContinue);

    send(usernameBase64);

    failIfReplyCodeNot(ReplyCode::AuthContinue);

    send(passwordBase64);

    failIfReplyCodeNot(ReplyCode::AuthSuccessful);
  }

  void send(const std::string& s)
  {
    LOG_DEBUG("C " << s);
    // asio::const_buffer request = asio::buffer(s);
    write(asio::buffer(s));

    // FIXME error handling ?
  }

  void failIfReplyCodeNot(ReplyCode expected)
  {
    int r = readResponse();

    if (r != static_cast<int>(expected))
      throw WException("Unexpected response " + std::to_string(r));
  }

  int readResponse() {
    int replyCode = -1;

    asio::streambuf response;
    for (;;) {
      read_until(response, "\r\n");

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

  template<typename Buffer>
  void read_until(Buffer &&buf, const char *str);
  template<typename Buffer>
  void write(Buffer &&buf);
  void close();
  void handshake(const Client::Configuration &config,
                 const std::string &hostName);
  void doInitTLS(const Client::Configuration &config,
                 const std::string &hostName);
  void doStartTLS(const Client::Configuration &config,
                  const std::string &hostName);

  tcp::socket & socket();
  const tcp::socket & socket() const;

private:
  asio::io_service io_service_;
  SocketData<ssl> data_;
};

template<>
template<typename Buffer>
void Client::Impl<false>::read_until(Buffer &&buf, const char *str)
{
  asio::read_until(data_.socket_, buf, str);
}

template<>
template<typename Buffer>
void Client::Impl<false>::write(Buffer &&buf)
{
  asio::write(data_.socket_, buf);
}

template<>
void Client::Impl<false>::close()
{
  if (!data_.socket_.is_open())
    return;

  AsioWrapper::error_code ignored;
  data_.socket_.shutdown(tcp::socket::shutdown_both, ignored);
  data_.socket_.close(ignored);
}

template<>
void Client::Impl<false>::doInitTLS(const Client::Configuration &config,
                                    const std::string &hostName)
{
  // no TLS, do nothing
  (void)config;
  (void)hostName;
}

template<>
void Client::Impl<false>::doStartTLS(const Client::Configuration &config,
                                     const std::string &hostName)
{
  // no TLS, do nothing
  (void)config;
  (void)hostName;
}

template<>
tcp::socket & Client::Impl<false>::socket()
{
  return data_.socket_;
}

template<>
const tcp::socket & Client::Impl<false>::socket() const
{
  return data_.socket_;
}

#ifdef WT_WITH_SSL
template<>
template<typename Buffer>
void Client::Impl<true>::read_until(Buffer &&buf, const char *str)
{
  if (data_.encrypted_) {
    asio::read_until(data_.stream_, buf, str);
  } else {
    asio::read_until(data_.stream_.next_layer(), buf, str);
  }
}

template<>
template<typename Buffer>
void Client::Impl<true>::write(Buffer &&buf)
{
  if (data_.encrypted_) {
    asio::write(data_.stream_, buf);
  } else {
    asio::write(data_.stream_.next_layer(), buf);
  }
}

template<>
void Client::Impl<true>::close()
{
  if (!data_.stream_.next_layer().is_open())
    return;

  AsioWrapper::error_code ignored;
  if (data_.encrypted_) {
    data_.stream_.shutdown(ignored);
  }
  data_.stream_.next_layer().shutdown(tcp::socket::shutdown_both, ignored);
  data_.stream_.next_layer().close(ignored);
}

template<>
void Client::Impl<true>::handshake(const Client::Configuration &config,
                                   const std::string &hostName)
{
  if (config.certificateVerificationEnabled_) {
    data_.stream_.set_verify_mode(ssl::verify_peer);
    LOG_DEBUG("verifying that peer is " << hostName);
    data_.stream_.set_verify_callback
        (ssl::rfc2818_verification(hostName));
  }
  data_.stream_.handshake(ssl::stream_base::client);
}

template<>
void Client::Impl<true>::doInitTLS(const Client::Configuration &config,
                                   const std::string &hostName)
{
  if (config.transportEncryption_ != TransportEncryption::TLS)
    return;

  data_.encrypted_ = true;

  handshake(config, hostName);
}

template<>
void Client::Impl<true>::doStartTLS(const Client::Configuration &config,
                                    const std::string &hostName)
{
  if (config.transportEncryption_ != TransportEncryption::StartTLS)
    return;

  send("STARTTLS\r\n");

  failIfReplyCodeNot(ReplyCode::Ready);

  data_.encrypted_ = true;

  handshake(config, hostName);

  send("EHLO " + config.selfHost_ + "\r\n");

  failIfReplyCodeNot(ReplyCode::Ok);
}

template<>
tcp::socket & Client::Impl<true>::socket()
{
  return data_.stream_.next_layer();
}

template<>
const tcp::socket & Client::Impl<true>::socket() const
{
  return data_.stream_.next_layer();
}
#endif // WT_WITH_SSL

Client::Client(const std::string& selfHost)
  : configuration_({
      selfHost,
      "",
      "",
      AuthenticationMethod::None,
      TransportEncryption::None,
      true
    })
{
  if (configuration_.selfHost_.empty()) {
    configuration_.selfHost_ = "localhost";
    WApplication::readConfigurationProperty("smtp-self-host", configuration_.selfHost_);

    if (!logged)
      LOG_INFO("using '" << configuration_.selfHost_ << "' (from smtp-self-host property) "
	       "as self host");
  } else
    if (!logged)
      LOG_INFO("using '" << configuration_.selfHost_ << "' as self host");

  {
    std::string authMethod = "none";
    if (WApplication::readConfigurationProperty("smtp-auth-method", authMethod)) {
      if (boost::iequals(authMethod, "plain")) {
        configuration_.authenticationMethod_ = AuthenticationMethod::Plain;
      } else if (boost::iequals(authMethod, "login")) {
        configuration_.authenticationMethod_ = AuthenticationMethod::Login;
      } else if (!boost::iequals(authMethod, "none")) {
        LOG_WARN("Unrecognized authentication method in 'smtp-auth-method' property: '" << authMethod << '\'');
      }
    }
  }

  if (configuration_.authenticationMethod_ != AuthenticationMethod::None) {
    const bool haveUsername = WApplication::readConfigurationProperty("smtp-auth-username", configuration_.username_);
    const bool havePassword = WApplication::readConfigurationProperty("smtp-auth-password", configuration_.password_);
    if (!haveUsername) {
      LOG_ERROR("Authentication enabled, but 'smtp-auth-username' property not configured, disabling authentication");
    }
    if (!havePassword) {
      LOG_ERROR("Authentication enabled, but 'smtp-auth-password' property not configured, disabling authentication");
    }
    if (!haveUsername ||
        !havePassword) {
      configuration_.authenticationMethod_ = AuthenticationMethod::None;
    }
  }

  {
    std::string encryptionMethod = "none";
    if (WApplication::readConfigurationProperty("smtp-transport-encryption", encryptionMethod)) {
      if (boost::iequals(encryptionMethod, "starttls")) {
        configuration_.transportEncryption_ = TransportEncryption::StartTLS;
      } else if (boost::iequals(encryptionMethod, "tls")) {
        configuration_.transportEncryption_ = TransportEncryption::TLS;
      } else if (!boost::iequals(encryptionMethod, "none")) {
        LOG_WARN("Unrecognized encryption method in 'smtp-transport-encryption' property: '" << encryptionMethod << '\'');
      }
    }
  }
}

Client::~Client()
{
  disconnect();
}

void Client::enableAuthentication(const std::string &username,
                                  const std::string &password,
                                  AuthenticationMethod method)
{
  configuration_.username_ = username;
  configuration_.password_ = password;
  configuration_.authenticationMethod_ = method;
}

void Client::setSslCertificateVerificationEnabled(bool enabled)
{
  configuration_.certificateVerificationEnabled_ = enabled;
}

void Client::setTransportEncryption(TransportEncryption method)
{
  configuration_.transportEncryption_ = method;
}

bool Client::connect()
{
  std::string smtpHost = "localhost";
  std::string smtpPortStr = "25";
  
  WApplication::readConfigurationProperty("smtp-host", smtpHost);
  WApplication::readConfigurationProperty("smtp-port", smtpPortStr);

  int smtpPort = Utils::stoi(smtpPortStr);

  if (!logged)
    LOG_INFO("using '" << smtpHost << ":" << smtpPortStr
	     << "' (from smtp-host and smtp-port properties) as SMTP host");

  return connect(smtpHost, smtpPort);
}

bool Client::connect(const std::string& smtpHost, int smtpPort)
{
  if (!logged) {
    LOG_INFO("connecting to '" << smtpHost << ':' << smtpPort << '\'');
    logged = true;
  }

  // Exception safety: make sure old impl is removed before we create a new one
  impl_ = nullptr;
  if (transportEncryption() == TransportEncryption::None) {
    impl_ = std::make_unique<Impl<false>>(configuration_, smtpHost, smtpPort);
  } else {
#ifdef WT_WITH_SSL
    impl_ = std::make_unique<Impl<true>>(configuration_, smtpHost, smtpPort);
#else // WT_WITH_SSL
    LOG_ERROR("TLS requested, but Wt built without OpenSSL");
    return false;
#endif // WT_WITH_SSL
  }

  return impl_->good();
}

void Client::disconnect()
{
  impl_ = nullptr;
}

bool Client::send(const Message& message)
{
  return impl_->send(message);
}

  }
}
