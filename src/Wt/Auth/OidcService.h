// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef OIDC_SERVICE_H_
#define OIDC_SERVICE_H_

#include <Wt/Auth/OAuthService.h>
#include <Wt/Http/Client.h>

namespace Wt {
  namespace Auth {

class OidcService;

/*! \class OidcProcess Auth/OidcService
 *  \brief An OpenId Connect authentication process.
 *
 * The process implements the state machine that is needed to complete
 * an OpenID Connect authentication cycle.
 *
 * A process is created for a particular scope, which represents
 * what kind of information one wants to access, and which is used
 * to inform the user of the kind of operations he needs to
 * authorize for your application to make with his protected data.
 *
 * \sa OidcService::createProcess()
 * \sa OAuthProcess
 *
 * \ingroup auth
 */
class WT_API OidcProcess : public OAuthProcess
{
public:
  OidcProcess(const OidcService& service, const std::string& scope);

  /*! \brief Starts an authorization and authentication process.
   *
   * This is startAuthorize() followed by getIdentity().
   *
   * The authentication process ends with the authenticated() signal which
   * signals the obtained identity.
   *
   * \note To be able to use a popup (instead of a page redirect), you
   *       should connect this method directly to an, since popup
   *       windows are blocked in most web browsers unless they are
   *       the direct consequence of an event.
   */
  virtual void startAuthenticate() override { OAuthProcess::startAuthenticate(); }
  // TODO fix

  /*! \brief Obtains an authenticated identity.
   *
   * The authentication process will either use the ID token included
   * with the access token or, when this is not available, request the
   * identity at the user info endpoint using claims.
   *
   * The authentication process ends with the authenticated() signal
   * which signals the obtained identity.
   */
  virtual void getIdentity(const OAuthAccessToken& token) override;

private:
  void handleResponse(AsioWrapper::error_code err, const Http::Message& response);
  Identity parseIdToken(const std::string& idToken);
  Identity parseClaims(const Json::Object& claims);

  std::unique_ptr<Http::Client> httpClient_;
};

/*! \class OidcService Wt/Auth/OidcService.h
 *  \brief An OpenId Connect authentication service provider.
 *
 * This class implements an OpenID Connect client (<a
 * href="http://openid.net/specs/openid-connect-core-1_0.html">core
 * specification</a>), which can be used to allow the user to be
 * safely authenticated with your web application without needing to
 * store or even handle his authorization credentials (such as a
 * password).
 *
 * OpenID Connect is a simple identity layer on top of the OAuth 2.0
 * protocol. It enables Clients to verify the identity of the End-User
 * based on the authentication performed by an Authorization Server,
 * as well as to obtain basic profile information about the End-User
 * in an interoperable and REST-like manner.
 *
 * This implementation only supports authentication using the
 * Authorization Code Flow.
 *
 * The configuration of this service is done by using the setters the
 * service class exposes. Before the authentication process can be
 * started these settings must be configured first and may not be
 * changed afterwards. \if cpp A "const OidcService" object is
 * thread-safe. \endif
 *
 * <p>
 * The OpenID Connect protocol, including the subsequent use for
 * authentication, consists of a number of consecutive steps, some of
 * which require user interaction, and some which require the use of
 * remote web services. The state machine for this process is
 * implemented in an OidcProcess. To use OpenID Connect, you need to
 * create such a process and listen for state changes.
 * </p>
 *
 * \ingroup auth
 */
class WT_API OidcService : public OAuthService
{
public:
  /*! \brief Constructor.
   */
  OidcService(const AuthService& baseAuth);

  /*! \brief Returns the provider name.
   *
   * This is a short identifier.
   *
   * \sa Identify::provider()
   * \sa description()
   * \sa setName()
   */
  virtual std::string name() const override { return name_; };

  /*! \brief Returns the provider description.
   *
   * This returns a description useful for e.g. tool tips on a login
   * icon.
   *
   * \sa name()
   * \sa setDescription()
   */
  virtual WString description() const override { return description_; };

  /*! \brief Returns the desired width for the popup window.
   *
   * Defaults to 670 pixels.
   *
   * \sa setPopupWidth()
   */
  virtual int popupWidth() const override { return popupWidth_; };

  /*! \brief Returns the desired height of the popup window.
   *
   * Defaults to 400 pixels.
   *
   * \sa setPopupHeight()
   */
  virtual int popupHeight() const override { return popupHeight_; };

  /*! \brief Returns the scope needed for authentication.
   *
   * This returns the scope that is needed (and sufficient) for
   * obtaining identity information, and thus to authenticate the
   * user.
   *
   * This defaults to "openid".
   *
   * \sa OidcProcess::startAuthenticate()
   * \sa OidcService::createProcess()
   * \sa OidcService::setAuthenticationScope()
   */
  virtual std::string authenticationScope() const override { return scope_; };

  /*! \brief Returns the redirection endpoint URL.
   *
   * This is the local URL to which the browser is redirect from the
   * service provider, after the authorization process. You need to
   * configure this URL with the third party authentication service.
   *
   * A static resource will be deployed at this URL.
   *
   * \sa setRedirectEndpoint()
   * \if cpp
   * \sa WServer::addResource()
   * \elseif java
   * @see WtServlet#addResource(WResource r, String path)
   * \endif
   */
  virtual std::string redirectEndpoint() const override { return redirectEndpoint_; };

  /*! \brief Returns the authorization endpoint URL.
   *
   * This is a remote URL which hosts the OpenID Connect authorization
   * user interface. This URL is loaded in the popup window at the
   * start of an authorization process.
   *
   * \sa setAuthEndpoint()
   */
  virtual std::string authorizationEndpoint() const override { return authorizationEndpoint_; };

  /*! \brief Returns the token endpoint URL.
   *
   * This is a remote URL which hosts a web-service that generates
   * access and id tokens.
   *
   * \sa setTokenEndpoint()
   */
  virtual std::string tokenEndpoint() const override { return tokenEndpoint_; };

  /*! \brief Returns the user info endpoint URL.
   *
   * This is a remote URL which hosts a web-service that provides the claims
   * that are associated with the requested scope.
   *
   * \sa setTokenEndpoint()
   */
  virtual std::string userInfoEndpoint() const override { return userInfoEndpoint_; };

  /*! \brief Returns the client ID.
   *
   * This is the identification for this web application with the
   * OpenID Connect provider.
   *
   * \sa setClientId()
   */
  virtual std::string clientId() const override { return clientId_; };

  /*! \brief Returns the client secret.
   *
   * This is the secret credentials for this web application with the
   * OpenID Connect provider.
   *
   * \sa setClientSecret()
   */
  virtual std::string clientSecret() const override { return clientSecret_; };

  /*! \brief Returns the method to transfer the client secret.
  *
  * The default implementation returns HttpAuthorizationBasic
  * (the recommended method).
  */
  virtual ClientSecretMethod clientSecretMethod() const override { return method_; };

  /*! \brief Creates a new authentication process.
   *
   * This creates a new authentication process for the indicated
   * scope.  Valid names for the scope are service provider dependent.
   *
   * The service needs to be correctly configured before being able to
   * call this function. configure() needs to be called
   * first to check if the configuration is valid.
   *
   * \note The returned process will be an instance of OidcService
   *
   * \sa configure()
   * \sa authenticationScope()
   */
  virtual std::unique_ptr<OAuthProcess> createProcess(const std::string& scope) const override;

  /*! \brief Sets the redirection endpoint URL.
   *
   * \sa redirectEndoint()
   */
  void setRedirectEndpoint(const std::string& url);

  /*! \brief Sets the client ID.
   *
   * This setting is required.
   *
   * \sa clientId()
   */
  void setClientId(const std::string& id);

  /*! \brief Sets the client secret.
   *
   * This setting is required.
   *
   * \sa setClientSecret()
   */
  void setClientSecret(const std::string& secret);

  /*! \brief Sets the authorization endpoint URL.
   *
   * This setting is required.
   *
   * \sa authorizationEndpoint()
   */
  void setAuthEndpoint(const std::string& url);

  /*! \brief Sets the token endpoint URL.
   *
   * This setting is required.
   *
   * \sa tokenEndpoint()
   */
  void setTokenEndpoint(const std::string& url);

  /*! \brief Sets the user info endpoint URL.
   *
   * This setting is required.
   *
   * \sa userInfoEndpoint()
   */
  void setUserInfoEndpoint(const std::string& url);

  /*! \brief Sets the scope needed for authentication.
   *
   * This setting is required.
   *
   * \sa authenticationScope()
   */
  void setAuthenticationScope(const std::string& scope);

  /*! \brief Sets the provider name.
   *
   * \sa name()
   */
  void setName(const std::string& name);

  /*! \brief Sets the provider description.
   *
   * This setting is required.
   *
   * \sa name()
   */
  void setDescription(const std::string& description);

  /*! \brief Sets the method to transfer the client secret.
   *
   * \sa name()
   */
  void setClientSecretMethod(ClientSecretMethod method);

  /*! \brief Sets the desired width for the popup window
   *
   * \sa popupWidth()
   */
  void setPopupWidth(int width);

  /*! \brief Sets the desired height for the popup window
   *
   * \sa popupHeight()
   */
  void setPopupHeight(int height);

private:
  bool configure();
  std::string redirectEndpoint_;
  std::string authorizationEndpoint_;
  std::string tokenEndpoint_;
  std::string userInfoEndpoint_;
  std::string clientId_;
  std::string clientSecret_;
  std::string name_;
  std::string description_;
  std::string scope_;
  int popupWidth_;
  int popupHeight_;
  ClientSecretMethod method_;
  bool configured_;
};
  }
}

#endif // OIDC_SERVICE_H_
