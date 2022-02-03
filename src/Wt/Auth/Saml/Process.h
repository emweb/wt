// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_SAML_PROCESS_H_
#define WT_AUTH_SAML_PROCESS_H_

#include <Wt/WJavaScript.h>
#include <Wt/WObject.h>
#include <Wt/WSignal.h>

#include <Wt/Auth/Saml/Assertion.h>

namespace Wt {
namespace Auth {
namespace Saml {

class ProcessImpl;

/*! \class Process Wt/Auth/Saml/Process.h
 *  \brief A %SAML authentication process.
 *
 * The process implements the state machine that is needed to complete
 * a %SAML authentication cycle.
 */
class WT_API Process : public WObject {
public:
  ~Process() override;

  /*! \brief Returns the %SAML service which spawned this process.
   *
   * \sa Service::createProcess()
   */
  const Service &service() const { return service_; }

  /*! \brief Error information, in case authentication or identification
   *         failed.
   *
   * The error message contains details when the authenticated() signal
   * indicates an invalid identity.
   */
  const WString &error() const { return error_; }

  /*! \brief Starts an authorization and authentication process.
   *
   * The authentication process ends with the authenticated() signal which
   * signals the obtained identity.
   *
   * \note To be able to use a popup (instead of a page redirect), you
   *       should connect this method directly to an event signal, since popup
   *       windows are blocked in most web browsers unless they are
   *       the direct consequence of an event.
   */
  void startAuthenticate();

#ifdef WT_TARGET_JAVA
    /*! \brief Connects an implementation to start an authentication process to a signal
   *
   * If JavaScript is available, this method connects a JavaScript function to
   * the \p signal, otherwise startAuthenticate() is connected to \p signal.
   */
  void connectStartAuthenticate(EventSignalBase &signal);
#endif

  /*! \brief Authentication signal.
   *
   * This signal indicates the end of an authentication process. If the
   * authentication process was successful, then the parameter is a
   * valid and authentic identity. If the authentication process
   * failed then the identity parameter is invalid, and you can get
   * more information using error().
   *
   * \sa startAuthenticate(), Service::getIdentity(), Identity::isValid()
   */
  Signal<Identity>& authenticated() { return authenticated_; }

protected:
  /*! \brief Constructor.
   *
   * \sa Service::createProcess()
   */
  explicit Process(const Service &service);

private:
  class AuthnRequestResource;
  class PrivateAcsResource;

  std::unique_ptr<ProcessImpl> impl_;
  const Service &service_;
  std::unique_ptr<AuthnRequestResource> authnRequestResource_;
  std::unique_ptr<PrivateAcsResource> privateAcsResource_;
  Signal<Identity> authenticated_;
  JSignal<> redirected_;

  std::string requestIdentifier_;

  Assertion assertion_;
  WString error_;
  std::string startInternalPath_;

  // onSamlDone() gets called from WApplication::unsuspended(), keeping
  // the connection object allows us to disconnect after it's done
  Wt::Signals::connection doneCallbackConnection_;

  // response is a response carrying a redirect to the SSO service with the AuthnRequest
  bool createAuthnRequest(Http::Response &response);
  // request is a request containing the SAMLResponse
  bool handleResponse(const Http::Request &request);

  void onSamlDone();

  std::string privateAcsResourceUrl() const;

  friend class ProcessImpl;
  friend class Service;
};

}
}
}

#endif // WT_AUTH_SAML_PROCESS_H_
