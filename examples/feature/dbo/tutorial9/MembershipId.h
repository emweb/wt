// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef MEMBERSHIPID_H
#define	MEMBERSHIPID_H

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>

class Person;
class Organisation;
class Membership;

struct MembershipId
{
  Wt::Dbo::ptr<Person> person;
  Wt::Dbo::ptr<Organisation> organisation;
  
  MembershipId()
  {
  }
  
  MembershipId(Wt::Dbo::ptr<Person> p, Wt::Dbo::ptr<Organisation> o)
  : person(p),
    organisation(o)
  {
  }
  
  bool operator==(const MembershipId & other) const
  {
    return person == other.person && organisation == other.organisation;
  }
  
  bool operator<(const MembershipId & other) const
  {
    if (person < other.person)
      return true;
    else if (person == other.person)
      return organisation < other.organisation;
    else
      return false;
  }
};


inline std::ostream& operator<<(std::ostream& o, const MembershipId& mid)
{
  return o << "(" << mid.person << ", " << mid.organisation << ")";
}

namespace Wt
{
  namespace Dbo
  {
    
    template<>
    struct dbo_traits<Membership> : public dbo_default_traits
    {
      typedef MembershipId IdType;
      
      static IdType invalidId()
      {
	return MembershipId();
      }
      
      static const char *surrogateIdField()
      {
	return 0;
      }
    };

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

#endif	/* MEMBERSHIPID_H */
