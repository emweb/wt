// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef ALL_ENTRIES_DIALOG_H_
#define ALL_ENTRIES_DIALOG_H_

#include <Wt/WDialog.h>
#include <Wt/WString.h>

#include "CalendarCell.h"

using namespace Wt;

class AllEntriesDialog : public WDialog
{
public:
  AllEntriesDialog(const WString& title, CalendarCell* cell);

private:
  void closeDialog();
};

#endif // ALL_ENTRIES_DIALOG_H_
