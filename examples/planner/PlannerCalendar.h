// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef PLANNER_CALENDAR_H_
#define PLANNER_CALENDAR_H_

#include "UserAccount.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WCalendar.h>

using namespace Wt;

class PlannerCalendar : public WCalendar
{
public: 
  PlannerCalendar(dbo::ptr<UserAccount> user);
protected:
  virtual WWidget* renderCell(WWidget* widget, const WDate& date) override;
 
private:
  dbo::ptr<UserAccount> user_;
};

#endif //PLANNER_CALENDAR_H_
