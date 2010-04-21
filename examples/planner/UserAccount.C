/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "UserAccount.h"
#include "Entry.h"

#include <Wt/Dbo/WtSqlTraits>

using namespace Wt;
using namespace Wt::Dbo;

collection< ptr<Entry> > UserAccount::getEntries(Session& session, 
						 ptr<UserAccount> user,
						 const WDate& from, 
						 const WDate& untill) 
{
   collection< ptr<Entry> > entries = 
    session.find<Entry>("where start >= ? and start < ? and user_id = ?")
      .bind(from)
      .bind(untill)
      .bind(user.id());

    return entries;
}

ptr<UserAccount> UserAccount::login(Session& session, 
				    const std::string& userName)
{
  Transaction transaction(session);

  ptr<UserAccount> ua = 
    session.find<UserAccount>("where name = ?").bind(userName);
  
  if (!ua) {
    std::cerr << "create user" << std::endl;
    ua = session.add(new UserAccount());
    ua.modify()->name = userName;
  } else {
    std::cerr << "user exists" << std::endl;
  }
 
  transaction.commit();
  
  return ua;
}
