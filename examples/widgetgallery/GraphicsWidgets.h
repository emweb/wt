// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#ifndef GRAPHICSWIDGETS_H_
#define GRAPHICSWIDGETS_H_

#include "ControlsWidget.h"

#include "Wt/WTable"
#include "Wt/WColor"
#include "Wt/WSignalMapper"

class GraphicsWidgets : public ControlsWidget
{
public:
  GraphicsWidgets(EventDisplayer *ed);
  virtual ~GraphicsWidgets();
  virtual void populateSubMenu(Wt::WMenu *menu);
  
private:
  Wt::WSignalMapper<Wt::WColor > * colorMapper_;

  Wt::WWidget* emwebLogo();
  Wt::WWidget* paintbrush();
  void addColor(Wt::WTable* table, 
		int row, 
		int column, 
		const Wt::WColor& color);
};

#endif
