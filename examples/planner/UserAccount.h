/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef USER_ACCOUNT_H_
#define USER_ACCOUNT_H_

#include <Wt/Dbo/Dbo>
#include <Wt/WDate>
#include <string>

class Entry;

namespace dbo = Wt::Dbo;

class UserAccount {
 public:
  dbo::collection< dbo::ptr<Entry> > entries;
  std::string name;
  
  template<class Action>
  void persist(Action& a)
  {
    dbo::hasMany(a, entries, dbo::ManyToOne, "entry");
    dbo::field(a, name, "name");
  }
  
  static dbo::collection< dbo::ptr<Entry> > 
    getEntries(dbo::Session& session, 
	       dbo::ptr<UserAccount> user, 
	       const Wt::WDate& from, const Wt::WDate& untill);

  static dbo::ptr<UserAccount> login(dbo::Session& session, 
				     const std::string& user);
};

#endif //USER_ACCOUNT_H_
