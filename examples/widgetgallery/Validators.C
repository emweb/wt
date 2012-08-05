/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#include "Validators.h"
#include "EventDisplayer.h"
#include <Wt/WText>
#include <Wt/WLineEdit>
#include <Wt/WIntValidator>
#include <Wt/WDoubleValidator>
#include <Wt/WDateValidator>
#include <Wt/WLengthValidator>
#include <Wt/WRegExpValidator>
#include <Wt/WTable>
#include <Wt/WTableCell>
#include <Wt/WTextArea>
#include <Wt/WPushButton>
#include <Wt/WBreak>

using namespace Wt;

Validators::Validators(EventDisplayer *ed)
  : ControlsWidget(ed, false)
{
  topic("WValidator", this);

  addText(tr("validators-intro"),
	    this);

  addText("<h2>Validator types</h2>", this);
  WTable *table = new WTable(this);
  table->setStyleClass("validators");
  WLineEdit *le;

  addText("<tt>WIntValidator</tt>: input is mandatory and in range [50 - 100]",
	    table->elementAt(0, 0));
  le = new WLineEdit(table->elementAt(0, 1));
  WIntValidator *iv = new WIntValidator(50, 100);
  iv->setMandatory(true);
  le->setValidator(iv);
  fields_.push_back(std::pair<WFormWidget *, WText *>
		    (le, addText("", table->elementAt(0, 2))));

  addText("<tt>WDoubleValidator</tt>: range [-5.0 to 15.0]",
	  table->elementAt(1, 0));
  le = new WLineEdit(table->elementAt(1, 1));
  le->setValidator(new WDoubleValidator(-5, 15));
  fields_.push_back(std::pair<WFormWidget *, WText *>
		    (le, addText("", table->elementAt(1, 2))));

  addText("<tt>WDateValidator</tt>, default format \"yyyy-MM-dd\"",
	  table->elementAt(2, 0));
  le = new WLineEdit(table->elementAt(2, 1));
  le->setValidator(new WDateValidator());
  fields_.push_back(std::pair<WFormWidget *, WText *>
		    (le, addText("", table->elementAt(2, 2))));

  addText("<tt>WDateValidator</tt>, format \"dd-MM-yy\"",
	  table->elementAt(3, 0));
  le = new WLineEdit(table->elementAt(3, 1));
  le->setValidator(new WDateValidator("dd-MM-yy"));
  fields_.push_back(std::pair<WFormWidget *, WText *>
		    (le, addText("", table->elementAt(3, 2))));

  addText("<tt>WDateValidator</tt>, format \"yy-MM-dd\", range 1 to 15 October 08",
	    table->elementAt(4, 0));
  le = new WLineEdit(table->elementAt(4, 1));
  le->setValidator(new WDateValidator("yy-MM-dd", WDate(2008, 10, 1),
				      WDate(2008, 10, 15)));
  fields_.push_back(std::pair<WFormWidget *, WText *>
		    (le, addText("", table->elementAt(4, 2))));

  addText("<tt>WLengthValidator</tt>, 6 to 11 characters",
	  table->elementAt(5, 0));
  le = new WLineEdit(table->elementAt(5, 1));
  le->setValidator(new WLengthValidator(6, 11));
  fields_.push_back(std::pair<WFormWidget *, WText *>(le, addText("", table->elementAt(5, 2))));

  std::string ipRegexp =
    "((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)";
  addText("<tt>WRegExpValidator</tt>, IP address", table->elementAt(6, 0));
  le = new WLineEdit(table->elementAt(6, 1));
  le->setValidator(new WRegExpValidator(ipRegexp));
  fields_.push_back(std::pair<WFormWidget *, WText *>
		    (le, addText("", table->elementAt(6, 2))));
  
  addText("<p>The IP address validator regexp is: <tt>" + ipRegexp
	  + "</tt></p>", this);

  addText("<p>All WFormWidgets can have validators, so also the "
	  "WTextArea. Type up to 50 characters in the box below</p>", this);
  WTextArea *ta = new WTextArea(this);
  ta->setMargin(4, Right);
  ta->setValidator(new WLengthValidator(0, 50));
  fields_.push_back(std::pair<WFormWidget *, WText *>(ta, addText("", this)));


  addText("<h2>Server-side validation</h2>", this);
  addText("<p>The button below causes the server to validate all "
	  "input fields above server-side, and puts the state of the "
	  "validation on the right of every widget: "
	  "<dl>"
	  " <dt><tt>Valid</tt></dt><dd>data is valid</dd>"
	  " <dt><tt>Invalid</tt></dt><dd>data is invalid</dd>"
	  " <dt><tt>InvalidEmpty</tt></dt><dd>field is empty, "
	  "but was indicated to be mandatory</dd>"
	  "</dl></p>", this);
  WPushButton *pb = new WPushButton("Validate server-side", this);
  pb->clicked().connect(this, &Validators::validateServerside);
  ed->showSignal(pb->clicked(), "WPushButton: request server-side validation");
}

void Validators::validateServerside()
{
  for (unsigned int i = 0; i < fields_.size(); ++i) {
    switch (fields_[i].first->validate()) {
    case WValidator::Valid:
      fields_[i].second->setText("Valid");
      break;
    case WValidator::InvalidEmpty:
      fields_[i].second->setText("InvalidEmpty");
      break;
    case WValidator::Invalid:
      fields_[i].second->setText("Invalid");
      break;
    } 
  }
}
