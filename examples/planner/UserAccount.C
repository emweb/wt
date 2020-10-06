/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "UserAccount.h"
#include "Entry.h"

#include <Wt/WApplication.h>
#include <Wt/WLogger.h>

#include <Wt/Dbo/WtSqlTraits.h>

using namespace Wt::Dbo;

UserAccount::UserAccount()
{ }

UserAccount::UserAccount(const WString& aName)
  : name(aName)
{ }

collection< ptr<Entry> > UserAccount::entriesInRange(const WDate& from, 
						     const WDate& until) const
{
  return entries.find()
    .where("start >= ?").bind(WDateTime(from))
    .where("start < ?").bind(WDateTime(until));
}

ptr<UserAccount> UserAccount::login(Session& session, 
				    const WString& userName)
{
  Transaction transaction(session);

  ptr<UserAccount> ua = 
    session.find<UserAccount>("where name = ?").bind(userName);

  if (!ua) {
    WApplication::instance()
      ->log("notice") << "Creating user: " << userName.toUTF8();

    ua = session.add(std::make_unique<UserAccount>(userName));
  }

  transaction.commit(); 
 
  return ua;
}
