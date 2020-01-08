// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef ENTRY_H_
#define ENTRY_H_

#include <Wt/WDateTime.h>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

using namespace Wt;

class UserAccount;

namespace dbo = Dbo;

class Entry {
public:
  dbo::ptr<UserAccount> user;

  WDateTime start;
  WDateTime stop;
  WString summary;
  WString text;

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
