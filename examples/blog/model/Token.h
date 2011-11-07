// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef TOKEN_H_
#define TOKEN_H_

#include <Wt/WDate>

#include <Wt/Dbo/Types>
#include <Wt/Dbo/WtSqlTraits>

class User;

namespace dbo = Wt::Dbo;

class Token : public dbo::Dbo<Token> {
public:
  Token();
  Token(const std::string& value, const Wt::WDateTime& expires);

  dbo::ptr<User> user;

  std::string    value;
  Wt::WDateTime  expires;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, value,   "value");
    dbo::field(a, expires, "expires");

    dbo::belongsTo(a, user, "user");
  }
};

DBO_EXTERN_TEMPLATES(Token);

#endif // TOKEN_H_
