/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "BasicControls.h"
#include "EventDisplayer.h"

#include <Wt/WAnchor>
#include <Wt/WBreak>
#include <Wt/WCssDecorationStyle>
#include <Wt/WGroupBox>
#include <Wt/WImage>
#include <Wt/WIconPair>
#include <Wt/WLineEdit>
#include <Wt/WLength>
#include <Wt/WPanel>
#include <Wt/WProgressBar>
#include <Wt/WPushButton>
#include <Wt/WTable>
#include <Wt/WTabWidget>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WTree>
#include <Wt/WTreeNode>
#include <Wt/WTreeTable>
#include <Wt/WTreeTableNode>

using namespace Wt;

BasicControls::BasicControls(EventDisplayer *ed)
  : ControlsWidget(ed, true)
{
  new WText(tr("basics-intro"), this);
}

void BasicControls::populateSubMenu(WMenu *menu)
{
  menu->addItem("WContainerWidget", wContainerWidget());
  menu->addItem("WTemplate", wTemplate());
  menu->addItem("WText", wText());
  menu->addItem("WAnchor", wAnchor());
  menu->addItem("WBreak", wBreak());
  menu->addItem("WImage", wImage());
  menu->addItem("WGroupBox", wGroupBox());
  menu->addItem("WStackedWidget", wStackedWidget());
  menu->addItem("WTable", wTable());
  menu->addItem("WMenu", wMenu());
  menu->addItem("WTree", wTree());
  menu->addItem("WTreeTable", wTreeTable());
  menu->addItem("WPanel", wPanel());
  menu->addItem("WTabWidget", wTabWidget());
  menu->addItem("WProgressBar", wProgressBar());
}

WWidget *BasicControls::wText()
{
  WContainerWidget *result = new WContainerWidget();
  topic("WText", result);
  new WText(tr("basics-WText"), result);
  
  new WText("<p>This WText unexpectedly contains JavaScript, wich the "
	    "XSS attack preventer detects and disables. "
	    "<script>alert(\"You are under attack\");</script>"
	    "A warning is printed in Wt's log messages.</p>",
	    result);
    
  new WText("<p>This WText contains malformed XML <h1></h2>."
	    "It will be turned into a PlainText formatted string.</p>",
	    result);

  new WText(tr("basics-WText-events"), result);

  WText *text;

  text = new WText("This text reacts to <tt>clicked()</tt><br/>",
		   result);
  text->setStyleClass("reactive");
  ed_->showSignal(text->clicked(), "Text was clicked");

  text = new WText("This text reacts to <tt>doubleClicked()</tt><br/>",
		   result);
  text->setStyleClass("reactive");
  ed_->showSignal(text->doubleClicked(), "Text was double clicked");

  text = new WText("This text reacts to <tt>mouseWentOver()</tt><br/>", result);
  text->setStyleClass("reactive");
  ed_->showSignal(text->mouseWentOver(), "Mouse went over text");

  text = new WText("This text reacts to <tt>mouseWentOut()</tt><br/>", result);
  text->setStyleClass("reactive");
  ed_->showSignal(text->mouseWentOut(), "Mouse went out text");

  return result;
}

WWidget *BasicControls::wTemplate()
{
  WContainerWidget *result = new WContainerWidget();
  topic("WTemplate", result);

  new WText(tr("basics-WTemplate"), result);

  WTemplate *pre = new WTemplate("<pre>${text}</pre>", result);
  pre->bindString("text", tr("basics-WTemplate-example"), PlainText);

  new WText(tr("basics-WTemplate2"), result);

  WTemplate *temp = new WTemplate(tr("basics-WTemplate-example"), result);

  temp->bindWidget("name-edit", new WLineEdit());
  temp->bindWidget("save-button", new WPushButton("Save"));
  temp->bindWidget("cancel-button", new WPushButton("Cancel"));

  return result;
}

WWidget *BasicControls::wBreak()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WBreak", result);

  new WText(tr("basics-WBreak"), result);

  new WBreak(result); // does not really do anything useful :-)

  return result;
}

WWidget *BasicControls::wAnchor()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WAnchor", result);

  new WText(tr("basics-WAnchor"), result);

  WAnchor *a1 = new WAnchor("http://www.webtoolkit.eu/",
			    "Wt homepage (in a new window)", result);
  a1->setTarget(TargetNewWindow);

  new WText(tr("basics-WAnchor-more"), result);

  WAnchor *a2 = new WAnchor("http://www.webtoolkit.eu/", result);
  a2->setTarget(TargetNewWindow);
  new WImage("icons/wt_powered.jpg", a2);

  new WText(tr("basics-WAnchor-related"), result);
    
  return result;
}

WWidget *BasicControls::wImage()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WImage", result);

  new WText(tr("basics-WImage"), result);

  new WText("An image: ", result);
  new WImage("icons/wt_powered.jpg", result);

  new WText(tr("basics-WImage-more"), result);

  return result;
}

WWidget *BasicControls::wTable()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WTable", result);
  
  new WText(tr("basics-WTable"), result);

  WTable *table = new WTable(result);
  table->setStyleClass("example-table");

  new WText("First warning signal", table->elementAt(0, 0));
  new WText("09:25am", table->elementAt(0, 1));
  WImage *img = new WImage("icons/Pennant_One.png", table->elementAt(0, 2));
  img->resize(WLength::Auto, WLength(30, WLength::Pixel));
  new WText("First perparatory signal", table->elementAt(1, 0));
  new WText("09:26am", table->elementAt(1, 1));
  img = new WImage("icons/Pennant_One.png", table->elementAt(1, 2));
  img->resize(WLength::Auto, WLength(30, WLength::Pixel));
  img = new WImage("icons/Papa.png", table->elementAt(1, 2));
  img->resize(WLength::Auto, WLength(30, WLength::Pixel));
  new WText("Second perparatory signal", table->elementAt(2, 0));
  new WText("09:29am", table->elementAt(2, 1));
  img = new WImage("icons/Pennant_One.png", table->elementAt(2, 2));
  img->resize(WLength::Auto, WLength(30, WLength::Pixel));
  new WText("Start", table->elementAt(3, 0));
  new WText("09:30am", table->elementAt(3, 1));

  new WText(tr("basics-WTable-more"), result);

  return result;
}


WWidget *BasicControls::wTree()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WTree", "WTreeNode", result);

  new WText(tr("basics-WTree"), result);

  WIconPair *folderIcon = new WIconPair("icons/yellow-folder-closed.png",
					"icons/yellow-folder-open.png", false);

  WTree *tree = new WTree(result);
  tree->setSelectionMode(ExtendedSelection);

  WTreeNode *node = new WTreeNode("Furniture", folderIcon);
  tree->setTreeRoot(node);
  node->label()->setTextFormat(PlainText);
  node->setLoadPolicy(WTreeNode::NextLevelLoading);
  node->addChildNode(new WTreeNode("Table"));
  node->addChildNode(new WTreeNode("Cupboard"));

  WTreeNode *three = new WTreeNode("Chair");
  node->addChildNode(three);
  node->addChildNode(new WTreeNode("Coach"));
  node->expand();
  three->addChildNode(new WTreeNode("Doc"));
  three->addChildNode(new WTreeNode("Grumpy"));
  three->addChildNode(new WTreeNode("Happy"));
  three->addChildNode(new WTreeNode("Sneezy"));
  three->addChildNode(new WTreeNode("Dopey"));
  three->addChildNode(new WTreeNode("Bashful"));
  three->addChildNode(new WTreeNode("Sleepy"));

  new WText(tr("basics-WTree-more"), result);

  return result;
}

WWidget *BasicControls::wTreeTable()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WTreeTable","WTreeTableNode", result);
  new WText(tr("basics-WTreeTable"), result);

  WTreeTable *tt = new WTreeTable(result);
  tt->resize(650, 200);
  tt->tree()->setSelectionMode(ExtendedSelection);
  tt->addColumn("Yuppie Factor", 125);
  tt->addColumn("# Holidays", 125);
  tt->addColumn("Favorite Item", 125);
  WTreeTableNode *ttr = new WTreeTableNode("All Personnel");
  ttr->setImagePack("resources/");
  tt->setTreeRoot(ttr, "Emweb Organigram");
  WTreeTableNode *ttr1 = new WTreeTableNode("Upper Management", 0, ttr);
  WTreeTableNode *ttn;
  ttn = new WTreeTableNode("Chief Anything Officer", 0, ttr1);
  ttn->setColumnWidget(1, new WText("-2.8"));
  ttn->setColumnWidget(2, new WText("20"));
  ttn->setColumnWidget(3, new WText("Scepter"));
  ttn = new WTreeTableNode("Vice President of Parties", 0, ttr1);
  ttn->setColumnWidget(1, new WText("13.57"));
  ttn->setColumnWidget(2, new WText("365"));
  ttn->setColumnWidget(3, new WText("Flag"));
  ttn = new WTreeTableNode("Vice President of Staplery", 0, ttr1);
  ttn->setColumnWidget(1, new WText("3.42"));
  ttn->setColumnWidget(2, new WText("27"));
  ttn->setColumnWidget(3, new WText("Perforator"));
  ttr1 = new WTreeTableNode("Middle management", 0, ttr);
  ttn = new WTreeTableNode("Boss of the house", 0, ttr1);
  ttn->setColumnWidget(1, new WText("9.78"));
  ttn->setColumnWidget(2, new WText("35"));
  ttn->setColumnWidget(3, new WText("Happy Animals"));
  ttn = new WTreeTableNode("Xena caretaker", 0, ttr1);
  ttn->setColumnWidget(1, new WText("8.66"));
  ttn->setColumnWidget(2, new WText("10"));
  ttn->setColumnWidget(3, new WText("Yellow bag"));
  ttr1 = new WTreeTableNode("Actual Workforce", 0, ttr);
  ttn = new WTreeTableNode("The Dork", 0, ttr1);
  ttn->setColumnWidget(1, new WText("9.78"));
  ttn->setColumnWidget(2, new WText("22"));
  ttn->setColumnWidget(3, new WText("Mojito"));
  ttn = new WTreeTableNode("The Stud", 0, ttr1);
  ttn->setColumnWidget(1, new WText("8.66"));
  ttn->setColumnWidget(2, new WText("46"));
  ttn->setColumnWidget(3, new WText("Toothbrush"));
  ttn = new WTreeTableNode("The Ugly", 0, ttr1);
  ttn->setColumnWidget(1, new WText("13.0"));
  ttn->setColumnWidget(2, new WText("25"));
  ttn->setColumnWidget(3, new WText("Paper bag"));
  ttr->expand();

  return result;
}

WWidget *BasicControls::wPanel()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WPanel", result);

  new WText(tr("basics-WPanel"), result);
  WPanel *panel = new WPanel(result);
  panel->setCentralWidget(new WText("This is a default panel"));
  new WBreak(result);
  panel = new WPanel(result);
  panel->setTitle("My second WPanel.");
  panel->setCentralWidget(new WText("This is a panel with a title"));
  new WBreak(result);
  panel = new WPanel(result);
  panel->setTitle("My third WPanel");
  panel->setCentralWidget(new WText("This is a collapsible panel with "
				    "a title"));
  panel->setCollapsible(true);

  new WText(tr("basics-WPanel-related"), result);

  return result;
}

WWidget *BasicControls::wTabWidget()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WTabWidget", result);
  new WText(tr("basics-WTabWidget"), result);    
  WTabWidget *tw = new WTabWidget(result);
  tw->addTab(new WText("These are the contents of the first tab"),
	     "Picadilly", WTabWidget::PreLoading);
  tw->addTab(new WText("The contents of these tabs are pre-loaded in "
		       "the browser to ensure swift switching."),
	     "Waterloo", WTabWidget::PreLoading);
  tw->addTab(new WText("This is yet another pre-loaded tab. "
		       "Look how good this works."),
	     "Victoria", WTabWidget::PreLoading);
  tw->addTab(new WText("The colors of the tab widget can be changed by "
		       "modifying some images."),
	     "Tottenham");

  tw->setStyleClass("tabwidget");

  new WText(tr("basics-WTabWidget-more"), result);    

  return result;
}

WWidget *BasicControls::wContainerWidget()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WContainerWidget", result);

  new WText(tr("basics-WContainerWidget"), result);

  return result;
}

WWidget *BasicControls::wMenu()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WMenu", result);
  new WText(tr("basics-WMenu"), result);

  return result;
}

WWidget *BasicControls::wGroupBox()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WGroupBox", result);
  new WText(tr("basics-WGroupBox"), result);

  WGroupBox *gb = new WGroupBox("A group box", result);
  gb->addWidget(new WText(tr("basics-WGroupBox-contents")));

  new WText(tr("basics-WGroupBox-related"), result);

  return result;
}

WWidget *BasicControls::wStackedWidget()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WStackedWidget", result);
  new WText(tr("basics-WStackedWidget"), result);

  return result;
}

WWidget *BasicControls::wProgressBar()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WProgressBar", result);

  result->addWidget(new WText(tr("basics-WProgressBar")));
  WProgressBar *pb = new WProgressBar(result);
  pb->setValue(27);

  return result;
}
