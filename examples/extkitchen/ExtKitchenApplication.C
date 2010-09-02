/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ExtKitchenApplication.h"
#include "CsvUtil.h"

#include <fstream>

#include <Wt/WBorderLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WFitLayout>
#include <Wt/WOverlayLoadingIndicator>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WTable>
#include <Wt/WTableCell>
#include <Wt/WText>
#include <Wt/WTree>
#include <Wt/WTableView>
#include <Wt/WIconPair>
#include <Wt/WTreeNode>

#include <Wt/Ext/Button>
#include <Wt/Ext/Calendar>
#include <Wt/Ext/CheckBox>
#include <Wt/Ext/ComboBox>
#include <Wt/Ext/Container>
#include <Wt/Ext/DateField>
#include <Wt/Ext/Dialog>
#include <Wt/Ext/Menu>
#include <Wt/Ext/MessageBox>
#include <Wt/Ext/ProgressDialog>
#include <Wt/Ext/Splitter>
#include <Wt/Ext/TabWidget>
#include <Wt/Ext/TableView>
#include <Wt/Ext/TextEdit>
#include <Wt/Ext/ToolBar>

#include <iostream>

#ifdef WIN32
#include <windows.h> // for Sleep()
#undef MessageBox
#endif

using namespace Wt;

ExtKitchenApplication::ExtKitchenApplication(const WEnvironment& env)
  : WApplication(env)
{
  setTitle("Wt-Ext, including a kitchen sink");
  setLoadingIndicator(new WOverlayLoadingIndicator());

  useStyleSheet("extkitchen.css");
  messageResourceBundle().use(appRoot() + "extkitchen");

  Ext::Container *viewPort = new Ext::Container(root());
  WBorderLayout *layout = new WBorderLayout(viewPort);

  /* North */
  Ext::Panel *north = new Ext::Panel();
  north->setBorder(false);
  WText *head = new WText(WString::tr("header"));
  head->setStyleClass("north");
  north->setLayout(new WFitLayout());
  north->layout()->addWidget(head);
  north->resize(WLength::Auto, 35);
  layout->addWidget(north, WBorderLayout::North);

  /* West */
  Ext::Panel *west = new Ext::Panel();
  west->layout()->addWidget(createExampleTree());

  west->setTitle("Widgets");
  west->resize(200, WLength::Auto);
  west->setResizable(true);
  west->setCollapsible(true);
  west->setAnimate(true);
  west->setAutoScrollBars(true);
  layout->addWidget(west, WBorderLayout::West);

  /* Center */
  Ext::Panel *center = new Ext::Panel();
  center->setTitle("Demo widget");
  center->layout()->addWidget(exampleContainer_ = new WContainerWidget());
  center->setAutoScrollBars(true);
  layout->addWidget(center, WBorderLayout::Center);

  exampleContainer_->setPadding(5);

  WContainerWidget *container = new WContainerWidget(exampleContainer_);
  container->addWidget(new WText(WString::tr("about")));
  currentExample_ = container;

  // load an Ext them, after at least one Ext widget. For example, here:
  //useStyleSheet("ext/resources/css/xtheme-gray.css");
}

WWidget *ExtKitchenApplication::createExampleTree()
{
  WIconPair *mapIcon
    = new WIconPair("icons/yellow-folder-closed.png",
		    "icons/yellow-folder-open.png", false);

  WTreeNode *rootNode = new WTreeNode("Examples", mapIcon);
  rootNode->setImagePack("icons/");
  rootNode->expand();
  rootNode->setLoadPolicy(WTreeNode::NextLevelLoading);

  createExampleNode("Menu and ToolBar", rootNode,
		    &ExtKitchenApplication::menuAndToolBarExample);
  createExampleNode("Form widgets", rootNode,
		    &ExtKitchenApplication::formWidgetsExample);
  createExampleNode("TableView", rootNode,
		    &ExtKitchenApplication::tableViewExample);
  createExampleNode("Dialogs", rootNode,
		    &ExtKitchenApplication::dialogExample);
  createExampleNode("TabWidget", rootNode,
		    &ExtKitchenApplication::tabWidgetExample);

  rootNode->setMargin(5);
 
  return rootNode;
}

WTreeNode *ExtKitchenApplication::createExampleNode(const WString& label,
						    WTreeNode *parentNode,
						    ShowExample f)
{
  WIconPair *labelIcon
    = new WIconPair("icons/document.png", "icons/document.png", false);

  WTreeNode *node = new WTreeNode(label, labelIcon, parentNode);
  node->label()->setTextFormat(PlainText);
  node->label()->clicked().connect(this, f);

  return node;
}

void ExtKitchenApplication::setExample(WWidget *example)
{
  delete currentExample_;
  currentExample_ = example;
  exampleContainer_->addWidget(currentExample_);
}

void ExtKitchenApplication::menuAndToolBarExample()
{
  WContainerWidget *ex = new WContainerWidget();

  WText *wt = new WText(WString::tr("ex-menu-and-toolbar"), ex);
  wt->setMargin(5, Bottom);

  // Create a menu with some items

  Ext::Menu *menu = new Ext::Menu();
  Ext::MenuItem *item;

  item = menu->addItem("File open...");
  item->setIcon("icons/yellow-folder-open.png");

  item = menu->addItem("I dig Wt");
  item->setCheckable(true);
  item->setChecked(true);

  item = menu->addItem("I dig Wt too");
  item->setCheckable(true);

  menu->addSeparator();
  menu->addItem("Menu item");
  menu->addSeparator();

  // Add a sub menu

  Ext::Menu *subMenu = new Ext::Menu();
  subMenu->addItem("Do this");
  subMenu->addItem("And that");

  item = menu->addMenu("More ...", subMenu);
  item->setIcon("icons/yellow-folder-open.png");

  // Create a tool bar

  Ext::ToolBar *toolBar = new Ext::ToolBar(ex);

  Ext::Button *b
    = toolBar->addButton("Button w/Menu", menu);
  b->setIcon("icons/yellow-folder-closed.png");

  toolBar->addButton("Button");

  toolBar->addSeparator();
  toolBar->addButton("Separated");
  toolBar->addSeparator();
  Ext::Button *button = toolBar->addButton("Toggle me");
  button->setCheckable(true);

  Ext::ComboBox *cb = new Ext::ComboBox();
  cb->addItem("Winter");
  cb->addItem("Spring");
  cb->addItem("Summer");
  cb->addItem("Autumn");
  toolBar->add(cb);

  setExample(ex);
}

void ExtKitchenApplication::formWidgetsExample()
{
  WContainerWidget *ex = new WContainerWidget();

  WText *wt = new WText( WString::tr("ex-form-widgets"), ex);
  wt->setMargin(5, Bottom);

  WTable *table = new WTable(ex);

  // ComboBox
  cb = new Ext::ComboBox(table->elementAt(0, 0));
  cb->addItem("One");
  cb->addItem("Two");
  cb->addItem("Three");
  cb->setFocus();

  /*
    This is how you would keep the data on the server (for really big
    data models:

    cb->setDataLocation(Ext::ServerSide);
    cb->setMinQueryLength(0);
    cb->setQueryDelay(0);
    cb->setPageSize(10);
    cb->setTextSize(20);
  */

  // Button
  Ext::Button *button = new Ext::Button("Modify", table->elementAt(0, 1));
  button->setMargin(5, Left);
  button->activated().connect(this, &ExtKitchenApplication::formModify);

  // CheckBox
  Ext::CheckBox *cb1 = new Ext::CheckBox("Check 1", table->elementAt(1, 0));
  Ext::CheckBox *cb2 = new Ext::CheckBox("Check 2", table->elementAt(2, 0));
  cb2->setChecked();

  /*
    -- test setHideWithOffsets() of Ext::ComboBox
    table->hide();
    WPushButton *b = new WPushButton("show", ex);
    b->clicked().connect(table, &WWidget::show);
  */

  // DateField
  WContainerWidget *w = new WContainerWidget(ex);
  w->setMargin(5, Top | Bottom);
  Ext::DateField *df = new Ext::DateField(w);
  df->setDate(WDate(2007, 9, 7));

  // Calendar
  Ext::Calendar *dp = new Ext::Calendar(false, ex);

  // TextEdit
  html_ = new Ext::TextEdit("Hello there, <b>brothers and sisters</b>", ex);
  html_->setMargin(5, Top | Bottom);
  html_->resize(600, 300);

  // Horizontal Splitter
  Ext::Splitter *split = new Ext::Splitter(ex);
  split->resize(400, 100);

  split->addWidget(new WText("Left"));
  split->children().back()->resize(150, WLength::Auto);
  split->children().back()->setMinimumSize(130, WLength::Auto);
  split->children().back()->setMaximumSize(170, WLength::Auto);

  split->addWidget(new WText("Center"));
  split->children().back()->resize(100, WLength::Auto);
  split->children().back()->setMinimumSize(50, WLength::Auto);

  split->addWidget(new WText("Right"));
  split->children().back()->resize(50, WLength::Auto);
  split->children().back()->setMinimumSize(50, WLength::Auto);

  // Vertical Splitter
  split = new Ext::Splitter(Vertical, ex);
  split->resize(100, 200);

  split->addWidget(new WText("Top"));
  split->children().back()->resize(WLength::Auto, 100);
  split->children().back()->setMinimumSize(WLength::Auto, 50);
  split->children().back()->setMaximumSize(WLength::Auto, 196);

  split->addWidget(new WText("Center"));
  split->children().back()->resize(WLength::Auto, 100);

  setExample(ex);
}

void ExtKitchenApplication::tableViewExample()
{
  WContainerWidget *ex = new WContainerWidget();

  WText *wt = new WText(WString::tr("ex-table-view"), ex);
  wt->setMargin(5, Bottom);

  /*
   * Create the data model, and load from a CSV file
   */
  model_ = new WStandardItemModel(0,0,ex);
  std::ifstream f((appRoot() + "compare.csv").c_str());
  readFromCsv(f, model_);

  /*
   * Convert the last column to WDate
   */
  for (int i = 0; i < model_->rowCount(); ++i) {
    int j = model_->columnCount() - 1;
    WString dStr = boost::any_cast<WString>(model_->data(i, j));
    model_->setData(i, j, boost::any(WDate::fromString(dStr, "d/M/yyyy")));
  }

  /*
   * Create a read-only TableView for the model
   */
  table1_ = new Ext::TableView(ex);
  table1_->resize(700, 250);
  table1_->setModel(model_);
  table1_->setColumnSortable(0, true);
  table1_->enableColumnHiding(0, true);
  table1_->setAlternatingRowColors(true);
  table1_->setAutoExpandColumn(2);
  table1_->setRenderer(model_->columnCount() - 1,
		       Ext::TableView::dateRenderer("d MMM yyyy"));

  /*
   * Leave the data on the server, and add a paging tool
   */
  table1_->setDataLocation(Ext::ServerSide);
  table1_->setPageSize(10);
  table1_->setBottomToolBar(table1_->createPagingToolBar());
  table1_->bottomToolBar()->addButton("Other button");

  /*
   * A second editable TableView for the same model inside a tab
   * widget.
   */
  wt = new WText(WString::tr("ex-table-view2"), ex);
  wt->setMargin(5, Bottom);

  Ext::TabWidget *tb = new Ext::TabWidget(ex);
  tb->addTab(table2_ = new Ext::TableView());
  tb->addTab(new WText(WString::tr("tab-2-content")), "Tab 2");

  tb->resize(600, 250);

  table2_->setTitle("Editable TableView");
  //table2_->setModel(new WStandardItemModel(5, 5));
  table2_->setModel(model_);
  table2_->resizeColumnsToContents(true);
  table2_->setAutoExpandColumn(2);
  table2_->setRenderer(model_->columnCount() - 1,
  		       Ext::TableView::dateRenderer("dd/MM/yyyy"));

  // Set a LineEdit for the first field
  table2_->setEditor(0, new Ext::LineEdit());

  // Set a ComboBox for the second field
  Ext::ComboBox *cb = new Ext::ComboBox();
  cb->addItem("Library");
  cb->addItem("Servlet");
  cb->addItem("Framework");
  table2_->setEditor(1, cb);

  // Set a DateField for the last field
  Ext::DateField *df = new Ext::DateField();
  df->setFormat("dd/MM/yyyy");
  table2_->setEditor(model_->columnCount() - 1, df);

  Ext::ToolBar *toolBar = new Ext::ToolBar();
  toolBar->addButton("Add 100 rows",
		     this, &ExtKitchenApplication::addRow);
  toolBar->addButton("Remove 100 row",
		     this, &ExtKitchenApplication::removeRow);
  toolBar->addButton("Reset",
		     this, &ExtKitchenApplication::resetModel);
  table2_->setBottomToolBar(toolBar);


  /*
   * A WTableView in another tab widget, working on the same model!
   */
  tableView_ = new WTableView();
  tableView_->setSelectionMode(Wt::ExtendedSelection);
  tableView_->setModel(model_);
  tableView_->setRowHeight(21);
  tableView_->setDragEnabled(true);
  tableView_->setDropsEnabled(true);
  tableView_->setAlternatingRowColors(true);
  for (int i = 0; i < model_->columnCount(); ++i)
    tableView_->setColumnWidth(i, 90);

  Ext::Panel *p = new Ext::Panel();
  p->setLayout(new WFitLayout());
  p->layout()->addWidget(tableView_);
  p->setTitle("WTableView");
  toolBar = new Ext::ToolBar();
  toolBar->addButton("Add 100 rows",
		     this, &ExtKitchenApplication::addRow);
  p->setTopToolBar(toolBar);

  tb->addTab(p);

  setExample(ex);
}

void ExtKitchenApplication::addRow()
{
  /* Add some new row at the end of the model */
  for (int i = 0; i < 100; ++i) {
    int r = model_->rowCount();
    model_->insertRow(r);
    model_->setData(r, 0, boost::any(WString("Mine")));
    model_->setData(r, 1, boost::any(WString("Framework")));
    model_->setData(r, 2, boost::any(WString("JavaScript")));
    model_->setData(r, 3, boost::any(WString("No")));
    model_->setData(r, 4, boost::any(WString("No")));
    model_->setData(r, 5, boost::any(WDate::currentDate()));

    WStandardItem *c0 = model_->item(r, 0);
    c0->setFlags(c0->flags() | ItemIsDropEnabled);
  }
}

void ExtKitchenApplication::removeRow()
{
  /* Remove the first row */
  for (int i = 0; i < 100; ++i)
    model_->removeRow(0);
}

void ExtKitchenApplication::resetModel()
{
  /* Reset the original model */
  WStandardItemModel *model = new WStandardItemModel(0,0,this);

  std::ifstream f((appRoot() + "compare.csv").c_str());
  readFromCsv(f, model);

  table1_->setModel(model);
  table2_->setModel(model);
  tableView_->setModel(model);

  delete model_;
  model_ = model;
}

void ExtKitchenApplication::formModify()
{
  std::cerr << cb->currentText() << ", " << cb->currentIndex() << std::endl;
  cb->addItem("Four?");
}

void ExtKitchenApplication::dialogExample()
{
  WContainerWidget *ex = new WContainerWidget();

  WVBoxLayout *vLayout = new WVBoxLayout();
  ex->setLayout(vLayout, AlignTop | AlignLeft);
  vLayout->setContentsMargins(0, 0, 0, 0);
  vLayout->setSpacing(3);

  vLayout->addWidget(new WText(WString::tr("ex-dialogs")));

  Ext::Button *button;

  vLayout->addWidget(button = new Ext::Button("Dialog 1"));
  button->activated().connect(this, &ExtKitchenApplication::createDialog);
  vLayout->addWidget(button = new Ext::Button("Dialog 2"));
  button->activated().connect(this, &ExtKitchenApplication::createDialog2);
  vLayout->addWidget(button = new Ext::Button("Dialog 3"));
  button->activated().connect(this, &ExtKitchenApplication::createDialog3);
  vLayout->addWidget(button = new Ext::Button("Dialog 4"));
  button->activated().connect(this, &ExtKitchenApplication::createDialog4);
  vLayout->addWidget(button = new Ext::Button("Dialog 5"));
  button->activated().connect(this, &ExtKitchenApplication::createDialog5);
  vLayout->addWidget(button = new Ext::Button("Dialog 6"));
  button->activated().connect(this, &ExtKitchenApplication::createDialog6);
  vLayout->addWidget(button = new Ext::Button("Dialog 7"));
  button->activated().connect(this, &ExtKitchenApplication::createDialog7);

  setExample(ex);
}

void ExtKitchenApplication::createDialog()
{
  mbox_ = new Ext::MessageBox();
  mbox_->resize(300, 100);
  mbox_->setWindowTitle("Hello there");

  mbox_->setButtons(Wt::Ok);
  mbox_->finished().connect(this, &ExtKitchenApplication::testDelete);

  mbox_->show();
}

void ExtKitchenApplication::testDelete()
{
  delete mbox_;
}

void ExtKitchenApplication::createDialog2()
{
  Ext::Dialog d;
  d.setWindowTitle("Hello there too");
  d.resize(300,100);

  Ext::Button *okButton = new Ext::Button("Ok");
  okButton->activated().connect(&d, &Ext::Dialog::accept);
  d.addButton(okButton);
  okButton->setDefault(true);

  Ext::Button *cancelButton = new Ext::Button("Cancel");
  cancelButton->activated().connect(&d, &Ext::Dialog::reject);
  d.addButton(cancelButton);

  WText *contents = new WText("I'm right here.");
  d.contents()->addWidget(contents);
  d.exec();

  d.setWindowTitle("Good to see you.");
  contents->setText("I've been waiting for you.");
  d.exec();
}

void ExtKitchenApplication::createDialog3()
{
  Ext::Dialog d;
  d.setWindowTitle("Ext::Dialog with WBorderLayout");
  d.resize(400,300);
  d.setStyleClass("dialog");

  Ext::Button *okButton = new Ext::Button("Ok");
  okButton->activated().connect(&d, &Ext::Dialog::accept);
  d.addButton(okButton);
  okButton->setDefault(true);

  Ext::Button *cancelButton = new Ext::Button("Cancel");
  cancelButton->activated().connect(&d, &Ext::Dialog::reject);
  d.addButton(cancelButton);

  WBorderLayout *layout = new WBorderLayout();
  d.setLayout(layout);

  Ext::Panel *west = new Ext::Panel();
  west->setTitle("West");
  west->setResizable(true);
  west->resize(100, WLength::Auto);
  layout->addWidget(west, WBorderLayout::West);

  Ext::Panel *center = new Ext::Panel();
  center->setTitle("Center");

  WBorderLayout *nestedLayout = new WBorderLayout();
  center->setLayout(nestedLayout);

  Ext::Panel *nestedNorth = new Ext::Panel();
  nestedLayout->addWidget(nestedNorth, WBorderLayout::North);
  nestedNorth->resize(WLength::Auto, 35);
  nestedNorth->layout()->addWidget(new WText(WString::tr("nested-header")));

  Ext::Panel *nestedCenter = new Ext::Panel();
  nestedLayout->addWidget(nestedCenter, WBorderLayout::Center);
  nestedCenter->layout()->addWidget(new WText(WString::tr("dialog-nested")));

  layout->addWidget(center, WBorderLayout::Center);

  d.exec();
}

void ExtKitchenApplication::createDialog4()
{
  if (Ext::MessageBox::show("Confirm", "I am amazed", Ok | Cancel)
      == Ok)
    std::cerr << "Got ok.";
  else
    std::cerr << "Got cancel.";
}

void ExtKitchenApplication::createDialog5()
{
  WString v = "Jozef";

  if (Ext::MessageBox::prompt("Info", "Please enter your name:", v) == Ok)
    std::cerr << "You entered: '" << v << '\'' << std::endl;
}

void ExtKitchenApplication::createDialog6()
{
  Ext::ProgressDialog d("Converting contact details...", "Cancel", 0, 7);
  d.setWindowTitle("Import Contacts");

  d.show();

  for (unsigned i = 0; i < 7; ++i) {
    d.setValue(i);
    processEvents();

    if (!d.wasCanceled()) {
      /* Do some work ... */
#ifdef WIN32
      Sleep(1000);
#else
      sleep(1);
#endif
    } else {
      Ext::MessageBox
	::show("Operation cancelled",
	       "You may import your contact details any time later.", Ok);
      break;
    }
  }
}

void ExtKitchenApplication::createDialog7()
{
  Ext::Dialog d;
  d.setWindowTitle("Shhh...");

  d.resize(350,120);

  Ext::Button *okButton = new Ext::Button("Ok");
  okButton->activated().connect(&d, &Ext::Dialog::accept);
  d.addButton(okButton);
  okButton->setDefault(true);

  Ext::Button *cancelButton = new Ext::Button("Cancel");
  cancelButton->activated().connect(&d, &Ext::Dialog::reject);
  d.addButton(cancelButton);

  d.contents()->setPadding(8);
  new WText("Please give your password:", d.contents());

  Ext::LineEdit passwd(d.contents());
  passwd.setEchoMode(Ext::LineEdit::Password);
  passwd.setTextSize(8);
  passwd.setMargin(5, Left);
  passwd.setInline(true);

  d.contents()->enterPressed().connect(&d, &Ext::Dialog::accept);

  if (d.exec() == Ext::Dialog::Accepted) {
    // ...
  }
}

void ExtKitchenApplication::tabWidgetExample()
{
  WContainerWidget *ex = new WContainerWidget();

  WText *wt = new WText(WString::tr("ex-tabwidget"), ex);
  wt->setMargin(5, Bottom);

  tb = new Ext::TabWidget(ex);
  tb->resize(500, 200);
  tb->addTab(new WText(WString::tr("tab-1-content")), "Tab 1");
  tb->addTab(new WText(WString::tr("tab-2-content")), "Tab 2");

  WContainerWidget *w = new WContainerWidget(ex);
  WHBoxLayout *hLayout = new WHBoxLayout();
  w->setLayout(hLayout, AlignTop | AlignLeft);
  hLayout->setContentsMargins(0, 9, 0, 0);

  Ext::Button *b;
  hLayout->addWidget(b = new Ext::Button("Hide"));
  b->clicked().connect(this, &ExtKitchenApplication::hideTab);

  hLayout->addWidget(b = new Ext::Button("Show"));
  b->clicked().connect(this, &ExtKitchenApplication::showTab);

  hLayout->addWidget(b = new Ext::Button("Add tab"));
  b->clicked().connect(this, &ExtKitchenApplication::modifyTabWidget);
  b->setToolTip("Adds a tab");

  setExample(ex);
}

void ExtKitchenApplication::modifyTabWidget()
{
  tb->addTab(new WText(WString::tr("tab-x-content")),
	     "Tab " + boost::lexical_cast<std::string>(tb->count() + 1));
}

void ExtKitchenApplication::hideTab()
{
  tb->setCurrentIndex(1);
  tb->setTabHidden(0, true);
}

void ExtKitchenApplication::showTab()
{
  tb->setTabHidden(0, false);
}

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new ExtKitchenApplication(env);

  return app;
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
