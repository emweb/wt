/* this is a -*-C++-*- file
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
#include <vector>

namespace dbo = Wt::Dbo;

class User {
public:
  std::string     name;
  std::string     password;
  int             gamesPlayed;
  long long       score;
  Wt::WDateTime   lastLogin;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, name,        "name");
    dbo::field(a, password,    "password");
    dbo::field(a, gamesPlayed, "gamesPlayed");
    dbo::field(a, score,       "score");
    dbo::field(a, lastLogin,   "lastLogin");
  }
  
  User() {}

  User(const std::string &name, const std::string &password);

  int findRanking(dbo::Session& session) const;
};

typedef dbo::collection< dbo::ptr<User> > Users;

#endif //USER_H_
