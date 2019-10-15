// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_OAUTH_H_
#define WT_AUTH_OAUTH_H_

#include <Wt/WObject.h>
#include <Wt/WJavaScript.h>
#include <Wt/WString.h>

#include <Wt/Auth/WAuthGlobal.h>
#include <Wt/Auth/Identity.h>
#include <Wt/Auth/User.h>
#include <Wt/Http/Message.h>
#include <Wt/Http/Method.h>

#include <Wt/AsioWrapper/system_error.hpp>

namespace Wt {
  namespace Auth {

class AuthService;
class OAuthService;
class OAuthRedirectEndpoint;

/*! \class OAuthAccessToken Wt/Auth/OAuthService.h
 *  \brief An OAuth access token.
 *
 * A access token is the result of an authorization process, and
 * encapsulates the authorization to access protected information.
 *
 * Next to its value(), it also contains optionally an expires date
 * and a refresh token.
 *
 * \sa OAuthProcess::startAuthorize(), OAuthProcess::authorized()
 *
 * \ingroup auth
 */
class OAuthAccessToken
{
public:

  /*! \brief Default constructor.
   *
   * Creates an invalid access token.
   */
  OAuthAccessToken();

  /*! \brief Constructor.
   */
  OAuthAccessToken(const std::string& value,
		   const WDateTime& expires,
		   const std::string& refreshToken);

  /*! \brief Constructor with an OpenID Connect ID token.
   */
  OAuthAccessToken(const std::string& value,
		   const WDateTime& expires,
		   const std::string& refreshToken,
                   const std::string& idToken);

  /*! \brief Returns whether the token is valid.
   *
   * An invalid access token is used to signal for example that the
   * user denied the authorization request.
   */
  bool isValid() const { return !accessToken_.empty(); }

  /*! \brief Returns the access token value.
   *
   * This value can be used to access protected resources.
   */
  const std::string& value() const { return accessToken_; }

  /*! \brief Returns the token expires time (if available).
   *
   * \if cpp
   * Returns a Null date-time if not available.
   * \elseif java
   * Returns null if not available.
   * \endif
   */
  WDateTime expires() const { return expires_; }

  /*! \brief Returns the refresh token (if available).
   *
   * The refresh token is an optional token that can be used when the
   * access token has expired.
   *
   * If not available, returns an empty string.
   */
  const std::string& refreshToken() const { return refreshToken_; }

  const std::string& idToken() const { return idToken_; }

  /*! \brief An invalid token constant
   *
   * This is a token that is not isValid().
   */
  static const OAuthAccessToken Invalid;
    
private:
  std::string accessToken_, refreshToken_, idToken_;
  WDateTime expires_;
};

/*! \class OAuthProcess Auth/OAuthService
 *  \brief An OAuth authorization (and authentication) process.
 *
 * The process implements the state machine that is needed to complete
 * an %OAuth authorization cycle.
 *
 * Optionally, it also provides authentication, by using the
 * service-specific logic which uses the access token to return
 * identity information.
 *
 * A process is created for a particular scope, which represents
 * what kind of information one wants to access, and which is used
 * to inform the user of the kind of operations he needs to
 * authorize for your application to make with his protected data.
 *
 * \if cpp
 * The configuration of the service is done using properties which are
 * specific to the 3rd party provider and discussed in sub classes of
 * this class. There is one property, which is optional, which can be
 * configured here which is a secret used for creating the OAuth2
 * 'state' hash. By default, the library will create this secret from a
 * random generator, and this is sufficient for single-process deployments.
 * For multi-process deployments (e.g. FastCGI), however the same value
 * must be used in all processes and thus needs to be pre-configured.
 *
 * - <tt>oauth2-secret</tt>: a secret used for creating a state hashes.
 *
 * For example:
 * \code
 * <properties>
 *   ...
 *   <property name="oauth2-secret">U6EKaj5QurAJCWwBonvkM4i4pi7Wro2z9lEJRAKY</property>
 * </properties>
 * \endcode
 * \endif
 *
 * \sa OAuthService::createProcess()
 *
 * \ingroup auth
 */
class OAuthProcess : public WObject 
{
public:
  virtual ~OAuthProcess();

  /*! \brief Returns the scope for which this process was created.
   *
   * The scope represents how much protected information the web
   * application wants to access, and in what way.
   *
   * \sa OAuthService::createProcess(), OAuthService::authenticationScope()
   */
  const std::string& scope() const { return scope_; }

  /*! \brief Returns the %OAuth service which spawned this process.
   *
   * \sa OAuthService::createProcess()
   */
  const OAuthService& service() const { return service_; }

  /*! \brief Starts an authorization process.
   *
   * This starts an authorization process to request an accesstoken
   * to access protected information within the process scope.
   *
   * The authorization process ends with the authorized() signal which
   * signals the obtained token.
   *
   * \note To be able to use a popup (instead of a page redirect), you
   *       should connect this method directly to an, since popup
   *       windows are blocked in most web browsers unless they are
   *       the direct consequence of an event.
   */
  virtual void startAuthorize();

  /*! \brief Starts an authorization and authentication process.
   *
   * This is startAuthorize() followed by getIdentity().
   *
   * This requires that the process is created with an authorization
   * scope that includes sufficient rights for authentication (at
   * least OAuthService::authenticationScope())
   *
   * The authentication process ends with the authenticated() signal which
   * signals the obtained identity.
   *
   * \note To be able to use a popup (instead of a page redirect), you
   *       should connect this method directly to an, since popup
   *       windows are blocked in most web browsers unless they are
   *       the direct consequence of an event.
   */
  virtual void startAuthenticate();

#ifdef WT_TARGET_JAVA
  /*! \brief Connects an implementation to start an authentication process to a signal 
   *
   * If JavaScript is available, this method connects a JavaScript function to
   * the \p signal, otherwise startAuthenticate() is connected to \p signal. 
   */
  void connectStartAuthenticate(EventSignalBase &signal);
#endif

  /*! \brief Obtains an authenticated identity.
   *
   * The authentication process uses an access token to issue one or
   * more protected requests for obtaining identity
   * information. This is not part of the %OAuth protocol, since
   * %OAuth does not standardize the use of the access token to obtain
   * this information.
   *
   * The authentication process ends with the authenticated() signal
   * which signals the obtained identity.
   */
  virtual void getIdentity(const OAuthAccessToken& token);

  /*! \brief Error information, in case authentication or identification
   *         failed.
   *
   * The error message contains details when the authorized() or
   * authenticated() signals indicate respectively an invalid token
   * or invalid identity.
   */
  WString error() const { return error_; }

  /*! \brief Returns the access token.
   *
   * This returns the access token that was obtained in the last
   * authorization cycle.
   */
  const OAuthAccessToken& token() const { return token_; }

  /*! \brief Authorization signal.
   *
   * This signal indicates the end of an authorization process
   * started with startAuthorize(). If the authorization process was
   * successful, then the parameter carries a valid access token
   * that was obtained. If the authorization process failed then the
   * access token parameter is invalid, and you can get more
   * information using error().
   *
   * Authorization can fail because of a protocol error,
   * aconfiguration problem, or because the user denied the
   * authorization.
   *
   * \sa startAuthorize(), OAuthAccessToken::isValid()
   */
  Signal<OAuthAccessToken>& authorized() { return authorized_; }

  /*! \brief Authentication signal.
   *
   * This signal indicates the end of an authentication process
   * started with startAuthenticate() or getIdentity(). If the
   * authentication process was successful, then the parameter is a
   * valid and authentic identity. If the authentication process
   * failed then the identity parameter is invalid, and you can get
   * more information using error().
   *
   * Authentication can fail because authorization failed (in case
   * of startAuthenticate()), or because of a protocol error,
   * or configuration problem.
   *
   * \sa startAuthenticate(), getIdentity(), Identity::isValid()
   */
  Signal<Identity>& authenticated() { return authenticated_; }

protected:
  /*! \brief Constructor.
   *
   * \sa OAuthService::createProcess()
   */
  OAuthProcess(const OAuthService& service, const std::string& scope);

  /*! \brief Exception thrown while parsing a token response.
   *
   * \sa parseTokenResponse()
   */
  class TokenError
#ifdef WT_TARGET_JAVA
    : public std::exception
#endif
  {
  public:
    /*! \brief Constructor.
     */
    TokenError(const WString& error) : error_(error) { }

    /*! \brief The error.
     */
    WString error() const { return error_; } 

  private:
    WString error_;
  };

  /*! \brief Parses the response for a token request.
   *
   * Throws a TokenError when the response indicates an error,
   * or when the response could not be properly parsed.
   *
   * Some OAuth implementations may uses a non-standard encoding of
   * the token.
   */
  virtual OAuthAccessToken parseTokenResponse(const Http::Message& response);

  /*! \brief Sets the error.
   *
   * This should be used in getIdentity() implementations to set the
   * error, before emitting authenticated() with an invalid Identity.
   */
  virtual void setError(const WString& e);

private:
  const OAuthService& service_;
  std::string scope_;
  bool authenticate_;

  Signal<OAuthAccessToken> authorized_;
  Signal<Identity> authenticated_;
  JSignal<> redirected_;

  std::string oAuthState_;
  OAuthAccessToken token_;
  WString error_;
  std::string startInternalPath_;

  std::unique_ptr<OAuthRedirectEndpoint> redirectEndpoint_;
  std::unique_ptr<Http::Client> httpClient_;

  void requestToken(const std::string& authorizationCode);
  void handleToken(AsioWrapper::error_code err,
		   const Http::Message& response);
  OAuthAccessToken parseUrlEncodedToken(const Http::Message& response);
  OAuthAccessToken parseJsonToken(const Http::Message& response);

  std::string authorizeUrl() const;
  void doParseTokenResponse(const Http::Message& response);
  void onOAuthDone();
#ifndef WT_TARGET_JAVA
  void handleAuthComplete(); // For non-Ajax sessions
#endif // WT_TARGET_JAVA

  friend class OAuthRedirectEndpoint;
};

/*! \class OAuthService Wt/Auth/OAuthService.h
 *  \brief An %OAuth authorization (and authentication) service provider.
 *
 * This class implements an %OAuth client (<a
 * href="http://tools.ietf.org/html/draft-ietf-oauth-v2-22">2.0
 * draft</a>), which can be used to allow the user to authorize access
 * to information provided by a third-party %OAuth service
 * provider. This allows, among other things, for a user to safely
 * authenticate with your web application without needing to store or
 * even handle his authorization credentials (such as a password), a
 * pattern called "OAuth2 connect".
 *
 * The %OAuth protocol provides a standard for a user to authorize
 * access to protected resources made available by a third party
 * service. The process starts with the user authenticating on an
 * "authorization page" and authorizing access. This results
 * eventually in an access token for the web application. The actual
 * use of this token, to obtain certain information (such as an
 * authenticated identity) from the third party is however not
 * standardized, and there are many other uses of %OAuth besides
 * authentication.
 *
 * Because the focus of the %Wt::Auth library is authentication, the
 * %OAuth class also contains the implementation for using the access
 * token for authentication (OAuthProcess::getIdentity()).
 *
 * Like all <b>service classes</b>, this class holds only
 * configuration state. Thus, once configured, it can be safely shared
 * between multiple sessions since its state (the configuration) is
 * read-only. 
 * \if cpp
 * A "const OAuthService" object is thus thread-safe.
 * \endif
 *
 * The %OAuth authorization protocol, including the subsequent use for
 * authentication, consists of a number of consecutive steps, some of
 * which require user interaction, and some which require the use of
 * remote web services. The state machine for this process is
 * implemented in an OAuthProcess. To use %OAuth, you need to create
 * such a process and listen for state changes.
 *
 * Usage example:
 * \if cpp
 * \code
 * const OAuthService *oauth = ...;
 *
 * // Creates an icon which prompts for authentication using this OAuth service.
 * WImage *icon = icons->addWidget(std::make_unique<WImage>("css/oauth-" + auth->name() + ".png"));
 * icon->setToolTip(auth->description());
 *
 * // Creates a new process for authentication, which is started by a click on the icon
 * process = oauth->createProcess(oauth->authenticationScope());
 * icon->clicked().connect(process_, &OAuthProcess::startAuthenticate);
 *
 * // And capture the result in a method.
 * process_->authenticated().connect(this, &MyWidget::oauthDone);
 * \endcode
 * \elseif java
 * \code
 * OAuthService oauth = ...;
 *
 * // Creates an icon which prompts for authentication using this OAuth service.
 * WImage icon = new WImage("css/oauth-" + auth.getName() + ".png", icons);
 * icon.setToolTip(auth.getDescription());
 *
 * // Creates a new process for authentication, which is started by a click on the icon
 * process = oauth.createProcess(oauth.getAuthenticationScope());
 * process.connectStartAuthenticate(icon.clicked());
 *
 * // And capture the result in a method.
 * process.authenticated().addListener(this, new Signal1.Listener<Identity>() {
 *  public void trigger(Identity id) {
 *    MyWidget.this.oAuthDone(id);
 *  }
 * });
 * \endcode
 * \endif
 *
 * \ingroup auth
 */
class WT_API OAuthService
{
public:
  /*! \brief Constructor.
   *
   * Creates a new %OAuth service.
   */
  OAuthService(const AuthService& baseAuth);

  /*! \brief Destructor.
   */
  virtual ~OAuthService();

  /*! \brief Returns the basic authentication service.
   */
  const AuthService& baseAuth() const { return baseAuth_; }

  /*! \brief Creates a new authorization process.
   *
   * This creates a new authorization process for the indicated scope.
   * Valid names for the scope are service provider dependent.
   *
   * \sa authenticationScope()
   */
  virtual std::unique_ptr<OAuthProcess>
    createProcess(const std::string& scope) const = 0;

  /*! \brief Returns the provider name.
   *
   * This is a short identifier.
   *
   * \sa description()
   */
  virtual std::string name() const = 0;

  /*! \brief Returns the provider description.
   *
   * This returns a description useful for e.g. tool tips on a login
   * icon.
   *
   * \sa name()
   */
  virtual WString description() const = 0;

  /*! \brief Returns the desired width for the popup window.
   *
   * \sa popupHeight()
   */
  virtual int popupWidth() const = 0;

  /*! \brief Returns the desired height for the popup window.
   *
   * \sa popupWidth()
   */
  virtual int popupHeight() const = 0;

  // FIXME: add flags to include email (and "real name" ?)
  /*! \brief Returns the scope needed for authentication.
   *
   * This returns the scope that is needed (and sufficient) for
   * obtaining identity information, and thus to authenticate the
   * user.
   *
   * \sa OAuthProcess::getIdentity(), OAuthProcess::startAuthenticate()
   */
  virtual std::string authenticationScope() const = 0;

  /*! \brief Returns the redirection endpoint URL.
   *
   * This is the local URL to which the browser is redirect from the
   * service provider, after the authorization process. You need to
   * configure this URL with the third party authentication service.
   *
   * A static resource will be deployed at this URL.
   *
   * \if cpp
   * \sa WServer::addResource()
   * \elseif java 
   * @see WtServlet#addResource(WResource r, String path)
   * \endif
   */
  virtual std::string redirectEndpoint() const = 0;

  /*! \brief Returns the deployment path of the redirection endpoint.
   *
   * This returns the path at which the static resource is deployed
   * that corresponds to the redirectEndpoint().
   *
   * The default implementation will derive this path from the
   * redirectEndpoint() URL.
   */
  virtual std::string redirectEndpointPath() const;

  /*! \brief Returns the authorization endpoint URL.
   *
   * This is a remote URL which hosts the %OAuth authorization user
   * interface. This URL is loaded in the popup window at the start of
   * an authorization process.
   */
  virtual std::string authorizationEndpoint() const = 0;

  /*! \brief Returns the token endpoint URL.
   *
   * This is a remote URL which hosts a web-service that generates access
   * tokens.
   */
  virtual std::string tokenEndpoint() const = 0;

  virtual std::string userInfoEndpoint() const;

  /*! \brief Returns the client ID.
   *
   * This is the identification for this web application with the
   * %OAuth authorization server.
   */
  virtual std::string clientId() const = 0;

  /*! \brief Returns the client secret.
   *
   * This is the secret credentials for this web application with the
   * %OAuth authorization server.
   */
  virtual std::string clientSecret() const = 0;

  /*! \brief Derives a state value from the session ID.
   *
   * The state value protects the authorization protocol against
   * misuse, and is used to connect an authorization code response
   * with a particular request.
   *
   * In the default implementation the state is the \p sessionId,
   * crytpographically signed. This signature is verified in
   * decodeState().
   */
  virtual std::string encodeState(const std::string& sessionId) const;

  /*! \brief Validates and decodes a state parameter.
   *
   * This function returns the sessionId that is encoded in the state,
   * if the signature validates that it is an authentic state
   * generated by encodeState().
   */
  virtual std::string decodeState(const std::string& state) const;

  /*! \brief Returns the HTTP method used for the token request.
   *
   * While the current %OAuth 2.0 draft mandates the use of POST, some
   * implementations (like Facebook) use URL-encoding and a GET
   * request.
   *
   * The default implementation returns Http::Method::Post (corresponding to the
   * current draft).
   */
  virtual Http::Method tokenRequestMethod() const;

  /*! \brief Returns the method to transfer the client secret.
  *
  * Some implementations (like Facebook) encode the secret in the GET
  * request parameters, while this is explicitly not allowed in the
  * OAuth 2.0 specification.
  *
  * The default implementation returns HttpAuthorizationBasic
  * (the recommended method).
  */
  virtual ClientSecretMethod clientSecretMethod() const = 0;

  std::string generateRedirectEndpoint() const;

  // FIXME document later
  virtual std::string redirectInternalPath() const;

  /*! \brief Configures the static resource implementing the redirect endpoint.
   *
   * By default, this endpoint is configured whenever it's necessary,
   * but one may also configure it in advance, for example in a
   * multi-process deployment (FastCGI).
   */
  void configureRedirectEndpoint() const;

protected:
  static std::string configurationProperty(const std::string& property);

private:
  OAuthService(const OAuthService&);
  const AuthService& baseAuth_;

  struct Impl;
  std::unique_ptr<Impl> impl_;
};

  }
}

#endif // WT_AUTH_OAUTH_H_
