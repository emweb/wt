// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef USER_ACCOUNT_H_
#define USER_ACCOUNT_H_

#include <Wt/WDate>
#include <Wt/WString>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/WtSqlTraits>

class Entry;

namespace dbo = Wt::Dbo;

class UserAccount
{
public:
  Wt::WString name;
  dbo::collection< dbo::ptr<Entry> > entries;
  
  UserAccount();
  UserAccount(const Wt::WString& name);

  dbo::collection< dbo::ptr<Entry> >
    entriesInRange(const Wt::WDate& from, const Wt::WDate& until) const;

  static dbo::ptr<UserAccount> login(dbo::Session& session, 
				     const Wt::WString& user);
  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, name, "name");
    dbo::hasMany(a, entries, dbo::ManyToOne, "user");
  }
};

#endif // USER_ACCOUNT_H_
