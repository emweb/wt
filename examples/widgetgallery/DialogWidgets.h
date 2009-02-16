// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#ifndef DIALOGWIDGETS_H_
#define DIALOGWIDGETS_H_

#include <Wt/WMessageBox>

#include "ControlsWidget.h"

namespace Wt {
  namespace Ext {
    class Dialog;
  }
}

class DialogWidgets : public ControlsWidget
{
public:
  DialogWidgets(EventDisplayer *ed);

  void populateSubMenu(Wt::WMenu *menu);

private:
  Wt::WWidget *wDialog();
  Wt::WWidget *wMessageBox();
  Wt::WWidget *eDialogs();

  void messageBox1();
  void messageBox2();
  void messageBox3();
  void messageBox4();
  void custom();

  void messageBoxDone(Wt::StandardButton result);

  void setStatus(const Wt::WString& text);

  Wt::WMessageBox *messageBox_;
  Wt::WText *status_;

  void createExtDialog();
  void createExtMessageBox();
  void createExtProgress();
  void deleteExtDialog();
  Wt::Ext::Dialog *extDialog_;

};

#endif
