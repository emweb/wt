// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef PLANNER_CALENDAR_H_
#define PLANNER_CALENDAR_H_

#include "UserAccount.h"

#include <Wt/Dbo/Dbo>
#include <Wt/WCalendar>

class PlannerCalendar : public Wt::WCalendar
{
public: 
  PlannerCalendar(Wt::WContainerWidget* parent, 
		  dbo::ptr<UserAccount> user);
protected:
  virtual Wt::WWidget* renderCell(Wt::WWidget* widget, const Wt::WDate& date);
 
private:
  dbo::ptr<UserAccount> user_;
};

#endif //PLANNER_CALENDAR_H_
