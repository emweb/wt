// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SPECIALPURPOSEWIDGETS_H_
#define SPECIALPURPOSEWIDGETS_H_

#include "ControlsWidget.h"

#include "Wt/WGoogleMap"

#include <vector>

class EventDisplayer;

class SpecialPurposeWidgets: public ControlsWidget
{
public:
  SpecialPurposeWidgets(EventDisplayer *ed);

  void populateSubMenu(Wt::WMenu *menu);

private:
  Wt::WWidget *wGoogleMap();
  Wt::WWidget *wMediaPlayer();
  Wt::WWidget *wSound();
  Wt::WWidget *wAudio();
  Wt::WWidget *wVideo();
  Wt::WWidget *wFlashObject();
};

#endif
