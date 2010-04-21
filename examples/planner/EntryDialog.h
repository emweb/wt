/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef ENTRY_DIALOG_H_
#define ENTRY_DIALOG_H_

#include <Wt/WDialog>
#include <Wt/WDate>
#include <Wt/WDateTime>
#include <Wt/WLineEdit>
#include <Wt/WTextArea>

#include "CalendarCell.h"

class EntryDialog : public Wt::WDialog {
 public:
  EntryDialog(const Wt::WString& title, CalendarCell* cell);

 private:
  Wt::WDateTime timeStamp(const Wt::WString& time, const Wt::WDate& day);
  Wt::WString description();
  void ok();
  void cancel();

 public:
  static Wt::WString timeFormat;

 private:
  CalendarCell* cell_;

  Wt::WLineEdit* summary_;
  Wt::WLineEdit* start_;
  Wt::WLineEdit* stop_;
  Wt::WTextArea* description_;
};

#endif //ENTRY_DIALOG_H_
