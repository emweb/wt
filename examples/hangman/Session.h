// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SESSION_H_
#define SESSION_H_

#include <vector>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Sqlite3>

#include "User.h"
#include "Dictionary.h"

class Session
{
public:
  Session();

  const User* user() { 
    if (user_)
      return &(*user_);
    else 
      return 0;
  } 

  Dictionary& dictionary() { return dictionary_; } 
  void setDictionary(const Dictionary& d) { dictionary_ = d;} 

  bool login(std::string name, std::string password);
  std::vector<User> topUsers(int limit);
  int findRanking(const User* user);
  void addToScore(int s);

private:
  Wt::Dbo::backend::Sqlite3 sqlite3_;

  Wt::Dbo::Session   session_;
  Wt::Dbo::ptr<User> user_;
  Dictionary         dictionary_;
};

#endif //SESSION_H_
