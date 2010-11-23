// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef FORMWIDGETS_H_
#define FORMWIDGETS_H_

#include "ControlsWidget.h"

#include "Wt/WStandardItemModel"

class EventDisplayer;

class FormWidgets: public ControlsWidget
{
public:
  FormWidgets(EventDisplayer *ed);

  void populateSubMenu(Wt::WMenu *menu);

private:
  Wt::WWidget *wPushButton();
  Wt::WWidget *wCheckBox();
  Wt::WWidget *wRadioButton();
  Wt::WWidget *wComboBox();
  Wt::WWidget *wSelectionBox();
  Wt::WWidget *wLineEdit();
  Wt::WWidget *wSpinBox();
  Wt::WWidget *wTextArea();
  Wt::WWidget *wCalendar();
  Wt::WWidget *wDatePicker();
  Wt::WWidget *wInPlaceEdit();
  Wt::WWidget *wSuggestionPopup();
  Wt::WWidget *wTextEdit();
  Wt::WWidget *wFileUpload();
#ifndef WT_TARGET_JAVA
  Wt::WWidget *wPopupMenu();
#endif

  void addColorElement(Wt::WStandardItemModel* model,
		       std::string name, 
		       std::string style);
};

#endif
