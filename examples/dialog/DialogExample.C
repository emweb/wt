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

  WContainerWidget *textdiv = new WContainerWidget(root());
  textdiv->setStyleClass("text");

  new WText("<h2>Wt dialogs example</h2>", textdiv);
  new WText("You can use WMessageBox for simple modal dialog boxes. <br />",
	    textdiv);

  WContainerWidget *buttons = new WContainerWidget(root());
  buttons->setStyleClass("buttons");

  WPushButton *button;

  button = new WPushButton("One liner", buttons);
  button->clicked().connect(this, &DialogExample::messageBox1);

  button = new WPushButton("Comfortable ?", buttons);
  button->clicked().connect(this, &DialogExample::messageBox2);

  button = new WPushButton("Havoc!", buttons);
  button->clicked().connect(this, &DialogExample::messageBox3);

  button = new WPushButton("Discard", buttons);
  button->clicked().connect(this, &DialogExample::messageBox4);

  button = new WPushButton("Familiar", buttons);
  button->clicked().connect(this, &DialogExample::custom);

  textdiv = new WContainerWidget(root());
  textdiv->setStyleClass("text");

  status_ = new WText("Go ahead...", textdiv);

  styleSheet().addRule(".buttons",
		       "padding: 5px;");
  styleSheet().addRule(".buttons BUTTON",
		       "padding-left: 4px; padding-right: 4px;"
		       "margin-top: 4px; display: block");

  // avoid scrollbar problems
  styleSheet().addRule(".text", "padding: 4px 8px");
  styleSheet().addRule("body", "margin: 0px;");
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
		      Question, Yes | No | Cancel);

  messageBox_
    ->buttonClicked().connect(this, &DialogExample::messageBoxDone);

  messageBox_->animateShow
    (WAnimation(WAnimation::Pop | WAnimation::Fade, WAnimation::Linear, 250));
}

void DialogExample::messageBox3()
{
  StandardButton
    result = WMessageBox::show("Confirm", "About to wreak havoc... Continue ?",
			       Ok | Cancel,
			       WAnimation(WAnimation::SlideInFromTop));

  if (result == Ok)
    setStatus("Wreaking havoc.");
  else
    setStatus("Cancelled!");
}

void DialogExample::messageBox4()
{
  messageBox_
    = new WMessageBox("Warning!",
		      "Are you sure you want to continue?\n"
		      "You have unsaved changes.",
		      NoIcon, NoButton);

  messageBox_->addButton("Discard Modifications", Ok);
  WPushButton *continueButton
    = messageBox_->addButton("Cancel", Cancel);
  messageBox_->setDefaultButton(continueButton);

  messageBox_
    ->buttonClicked().connect(this, &DialogExample::messageBoxDone);

  messageBox_->setOffsets(0, Bottom);
  messageBox_->animateShow
    (WAnimation(WAnimation::SlideInFromBottom
		| WAnimation::Fade, WAnimation::Linear, 250));
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
    setStatus("Unknown result?");
  }

  delete messageBox_;
  messageBox_ = 0;
}

void DialogExample::custom()
{
  WDialog dialog("Personalia");
  dialog.setClosable(true);
  dialog.setResizable(true);
  dialog.rejectWhenEscapePressed(true);

  new WText("Enter your name: ", dialog.contents());
  WLineEdit edit(dialog.contents());
  WPushButton ok("Ok", dialog.footer());
  ok.setDefault(true);

  edit.setFocus();
  ok.clicked().connect(&dialog, &WDialog::accept);

  if (dialog.exec() == WDialog::Accepted) {
    setStatus("Welcome, " + edit.text());
  } else {
    setStatus("Oh nevermind!");
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

