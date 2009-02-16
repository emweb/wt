/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iostream>

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WLineEdit>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WText>

#include "DialogExample.h"

using namespace Wt;

DialogExample::DialogExample(const WEnvironment& env)
  : WApplication(env),
    messageBox_(0)
{
  setTitle("Dialog example");

  new WText("<h2>Wt dialogs example</h2>", root());

  WContainerWidget *textdiv = new WContainerWidget(root());
  textdiv->setStyleClass("text");

  new WText("You can use WMessageBox for simple modal dialog boxes. <br />",
	    textdiv);

  WContainerWidget *buttons = new WContainerWidget(root());
  buttons->setStyleClass("buttons");

  WPushButton *button;

  button = new WPushButton("One liner", buttons);
  button->clicked.connect(SLOT(this, DialogExample::messageBox1));

  button = new WPushButton("Comfortable ?", buttons);
  button->clicked.connect(SLOT(this, DialogExample::messageBox2));

  button = new WPushButton("Havoc!", buttons);
  button->clicked.connect(SLOT(this, DialogExample::messageBox3));

  button = new WPushButton("Discard", buttons);
  button->clicked.connect(SLOT(this, DialogExample::messageBox4));

  button = new WPushButton("Familiar", buttons);
  button->clicked.connect(SLOT(this, DialogExample::custom));

  textdiv = new WContainerWidget(root());
  textdiv->setStyleClass("text");

  status_ = new WText("Go ahead...", textdiv);

  styleSheet().addRule(".buttons",
		       "padding: 5px;");
  styleSheet().addRule(".buttons BUTTON",
		       "padding-left: 4px; padding-right: 4px;"
		       "margin-top: 4px; display: block");
  styleSheet().addRule(".text", "padding: 4px 8px");

  if (environment().agentIE())
    styleSheet().addRule("body", "margin: 0px;"); // avoid scrollbar problems
}

void DialogExample::messageBox1()
{
  WMessageBox::show("Information",
		    "Enjoy displaying messages with a one-liner.", Ok);
  setStatus("Ok'ed");
}

void DialogExample::messageBox2()
{
  messageBox_
    = new WMessageBox("Question",
		      "Are you getting comfortable ?",
		      NoIcon, Yes | No | Cancel);

  messageBox_
    ->buttonClicked.connect(SLOT(this, DialogExample::messageBoxDone));

  messageBox_->show();
}

void DialogExample::messageBox3()
{
  StandardButton
    result = WMessageBox::show("Confirm", "About to wreak havoc... Continue ?",
			       Ok | Cancel);

  if (result == Ok)
    setStatus("Wreaking havoc.");
  else
    setStatus("Cancelled!");
}

void DialogExample::messageBox4()
{
  messageBox_
    = new WMessageBox("Your work",
		      "Your work is not saved",
		      NoIcon, NoButton);

  messageBox_->addButton("Cancel modifications", Ok);
  messageBox_->addButton("Continue modifying work", Cancel);

  messageBox_
    ->buttonClicked.connect(SLOT(this, DialogExample::messageBoxDone));

  messageBox_->show();
}

void DialogExample::messageBoxDone(StandardButton result)
{
  switch (result) {
  case Ok:
    setStatus("Ok'ed"); break;
  case Cancel:
    setStatus("Cancelled!"); break;
  case Yes:
    setStatus("Me too!"); break;
  case No:
    setStatus("Me neither!"); break;
  default:
    setStatus("Unkonwn result?");
  }

  delete messageBox_;
  messageBox_ = 0;
}

void DialogExample::custom()
{
  WDialog dialog("Personalia");

  new WText("Enter your name: ", dialog.contents());
  WLineEdit edit(dialog.contents());
  new WBreak(dialog.contents());
  WPushButton ok("Ok", dialog.contents());

  edit.setFocus();

  edit.enterPressed.connect(SLOT(&dialog, WDialog::accept));
  ok.clicked.connect(SLOT(&dialog, WDialog::accept));

  if (dialog.exec() == WDialog::Accepted) {
    setStatus("Welcome, " + edit.text());
  }
}

void DialogExample::setStatus(const WString& result)
{
  status_->setText(result);
}

WApplication *createApplication(const WEnvironment& env)
{
  return new DialogExample(env);
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}

