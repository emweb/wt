// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef USER_ACCOUNT_H_
#define USER_ACCOUNT_H_

#include <Wt/WDate.h>
#include <Wt/WString.h>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/WtSqlTraits.h>

using namespace Wt;

class Entry;

namespace dbo = Wt::Dbo;

class UserAccount
{
public:
  WString name;
  dbo::collection< dbo::ptr<Entry> > entries;
  
  UserAccount();
  UserAccount(const WString& name);

  dbo::collection< dbo::ptr<Entry> >
    entriesInRange(const WDate& from, const WDate& until) const;

  static dbo::ptr<UserAccount> login(dbo::Session& session, 
                                     const WString& user);
  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, name, "name");
    dbo::hasMany(a, entries, dbo::ManyToOne, "user");
  }
};

#endif // USER_ACCOUNT_H_
