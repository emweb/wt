
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "DeferredWidget.h"
#include "Navigation.h"
#include "TopicTemplate.h"

#include <Wt/WMenu.h>
#include <Wt/WText.h>

Navigation::Navigation()
  : TopicWidget()
{
  addText(tr("navigation-intro"), this);
}

void Navigation::populateSubMenu(Wt::WMenu *menu)
{
  menu->addItem("Internal paths", internalPaths())->setPathComponent("");
  menu->addItem("Anchor", 
                deferCreate([this]{ return anchor(); }));
  menu->addItem("Stacked widget", 
                deferCreate([this]{ return stackedWidget(); }));
  menu->addItem("Menu", 
                deferCreate([this]{ return menuWidget(); }));
  menu->addItem("Tab widget", 
                deferCreate([this]{ return tabWidget(); }));
  menu->addItem("Navigation bar", 
                deferCreate([this]{ return navigationBar(); }));
  menu->addItem("Popup menu", 
                deferCreate([this]{ return popupMenu(); }));
  menu->addItem("Split button", 
                deferCreate([this]{ return splitButton(); }));
  menu->addItem("Toolbar", 
                deferCreate([this]{ return toolBar(); }));
}


#include "examples/Path.cpp"
#include "examples/PathChange.cpp"

std::unique_ptr<Wt::WWidget> Navigation::internalPaths()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("navigation-internalPaths");

  result->bindWidget("Path", Path());
  result->bindWidget("PathChange", PathChange());

  return std::move(result);
}


#include "examples/Anchor.cpp"
#include "examples/AnchorImage.cpp"

std::unique_ptr<Wt::WWidget> Navigation::anchor()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("navigation-anchor");

  result->bindWidget("Anchor", Anchor());
  result->bindWidget("AnchorImage", AnchorImage());

  return std::move(result);
}


#include "examples/Stack.cpp"

std::unique_ptr<Wt::WWidget> Navigation::stackedWidget()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("navigation-stackedWidget");

  result->bindWidget("Stack", Stack());

  return std::move(result);
}


#include "examples/Menu.cpp"

std::unique_ptr<Wt::WWidget> Navigation::menuWidget()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("navigation-menu");

  result->bindWidget("Menu", Menu());

  return std::move(result);
}


#include "examples/Tab.cpp"

std::unique_ptr<Wt::WWidget> Navigation::tabWidget()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("navigation-tabWidget");

  result->bindWidget("Tab", Tab());

  return std::move(result);
}


#include "examples/NavigationBar.cpp"

std::unique_ptr<Wt::WWidget> Navigation::navigationBar()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("navigation-navigationBar");

  result->bindWidget("NavigationBar", NavigationBar());

  return std::move(result);
}


#include "examples/Popup.cpp"

std::unique_ptr<Wt::WWidget> Navigation::popupMenu()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("navigation-popupMenu");

  result->bindWidget("Popup", Popup());

  return std::move(result);
}


#include "examples/SplitButton.cpp"

std::unique_ptr<Wt::WWidget> Navigation::splitButton()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("navigation-splitButton");

  result->bindWidget("SplitButton", SplitButton());

  return std::move(result);
}


#include "examples/ToolBar.cpp"

std::unique_ptr<Wt::WWidget> Navigation::toolBar()
{
  auto result = Wt::cpp14::make_unique<TopicTemplate>("navigation-toolBar");

  result->bindWidget("ToolBar", ToolBar());

  return std::move(result);
}
