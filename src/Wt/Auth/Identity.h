// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_IDENTITY_H_
#define WT_AUTH_IDENTITY_H_

#include <Wt/WString.h>

namespace Wt {
  namespace Auth {

/*! \class Identity Auth/Identity
 *  \brief A class that represents a user identity.
 *
 * The identity is the result of an authentication process. Although
 * the most common authentication method (password authentication)
 * only returns a user name, other methods (such as %OAuth, client SSL
 * certificates, or an authentication reverse proxy server) may return
 * more information.
 *
 * At the very least, the user is identified using a unique ID, and it
 * optionally also contains name and email address information.
 *
 * \sa OAuthService::getIdentity(), RegistrationWidget::registerIdentified()
 *
 * \ingroup auth
 */
class WT_API Identity 
{
public:
  /*! \brief Default constructor.
   *
   * Creates an invalid identity.
   */
  Identity();

  /*! \brief Constructor.
   */
  Identity(const std::string& provider,
	   const std::string& id, const WT_USTRING& name,
	   const std::string& email, bool emailVerified);

  /*! \brief Returns whether the identity is valid.
   *
   * An invalid identity is used to indicate for example that no
   * identity information could be obtained.
   */
  bool isValid() const { return !id_.empty(); }

  /*! \brief Returns the provider name.
   *
   * This is a unique id that names the source for this identity (e.g.
   * "google-oauth", or "LDAP", or "user" (for a user-chosen
   * identity).
   */
  const std::string& provider() const { return provider_; }

  /*! \brief Returns the id.
   *
   * Returns a unique identifier for the user within the scope of this
   * provider.
   */
  const std::string& id() const { return id_; };

  /*! \brief Returns the name.
   *
   * Returns the user's name, or an empty string if not provided.
   */
  const WT_USTRING& name() const { return name_; }

  /*! \brief Returns an email address.
   *
   * Returns the user's email address, or an empty string if not provided.
   *
   * \sa emailVerified()
   */
  const std::string& email() const { return email_; }

  /*! \brief Returns whether the email address has been verified.
   *
   * The third party provider may be able to guarantee that the user indeed
   * also control's the given email address (e.g. because the third party
   * hosts that email account for the user).
   *
   * \sa email()
   */
  bool emailVerified() const { return emailVerified_; }

  /*! \brief An invalid identity constant.
   *
   * This is an identity that is not isValid().
   */
  static const Identity Invalid;

  /*! \brief The login name identity.
   *
   * This is a provider name for the (usually user-controlled) identity,
   * used for example for password-based authentication.
   */
  static const std::string LoginName;

private:
  std::string provider_, id_, email_;
  WT_USTRING name_;
  bool emailVerified_;
};

  }
}

#endif // WT_AUTH_IDENTITY_H_
