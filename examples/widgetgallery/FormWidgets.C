/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FormWidgets.h"
#include "EventDisplayer.h"
#include "DeferredWidget.h"

#include <Wt/WBreak>
#include <Wt/WButtonGroup>
#include <Wt/WCalendar>
#include <Wt/WCheckBox>
#include <Wt/WComboBox>
#include <Wt/WDatePicker>
#include <Wt/WFileUpload>
#include <Wt/WInPlaceEdit>
#include <Wt/WLineEdit>
#include <Wt/WProgressBar>
#include <Wt/WPushButton>
#include <Wt/WRadioButton>
#include <Wt/WSelectionBox>
#include <Wt/WSpinBox>
#include <Wt/WSuggestionPopup>
#include <Wt/WText>
#include <Wt/WTextArea>
#include <Wt/WTextEdit>
#include <Wt/WStandardItem>
#include <Wt/WPopupMenu>
#include <Wt/WPopupMenuItem>
#include <Wt/WLabel>

using namespace Wt;

FormWidgets::FormWidgets(EventDisplayer *ed)
  : ControlsWidget(ed, true)
{
  new WText(tr("formwidgets-intro"), this);
}

void FormWidgets::populateSubMenu(WMenu *menu)
{
  menu->addItem("WPushButton", wPushButton());
  menu->addItem("WCheckBox", wCheckBox());
  menu->addItem("WRadioButton", wRadioButton());
  menu->addItem("WComboBox", wComboBox());
  menu->addItem("WSelectionBox", wSelectionBox());
  menu->addItem("WLineEdit", wLineEdit());
  menu->addItem("WSpinBox", wSpinBox());
  menu->addItem("WTextArea", wTextArea());
  menu->addItem("WCalendar", wCalendar());
  menu->addItem("WDatePicker", wDatePicker());
  menu->addItem("WInPlaceEdit", wInPlaceEdit());
  menu->addItem("WSuggestionPopup", wSuggestionPopup());
  menu->addItem("WTextEdit",
		deferCreate(boost::bind(&FormWidgets::wTextEdit, this)));
  menu->addItem("WFileUpload", wFileUpload());
#ifndef WT_TARGET_JAVA
  menu->addItem("WPopupMenu", wPopupMenu());
#endif
}

WWidget *FormWidgets::wPushButton()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WPushButton", result);
  new WText(tr("formwidgets-WPushButton"), result);
  WPushButton *pb = new WPushButton("Click me!", result);
  ed_->showSignal(pb->clicked(), "WPushButton click");

  new WText(tr("formwidgets-WPushButton-more"), result);
  pb = new WPushButton("Try to click me...", result);
  pb->setEnabled(false);
  
  return result;
}

WWidget *FormWidgets::wCheckBox()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WCheckBox", result);
  new WText(tr("formwidgets-WCheckBox"), result);
  WCheckBox *cb = new WCheckBox("Check me!", result);
  cb->setChecked(true);
  ed_->showSignal(cb->checked(), "'Check me!' checked");
  new WBreak(result);
  cb = new WCheckBox("Check me too!", result);
  ed_->showSignal(cb->checked(), "'Check me too!' checked");
  new WBreak(result);
  cb = new WCheckBox("Check me, I'm tristate!", result);
  cb->setTristate();
  cb->setCheckState(PartiallyChecked);
  ed_->showSignal(cb->checked(), "'Check me, I'm tristate!' checked");

  return result;
}

WWidget *FormWidgets::wRadioButton()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WRadioButton", result);
  new WText(tr("formwidgets-WRadioButton"), result);
  WRadioButton *rb = 0;
  rb = new WRadioButton("Radio me!", result);
  ed_->showSignal(rb->checked(), "'Radio me!' checked (not in buttongroup)");
  new WBreak(result);
  rb = new WRadioButton("Radio me too!", result);
  ed_->showSignal(rb->checked(), "'Radio me too!' checked "
		 "(not in buttongroup)");
  
  new WText(tr("formwidgets-WRadioButton-group"), result);
  WButtonGroup *wgb = new WButtonGroup(result);
  rb = new WRadioButton("Radio me!", result);
  ed_->showSignal(rb->checked(), "'Radio me!' checked");
  wgb->addButton(rb);
  new WBreak(result);
  rb = new WRadioButton("No, radio me!", result);
  ed_->showSignal(rb->checked(), "'No, Radio me!' checked");
  wgb->addButton(rb);
  new WBreak(result);
  rb = new WRadioButton("Nono, radio me!", result);
  ed_->showSignal(rb->checked(), "'Nono, radio me!' checked");
  wgb->addButton(rb);

  wgb->setSelectedButtonIndex(0);

  return result;
}

WWidget *FormWidgets::wComboBox()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WComboBox", result);
  new WText(tr("formwidgets-WComboBox"), result);
  WComboBox *cb = new WComboBox(result);
  cb->addItem("Heavy");
  cb->addItem("Medium");
  cb->addItem("Light");
  cb->setCurrentIndex(1); // select 'Medium'
  ed_->showSignal(cb->sactivated(), "Combo-box 1 activated: ");

  new WText(tr("formwidgets-WComboBox-model"), result);
  
  new WText(tr("formwidgets-WComboBox-style"), result);
  WComboBox *colorCb = new WComboBox(result);
  WStandardItemModel* model = new WStandardItemModel(colorCb);
  model->insertColumns(0, 3);
  addColorElement(model, "Red", "combo-red");
  addColorElement(model, "Blue", "combo-blue");
  addColorElement(model, "Green", "combo-green");
  colorCb->setModel(model);
  colorCb->setCurrentIndex(0); // select 'Red'
  ed_->showSignal(colorCb->sactivated(), "Combo-box 2 activated: ");

  return result;
}

void FormWidgets::addColorElement(WStandardItemModel* model,
				  std::string name, 
				  std::string style)
{
  WStandardItem* item = new WStandardItem(name);
  item->setStyleClass(style);
  model->appendRow(item);
}


WWidget *FormWidgets::wSelectionBox()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WSelectionBox", result);
  new WText(tr("formwidgets-WSelectionBox"), result);
  WSelectionBox *sb1 = new WSelectionBox(result);
  sb1->addItem("Heavy");
  sb1->addItem("Medium");
  sb1->addItem("Light");
  sb1->setCurrentIndex(1); // Select 'medium'
  ed_->showSignal(sb1->sactivated(), "SelectionBox activated: ");
  new WText("<p>... or multiple options (use shift and/or ctrl-click "
	    "to select your pizza toppings)</p>", result);
  WSelectionBox *sb2 = new WSelectionBox(result);
  sb2->addItem("Bacon");
  sb2->addItem("Cheese");
  sb2->addItem("Mushrooms");
  sb2->addItem("Green peppers");
  sb2->addItem("Red peppers");
  sb2->addItem("Ham");
  sb2->addItem("Pepperoni");
  sb2->addItem("Turkey");
  sb2->setSelectionMode(ExtendedSelection);
  std::set<int> selection;
  selection.insert(1);
  selection.insert(2);
  selection.insert(5);
  sb2->setSelectedIndexes(selection);
  ed_->showSignal(sb2->changed(), "SelectionBox 2 changed");

  new WText(tr("formwidgets-WSelectionBox-model"), result);
  
  return result;
}

WWidget *FormWidgets::wLineEdit()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WLineEdit", result);
  new WText(tr("formwidgets-WLineEdit"), result);

  WLineEdit *le = new WLineEdit(result);
  le->setEmptyText("Edit me");
  ed_->showSignal(le->keyWentUp(), "Line edit key up event");

  new WText("<p>The line edit on the following line reacts on the "
	    "enter button:</p>", result);

  le = new WLineEdit(result);
  ed_->showSignal(le->enterPressed(), "Line edit enter pressed event");

  new WText(tr("formwidgets-WLineEdit-more"), result);

  return result;
}

WWidget *FormWidgets::wSpinBox()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WSpinBox", result);
  new WText(tr("formwidgets-WSpinBox"), result);

  new WText("Enter a number between 0 and 100: ", result);
  WSpinBox *le = new WSpinBox(result);
  ed_->showSignal(le->changed(), "Spin box value changed");
  le->setValue(30);

  return result;
}

WWidget *FormWidgets::wTextArea()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WTextArea", result);
  new WText(tr("formwidgets-WTextArea"), result);

  WTextArea *ta = new WTextArea(result);
  ta->setColumns(80);
  ta->setRows(15);
  ta->setText(tr("formwidgets-WTextArea-contents"));
  ed_->showSignal(ta->changed(), "Text area changed");
 
  new WText(tr("formwidgets-WTextArea-related"), result);

  return result;
}

WWidget *FormWidgets::wCalendar()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WCalendar", result);

  new WText(tr("formwidgets-WCalendar"), result);

  WCalendar *c = new WCalendar(result);
  ed_->showSignal(c->selectionChanged(), "First calendar's selection changed");
  new WText("<p>A flag indicates if multiple dates can be selected...</p>",
	    result);
  WCalendar *c2 = new WCalendar(result);
  c2->setSelectionMode(ExtendedSelection);
  ed_->showSignal(c2->selectionChanged(), "Second calendar's selection changed");

  return result;
}

WWidget *FormWidgets::wDatePicker()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WDatePicker", result);
  new WText("<p>The <tt>WDatePicker</tt> allows the entry of a date.</p>",
	    result);

  WDatePicker* dp1 = new WDatePicker(result);
  ed_->showSignal(dp1->lineEdit()->changed(), "Date picker 1 changed");
  new WText("(format " + dp1->format() + ")", result);
  
  new WBreak(result);
  
  WDatePicker* dp2 = new WDatePicker(result);
  ed_->showSignal(dp2->lineEdit()->changed(), "Date picker 2 changed");
  dp2->setFormat("dd MM yyyy");
  new WText("(format " + dp2->format() + ")", result);

  return result;
}

WWidget *FormWidgets::wInPlaceEdit()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WInPlaceEdit", result);
  new WText("<p>This widget allows you to edit a text in-place by clicking "
	    "on it. You can enable the save/cancel buttons (like here below) "
	    "or disable them (as used in the <tt>WCalendar</tt> widget to edit "
	    "the year).</p>",
	    result);
  new WText("Try it here: ", result);
  WInPlaceEdit *ipe = new WInPlaceEdit("This is editable text", result);
  ipe->setStyleClass("in-place-edit");
  ed_->showSignal(ipe->valueChanged(), "In-place edit changed: ");

  return result;
}

WWidget *FormWidgets::wSuggestionPopup()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WSuggestionPopup", result);
  new WText(tr("formwidgets-WSuggestionPopup"), result);

  // options for email address suggestions
  WSuggestionPopup::Options contactOptions;
  contactOptions.highlightBeginTag = "<span class=\"highlight\">";
  contactOptions.highlightEndTag = "</span>";
  contactOptions.listSeparator = ',';
  contactOptions.whitespace = " \\n";
  contactOptions.wordSeparators = "-., \"@\\n;";
  contactOptions.appendReplacedText = ", ";

  WSuggestionPopup *sp =
    new WSuggestionPopup(WSuggestionPopup::generateMatcherJS(contactOptions),
			 WSuggestionPopup::generateReplacerJS(contactOptions),
			 result);
  WLineEdit *le = new WLineEdit(result);
  le->setTextSize(50);
  le->setInline(false);
  sp->forEdit(le);
  sp->addSuggestion("John Tech <techie@mycompany.com>",
		    "John Tech <techie@mycompany.com>");
  sp->addSuggestion("Johnny Cash <cash@mycompany.com>", 
		    "Johnny Cash <cash@mycompany.com>");
  sp->addSuggestion("John Rambo <rambo@mycompany.com>",
		    "John Rambo <rambo@mycompany.com>");
  sp->addSuggestion("Johanna Tree <johanna@mycompany.com>",
		    "Johanna Tree <johanna@mycompany.com>");

  return result;
}

WWidget *FormWidgets::wTextEdit()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WTextEdit", result);
  new WText("<p>The <tt>WTextEdit</tt> is a full-featured editor for rich text"
	    "editing. It is based on the TinyMCE editor, which must be "
	    "downloaded separately from its author's website. The TinyMCE "
	    "toolbar layout and plugins can be configured through Wt's "
	    "interface. The default, shown below, covers only a small "
	    "portion of TinyMCE's capabilities.</p>", result);
  WTextEdit *te = new WTextEdit(result);
  ed_->showSignal(te->changed(), "Text edit changed");

  return result;
}

WWidget *FormWidgets::wFileUpload()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WFileUpload", result);
  new WText(tr("formwidgets-WFileUpload"), result);
  WFileUpload *const fu = new WFileUpload(result);
  fu->setProgressBar(new WProgressBar());
  fu->changed().connect(fu, &WFileUpload::upload);
  ed_->showSignal(fu->changed(), "File upload changed");
  ed_->showSignal(fu->uploaded(), "File upload finished");
  new WText(tr("formwidgets-WFileUpload-more"), result);

  return result;
}

#ifndef WT_TARGET_JAVA
WWidget *FormWidgets::wPopupMenu()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WPopupMenu", "WPopupMenuItem", result);
  new WText(tr("formwidgets-WPopupMenu"), result);

  WPopupMenu *popup = new WPopupMenu();
  popup->addItem("icons/house.png", "Build a house");
  popup->addItem("Roof included")->setCheckable(true);
  popup->addItem("Add a door")->setDisabled(true);
  popup->addSeparator();
  popup->addItem("Add a window");
  WPopupMenu *subMenu = new WPopupMenu();
  subMenu->addItem("Add a chair");
  subMenu->addItem("Add a table");
  popup->addMenu("Add furniture", subMenu);
  
  WLabel* clickMe = new WLabel("Clicking here will show a popup menu.", result);
  clickMe->setStyleClass("popupmenuLabel");
  clickMe->clicked().connect(popup, &WPopupMenu::popup);
  
  return result;
}
#endif

