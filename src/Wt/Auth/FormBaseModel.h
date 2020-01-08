// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_FORM_BASE_MODEL_H_
#define WT_AUTH_FORM_BASE_MODEL_H_

#include <Wt/WFormModel.h>
#include <Wt/Auth/Login.h>

namespace Wt {
  namespace Auth {

class AbstractPasswordService;
class AbstractUserDatabase;
class OAuthService;

/*! \class FormBaseModel Wt/Auth/FormBaseModel.h
 *  \brief A base model class for authentication-related forms
 *
 * This class manages the the auth services and the user database which
 * an authentication model will use to implement a form..
 *
 * \ingroup auth
 */
class WT_API FormBaseModel : public WFormModel
{
public:
  //! \brief Login name field.
  static const Field LoginNameField;

  /*! \brief Constructor.
   */
  FormBaseModel(const AuthService& baseAuth, AbstractUserDatabase& users);

  /*! \brief Returns the authentication base service.
   *
   * This returns the service passed through the constructor.
   */
  const AuthService *baseAuth() const { return &baseAuth_; }

  /*! \brief Returns the user database.
   */
  AbstractUserDatabase& users() { return users_; }

  /*! \brief Adds a password authentication service.
   *
   * This enables password-based registration, including choosing a proper
   * password.
   *
   * Only one password authentication service can be configured.
   *
   * \sa addOAuth()
   * \sa AbstractPasswordService::validatePassword()
   */
  virtual void addPasswordAuth(const AbstractPasswordService *auth);

  /*! \brief Returns the password authentication service.
   *
   * \sa addPasswordAuth()
   */
  const AbstractPasswordService *passwordAuth() const { return passwordAuth_; }

  /*! \brief Adds an OAuth authentication service provider.
   *
   * This enables OAuth-based registration. More than one OAuth
   * authentication service can be configured: one for each supported
   * third-party OAuth identity provider.
   *
   * \sa addPasswordAuth()
   */
  virtual void addOAuth(const OAuthService *auth);

  /*! \brief Adds a list of OAuth authentication service providers.
   *
   * \sa addOAuth()
   */
  virtual void addOAuth(const std::vector<const OAuthService *>& auth);

  /*! \brief Returns the list of OAuth authentication service providers.
   *
   * \sa addOAuth()
   */
  std::vector<const OAuthService *> oAuth() const { return oAuth_; }

  virtual WString label(Field field) const override;

  /*! \brief Logs the user in.
   *
   * Logs in the user, after checking whether the user can actually be
   * logged in. A valid user may be refused to login if its account is
   * disabled (see User::status()) or if it's email address is unconfirmed
   * and email confirmation is required.
   *
   * Returns whether the user could be logged in.
   */
  virtual bool loginUser(Login& login, User& user, 
			 LoginState state = LoginState::Strong);

protected:
  void setValid(Field field);
  void setValid(Field field, const WString& message);

private:
  const AuthService& baseAuth_;
  AbstractUserDatabase& users_;

  const AbstractPasswordService *passwordAuth_;
  std::vector<const OAuthService *> oAuth_;
};

  }
}

#endif // WT_AUTH_FORM_BASE_MODEL_H_
