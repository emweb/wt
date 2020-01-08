// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_LOGIN_H_
#define WT_AUTH_LOGIN_H_

#include <Wt/WObject.h>
#include <Wt/WSignal.h>
#include <Wt/Auth/User.h>

namespace Wt {
  namespace Auth {

class User;

/*! \brief Enumeration for a login state.
 *
 * \sa Login::state()
 *
 * \ingroup auth
 */
enum class LoginState {
  /*! \brief No user is currently identified.
   */
  LoggedOut,

  /*! \brief The identified user was refused to login.
   *
   * This is caused by for example User::status() returning
   * AccountStatus::Disabled, or if email verification is required but the
   * email hasn't been verified yet.
   */
  Disabled,

  /*! \brief A user is weakly authenticated.
   *
   * The authentication method was weak, typically this means that a secondary
   * authentication system was used (e.g. an authentication cookie) instead
   * of a primary mechanism (like a password).
   *
   * You may want to allow certain operations, but request to authenticate
   * fully before more sensitive operations.
   */
  Weak,

  /*! \brief A user is strongly authenticated.
   */
  Strong
};

/*! \class Login Wt/Auth/Login.h
 *  \brief A class that manages the current login state.
 *
 * This is a model class which is typically associated with a single
 * session, for the duration of the session.
 *
 * Widgets that implement authentication (and thus produce
 * authentication changes), will indicate their result in this object
 * using the login() or logout() methods.
 *
 * Widgets that want to react to login state changes (typically, as
 * user logging in or out) should listen to the changed() signal of
 * this object.
 *
 * \sa AuthWidget
 *
 * \ingroup auth
 */
class WT_API Login : public WObject
{
public:
  /*! \brief Typedef for enum Wt::Auth::LoginState */
  typedef LoginState State;

  /*! \brief Default constructor.
   *
   * Creates a login object in the LoginState::LoggedOut state.
   */
  Login();

  /*! \brief Logs a user in.
   *
   * A user can be logged in using either a LoginState::Disabled, LoginState::Weak
   * or LoginState::Strong \p state. The login state is forced to LoginState::Disabled if
   * User::status() returns Disabled.
   *
   * \sa logout(), loggedIn()
   */
  void login(const User& user, LoginState state = LoginState::Strong);

  /*! \brief Logs the current user out.
   *
   * Sets the state to LoginState::LoggedOut.
   */
  void logout();

  /*! \brief Returns the current login state.
   *
   * \sa login(), logout()
   */
  LoginState state() const;

  /*! \brief Returns whether a user has successfully logged in.
   *
   * This returns \c true only if the state is LoginState::Weak or LoginState::Strong.
   *
   * \sa state()
   */
  bool loggedIn() const;

  /*! \brief Returns the user currently identified.
   *
   * Returns the user currently identified.
   *
   * \note This may also be a user whose account is currently disabled.
   */
  const User& user() const;

  /*! \brief %Signal that indicates login changes.
   *
   * This signal is emitted as a result of login() or logout(). If no
   * user was logged in, then a changed() signal does not necessarily
   * mean that user is loggedIn() as the user may have been identified
   * correctly but have a LoginState::Disabled state() for example.
   */
  Signal<>& changed() { return changed_; }

private:
  Signal<> changed_;

  User user_;
  LoginState state_;
};

  }
}

#endif // WT_AUTH_LOGIN
