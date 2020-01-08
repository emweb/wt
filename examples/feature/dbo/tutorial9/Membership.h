// This may look like C code, but it's really -*- C++ -*-
/*
 *  Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *  
 *  See the LICENSE file for terms of use.
 */

#ifndef MEMBERSHIP_H
#define	MEMBERSHIP_H

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>

#include "MembershipId.h"

class Membership
{
public:
  MembershipId id;
  int karma;
  
  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::id(a, id, "id");
    Wt::Dbo::field(a, karma, "karma");
  }
};

#endif	/* MEMBERSHIP_H */

