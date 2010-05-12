// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CALENDAR_CELL_H_
#define CALENDAR_CELL_H_

#include "UserAccount.h"

#include <Wt/WContainerWidget>

class CalendarCell : public Wt::WContainerWidget
{
public:
  CalendarCell();

  void update(const dbo::ptr<UserAccount>& user, const Wt::WDate& date);
  
  Wt::WDate date() {return date_; }
  dbo::ptr<UserAccount> user() { return user_; }  

private:
  Wt::WDate date_;
  dbo::ptr<UserAccount> user_;

  void showEntryDialog();
  void showAllEntriesDialog();
};

#endif //CALENDAR_CELL_H_
