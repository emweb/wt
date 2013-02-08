
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "DeferredWidget.h"
#include "Navigation.h"
#include "TopicTemplate.h"

#include <Wt/WText>

Navigation::Navigation()
  : TopicWidget()
{
  addText(tr("navigation-intro"), this);
}

void Navigation::populateSubMenu(Wt::WMenu *menu)
{
  menu->addItem("Internal paths", internalPaths())->setPathComponent("");
  menu->addItem("Anchor", 
		deferCreate(boost::bind
			    (&Navigation::anchor, this)));
  menu->addItem("Stacked widget", 
		deferCreate(boost::bind
			    (&Navigation::stackedWidget, this)));
  menu->addItem("Menu", 
		deferCreate(boost::bind
			    (&Navigation::menuWidget, this)));
  menu->addItem("Tab widget", 
		deferCreate(boost::bind
			    (&Navigation::tabWidget, this)));
  menu->addItem("Navigation bar", 
		deferCreate(boost::bind
			    (&Navigation::navigationBar, this)));
  menu->addItem("Popup menu", 
		deferCreate(boost::bind
			    (&Navigation::popupMenu, this)));
  menu->addItem("Split button", 
		deferCreate(boost::bind
			    (&Navigation::splitButton, this)));
  menu->addItem("Toolbar", 
		deferCreate(boost::bind
			    (&Navigation::toolBar, this)));
}


#include "examples/Path.cpp"
#include "examples/PathChange.cpp"

Wt::WWidget *Navigation::internalPaths()
{
  Wt::WTemplate *result = new TopicTemplate("navigation-internalPaths");

  result->bindWidget("Path", Path());
  result->bindWidget("PathChange", PathChange());

  return result;
}


#include "examples/Anchor.cpp"
#include "examples/AnchorImage.cpp"

Wt::WWidget *Navigation::anchor()
{
  Wt::WTemplate *result = new TopicTemplate("navigation-anchor");

  result->bindWidget("Anchor", Anchor());
  result->bindWidget("AnchorImage", AnchorImage());

  return result;
}


#include "examples/Stack.cpp"

Wt::WWidget *Navigation::stackedWidget()
{
  Wt::WTemplate *result = new TopicTemplate("navigation-stackedWidget");

  result->bindWidget("Stack", Stack());

  return result;
}


#include "examples/Menu.cpp"

Wt::WWidget *Navigation::menuWidget()
{
  Wt::WTemplate *result = new TopicTemplate("navigation-menu");

  result->bindWidget("Menu", Menu());

  return result;
}


#include "examples/Tab.cpp"

Wt::WWidget *Navigation::tabWidget()
{
  Wt::WTemplate *result = new TopicTemplate("navigation-tabWidget");

  result->bindWidget("Tab", Tab());

  return result;
}


#include "examples/NavigationBar.cpp"

Wt::WWidget *Navigation::navigationBar()
{
  Wt::WTemplate *result = new TopicTemplate("navigation-navigationBar");

  result->bindWidget("NavigationBar", NavigationBar());

  return result;
}


#include "examples/Popup.cpp"

Wt::WWidget *Navigation::popupMenu()
{
  Wt::WTemplate *result = new TopicTemplate("navigation-popupMenu");

  result->bindWidget("Popup", Popup());

  return result;
}


#include "examples/SplitButton.cpp"

Wt::WWidget *Navigation::splitButton()
{
  Wt::WTemplate *result = new TopicTemplate("navigation-splitButton");

  result->bindWidget("SplitButton", SplitButton());

  return result;
}


#include "examples/ToolBar.cpp"

Wt::WWidget *Navigation::toolBar()
{
  Wt::WTemplate *result = new TopicTemplate("navigation-toolBar");

  result->bindWidget("ToolBar", ToolBar());

  return result;
}
