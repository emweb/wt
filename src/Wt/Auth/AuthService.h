// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_BASE_AUTH_H_
#define WT_AUTH_BASE_AUTH_H_

#include <Wt/WString.h>

#include <Wt/Auth/User.h>
#include <Wt/Auth/WAuthGlobal.h>

namespace Wt {
  namespace Mail {
    class Message;
  }

  /*! \brief Namespace for the \ref auth
   */
  namespace Auth {

class HashFunction;
class Identity;
class User;
class AbstractUserDatabase;

/*! \brief Enumeration for an identity policy.
 *
 * This enumeration lists possible choices for the user identity
 * (login name).
 *
 * When using password authentication, it is clear that the user has
 * to provide an identity to login. The only choice is whether you
 * will use the user's email address or another login name.
 *
 * When using a 3rd party authenticator, e.g. using OAuth, a login
 * name is no longer needed, but you may still want to give the user
 * the opportunity to choose one.
 *
 * \sa AuthService::setIdentityPolicy()
 *
 * \ingroup auth
 */
enum class IdentityPolicy {
  /*! \brief A unique login name chosen by the user.
   *
   * Even if not really required for authentication, a user still
   * chooses a unique user name. If possible, a third party
   * autheticator may suggest a user name.
   *
   * This may be useful for sites which have a social aspect.
   */
  LoginName,

  /*! \brief The email address serves as the identity.
   *
   * This may be useful for sites which do not have any social
   * character, but instead render a service to individual users.
   * When the site has a social character, you will likely not want to
   * display the email address of other users, but instead a
   * user-chosen login name.
   */
  EmailAddress,

  /*! \brief An identity is optional, and only asked if needed for
   *         authentication.
   *
   * Unless the authentication procedure requires a user name, no
   * particular identity is asked for. In this case, the identity is a
   * unique internal identifier.
   *
   * This may be useful for sites which do not have any social
   * character, but instead render a service to individual users.
   */
  Optional
};

/*! \brief A token validation state
 */
enum class EmailTokenState {
  Invalid,        //!< The token was invalid
  Expired,        //!< The token has expired

  /*! \brief A token was presented which requires the user to enter a new
   *         password.
   *
   * The presented token was a token sent by the
   * AuthService::lostPassword() function.  When this is returned as
   * result of AuthService::processEmailToken(), you should present the
   * user with a dialog where he can enter a new password.
   */
  UpdatePassword,

  /*! \brief A The token was presented which verifies the email address.
   *
   * The presented token was a token sent by the
   * AuthService::verifyEmailAddress() function. When this is returned
   * as result of processEmailToken(), you can indicate to the user
   * that his email address is now confirmed.
   */
  EmailConfirmed
};

/*! \brief Enumeration that describes an auth token validation state.
 */
enum class AuthTokenState {
  Invalid, //!< The presented auth token was invalid
  Valid    //!< The presented auth token could be used to identify a user.
};

/*! \brief Enumeration that describes the possible auth token type.
 */
enum class AuthTokenType {
  Password, //!< Regular username/password login
  MFA      //!< Multi Factor Authantification
};

/*! \class EmailTokenResult Wt/Auth/AuthService.h Wt/Auth/AuthService.h
 *  \brief The result of processing an email-sent token.
 *
 * An email token can be used for two purposes:
 *
 * - the user needs to verify his email address by returning a token sent
 *   to his supplied email address.
 * - the user indicates that he lost his email and wants to prove his identity
 *   by acknowledging an email to a previously verified email account.
 *
 * \sa AuthService::processEmailToken()
 * \sa AuthService::verifyEmailAddress(), AuthService::lostPassword()
 *
 * \ingroup auth
 */
class WT_API EmailTokenResult
{
public:
  /*! \brief Typedef for enum Wt::Auth::EmailTokenState */
  typedef EmailTokenState State;

  /*! \brief Constructor
   *
   * Creates an email token result.
   */
  EmailTokenResult(EmailTokenState state, const User& user = User());

  /*! \brief Returns the result.
   */
  EmailTokenState state() const { return state_; }

  /*! \brief Returns the user, if any.
   *
   * The identified user is only valid when the token state is
   * UpdatePassword or EmailConfirmed. In that case, you may login the
   * user as strongly authenticated since he presented a random token
   * that was sent to his own email address.
   */
  const User& user() const;

private:
  EmailTokenState state_;
  User user_;
};

/*! \class AuthTokenResult Wt/Auth/AuthService.h Wt/Auth/AuthService.h
 *  \brief The result of processing an authentication token.
 *
 * An authentication token is usually taken from a browser cookie, and
 * used to identify (and possibly authenticate) a user across
 * sessions.
 *
 * \sa AuthService::processAuthToken()
 * \sa AuthService::createAuthToken()
 *
 * \ingroup auth
 */
class WT_API AuthTokenResult
{
public:
  /*! \brief Typedef for enum Wt::Auth::AuthTokenState */
  typedef AuthTokenState State;

  /*! \brief Constructor
   *
   * Creates an authentication token result.
   */
  AuthTokenResult(AuthTokenState state, const User& user = User(),
                  const std::string& newToken = std::string(),
                  int newTokenValidity = -1);

  /*! \brief Returns the result.
   */
  AuthTokenState state() const { return state_; }

  /*! \brief Returns the identified user.
   *
   * The user is valid only if the the state() == AuthTokenState::Valid.
   */
  const User& user() const;

  /*! \brief Returns a new token for this user.
   *
   * Returns the empty string if there is no new token. See
   * AuthService::authTokenUpdateEnabled().
   *
   * The returned token is valid only if the state() == AuthTokenState::Valid.
   */
  std::string newToken() const;

  /*! \brief Returns the validity of the new token.
   *
   * This returns the token validity in seconds.
   *
   * Returns -1 if there is no new token, or result() != Valid.
   *
   * \sa newToken()
   */
  int newTokenValidity() const;

private:
  AuthTokenState state_;
  User user_;
  std::string newToken_;
  int newTokenValidity_;
};

/*! \class AuthService Wt/Auth/AuthService.h Wt/Auth/AuthService.h
 *  \brief Basic authentication service
 *
 * This class presents an basic authentication service, which offers
 * authentication functionality that is not specific to an
 * authentication mechanism (such as password authentication or %OAuth
 * authentication).
 *
 * Like all <b>service classes</b>, this class holds only
 * configuration state. Thus, once configured, it can be safely shared
 * between multiple sessions since its state (the configuration) is
 * read-only.
 * \if cpp
 * A "const AuthService" object is thus thread-safe.
 * \endif
 *
 * The class provides the following services (and relevant configuration):
 *
 *  - settings for generating random tokens:
 *    - setRandomTokenLength()
 *  - authentication tokens, used by e.g. remember-me functionality:
 *    - setAuthTokensEnabled()
 *    - processAuthToken()
 *  - email tokens, for email verification and lost password functions:
 *    - setEmailVerificationEnabled()
 *    - lostPassword()
 *    - verifyEmailAddress()
 *    - processEmailToken()
 *  - MFA tokens, used by multi-factor authentication functionality:
 *    - setMfaTokenCookieName()
 *    - setMfaTokenValidity()
 *    - processAuthToken()
 *
 * \ingroup auth
 */
class WT_API AuthService
{
public:
  /*! \brief Constructor
   */
  AuthService();

  /*! \brief Destructor
   */
  virtual ~AuthService();

  /*! \brief Sets the token length.
   *
   * Configures the length used for random tokens. Random tokens are
   * generated for authentication tokens, and email tokens.
   *
   * The default length is 32 characters.
   *
   * \sa WRandom::generateId()
   */
  void setRandomTokenLength(int length) { tokenLength_ = length; }

  /*! \brief Returns the token length.
   *
   * \sa setRandomTokenLength()
   */
  int randomTokenLength() const { return tokenLength_; }

  /*! \brief Configures the identity policy.
   *
   * The identity policy has an impact on the login and registration
   * procedure.
   */
  void setIdentityPolicy(IdentityPolicy policy);

  /*! \brief Returns the identity policy.
   *
   * \sa setIdentityPolicy()
   */
  IdentityPolicy identityPolicy() const { return identityPolicy_; }

  /*! \brief Tries to match the identity to an existing user.
   *
   * When authenticating using a 3rd party Identity Provider, the
   * identity is matched against the existing users, based on the id
   * (with AbstractUserDatabase::findWithIdentity()), or if not matched,
   * based on whether there is a user with the same verified email
   * address as the one indicated by the identity.
   */
  virtual User identifyUser(const Identity& identity,
                            AbstractUserDatabase& users) const;

  /** @name Authentication token support
   */
  //!@{
  /*! \brief Configures authentication token support.
   *
   * This method allows you to configure whether authentication tokens
   * are in use. Authentication tokens are used for the user to bypass
   * a more elaborate authentication method, and are a secret shared with
   * the user's user agent, usually in a cookie. They are typically presented
   * in the user interfaces as a "remember me" option.
   *
   * Whenever a valid authentication token is presented in
   * processToken(), it is invalidated a new token is generated and
   * stored for the user.
   *
   * The tokens are generated and subsequently hashed using the token
   * hash function. Only the hash values are stored in the user
   * database so that a compromised user database does not compromise
   * these tokens.
   *
   * Authentication tokens are disabled by default.
   *
   * The default name for these cookies is \p wtauth, and if MFA is
   * enabled, their corresponding name will be \p wtauth-mfa. The name
   * of these tokens can be changed (setMfaTokenCookieName()).
   *
   * \sa setTokenHashFunction(), setAuthTokenValidity()
   */
  void setAuthTokensEnabled(bool enabled,
                            const std::string& cookieName = "wtauth",
                            const std::string& cookieDomain = std::string());

  /*! \brief Returns whether authentication tokens are enabled.
   *
   * \sa setAuthTokensEnabled()
   */
  bool authTokensEnabled() const { return authTokens_; }

  /*! \brief Set whether processAuthToken() updates the auth token
   *
   * If this option is enabled, processAuthToken() will replace the auth token
   * with a new token. This is a bit more secure, because an auth token can
   * only be used once. This is enabled by default.
   *
   * However, this means that if a user concurrently opens multiple sessions within
   * the same browsers (e.g. multiple tabs being restored at the same time)
   * or refreshes before they receive the new cookie, the user will be logged out,
   * unless the AbstractUserDatabase implementation takes this into account
   * (e.g. keeps the old token valid for a little bit longer)
   *
   * The default Dbo::UserDatabase does not handle concurrent token updates well,
   * so disable this option if you want to prevent that issue.
   *
   * \note If using MFA, processAuthToken() will also use this
   * functionality.
   *
   * \sa processAuthToken()
   * \sa authTokenUpdateEnabled()
   */
  void setAuthTokenUpdateEnabled(bool enabled) { authTokenUpdateEnabled_ = enabled; }

  /*! \brief Returns whether the auth token is updated
   *
   * \sa setAuthTokenUpdateEnabled()
   */
  bool authTokenUpdateEnabled() const { return authTokenUpdateEnabled_; }

  /*! \brief Returns the authentication token cookie name.
   *
   * This is the default cookie name used for storing the authentication
   * token in the user's browser.
   *
   * \sa setAuthTokensEnabled()
   */
  std::string authTokenCookieName() const { return authTokenCookieName_; }

  /*! \brief Returns the authentication token cookie domain.
   *
   * This is the domain used for the authentication cookie. By default this
   * is empty, which means that a cookie will be set for this application.
   *
   * You may want to set a more general domain if you are sharing the authentication
   * with multiple applications.
   *
   * \sa setAuthTokensEnabled()
   */
  std::string authTokenCookieDomain() const { return authTokenCookieDomain_; }

  /*! \brief Sets the token hash function.
   *
   * Sets the hash function used to safely store authentication tokens
   * in the database. Ownership of the hash function is transferred.
   *
   * The default token hash function is an MD5HashFunction.
   */
  void setTokenHashFunction(std::unique_ptr<HashFunction> function);

  /*! \brief Returns the token hash function.
   *
   * \sa setTokenHashFunction()
   */
  HashFunction *tokenHashFunction() const;

  /*! \brief Creates and stores an authentication token for the user.
   *
   * This creates and stores a new authentication token for the given
   * user. The token will expire after \p authTokenValidity minutes if
   * \p authTokenValidity is positive and after authTokenValidity()
   * minutes otherwise.
   *
   * The returned value is the token that may be used to re-identify
   * the user in processAuthToken().
   * 
   * \sa createAuthToken(const User&, AuthTokenType) const
   */
  std::string createAuthToken(const User& user, int authTokenValidity = -1) const;

  /*! \brief Creates and stores an authentication token for the user.
   *
   * This creates and stores a new authentication token for the given
   * user. The token will expire after the token validity time
   * configured for the tokens of type \p authTokenType.
   *
   * For AuthTokenType::Password this uses authTokenValidity(),
   * for AuthTokenType::MFA this will utilize mfaTokenValidity().
   *
   * The returned value is the token that may be used to re-identify
   * the user in processAuthToken().
   * 
   * \sa createAuthToken(const User&, int) const
   */
  std::string createAuthToken(const User& user, AuthTokenType authTokenType) const;

  /*! \brief Processes an authentication token.
   *
   * This verifies an authentication token, and considers whether it
   * matches with a token hash value stored in database. This indicates
   * that the cookie a User has in their browser is still valid, and can
   * be used to uniquely identify them.
   *
   * If it matches and auth token update is enabled
   * (setAuthTokenUpdateEnabled()), the token is updated with a new hash.
   * In case AbstractUserDatabase::updateAuthToken() returns -1, a new
   * token that expires after authTokenValidity() minutes will be
   * stored for the user.
   *
   * \sa setAuthTokensEnabled()
   * \sa authTokenValidity()
   * \sa AbstractUserDatabase::updateAuthToken()
   */
  virtual AuthTokenResult processAuthToken(const std::string& token,
                                           AbstractUserDatabase& users) const;
  
  /*! \brief Processes an authentication token.
   *
   * This verifies an authentication token, and considers whether it
   * matches with a token hash value stored in database. This indicates
   * that the cookie a User has in their browser is still valid, and can
   * be used to uniquely identify them.
   *
   * If it matches and auth token update is enabled
   * (setAuthTokenUpdateEnabled()), the token is updated with a new
   * hash. In case AbstractUserDatabase::updateAuthToken() returns -1,
   * a new token is stored that will expire after the token validity
   * time configured for the tokens of type \p authTokenType. If
   * \p authTokenValidity is negative, then the new token will expire
   * after authTokenValidity() minutes.
   * 
   * \sa setAuthTokensEnabled()
   * \sa authTokenValidity()
   * \sa mfaTokenValidity()
   * \sa AbstractUserDatabase::updateAuthToken()
   */
  virtual AuthTokenResult processAuthToken(const std::string& token,
                                           AbstractUserDatabase& users,
                                           int authTokenValidity) const;
  
  /*! \brief Processes an authentication token.
   *
   * This verifies an authentication token, and considers whether it
   * matches with a token hash value stored in database. This indicates
   * that the cookie a User has in their browser is still valid, and can
   * be used to uniquely identify them.
   *
   * If it matches and auth token update is enabled
   * (setAuthTokenUpdateEnabled()), the token is updated with a new hash.
   * In case AbstractUserDatabase::updateAuthToken() returns -1, a new
   * token is stored that will expire after the token validity time
   * configured for the tokens of type \p authTokenType.
   *
   * \sa setAuthTokensEnabled()
   * \sa authTokenValidity()
   * \sa mfaTokenValidity()
   * \sa AbstractUserDatabase::updateAuthToken()
   */
  virtual AuthTokenResult processAuthToken(const std::string& token,
                                           AbstractUserDatabase& users,
                                           AuthTokenType authTokenType) const;

  /*! \brief Configures the duration for an authenticaton to remain valid.
   *
   * The default duration is two weeks (14 * 24 * 60 minutes).
   */
  void setAuthTokenValidity(int minutes) { authTokenValidity_ = minutes; }

  /*! \brief Returns the authentication token validity.
   *
   * \sa setAuthTokenValidity()
   */
  int authTokenValidity() const { return authTokenValidity_; }
  //!@}

  /** @name Email verification
   */
  //!@{

  /*! \brief Configures email verification.
   *
   * Email verification is useful for a user to recover a lost
   * password, or to be able to confidently confirm other events with
   * this user (such as order processing).
   */
  void setEmailVerificationEnabled(bool enabled);

  /*! \brief Returns whether email verification is configured.
   *
   * \sa setEmailVerificationEnabled()
   */
  bool emailVerificationEnabled() const { return emailVerification_; }

  /*! \brief Configure email verificiation to be required for login.
   *
   * When enabled, a user will not be able to login if the email-address was
   * not verified.
   */
  void setEmailVerificationRequired(bool enabled);

  /*! \ Returns whether email verification is required for login.
   *
   * \sa setEmailVerificationRequired()
   */
  bool emailVerificationRequired() const { return emailVerificationReq_; }

  /*! \brief Sets the internal path used to present tokens in emails.
   *
   * The default path is "/auth/mail/".
   */
  void setEmailRedirectInternalPath(const std::string& internalPath);

  /*! \brief Returns the internal path used for email tokens.
   *
   * \sa setEmailRedirectInternalPath()
   */
  std::string emailRedirectInternalPath() const
    { return redirectInternalPath_; }

  /*! \brief Parses the emailtoken from an internal path.
   *
   * This method parses an internal path and if it matches the email
   * redirection path, it returns the token contained.
   *
   * It returns an empty string if the internal path does not contain
   * an email token.
   */
  virtual std::string parseEmailToken(const std::string& internalPath) const;

  /*! \brief Verifies an email address.
   *
   * This registers a new email token with the user.
   *
   * Then it sends an email to the user's unverified email address
   * with instructions that redirect him to this site, using
   * sendConfirmEmail().
   *
   * \sa processEmailToken()
   */
  virtual void verifyEmailAddress(const User& user,
                                  const std::string& emailAddress) const;


  /*! \brief Implements lost password functionality.
   *
   * If email address verification is enabled, then a user may recover
   * his password (or rather, chose a new password) using a procedure
   * which involves sending an email to a verified email address.
   *
   * This method triggers this process, starting from an email
   * address, if this email address corresponds to a verified email
   * address in the database. The current password is not invalidated.
   *
   * \sa processEmailToken()
   */
  virtual void lostPassword(const std::string& emailAddress,
                            AbstractUserDatabase& users) const;

  /*! \brief Processes an email token.
   *
   * This processes a token received through an email. If it is an
   * email verification token, the token is removed from the database.
   *
   * This may return two successful results:
   * - EmailTokenState::EmailConfirmed: a token was presented which proves
   *   that the user is tied to the email address.
   * - EmailTokenState::UpdatePassword: a token was presented which requires
   *   the user to enter a new password.
   *
   * \note Since Wt 4.3.0, the behavior of this function changed. The lost password
   *       token is no longer removed by processEmailToken(). Instead, it is now
   *       removed in User::setPassword().
   *
   * \sa verifyEmailAddress()
   * \sa lostPassword()
   */
  virtual EmailTokenResult processEmailToken(const std::string& token,
                                             AbstractUserDatabase& users) const;

  /*! \brief Configures the duration for an email token to remain valid.
   *
   * The default duration is three days (3 * 24 * 60 minutes). Three is a divine
   * number.
   */
  void setEmailTokenValidity(int minutes) { emailTokenValidity_ = minutes; }

  /*! \brief Returns the duration for an email token to remain valid.
   *
   * \sa setEmailTokenValidity()
   */
  int emailTokenValidity() const { return emailTokenValidity_; }

  /*! \brief Sends an email
   *
   * Sends an email to the given address with subject and body.
   *
   * The default implementation will consult configuration properties
   * to add a sender address if it hasn't already been set:
   * - "auth-mail-sender-name": the sender name, with default value
   *   "Wt Auth module"
   * - "auth-mail-sender-address": the sender email address, with default value
   *   "noreply-auth@www.webtoolkit.eu"
   *
   * \if cpp
   * Then it uses Mail::Client to send the message, using default the
   * default client settings.
   * \elseif java
   * Then it uses the JavaMail API to send the message, the SMTP settings
   * are configured using the smtp.host and smpt.port JWt configuration
   * variables (see {javadoclink Configuration#setProperties(HashMap properties)}).
   * \endif
   */
  virtual void sendMail(const Mail::Message& message) const;
  //!@}

  /** @name Multi-factor authentication (Time-Based One-Time Password).
   */
  //!@{
  /*! \brief Sets whether multiple factors are enabled when logging in.
   *
   * If this is set to any value that isn't \p empty, a second factor
   * (currently only TOTP as a default implementation) will be enabled.
   * Instead of logging the User in with the LoginState::Strong (when
   * authenticating with a password), the state will be set to
   * LoginState::RequiresMfa.
   *
   * This indicates that the user is still required to provide an
   * additional login step. This applied to the regular login only. Login
   * via OAuth does not make use of MFA. The authenticator should be
   * sufficiently secure in itself, and should be trusted.
   *
   * By default %Wt offers TOTP (Time-based One-time password) as a
   * second factor. This is a code that any authenticator app/extension
   * can generate.
   *
   * By default this feature is disabled.
   *
   * \note If a value for a \p provider is set here, MFA will be enabled.
   * This will use TOTP by default. Any custom implementation (using
   * Mfa::AbstractMfaProcess) will require more changes.
   * \sa AuthWidget::createMfaProcess(), AuthWidget::createMfaView(), and
   * AuthModel::hasMfaStep().
   *
   * \sa setMfaRequired(), setMfaCodeLength(), setMfaTokenValidity()
   */
  void setMfaProvider(const std::string& provider);

  /*! \brief Returns the provider that manages the MFA step.
   *
   * \sa setMfaProvider()
   */
   const std::string& mfaProvider() const { return mfaProvider_; }

  /*! \brief Returns whether a second factor is enabled when logging in.
   *
   * This marks that users can enable the MFA feature. This can be any
   * implemenation of the Mfa::AbstractMfaProcess interface. %Wt supplies
   * a single default implementation, namely Mfa::TotpProcess.
   *
   * For the default %Wt implementation, this means that developers can
   * generate a TOTP secret key (Mfa::generateSecretKey()) which can be
   * used for MFA, by means of the Mfa::TotpProcess.
   *
   * \sa mfaRequired(), mfaCodeLength()
   */
  bool mfaEnabled() const { return !mfaProvider_.empty(); }

  /*! \brief Sets whether multiple factors are required when logging in.
   *
   * If this is set to \p true, a second factor (defaulting to TOTP) will
   * be required on login. Instead of logging the User in with the
   * LoginState::Strong (when authenticating with a password), the state
   * will be set to LoginState::RequiresMfa.
   *
   * This will only take effect is mfaEnabled() is \p true as well (and
   * thus if an MFA provider has been configured (setMfaProvider()). If
   * this is the case, all users will be enforced to verify their login
   * with an extra step, for more security.
   *
   * By %Wt's default implementation this will boil down to providing a
   * TOTP code. Either this will be the first time they do this. In which
   * case they will be given the secret key both in QR code format (using
   * Mfa::AbstractMfaWidgget::createSetupView()), and as a string. When
   * they enter a correct generated code, the secret will be attached to
   * the User record as an Identity, and will be used for subsequent
   * login attempts (using Mfa::AbstractMfaProcess::createInputView()).
   *
   * \sa setMfaProvider(), setMfaCodeLength(), setMfaTokenValidity()
   */
  void setMfaRequired(bool required);

  /*! \brief Returns whether multiple factors are required when logging in.
   *
   * This marks that the MFA feature is enforced for all users.
   *
   * For %Wt's default implementation this means that upon logging in they
   * will have to provide a TOTP code for verification.
   *
   * \sa setMfaRequired()
   */
  bool mfaRequired() const { return mfaRequired_; }

  /*! \brief Sets the length of the code that is expected for MFA.
   *
   * This function does not make sense for all implementations of MFA.
   *
   * Currently %Wt ships with a TOTP implementation, in which case the
   * code can be any length in the range [6-16]. Of course longer is
   * "better" generally speaking.
   *
   * By default the length is \p 6.
   *
   * \note Due to the TOTP algorithm a longer code may not always make
   * too much sense, as the code can start with a lot of leading zeroes.
   * Generally a length of \p 6 is most often seen.
   *
   * \sa setMfaProvider(), setMfaRequired()
   */
  void setMfaCodeLength(int length);

  /*! \brief Returns the length of the expected MFA codes.
   *
   * \sa setMfaCodeLength()
   */
  int mfaCodeLength() const { return mfaCodeLength_; }

  /*! \brief Sets the name of the cookie used for MFA "remember-me".
   *
   * This name can be changed to any name the developer desires, that is
   * a valid name for a cookie. By default this name is the same name as
   * authTokenCookieName() with "-mfa" appended. If this name is not
   * changed from the default, this will result in the "wtauth-mfa"
   * name.
   *
   * \note No check is performed for clashing names, so this can
   * potentially override existing cookies.
   *
   * \sa setMfaTokenValidity(), setAuthTokensEnabled()
   */
  void setMfaTokenCookieName(const std::string& name);

  /*! \brief Returns the name of the MFA "remember-me" cookie.
   *
   * This can be configured by the developer, or otherwise uses the same
   * name  as authTokenCookieName() with the "-mfa" suffix. If nothing is
   * changed, the default is "wtauth-mfa".
   */
  const std::string& mfaTokenCookieName() const { return mfaTokenCookieName_; }

  /*! \brief Returns the domain of the MFA "remember-me" cookie.
   *
   * This can be configured by the developer, or otherwise uses the same
   * name as authTokenCookieDomain(). If nothing is changed, the default
   * is an empty string.
   *
   * This should best be used if multiple applications are living on the
   * same
   */
  void setMfaTokenCookieDomain(const std::string& domain);

  /*! \brief Returns the MFA token cookie domain.
   *
   * \sa setMfaTokenCookieDomain()
   */
  const std::string& mfaTokenCookieDomain() const { return mfaTokenCookieDomain_; }

  /*! \brief Sets the validity duration of the MFA "remember-me" cookie
   *
   * The duration is specified in minutes. So a validity of 1440 will
   * keep the cookie valid for a whole day. This ensures that there is
   * a limit on how long the User's MFA verification can be bypassed.
   *
   * From a security aspect, this value should be kept on the lower side
   * so that a user will be required to demonstrate a TOTP code often to
   * the application, by which they validate their identity.
   *
   * \sa Mfa::AbstractMfaProcess.
   *
   * The default duration is 90 days.
   *
   * \note There are limits on the validity of a cookie in some browsers,
   * so it cannot be guaranteed that "forever" is actually applicable.
   */
  void setMfaTokenValidity(int validity);

  /*! \brief Returns the validity span of the MFA "remember-me" cookie.
   *
   * \sa setMfaTokenValidity()
   */
  int mfaTokenValidity() const { return mfaTokenValidity_; }

  /*! \brief Sets whether throttling is enabled during the MFA process.
   *
   * By default this is not enabled.
   *
   * \sa mfaThrottleEnabled(), AbstractMfaProcess::setMfaThrottle()
   */
  void setMfaThrottleEnabled(bool enabled);

  /*! \brief Returns if throttling is enabled during the MFA process.
   *
   * \sa setMfaThrottleEnabled()
   */
  bool mfaThrottleEnabled() const { return mfaThrottleEnabled_; }
  //!@}

protected:
  /*! \brief Sends a confirmation email to the user to verify his email address.
   *
   * Sends a confirmation email to the given address.
   *
   * The email content is provided by the following string keys:
   *  - subject: tr("Wt.auth.verification-mail.subject")
   *  - body: tr("Wt.auth.verification-mail.body") with {1} a place holder for
   *    the identity, and {2} a placeholder for the redirection URL.
   *  - HTML body: tr("Wt.auth.verification-mail.htmlbody") with the same place
   *    holders.
   */
  virtual void sendConfirmMail(const std::string& address,
                               const User& user,
                               const std::string& token) const;

  /*! \brief Sends an email to the user to enter a new password.
   *
   * This sends a lost password email to the given \p address, with
   * a given \p token.
   *
   * The default implementation will call sendMail() with the following
   * message:
   *  - tr("Wt.Auth.lost-password-mail.subject") as subject,
   *  - tr("Wt.Auth.lost-password-mail.body") as body to which it passes
   *    user.identity() and token as arguments.
   *  - tr("Wt.Auth.lost-password-mail.htmlbody") as HTML body to which
   *    it passes user.identity() and token as arguments.
   */
  virtual void sendLostPasswordMail(const std::string& address,
                                    const User& user,
                                    const std::string& token) const;

  virtual std::string createRedirectUrl(const std::string& token) const;

private:
  AuthService(const AuthService&);

  IdentityPolicy identityPolicy_;
  int minimumLoginNameLength_;

  std::unique_ptr<HashFunction> tokenHashFunction_;
  int tokenLength_;

  bool emailVerification_;
  bool emailVerificationReq_;
  int emailTokenValidity_; // minutes
  std::string redirectInternalPath_;

  bool authTokens_;
  bool authTokenUpdateEnabled_;
  int authTokenValidity_;  // minutes
  std::string authTokenCookieName_;
  std::string authTokenCookieDomain_;

  std::string mfaProvider_;
  bool mfaRequired_;
  int mfaCodeLength_;
  bool mfaTokens_;
  int mfaTokenValidity_;  // minutes
  std::string mfaTokenCookieName_;
  std::string mfaTokenCookieDomain_;
  bool mfaThrottleEnabled_;
};

  }
}

#endif // WT_AUTH_BASE_AUTH
