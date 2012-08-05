/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Login"

namespace Wt {
  namespace Auth {

Login::Login()
   : changed_(this)
{ }

void Login::login(const User& user, LoginState state)
{
  bool weakLogin = state == WeakLogin;

  if (user != user_) {
    user_ = user;
    weakLogin_ = weakLogin;

    changed_.emit();
  } else if (user_.isValid() && weakLogin != weakLogin_) {
    weakLogin_ = weakLogin;

    changed_.emit();
  }
}

void Login::logout()
{
  if (user_.isValid()) {
    user_ = User();

    changed_.emit();
  }
}

LoginState Login::state() const
{
  if (user_.isValid()) {
    if (user_.status() == User::Normal)
      if (weakLogin_)
	return WeakLogin;
      else
	return StrongLogin;
    else
      return DisabledLogin;
  } else
    return LoggedOut;
}

bool Login::loggedIn() const
{
  return user_.isValid() && user_.status() == User::Normal;
}

const User& Login::user() const
{
  return user_;
}

  }
}
