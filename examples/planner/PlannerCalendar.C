/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PlannerCalendar.h"
#include "CalendarCell.h"
#include "Entry.h"

using namespace Wt;

PlannerCalendar::PlannerCalendar(WContainerWidget* parent, 
				 dbo::ptr<UserAccount> user)
  : WCalendar(parent),
    user_(user)
{
  setStyleClass(styleClass() + " calendar");
  
  setSelectionMode(NoSelection);
}

WWidget* PlannerCalendar::renderCell(WWidget* widget, const WDate& date)
{
  if (!widget) 
    widget = new CalendarCell();
		
  CalendarCell* cc = (CalendarCell*)widget;
  cc->update(user_, date);
		
  return cc;
}
