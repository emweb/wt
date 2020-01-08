// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_DBO_AUTH_INFO_H_
#define WT_AUTH_DBO_AUTH_INFO_H_

#include <Wt/Auth/User.h>

#ifndef WT_AUTH_DBO_EXTERN_TEMPLATES
#include <Wt/Dbo/Dbo.h>
#else
#include <Wt/Dbo/Types.h>
#endif

#include <Wt/Dbo/WtSqlTraits.h>

namespace Wt {
  namespace Auth {
    namespace Dbo {

template <class AuthInfoType> class AuthToken;
template <class AuthIdentityType> class AuthIdentity;

/*! \class AuthInfo Wt/Auth/Dbo/AuthInfo.h
 *  \brief A default implementation for authentication data in %Wt::%Dbo
 * 
 * This class implements the requirements for use as a data type in
 * Wt::Auth::Dbo::UserDatabase.
 *
 * It is a template class, and needs as parameter the Dbo type which
 * models a user (e.g. name, birth date, ...). Thus, this class only
 * carries the authentication information for that user.
 *
 * It contains collections to two other types:
 *  - authTokens() references a collection of authentication tokens, see
 *    AuthService::setAuthTokensEnabled()
 *  - authIdentities() references a collection of identities, which represent
 *    all the authentication identities this user has (e.g. a login name, but
 *    also an OAuth identity, etc...)
 *
 * To use these classes, you need to map three classes to tables of your
 * choice.
 *
 * \code
 * class User; // your user Dbo type
 * using AuthInfo = Wt::Auth::Dbo::AuthInfo<User>;
 *
 * session->mapClass<User>("user");
 *
 * session->mapClass<AuthInfo>("auth_info");
 * session->mapClass<AuthInfo::AuthIdentityType>("auth_identity");
 * session->mapClass<AuthInfo::AuthTokenType>("auth_token");
 * \endcode
 *
 * \sa UserDatabase
 *
 * \ingroup auth
 */
template <class UserType>
class AuthInfo : public Wt::Dbo::Dbo< AuthInfo<UserType> >
{
public:
  /*! \brief Type for an auth token.
   */
  typedef AuthToken< AuthInfo< UserType> > AuthTokenType;

  /*! \brief Type for a collection of auth tokens.
   */
  typedef Wt::Dbo::collection< Wt::Dbo::ptr<AuthTokenType> > AuthTokens;

  /*! \brief Type for an identity.
   */
  typedef AuthIdentity< AuthInfo< UserType> > AuthIdentityType;

  /*! \brief Type for a collection of identites.
   */
  typedef Wt::Dbo::collection< Wt::Dbo::ptr<AuthIdentityType> > AuthIdentities;

  /*! \brief Constructor.
   */
  AuthInfo()
    : status_(AccountStatus::Normal),
      failedLoginAttempts_(0),
      emailTokenRole_(EmailTokenRole::VerifyEmail)
  { }

  /*! \brief Sets the user.
   *
   * This sets the user that owns this authentication information.
   */
  void setUser(Wt::Dbo::ptr<UserType> user) { user_ = user; }

  /*! \brief Returns a reference to the user.
   *
   * \sa setUser()
   */
  Wt::Dbo::ptr<UserType> user() const { return user_; }

  /*! \brief Sets a password.
   */
  void setPassword(const std::string& hash, const std::string& hashFunction,
		   const std::string& hashSalt) {
    passwordHash_ = hash;
    passwordMethod_ = hashFunction;
    passwordSalt_ = hashSalt;
  }

  /*! \brief Returns the password hash.
   *
   * \sa setPassword()
   */
  const std::string& passwordHash() const { return passwordHash_; }

  /*! \brief Returns the password method.
   *
   * \sa setPassword()
   */
  const std::string& passwordMethod() const { return passwordMethod_; }

  /*! \brief Returns the password salt.
   *
   * \sa setPassword()
   */
  const std::string& passwordSalt() const { return passwordSalt_; }

  /*! \brief Sets the email address.
   */
  void setEmail(const std::string& email) { email_ = email; }

  /*! \brief Returns the email address.
   *
   * \sa setEmail()
   */
  const std::string& email() const { return email_; }

  /*! \brief Sets the unverified email address.
   */
  void setUnverifiedEmail(const std::string& email) {
    unverifiedEmail_ = email;
  }

  /*! \brief Returns the unverified email address.
   *
   * \sa setUnverifiedEmail()
   */
  const std::string& unverifiedEmail() const { return unverifiedEmail_; }

  /*! \brief Sets the email token.
   */
  void setEmailToken(const std::string& hash, const WDateTime& expires,
		     EmailTokenRole role) {
    emailToken_ = hash;
    emailTokenExpires_ = expires;
    emailTokenRole_ = role;
  }

  /*! \brief Returns the email token.
   *
   * \sa setEmailToken()
   */
  const std::string& emailToken() const { return emailToken_; }

  /*! \brief Returns the email token expiration date.
   *
   * \sa setEmailToken()
   */
  const Wt::WDateTime& emailTokenExpires() const { return emailTokenExpires_; }

  /*! \brief Returns the email token role.
   *
   * \sa setEmailToken()
   */
  EmailTokenRole emailTokenRole() const { return emailTokenRole_; }

  /*! \brief Sets the status.
   */
  void setStatus(AccountStatus status) { status_ = status; }

  /*! \brief Returns the status.
   *
   * \sa setStatus()
   */
  AccountStatus status() const { return status_; }

  /*! \brief Sets the number of failed login attempts.
   */
  void setFailedLoginAttempts(int count) { failedLoginAttempts_ = count; }

  /*! \brief Returns the number of failed login attempts.
   *
   * \sa failedLoginAttempts()
   */
  int failedLoginAttempts() const { return failedLoginAttempts_; }

  /*! \brief Sets the time of the last login attempt.
   */
  void setLastLoginAttempt(const Wt::WDateTime& dt) { lastLoginAttempt_ = dt; }

  /*! \brief Returns the time of the last login attempt.
   *
   * \sa setLastLoginAttempt()
   */
  const Wt::WDateTime& lastLoginAttempt() const { return lastLoginAttempt_; }

  /*! \brief Returns the authentication tokens (read-only).
   */
  const AuthTokens& authTokens() const { return authTokens_; }

  /*! \brief Returns the authentication tokens.
   */
  AuthTokens& authTokens() { return authTokens_; }

  /*! \brief Returns the authentication identities (read-only).
   */
  const AuthIdentities& authIdentities() const { return authIdentities_; }

  /*! \brief Returns the authentication identities.
   */
  AuthIdentities& authIdentities() { return authIdentities_; }

  /*! \brief Finds an identity of a particular provider.
   *
   * Note, a user could in theory have multiple identities from a
   * single provider. If there are multiple, only one of them is
   * returned.
   */
  WString identity(const std::string& provider) const {
    AuthIdentities c
      = authIdentities_.find().where("provider = ?").bind(provider);

    typename AuthIdentities::const_iterator i = c.begin();

    if (i != c.end())
      return (*i)->identity();
    else
      return WString::Empty;
  }

  /*! \brief %Wt::%Dbo persist implementation.
   */
  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::belongsTo(a, user_, "user", Wt::Dbo::OnDeleteCascade);

    Wt::Dbo::field(a, passwordHash_, "password_hash", 100);
    Wt::Dbo::field(a, passwordMethod_, "password_method", 20);
    Wt::Dbo::field(a, passwordSalt_, "password_salt", 20);

    Wt::Dbo::field(a, status_, "status");

    Wt::Dbo::field(a, failedLoginAttempts_, "failed_login_attempts");
    Wt::Dbo::field(a, lastLoginAttempt_, "last_login_attempt");

    Wt::Dbo::field(a, email_, "email", 256);
    Wt::Dbo::field(a, unverifiedEmail_, "unverified_email", 256);
    Wt::Dbo::field(a, emailToken_, "email_token", 64);
    Wt::Dbo::field(a, emailTokenExpires_, "email_token_expires");
    Wt::Dbo::field(a, emailTokenRole_, "email_token_role");


    Wt::Dbo::hasMany(a, authTokens_, Wt::Dbo::ManyToOne);
    Wt::Dbo::hasMany(a, authIdentities_, Wt::Dbo::ManyToOne);
  }

private:
  std::string passwordHash_;
  std::string passwordMethod_;
  std::string passwordSalt_;

  AccountStatus status_;

  int failedLoginAttempts_;
  Wt::WDateTime lastLoginAttempt_;

  std::string email_, unverifiedEmail_;
  std::string emailToken_;
  Wt::WDateTime emailTokenExpires_;
  EmailTokenRole emailTokenRole_;

  Wt::Dbo::ptr<UserType> user_;
  AuthTokens authTokens_;
  AuthIdentities authIdentities_;
};

/*! \class AuthToken Wt/Auth/Dbo/AuthInfo.h
 *  \brief A default implementation for an authentication token in %Wt::%Dbo
 * 
 * This class is used by AuthInfo, and stores authentication tokens.
 *
 * \sa AuthInfo
 *
 * \ingroup auth
 */
template <class AuthInfoType>
class AuthToken : public Wt::Dbo::Dbo< AuthToken<AuthInfoType> >
{
public:
  /*! \brief Default constructor.
   */
  AuthToken() { }

  /*! \brief Constructor.
   */
  AuthToken(const std::string& value, const Wt::WDateTime& expires)
    : value_(value), expires_(expires) { }

  /*! \brief Returns the token owner.
   */
  Wt::Dbo::ptr<AuthInfoType> authInfo() const { return authInfo_; }

  /*! \brief Returns the token value.
   */
  const std::string& value() const { return value_; }

  /*! \brief Sets the token value.
   */
  void setValue(const std::string& value) { value_ = value; }

  /*! \brief Returns the token expiry date.
   */
  const Wt::WDateTime& expires() const { return expires_; }

  /*! \brief %Wt::%Dbo persist implementation.
   */
  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::belongsTo(a, authInfo_, Wt::Dbo::OnDeleteCascade);
    Wt::Dbo::field(a, value_, "value", 64);
    Wt::Dbo::field(a, expires_, "expires");
  }

private:
  Wt::Dbo::ptr<AuthInfoType> authInfo_;
  std::string value_;
  Wt::WDateTime expires_;
};

/*! \class AuthIdentity Wt/Auth/Dbo/AuthInfo.h
 *  \brief A default implementation for a authentication identity in %Wt::%Dbo
 * 
 * This class is used by AuthInfo, and stores identities.
 *
 * \sa AuthInfo
 *
 * \ingroup auth
 */
template <class AuthInfoType>
class AuthIdentity : public Wt::Dbo::Dbo< AuthIdentity<AuthInfoType> >
{
public:
  /*! \brief Default constructor.
   */
  AuthIdentity() { }

  /*! \brief Constructor.
   */
  AuthIdentity(const std::string& provider, const WString& identity)
    : provider_(provider), identity_(identity) { }

  /*! \brief Returns the identity owner.
   */
  Wt::Dbo::ptr<AuthInfoType> authInfo() const { return authInfo_; }

  const std::string& provider() const { return provider_; }
  const Wt::WString& identity() const { return identity_; }

  void setIdentity(const Wt::WString& identity) { identity_ = identity; }

  /*! \brief %Wt::%Dbo persist implementation.
   */
  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::belongsTo(a, authInfo_, Wt::Dbo::OnDeleteCascade);
    Wt::Dbo::field(a, provider_, "provider", 64);
    Wt::Dbo::field(a, identity_, "identity", 512);
  }

private:
  Wt::Dbo::ptr<AuthInfoType> authInfo_;
  std::string provider_;
  Wt::WString identity_;
};

    }
  }
}

#endif // WT_AUTH_DBO_AUTH_INFO
