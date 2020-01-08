// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef USER_H_
#define USER_H_

#include <Wt/Dbo/Types.h>
#include <Wt/WGlobal.h>

namespace dbo = Wt::Dbo;

class User;
using AuthInfo = Wt::Auth::Dbo::AuthInfo<User>;

class User {
public:
  /* You probably want to add other user information here */

  template<class Action>
  void persist(Action& a)
  {
  }
};


DBO_EXTERN_TEMPLATES(User)

#endif // USER_H_
