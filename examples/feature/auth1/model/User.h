// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef USER_H_
#define USER_H_

#include <Wt/Dbo/Types>

namespace dbo = Wt::Dbo;

namespace Wt {
  namespace Auth {
    namespace Dbo {
      template<class UserType> class AuthInfo;
    }
  }
}

class User;
typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;

class User {
public:
  User();

  enum Role {
    Visitor = 0,
    Admin = 1
  };

  Role role;
  std::string colorPreference;

  dbo::collection< dbo::ptr<AuthInfo> > authInfo;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, role, "role");
    dbo::field(a, colorPreference, "color_preference");

    dbo::hasMany(a, authInfo, dbo::ManyToOne, "user");
  }
};

DBO_EXTERN_TEMPLATES(User);

#endif // USER_H_
