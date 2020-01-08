// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_REGISTRATION_MODEL_H_
#define WT_AUTH_REGISTRATION_MODEL_H_

#include <Wt/Auth/Identity.h>
#include <Wt/Auth/FormBaseModel.h>
#include <Wt/Auth/User.h>

namespace Wt {
  namespace Auth {

/*! \brief Enumeration for an email policy
 */
enum class EmailPolicy {
  Disabled,  //!< The email address is not asked for
  Optional,  //!< A user may optionally provide an email address
  Mandatory  //!< A user must provide an email address
};

/*! \brief Method for confirming to be an existing user.
 */
enum class IdentityConfirmationMethod {
  ConfirmWithPassword,    //!< Confirm using a password prompt
  ConfirmWithEmail,       //!< Confirm by using an email procedure
  ConfirmationNotPossible //!< Confirmation is not possible
};

class Login;

/*! \class RegistrationModel Wt/Auth/RegistrationModel.h
 *  \brief Model for implementing a registration view.
 *
 * This model implements the logic for the registration of a new user.
 * It can deal with traditional username/password registration, or
 * registration of pre-identified users using federated login.
 *
 * The model exposes four fields:
 * - LoginNameField: the login name (used as an identity for the
 *   Identity::LoginName provider) -- this can be an email if the
 *   AuthService is configured to use email addresses as identity
 * - ChoosePasswordField: the password
 * - RepeatPasswordField: the password (repeated)
 * - EmailField: if an email address is to be asked (and is not used
 *   as identity).
 *
 * The largest complexity is in the handling of third party identity
 * providers, which is initiated with a call to registerIdentified().
 *
 * When a user is re-identified with the same identity, then the model
 * may require that the (original) user confirms this new
 * identity. The model indicates that this button should be made
 * visible with isConfirmUserButtonVisible(), the action to take is
 * determined by confirmIsExistingUser(), and existingUserConfirmed()
 * ends this process by merging the new identity into the existing
 * user.
 *
 * \sa RegistrationWidget
 *
 * \ingroup auth
 */
class WT_API RegistrationModel : public FormBaseModel
{
public:
  //! \brief Choose Password field
  static const Field ChoosePasswordField;

  //! \brief Repeat password field
  static const Field RepeatPasswordField;

  //! \brief Email field (if login name is not email)
  static const Field EmailField;

  /*! \brief Constructor.
   *
   * Creates a new registration model, using a basic authentication
   * service and user database.
   *
   * The \p login object is used to indicate that an existing user was
   * re-identified, and thus the registration process may be aborted.
   */
  RegistrationModel(const AuthService& baseAuth, AbstractUserDatabase& users,
		    Login& login);

  /*! \brief Resets the model.
   *
   * This resets the model to initial values, clearing any entered information
   * (login name, password, pre-identified identity).
   */
  virtual void reset() override;

  /*! \brief Returns the login object.
   */
  Login& login() { return login_; }

  /*! \brief Configures a minimum length for a login name.
   *
   * The default value is 4. 
   */
  void setMinLoginNameLength(int chars) { minLoginNameLength_ = chars; }

  /*! \brief Returns the minimum length for a login name.
   *
   * \sa setMinLoginNameLength()
   */
  int minLoginNameLength() const { return minLoginNameLength_; }

  /*! \brief Configures whether an email address needs to be entered.
   *
   * You may specify whether you want the user to enter an email
   * address.
   *
   * This has no effect when the IdentityPolicy is
   * IdentityPolicy::EmailAddress.
   *
   * The default policy is:
   * - EmailOptional when email address verification is enabled
   * - EmailDisabled otherwise
   */
  void setEmailPolicy(EmailPolicy policy);

  /*! \brief Returns the email policy.
   *
   * \sa setEmailPolicy()
   */
  EmailPolicy emailPolicy() const { return emailPolicy_; }

  /*! \brief Register a user authenticated by an identity provider.
   *
   * Using a 3rd party authentication service such as %OAuth, a user
   * may be identified which is not yet registered with the web
   * application.
   *
   * Then, you may still need to allow the user to complete
   * registration, but because the user already is identified and
   * authenticated, this simplifies the registration form, since
   * fields related to authentication can be dropped.
   *
   * Returns \c true if the given identity was already registered, and
   * has been logged in.
   */
  virtual bool registerIdentified(const Identity& identity);

  /*! \brief Returns the existing user that needs to be confirmed.
   *
   * When a user wishes to register with an identity that corresponds
   * to an existing user, he may be allowd to confirm that he is in
   * fact this existing user.
   *
   * \sa confirmIsExistingUser()
   */
  User existingUser() const { return existingUser_; }

  /*! \brief Returns the method to be used to confirm to be an existing user.
   *
   * When the ConfirmExisting field is visible, this returns an
   * appropriate method to use to let the user confirm that he is
   * indeed the identified existing user.
   *
   * The outcome of this method (if it is an online method, like a
   * password prompt), if successful, should be indicated using
   * existingUserConfirmed().
   *
   * \sa existingUserConfirmed()
   */
  virtual IdentityConfirmationMethod confirmIsExistingUser() const;

  /*! \brief Confirms that the user is indeed an existing user.
   *
   * The new identity is added to this existing user (if applicable),
   * and the user is logged in.
   */
  virtual void existingUserConfirmed();

  /*! \brief Validates the login name.
   *
   * This verifies that the login name is adequate (see also
   * setMinLoginNameLength()).
   */
  virtual WString validateLoginName(const WT_USTRING& userName) const;

  /*! \brief Verifies that a user with that name does not yet exist.
   *
   * If a user with that name already exists, it may in fact be the
   * same user that is trying to register again (perhaps using a
   * different identification method). If possible, we allow the user
   * to confirm his identity.
   */
  virtual void checkUserExists(const WT_USTRING& userName);

  /*! \brief Performs the registration process.
   */
  virtual User doRegister();

  virtual bool isVisible(Field field) const override;
  virtual bool isReadOnly(Field field) const override;
  virtual bool validateField(Field field) override;

  /*! \brief Returns whether an existing user needs to be confirmed.
   *
   * This returns whether the user is being identified as an existing
   * user and he can confirm that he is in fact the same user.
   */
  virtual bool isConfirmUserButtonVisible() const;

  /*! \brief Returns whether federated login options can be shown.
   *
   * This returns whether fields for federated login (such as OAuth)
   * should be shown. These are typically buttons corresponding to
   * identity providers.
   *
   * The result of a federated authentication procedure should be
   * indicated to registerIdentified().
   */
  virtual bool isFederatedLoginVisible() const;

  static void validatePasswordsMatchJS(WLineEdit *password,
				       WLineEdit *password2,
				       WText *info2);

private:
  Login& login_;
  int minLoginNameLength_;
  EmailPolicy emailPolicy_;

  Identity idpIdentity_;
  User existingUser_;
};

  }
}

#endif // WT_AUTH_REGISTRATION_MODEL_H_
