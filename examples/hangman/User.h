// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef USER_H_
#define USER_H_

#include <Wt/WDateTime.h>
#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Auth/Dbo/AuthInfo.h>

#include <string>

namespace dbo = Wt::Dbo;

class User;
using AuthInfo = Wt::Auth::Dbo::AuthInfo<User>;
using Users = dbo::collection<dbo::ptr<User>>;

class User
{
public:
  std::string name; /* a copy of auth info's user name */
  int gamesPlayed = 0;
  long long score = 0;
  Wt::WDateTime lastGame;
  dbo::weak_ptr<AuthInfo> authInfo;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, gamesPlayed, "gamesPlayed");
    dbo::field(a, score, "score");
    dbo::field(a, lastGame, "lastGame");
    dbo::hasOne(a, authInfo, "user");
  }
};

DBO_EXTERN_TEMPLATES(User)

#endif // USER_H_
