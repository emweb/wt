// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CALENDAR_CELL_H_
#define CALENDAR_CELL_H_

#include "UserAccount.h"

#include <Wt/WContainerWidget.h>

using namespace Wt;

class CalendarCell : public WContainerWidget
{
public:
  CalendarCell();

  void update(const dbo::ptr<UserAccount>& user, const WDate& date);
  
  WDate date() {return date_; }
  dbo::ptr<UserAccount> user() { return user_; }  

private:
  WDate date_;
  dbo::ptr<UserAccount> user_;
  std::unique_ptr<WDialog> dialog_;

  void showEntryDialog();
  void showAllEntriesDialog();
};

#endif //CALENDAR_CELL_H_
