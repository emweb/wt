/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iostream>

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include "DialogExample.h"

using namespace Wt;

DialogExample::DialogExample(const WEnvironment& env)
  : WApplication(env),
    messageBox_(nullptr)
{
  setTitle("Dialog example");

  WContainerWidget *textdiv = root()->addWidget(cpp14::make_unique<WContainerWidget>());
  textdiv->setStyleClass("text");

  textdiv->addWidget(cpp14::make_unique<WText>("<h2>Wt dialogs example</h2>"));
  textdiv->addWidget(cpp14::make_unique<WText>(
                       "You can use WMessageBox for simple modal dialog boxes. <br />"));

  WContainerWidget *buttons = root()->addWidget(cpp14::make_unique<WContainerWidget>());
  buttons->setStyleClass("buttons");

  WPushButton *button = buttons->addWidget(cpp14::make_unique<WPushButton>("One liner"));
  button->clicked().connect(this, &DialogExample::messageBox1);

  button = buttons->addWidget(cpp14::make_unique<WPushButton>("Comfortable?"));
  button->clicked().connect(this, &DialogExample::messageBox2);

  button = buttons->addWidget(cpp14::make_unique<WPushButton>("Havoc!"));
  button->clicked().connect(this, &DialogExample::messageBox3);

  button = buttons->addWidget(cpp14::make_unique<WPushButton>("Discard"));
  button->clicked().connect(this, &DialogExample::messageBox4);

  button = buttons->addWidget(cpp14::make_unique<WPushButton>("Familiar"));
  button->clicked().connect(this, &DialogExample::custom);

  textdiv = root()->addWidget(cpp14::make_unique<WContainerWidget>());
  textdiv->setStyleClass("text");

  status_ = textdiv->addWidget(cpp14::make_unique<WText>("Go ahead..."));

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
                    "Enjoy displaying messages with a one-liner.", StandardButton::Ok);
  setStatus("Ok'ed");
}

void DialogExample::messageBox2()
{
  messageBox_
    = cpp14::make_unique<WMessageBox>("Question",
              "Are you getting comfortable ?",
            Icon::Question,
            StandardButton::Yes | StandardButton::No | StandardButton::Cancel);

  messageBox_
    ->buttonClicked().connect(this, &DialogExample::messageBoxDone);

  messageBox_->animateShow
    (WAnimation(AnimationEffect::Pop | AnimationEffect::Fade, TimingFunction::Linear, 250));
}

void DialogExample::messageBox3()
{
  StandardButton
    result = WMessageBox::show("Confirm", "About to wreak havoc... Continue ?",
                   StandardButton::Ok | StandardButton::Cancel,
                   WAnimation(AnimationEffect::SlideInFromTop));

  if (result == StandardButton::Ok)
    setStatus("Wreaking havoc.");
  else
    setStatus("Cancelled!");
}

void DialogExample::messageBox4()
{
  messageBox_
    = cpp14::make_unique<WMessageBox>("Warning!",
              "Are you sure you want to continue?\n"
              "You have unsaved changes.",
	      Icon::None, StandardButton::None);

  messageBox_->addButton("Discard Modifications", StandardButton::Ok);
  WPushButton *continueButton
    = messageBox_->addButton("Cancel", StandardButton::Cancel);
  messageBox_->setDefaultButton(continueButton);

  messageBox_
    ->buttonClicked().connect(this, &DialogExample::messageBoxDone);

  messageBox_->setOffsets(0, Side::Bottom);
  messageBox_->animateShow
    (WAnimation(AnimationEffect::SlideInFromBottom
        | AnimationEffect::Fade, TimingFunction::Linear, 250));
}

void DialogExample::messageBoxDone(StandardButton result)
{
  switch (result) {
  case StandardButton::Ok:
    setStatus("Ok'ed"); break;
  case StandardButton::Cancel:
    setStatus("Cancelled!"); break;
  case StandardButton::Yes:
    setStatus("Me too!"); break;
  case StandardButton::No:
    setStatus("Me neither!"); break;
  default:
    setStatus("Unknown result?");
  }

  messageBox_.reset();
}

void DialogExample::custom()
{
  WDialog dialog("Personalia");
  dialog.setClosable(true);
  dialog.setResizable(true);
  dialog.rejectWhenEscapePressed(true);

  dialog.contents()->addWidget(cpp14::make_unique<WText>("Enter your name: "));
  WLineEdit *edit = dialog.contents()->addWidget(cpp14::make_unique<WLineEdit>());
  WPushButton *ok = dialog.footer()->addWidget(cpp14::make_unique<WPushButton>("Ok"));
  ok->setDefault(true);

  edit->setFocus();
  ok->clicked().connect(&dialog, &WDialog::accept);

  if (dialog.exec() == DialogCode::Accepted) {
    setStatus("Welcome, " + edit->text());
  } else {
    setStatus("Oh nevermind!");
  }
}

void DialogExample::setStatus(const WString& result)
{
  status_->setText(result);
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return cpp14::make_unique<DialogExample>(env);
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}

