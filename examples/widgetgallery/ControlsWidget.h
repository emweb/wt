// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#ifndef CONTROLS_WIDGET_H_
#define CONTROLS_WIDGET_H_

#include <Wt/WContainerWidget>
#include <Wt/WMenu>
#include <string>

#include "ControlsWidget.h"

class EventDisplayer;

class ControlsWidget : public Wt::WContainerWidget
{
public:
  ControlsWidget(EventDisplayer *ed, bool hasSubMenu);

  bool hasSubMenu() const { return hasSubMenu_; }

  virtual void populateSubMenu(Wt::WMenu *menu);

  // Inserts the classname in the parent, with a link to the
  // documentation
  void topic(const std::string &classname, WContainerWidget *parent) const;
  void topic(const std::string &classname1,
	     const std::string &classname2,
	     WContainerWidget *parent) const;
  void topic(const std::string &classname1,
	     const std::string &classname2,
	     const std::string &classname3,
	     WContainerWidget *parent) const;
  void topic(const std::string &classname1,
	     const std::string &classname2,
	     const std::string &classname3,
	     const std::string &classname4,
	     WContainerWidget *parent) const;

protected:
  EventDisplayer *ed_;

private:
  bool hasSubMenu_;

  std::string docAnchor(const std::string &classname) const;
  std::string title(const std::string &classname) const;
  std::string escape(const std::string &name) const;
};

#endif // CONTROLS_WIDGET_H_
