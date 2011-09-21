// This may look like C code, but it's really -*- C++ -*-
/* 
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef USER_H_
#define USER_H_

#include <Wt/WDateTime>
#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/WtSqlTraits>

#include <string>

class User : public Wt::Dbo::ptr<User> {
public:
  std::string   name;
  int           gamesPlayed;
  long long     score;
  Wt::WDateTime lastLogin;

  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::field(a, name,        "name");
    Wt::Dbo::field(a, password_,   "password");
    Wt::Dbo::field(a, gamesPlayed, "gamesPlayed");
    Wt::Dbo::field(a, score,       "score");
    Wt::Dbo::field(a, lastLogin,   "lastLogin");
  }
  
  User() {}

  void setPassword(const std::string& password);
  bool authenticate(const std::string& password) const;

  User(const std::string &name, const std::string &password);

private:
  std::string password_;
};

typedef Wt::Dbo::collection< Wt::Dbo::ptr<User> > Users;

#endif //USER_H_
