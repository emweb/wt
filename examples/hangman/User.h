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

using namespace Wt;

namespace dbo = Wt::Dbo;

class User;
typedef Auth::Dbo::AuthInfo<User> AuthInfo;
typedef dbo::collection< dbo::ptr<User> > Users;

class User
{
public:
  User();

  std::string name; /* a copy of auth info's user name */
  int gamesPlayed;
  long long score;
  WDateTime lastGame;
  dbo::collection<dbo::ptr<AuthInfo>> authInfos;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, gamesPlayed, "gamesPlayed");
    dbo::field(a, score, "score");
    dbo::field(a, lastGame, "lastGame");

    dbo::hasMany(a, authInfos, dbo::ManyToOne, "user");
  }
};

DBO_EXTERN_TEMPLATES(User);

#endif // USER_H_
