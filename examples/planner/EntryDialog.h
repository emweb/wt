/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef ENTRY_DIALOG_H_
#define ENTRY_DIALOG_H_

#include <Wt/WDialog.h>
#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTextArea.h>

#include "CalendarCell.h"

using namespace Wt;

class EntryDialog : public WDialog {
 public:
  EntryDialog(const WString& title, CalendarCell* cell);

 private:
  WDateTime timeStamp(const WString& time, const WDate& day);
  WString description();
  void ok();
  void cancel();

 public:
  static WString timeFormat;

 private:
  CalendarCell* cell_;

  WLineEdit* summary_;
  WLineEdit* start_;
  WLineEdit* stop_;
  WTextArea* description_;
};

#endif //ENTRY_DIALOG_H_
