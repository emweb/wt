// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef USER_H_
#define USER_H_

#include <Wt/Dbo/Types.h>
#include <Wt/WDate.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WGlobal.h>
#include <Wt/Auth/Dbo/AuthInfo.h>

namespace dbo = Wt::Dbo;

class User;
typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;

class User {
public:
  /* You probably want to add other user information here, e.g. */
  std::string favouritePet;
  dbo::weak_ptr<AuthInfo> authInfo;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, favouritePet, "favourite_pet");
    dbo::hasOne(a, authInfo, "user");
  }
};


DBO_EXTERN_TEMPLATES(User);

#endif // USER_H_
