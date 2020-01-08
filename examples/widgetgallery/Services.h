// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium
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

  void populateSubMenu(WMenu *menu);

private:
  std::unique_ptr<WWidget> localization();
  std::unique_ptr<WWidget> auth();
  std::unique_ptr<WWidget> dbo();
  std::unique_ptr<WWidget> payment();
  std::unique_ptr<WWidget> mail();
  std::unique_ptr<WWidget> http();
  std::unique_ptr<WWidget> json();
  std::unique_ptr<WWidget> render();
};

#endif // SERVICES_H_
