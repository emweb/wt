// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef ENTRY_H_
#define ENTRY_H_

#include <Wt/WDateTime>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/WtSqlTraits>

class UserAccount;

namespace dbo = Wt::Dbo;

class Entry {
public:
  dbo::ptr<UserAccount> user;

  Wt::WDateTime start;
  Wt::WDateTime stop;
  Wt::WString summary;
  Wt::WString text;

  template<class Action>
  void persist(Action& a)
  {
    dbo::belongsTo(a, user, "user");

    dbo::field(a, start, "start");
    dbo::field(a, stop, "stop");
    dbo::field(a, summary, "summary");
    dbo::field(a, text, "text");
  }
};

#endif //ENTRY_H_
