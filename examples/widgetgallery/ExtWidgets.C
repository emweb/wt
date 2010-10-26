/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "ExtWidgets.h"
#include "EventDisplayer.h"
#include "DeferredWidget.h"

#include <Wt/Ext/Button>
#include <Wt/Ext/LineEdit>
#include <Wt/Ext/ComboBox>
#include <Wt/Ext/CheckBox>
#include <Wt/Ext/RadioButton>
#include <Wt/Ext/Calendar>
#include <Wt/Ext/DateField>
#include <Wt/Ext/Menu>
#include <Wt/Ext/ToolBar>
#include <Wt/Ext/NumberField>
#include <Wt/WButtonGroup>
#include <Wt/WVBoxLayout>
#include <Wt/WText>

using namespace Wt;
using namespace Wt::Ext;

ExtWidgets::ExtWidgets(EventDisplayer *ed):
  ControlsWidget(ed, true)
{
  new WText(tr("ext-intro"), this);
}

void ExtWidgets::populateSubMenu(WMenu *menu)
{
  menu->addItem("Ext::Button",
		deferCreate(boost::bind(&ExtWidgets::eButton, this)));
  menu->addItem("Ext::LineEdit",
		deferCreate(boost::bind(&ExtWidgets::eLineEdit, this)));
  menu->addItem("Ext::NumberField",
		deferCreate(boost::bind(&ExtWidgets::eNumberField, this)));
  menu->addItem("Ext::CheckBox",
		deferCreate(boost::bind(&ExtWidgets::eCheckBox, this)));
  menu->addItem("Ext::ComboBox",
		deferCreate(boost::bind(&ExtWidgets::eComboBox, this)));
  menu->addItem("Ext::RadioButton",
		deferCreate(boost::bind(&ExtWidgets::eRadioButton, this)));
  menu->addItem("Ext::Calendar",
		deferCreate(boost::bind(&ExtWidgets::eCalendar, this)));
  menu->addItem("Ext::DateField",
		deferCreate(boost::bind(&ExtWidgets::eDateField, this)));
  menu->addItem("Ext::Menu/ToolBar",
		deferCreate(boost::bind(&ExtWidgets::eMenu, this)));
  menu->addItem("Ext::Dialog",
		deferCreate(boost::bind(&ExtWidgets::eDialog, this)));
  //menu->addItem("Ext::Splitter", new WText("TODO: Ext::Splitter"));
}

WWidget *ExtWidgets::eButton()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Ext::Button", result);
  new WText(tr("ext-Button"), result);

  WContainerWidget *ex = new WContainerWidget(result);
  WVBoxLayout *vLayout = new WVBoxLayout();
  ex->setLayout(vLayout, AlignTop);
  vLayout->setContentsMargins(0, 0, 0, 0);
  vLayout->setSpacing(3);

  Button *button;
  vLayout->addWidget(button = new Button("Push me!"));
  ed_->showSignal(button->clicked(), "Ext::Button clicked");
  vLayout->addWidget(button = new Button("Checkable button"));
  button->setCheckable(true);
  ed_->showSignal(button->clicked(), "Ext::Button (checkable) clicked");
  vLayout->addWidget(button = new Button("Button with icon"));
  button->setIcon("icons/yellow-folder-open.png");
  ed_->showSignal(button->clicked(), "Ext::Button (with icon) clicked");

  return result;
}

WWidget *ExtWidgets::eLineEdit()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Ext::LineEdit", result);

  new WText(tr("ext-LineEdit"), result);
  LineEdit *le;
  le = new LineEdit(result);
  le->setTextSize(50);
  ed_->showSignal(le->keyWentUp(), "Ext::LineEdit keyWentUp");

  return result;
}

WWidget *ExtWidgets::eNumberField()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Ext::NumberField", result);

  new WText(tr("ext-NumberField"), result);
  new WText("Total amount to pay: ", result);
  NumberField *nf = new NumberField(result);
  nf->setDecimalPrecision(2);
  nf->setInline(true);
  ed_->showSignal(nf->keyPressed(), "Ext::NumberField keyPressed");

  return result;
}

WWidget *ExtWidgets::eCheckBox()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Ext::CheckBox", result);
  
  new WText(tr("ext-CheckBox"), result);
  CheckBox *cb;
  cb = new CheckBox("Check me!", result);
  ed_->showSignal(cb->checked(), "Ext::CheckBox checked");
  cb = new CheckBox("Check me too!", result);
  cb->setChecked(true);
  ed_->showSignal(cb->checked(), "Ext::CheckBox too checked");

  return result;
}

WWidget *ExtWidgets::eComboBox()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Ext::ComboBox", result);

  new WText(tr("ext-ComboBox"), result);
  ComboBox *cb = new ComboBox(result);
  cb->addItem("Stella");
  cb->addItem("Duvel");
  cb->addItem("Sloeber");
  cb->addItem("Westmalle");
  cb->addItem("Kwak");
  cb->addItem("Hoegaarden");
  cb->addItem("Palm");
  cb->addItem("Westvleteren");
  cb->setCurrentIndex(1);
  ed_->showSignal(cb->activated(), "Ext::ComboBox activated");

  return result;
}

WWidget *ExtWidgets::eRadioButton()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Ext::RadioButton", result);

  new WText(tr("ext-RadioButton"), result);
  WButtonGroup *bg = new WButtonGroup(result);
  RadioButton *rb;
  rb = new RadioButton("Kitchen", result);
  bg->addButton(rb);
  ed_->showSignal(rb->checked(), "Ext::RadioButton Kitchen checked");
  rb = new RadioButton("Dining room", result);
  bg->addButton(rb);
  ed_->showSignal(rb->checked(), "Ext::RadioButton Dining Room checked");
  rb = new RadioButton("Garden", result);
  bg->addButton(rb);
  ed_->showSignal(rb->checked(), "Ext::RadioButton Garden checked");
  rb = new RadioButton("Attic", result);
  bg->addButton(rb);
  ed_->showSignal(rb->checked(), "Ext::RadioButton Attic checked");

  return result;
}

WWidget *ExtWidgets::eCalendar()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Ext::Calendar", result);

  new WText(tr("ext-Calendar"), result);
  Calendar *c = new Calendar(false, result);
  ed_->showSignal(c->selectionChanged(), "Ext::Calendar selectionChanged");

  return result;
}

WWidget *ExtWidgets::eDateField()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Ext::DateField", result);

  new WText(tr("ext-DateField"), result);
  DateField *df = new DateField(result);
  df->setFormat("ddd MMM d yyyy");
  df->setTextSize(25);

  return result;
}

WWidget *ExtWidgets::eMenu()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Ext::Menu", "Ext::ToolBar", result);

  new WText(tr("ext-Menu"), result);
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
  
  Ext::ToolBar *toolBar = new Ext::ToolBar(result);
  
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

  return result;
}

WWidget *ExtWidgets::eDialog()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Ext::Dialog", "Ext::MessageBox", "Ext::ProgressDialog", result);

  new WText(tr("ext-Dialog"), result);

  return result;
}

