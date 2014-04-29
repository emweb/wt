/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "DeferredWidget.h"
#include "Layout.h"
#include "TopicTemplate.h"

Layout::Layout()
  : TopicWidget()
{
  addText(tr("layout-intro"), this);
}

void Layout::populateSubMenu(Wt::WMenu *menu)
{
  menu->addItem("Containers", containers())->setPathComponent("");
  menu->addItem("HTML Templates",
		deferCreate(boost::bind
			    (&Layout::templates, this)));
  menu->addItem("Text", 
		deferCreate(boost::bind
			    (&Layout::text, this)));
  menu->addItem("Grouping widgets", 
		deferCreate(boost::bind
			    (&Layout::grouping, this)));
  menu->addItem("Layout managers", 
		deferCreate(boost::bind
			    (&Layout::layoutManagers, this)));
  menu->addItem("Dialogs", 
		deferCreate(boost::bind
			    (&Layout::dialogs, this)));
  menu->addItem("Images", 
		deferCreate(boost::bind
			    (&Layout::images, this)));
  menu->addItem("CSS", 
		deferCreate(boost::bind
			    (&Layout::css, this)));
  menu->addItem("Themes", 
		deferCreate(boost::bind
			    (&Layout::themes, this)));
}

#include "examples/Container.cpp"

Wt::WWidget *Layout::containers()
{
  Wt::WTemplate *result = new TopicTemplate("layout-Containers");

  result->bindWidget("Container", Container());

  return result;
}


#include "examples/Template.cpp"

Wt::WWidget *Layout::templates()
{
  Wt::WTemplate *result = new TopicTemplate("layout-Template");

  result->bindWidget("Template", Template());

  // Show the XML-template as text
  result->bindString("template-text", reindent(tr("WTemplate-example")),
                     Wt::PlainText);

  return result;
}


#include "examples/TextPlain.cpp"
#include "examples/TextXHTML.cpp"
#include "examples/TextXSS.cpp"
#include "examples/TextEvents.cpp"
#include "examples/TextToolTip.cpp"
#include "examples/TextDeferredToolTip.cpp"

Wt::WWidget *Layout::text()
{
  Wt::WTemplate *result = new TopicTemplate("layout-Text");

  result->bindWidget("TextPlain", TextPlain());
  result->bindWidget("TextXHTML", TextXHTML());
  result->bindWidget("TextXSS", TextXSS());
  result->bindWidget("TextEvents", TextEvents());
  result->bindWidget("TextToolTip", TextToolTip());
  result->bindWidget("TextDeferredToolTip", TextDeferredToolTip());

  return result;
}


#include "examples/GroupBox.cpp"
#include "examples/PanelNoTitle.cpp"
#include "examples/Panel.cpp"
#include "examples/PanelCollapsible.cpp"

Wt::WWidget *Layout::grouping()
{
  Wt::WTemplate *result = new TopicTemplate("layout-Grouping");

  result->bindWidget("GroupBox", GroupBox());
  result->bindWidget("PanelNoTitle", PanelNoTitle());
  result->bindWidget("Panel", Panel());
  result->bindWidget("PanelCollapsible", PanelCollapsible());

  return result;
}


#include "examples/HBoxLayout.cpp"
#include "examples/HBoxLayoutStretch.cpp"
#include "examples/VBoxLayout.cpp"
#include "examples/VBoxLayoutStretch.cpp"
#include "examples/NestedLayout.cpp"
#include "examples/GridLayout.cpp"
#include "examples/BorderLayout.cpp"

Wt::WWidget *Layout::layoutManagers()
{
  Wt::WTemplate *result = new TopicTemplate("layout-Managers");

  result->bindWidget("HBoxLayout", HBoxLayout());
  result->bindWidget("HBoxLayoutStretch", HBoxLayoutStretch());
  result->bindWidget("VBoxLayout", VBoxLayout());
  result->bindWidget("VBoxLayoutStretch", VBoxLayoutStretch());
  result->bindWidget("NestedLayout", NestedLayout());
  result->bindWidget("GridLayout", GridLayout());
  result->bindWidget("BorderLayout", BorderLayout());

  return result;
}


#include "examples/Dialog.cpp"
#include "examples/MessageBox.cpp"
#include "examples/MessageBoxSync.cpp"

Wt::WWidget *Layout::dialogs()
{
  Wt::WTemplate *result = new TopicTemplate("layout-Dialogs");

  result->bindWidget("Dialog", Dialog());
  result->bindWidget("MessageBox", MessageBox());
  result->bindWidget("MessageBoxSync", MessageBoxSync());

  return result;
}


#include "examples/Image.cpp"
#include "examples/ImageArea.cpp"

Wt::WWidget *Layout::images()
{
  Wt::WTemplate *result = new TopicTemplate("layout-Images");

  result->bindWidget("Image", Image());
  result->bindWidget("ImageArea", ImageArea());

  return result;
}


#include "examples/CSS.cpp"

Wt::WWidget *Layout::css()
{
  Wt::WTemplate *result = new TopicTemplate("layout-CSS");

  result->bindWidget("CSS", CSS());

  // Show the style sheet as text
  result->bindString("CSS-example-style", reindent(tr("CSS-example-style")),
                     Wt::PlainText);

  return result;
}


Wt::WWidget *Layout::themes()
{
  Wt::WTemplate *result = new TopicTemplate("layout-Themes");

  // Show the source code only for the theme example.
  result->bindString("Theme", reindent(tr("theme")), Wt::PlainText);

  return result;
}
