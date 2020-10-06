/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "DeferredWidget.h"
#include "Layout.h"
#include "TopicTemplate.h"

#include <Wt/WMenu.h>

Layout::Layout()
  : TopicWidget()
{
  addText(tr("layout-intro"), this);
}

void Layout::populateSubMenu(Wt::WMenu *menu)
{
  menu->addItem("Containers", containers())->setPathComponent("");
  menu->addItem("HTML Templates",
                deferCreate([this]{ return templates(); }));
  menu->addItem("Text", 
                deferCreate([this]{ return text(); }));
  menu->addItem("Grouping widgets", 
                deferCreate([this]{ return grouping(); }));
  menu->addItem("Layout managers", 
                deferCreate([this]{ return layoutManagers(); }));
  menu->addItem("Dialogs", 
                deferCreate([this]{ return dialogs(); }));
  menu->addItem("Images", 
                deferCreate([this]{ return images(); }));
  menu->addItem("CSS", 
                deferCreate([this]{ return css(); }));
  menu->addItem("Themes", 
                deferCreate([this]{ return themes(); }));
}

#include "examples/Container.cpp"

std::unique_ptr<Wt::WWidget> Layout::containers()
{
  auto result = std::make_unique<TopicTemplate>("layout-Containers");

  result->bindWidget("Container", Container());

  return std::move(result);
}


#include "examples/Template.cpp"

std::unique_ptr<Wt::WWidget> Layout::templates()
{
  auto result = std::make_unique<TopicTemplate>("layout-Template");

  result->bindWidget("Template", Template());

  // Show the XML-template as text
  result->bindString("template-text", reindent(tr("WTemplate-example")),
		     Wt::TextFormat::Plain);

  return std::move(result);
}


#include "examples/TextPlain.cpp"
#include "examples/TextXHTML.cpp"
#include "examples/TextXSS.cpp"
#include "examples/TextEvents.cpp"
#include "examples/TextToolTip.cpp"
#include "examples/TextDeferredToolTip.cpp"

std::unique_ptr<Wt::WWidget> Layout::text()
{
  auto result = std::make_unique<TopicTemplate>("layout-Text");

  result->bindWidget("TextPlain", TextPlain());
  result->bindWidget("TextXHTML", TextXHTML());
  result->bindWidget("TextXSS", TextXSS());
  result->bindWidget("TextEvents", TextEvents());
  result->bindWidget("TextToolTip", TextToolTip());
  result->bindWidget("TextDeferredToolTip", TextDeferredToolTip());

  return std::move(result);
}


#include "examples/GroupBox.cpp"
#include "examples/PanelNoTitle.cpp"
#include "examples/Panel.cpp"
#include "examples/PanelCollapsible.cpp"

std::unique_ptr<Wt::WWidget> Layout::grouping()
{
  auto result = std::make_unique<TopicTemplate>("layout-Grouping");

  result->bindWidget("GroupBox", GroupBox());
  result->bindWidget("PanelNoTitle", PanelNoTitle());
  result->bindWidget("Panel", Panel());
  result->bindWidget("PanelCollapsible", PanelCollapsible());

  return std::move(result);
}


#include "examples/HBoxLayout.cpp"
#include "examples/HBoxLayoutStretch.cpp"
#include "examples/VBoxLayout.cpp"
#include "examples/VBoxLayoutStretch.cpp"
#include "examples/NestedLayout.cpp"
#include "examples/GridLayout.cpp"
#include "examples/BorderLayout.cpp"

std::unique_ptr<Wt::WWidget> Layout::layoutManagers()
{
  auto result = std::make_unique<TopicTemplate>("layout-Managers");

  result->bindWidget("HBoxLayout", HBoxLayout());
  result->bindWidget("HBoxLayoutStretch", HBoxLayoutStretch());
  result->bindWidget("VBoxLayout", VBoxLayout());
  result->bindWidget("VBoxLayoutStretch", VBoxLayoutStretch());
  result->bindWidget("NestedLayout", NestedLayout());
  result->bindWidget("GridLayout", GridLayout());
  result->bindWidget("BorderLayout", BorderLayout());

  return std::move(result);
}


#include "examples/Dialog.cpp"
#include "examples/MessageBox.cpp"
#include "examples/MessageBoxSync.cpp"

std::unique_ptr<Wt::WWidget> Layout::dialogs()
{
  auto result = std::make_unique<TopicTemplate>("layout-Dialogs");

  result->bindWidget("Dialog", Dialog());
  result->bindWidget("MessageBox", MessageBox());
  result->bindWidget("MessageBoxSync", MessageBoxSync());

  return std::move(result);
}


#include "examples/Image.cpp"
#include "examples/ImageArea.cpp"

std::unique_ptr<Wt::WWidget> Layout::images()
{
  auto result = std::make_unique<TopicTemplate>("layout-Images");

  result->bindWidget("Image", Image());
  result->bindWidget("ImageArea", ImageArea());

  return std::move(result);
}


#include "examples/CSS.cpp"

std::unique_ptr<Wt::WWidget> Layout::css()
{
  auto result = std::make_unique<TopicTemplate>("layout-CSS");

  result->bindWidget("CSS", CSS());

  // Show the style sheet as text
  result->bindString("CSS-example-style", reindent(tr("CSS-example-style")),
		     Wt::TextFormat::Plain);

  return std::move(result);
}


std::unique_ptr<Wt::WWidget> Layout::themes()
{
  auto result = std::make_unique<TopicTemplate>("layout-Themes");

  // Show the source code only for the theme example.
  result->bindString("Theme", reindent(tr("theme")), Wt::TextFormat::Plain);

  return std::move(result);
}
