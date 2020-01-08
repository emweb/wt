// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef ORGANISATION_H
#define	ORGANISATION_H

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>

class Membership;

class Organisation
{
public:
  std::string name;

  Wt::Dbo::collection< Wt::Dbo::ptr<Membership> > memberships;
  
  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::field(a, name, "name");
    Wt::Dbo::hasMany(a, memberships, Wt::Dbo::ManyToOne, "organisation");
  }
};

#endif	/* ORGANISATION_H */

