// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SERVICES_H_
#define SERVICES_H_

#include "ControlsWidget.h"

class Services : public ControlsWidget
{
public:
  Services(EventDisplayer *ed);

  void populateSubMenu(Wt::WMenu *menu);

private:
  Wt::WWidget *localization();
  Wt::WWidget *auth();
  Wt::WWidget *dbo();
  Wt::WWidget *payment();
  Wt::WWidget *mail();
  Wt::WWidget *http();
  Wt::WWidget *json();
  Wt::WWidget *render();
};

#endif // SERVICES_H_
