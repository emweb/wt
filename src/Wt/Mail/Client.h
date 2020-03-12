// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_MAIL_CLIENT_H_
#define WT_MAIL_CLIENT_H_

#include <string>
#include <Wt/WDllDefs.h>

#include <memory>

namespace Wt {

  /*! \brief Namespace for the \ref mail
   */
  namespace Mail {

class Message;

/*! \defgroup mail SMTP client library (Wt::Mail)
 *  \brief An SMTP client implementation.
 */

/*! \enum AuthenticationMethod
 *  \brief Indicates the authentication method to use
 */
enum class AuthenticationMethod {
  None, //!< No authentication
  Plain, //!< PLAIN authentication
  Login //!< LOGIN authentication
};

/*! \enum TransportEncryption
 *  \brief Indicates the transport encryption method to use
 */
enum class TransportEncryption {
  None, //!< No encryption
  StartTLS, //!< Use STARTTLS (fail if not accepted by server)
  TLS //!< Use TLS directly, don't use STARTTLS
};

/*! \class Client Wt/Mail/Client.h Wt/Mail/Client.h
 *  \brief An SMTP mail client.
 *
 * The SMTP client can send one or more mail messages to an SMTP host.
 *
 * \code
 * #include <Wt/Mail/Client>
 * #include <Wt/Mail/Message>
 *
 * Mail::Message message;
 * message.setFrom(Mail::Mailbox("kudos@corp.org", "Kudos Dukos");
 * message.addRecipient(Mail::RecipientType::To, Mail::Mailbox("koen@emweb.be", "Koen Deforche");
 * message.setSubject("Hey there, koen!");
 * message.setBody("That mail client seems to be working.");
 * message.addHtmlBody ("<p>"
 *      "<a href=\"http://www.webtoolkit.eu/wt\">That mail client</a>"
 *      " seems to be working just great!</p>");
 *
 * Mail::Client client;
 * client.connect("localhost");
 * client.send(message);
 * \endcode
 *
 * Only the bare essentials of the SMTP protocol are current implemented,
 * although the Message itself supports proper unicode handling.
 *
 * \note Currently only a plain-text SMTP protocol is supported. SSL
 *       transport will be added in the future.
 *
 * \note Currently the client sends an email synchronously, and thus a slow
 *       connection to the SMTP server may block the current thread. We plan
 *       to change this in the future. It is thus beneficial to connect to a
 *       local SMTP daemon to minimize this deficiency.
 *
 * \ingroup mail
 */
class WT_API Client
{
public:
  /*! \brief Constructor.
   *
   * The \p selfHost is how the mail client will identify itself to the
   * mail server, in the EHLO command.
   *
   * If not defined, the "smtp-self-host" configuration property is used, and
   * if that property is not defined, it defaults to "localhost".
   */
  Client(const std::string& selfHost = std::string());

  /*! \brief Destructor.
   *
   * If the client is still connected, this disconnects the client.
   */
  ~Client();

  /*! \brief Enables authentication
   *
   * Enable authentication either through this method, or by specifying
   * the following configuration properties:
   *
   *  - smtp-auth-username
   *  - smtp-auth-password
   *  - smtp-auth-method
   *
   * "smtp-auth-method" can be one of: "none", "plain", or "login".
   */
  void enableAuthentication(const std::string &username,
                            const std::string &password,
                            AuthenticationMethod method = AuthenticationMethod::Plain);

  /*! \brief Returns the authentication method
   *
   * \sa enableAuthentication
   */
  AuthenticationMethod authenticationMethod() const { return configuration_.authenticationMethod_; }

  /*! \brief Returns whether authentication is enabled
   *
   * \sa enableAuthentication
   */
  bool authenticationEnabled() const { return configuration_.authenticationMethod_ != AuthenticationMethod::None; }

  /*! \brief Sets whether SSL certificate verification is enabled
   *
   * Defaults to true
   */
  void setSslCertificateVerificationEnabled(bool enabled);

  /*! \brief Gets whether SSL certificate verification is enabled
   *
   * \sa setSslCertificateVerificationEnabled
   */
  bool isSslCertificateVerificationEnabled() const { return configuration_.certificateVerificationEnabled_; }

  /*! \brief Sets the transport encryption method
   *
   * The default is TransportEncryption::None
   *
   * Enable transport encryption either through this method, or by specifying the
   * "smtp-transport-encryption" configuration property. Its value can be one of:
   * "none", "starttls", or "tls".
   */
  void setTransportEncryption(TransportEncryption method);

  /*! \brief Get the encryption method
   *
   * \sa setTransportEncryption
   */
  TransportEncryption transportEncryption() const { return configuration_.transportEncryption_; }

  /*! \brief Connects to the default SMTP server.
   *
   * This connects to the SMTP server defined by the "smtp-host"
   * property, on port defined by the "smtp-port" property. If these
   * properties are not set, "localhost" and 25 are used as defaults
   * respectively.
   *
   * Returns whether the connection could be established and the SMTP
   * hand-shake was successful.
   */
  bool connect();

  /*! \brief Connects to a given STMP server and port.
   *
   * Returns whether the connection could be established and the SMTP
   * hand-shake was successful.
   */
  bool connect(const std::string& smtpHost, int smtpPort = 25);

  /*! \brief Disconnects the client from the SMTP server.
   */
  void disconnect();

  /*! \brief Sends a message.
   *
   * The client must be connected before messages can be sent.
   *
   * The function returns true on success, false on error. The
   * reason for the error is logged in the log file.
   */
  bool send(const Message& message);

private:
  Client(const Client&);
  class BaseImpl;
  template<bool ssl>
  class Impl;

  std::unique_ptr<BaseImpl> impl_;

  struct Configuration {
    std::string selfHost_;
    std::string username_;
    std::string password_;
    AuthenticationMethod authenticationMethod_;
    TransportEncryption transportEncryption_;
    bool certificateVerificationEnabled_;
  };

  Configuration configuration_;
};

  }
}

#endif // WT_EMAIL_CLIENT_H_
