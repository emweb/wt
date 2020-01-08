/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>

#include "Person.h"
#include "Membership.h"
#include "Organisation.h"
#include "MembershipId.h"

namespace dbo = Wt::Dbo;

class Membership;
class Organisation;
class Person;

void run()
{
  /*
   * Setup a session, would typically be done once at application startup.
   */
  std::unique_ptr<dbo::backend::Sqlite3> sqlite3(new dbo::backend::Sqlite3(":memory:"));
  sqlite3->setProperty("show-queries", "true");
  dbo::Session session;
  session.setConnection(std::move(sqlite3));
  
  session.mapClass<Membership > ("membership");
  session.mapClass<Person > ("person");
  session.mapClass<Organisation > ("organisation");
  
  /*
   * Try to create the schema (will fail if already exists).
   */
  session.createTables();
  
  {
    dbo::Transaction transaction(session);
    
    std::unique_ptr<Person> p{new Person()};
    p->name = "Joe";
    dbo::ptr<Person> joe = session.add(std::move(p));
    
    std::unique_ptr<Organisation> o{new Organisation()};
    o->name = "Police";
    dbo::ptr<Organisation> police = session.add(std::move(o));
    
    std::unique_ptr<Membership> ms{new Membership()};
    ms->id.person = joe;
    ms->id.organisation = police;
    ms->karma = 42;
    
    session.add(std::move(ms));
  }
}

int main(int argc, char **argv)
{
  run();
}
