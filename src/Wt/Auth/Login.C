/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Login.h"

namespace Wt {
  namespace Auth {

Login::Login()
  : state_(LoginState::LoggedOut)
{ }

void Login::login(const User& user, LoginState state)
{
  if (state == LoginState::LoggedOut || !user.isValid()) {
    logout();
    return;
  } else {
    if (state != LoginState::Disabled && user.status() == AccountStatus::Disabled)
      state = LoginState::Disabled;

    if (user != user_) {
      user_ = user;
      state_ = state;
      changed_.emit();
    } else if (state != state_) {
      state_ = state;
      changed_.emit();
    }
  }
}

void Login::logout()
{
  if (user_.isValid()) {
    user_ = User();
    state_ = LoginState::LoggedOut;
    changed_.emit();
  }
}

LoginState Login::state() const
{
  return state_;
}

bool Login::loggedIn() const
{
  return user_.isValid() && state_ != LoginState::Disabled;
}

const User& Login::user() const
{
  return user_;
}

  }
}
