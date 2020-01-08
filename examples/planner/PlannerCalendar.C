/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PlannerCalendar.h"
#include "CalendarCell.h"
#include "Entry.h"

PlannerCalendar::PlannerCalendar(dbo::ptr<UserAccount> user)
  : WCalendar(),
    user_(user)
{
  setStyleClass(styleClass() + " calendar");
  
  setSelectionMode(SelectionMode::None);
}

WWidget* PlannerCalendar::renderCell(WWidget* widget, const WDate& date)
{
  if (!widget) 
    widget = new CalendarCell();
		
  CalendarCell* cc = (CalendarCell*)widget;
  cc->update(user_, date);
		
  return cc;
}
