/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/*****
 * This file is part of the Wt::Dbo tutorial:
 * http://www.webtoolkit.eu/wt/doc/tutorial/dbo/tutorial.html
 *****/

/*****
 * Dbo tutorial section 7.4
 *  Specifying a composite natural primary key (with foreign keys)
 *****/

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>

namespace dbo = Wt::Dbo;

class Membership;
class Organisation;
class Person;

struct MembershipId {
  dbo::ptr<Person>       person;
  dbo::ptr<Organisation> organisation;

  MembershipId() { }

  MembershipId(dbo::ptr<Person> p, dbo::ptr<Organisation> o)
    : person(p),
      organisation(o) 
  { }

  bool operator== (const MembershipId& other) const {
    return person == other.person && organisation == other.organisation;
  }

  bool operator< (const MembershipId& other) const {
    if (person < other.person)
      return true;
    else if (person == other.person)
      return organisation < other.organisation;
    else
      return false;
  }
};

std::ostream& operator<< (std::ostream& o, const MembershipId& mid)
{
  return o << "(" << mid.person << ", " << mid.organisation << ")";
}

namespace Wt {
  namespace Dbo {

    template <class Action>
    void field(Action& action, MembershipId& mid, const std::string& name,
	       int /*size*/ = -1)
    {
      /*
       * Note: here we ignore name because MembershipId is used only
       * as primary key of membership, and the name needs to match the
       * names of the reciproce collection in Person and Organisation.
       */
      belongsTo(action, mid.person, "person");
      belongsTo(action, mid.organisation, "organisation");
    }
  }
}

namespace Wt {
  namespace Dbo {

    template<>
    struct dbo_traits<Membership> : public dbo_default_traits
    {
      typedef MembershipId IdType;
      static IdType invalidId() { return MembershipId(); }
      static const char *surrogateIdField() { return 0; }
    };
  }
}

class Membership {
public:
  MembershipId id;
  int karma;

  template<class Action>
  void persist(Action& a)
  {
    dbo::id(a, id, "id");
    dbo::field(a, karma, "karma");
  }
};

class Person {
public:
  std::string name;

  dbo::collection< dbo::ptr<Membership> > memberships;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, name, "name");
    dbo::hasMany(a, memberships, dbo::ManyToOne, "person");
  }
};

class Organisation {
public:
  std::string name;

  dbo::collection< dbo::ptr<Membership> > memberships;

  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, name, "name");
    dbo::hasMany(a, memberships, dbo::ManyToOne, "organisation");
  }
};

void run()
{
  /*
   * Setup a session, would typically be done once at application startup.
   */
  std::unique_ptr<dbo::backend::Sqlite3> sqlite3(new dbo::backend::Sqlite3(":memory:"));
  sqlite3->setProperty("show-queries", "true");
  dbo::Session session;
  session.setConnection(std::move(sqlite3));

  session.mapClass<Membership>("membership");
  session.mapClass<Person>("person");
  session.mapClass<Organisation>("organisation");

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

    std::cerr << "Joe is member of " << joe->memberships.size()
	      << " organisation(s):" << std::endl;
    for (dbo::collection< dbo::ptr<Membership> >::const_iterator i = 
	   joe->memberships.begin(); i != joe->memberships.end(); ++i) {
      const Membership& ms = **i;
      std::cerr << " " << ms.id.organisation->name
		<< " (karma: " << ms.karma << ")" << std::endl;
    }
  }
}

int main(int argc, char **argv)
{
  run();
}
