// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#ifndef EXTWIDGETS_H_
#define EXTWIDGETS_H_

#include <string>

#include "ControlsWidget.h"

class ExtWidgets : public ControlsWidget
{
public:
  ExtWidgets(EventDisplayer *ed);

  void populateSubMenu(Wt::WMenu *menu);

private:
  Wt::WWidget *eButton();
  Wt::WWidget *eLineEdit();
  Wt::WWidget *eNumberField();
  Wt::WWidget *eCheckBox();
  Wt::WWidget *eComboBox();
  Wt::WWidget *eRadioButton();
  Wt::WWidget *eCalendar();
  Wt::WWidget *eDateField();
  Wt::WWidget *eMenu();
  Wt::WWidget *eDialog();
};

#endif
