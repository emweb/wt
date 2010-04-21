/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CALENDAR_CELL_H_
#define CALENDAR_CELL_H_

#include "UserAccount.h"

#include <Wt/WContainerWidget>

class CalendarCell : public Wt::WContainerWidget {
 public:
  CalendarCell();
  void update(const Wt::Dbo::ptr<UserAccount>& user, const Wt::WDate& date);
  
  Wt::Dbo::ptr<UserAccount> user() {
    return user_;
  }
  
  Wt::WDate date() {
    return date_;
  }

 private:
  void showEntryDialog();
  void showAllEntriesDialog();

 private:
  Wt::WDate date_;
  Wt::Dbo::ptr<UserAccount> user_;
};

#endif //CALENDAR_CELL_H_
