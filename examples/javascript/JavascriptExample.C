/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iostream>

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>

#include "JavascriptExample.h"
#include "Popup.h"

JavascriptExample::JavascriptExample(const WEnvironment& env)
  : WApplication(env)
{
  setTitle("Javascript example");

  // Create a popup for prompting the amount of money, and connect the
  // okPressed button to the slot for setting the amount of money.
  //
  // Note that the input provided by the user in the prompt box is passed as
  // an argument to the slot.
  promptAmount_ = Popup::createPrompt("How much do you want to pay?","");
  promptAmount_->okPressed().connect(std::bind(&JavascriptExample::setAmount,
                                               this, std::placeholders::_1));

  // Create a popup for confirming the payment.
  //
  // Since a confirm popup does not allow input, we ignore the
  // argument carrying the input (which will be empty anyway).
  confirmPay_ = Popup::createConfirm("");
  confirmPay_->okPressed().connect(this, &JavascriptExample::confirmed);

  root()->addWidget(std::make_unique<WText>("<h2>Wt Javascript example</h2>"
	    "<p>Wt makes abstraction of Javascript, and therefore allows you"
	    " to develop web applications without any knowledge of Javascript,"
	    " and which are not dependent on Javascript."
	    " However, Wt does allow you to add custom Javascript code:</p>"
	    " <ul>"
	    "   <li>To call custom JavaScript code from an event handler, "
	    "connect the Wt::EventSignal to a Wt::JSlot.</li>"
	    "   <li>To call C++ code from custom JavaScript, use "
	    "Wt.emit() to emit a Wt::JSignal.</li>"
	    "   <li>To call custom JavaScript code from C++, use "
	    "WApplication::doJavascript() or Wt::JSlot::exec().</li>"
	    " </ul>"
	    "<p>This simple application shows how to interact between C++ and"
	    " JavaScript using the JSlot and JSignal classes.</p>"));

  currentAmount_
    = root()->addWidget(std::make_unique<WText>("Current amount: $" +
                                                  promptAmount_->defaultValue()));

  auto amountButton =
      root()->addWidget(std::make_unique<WPushButton>("Change ..."));
  amountButton->setMargin(10, Side::Left | Side::Right);

  root()->addWidget(std::make_unique<WBreak>());

  auto confirmButton =
      root()->addWidget(std::make_unique<WPushButton>("Pay now."));
  confirmButton->setMargin(10, Side::Top | Side::Bottom);

  // Connect the event handlers to a JSlot: this will execute the JavaScript
  // immediately, without a server round trip.
  amountButton->clicked().connect(promptAmount_->show);
  confirmButton->clicked().connect(confirmPay_->show);

  // Set the initial amount
  setAmount("1000");
}

void JavascriptExample::setAmount(const std::string amount)
{
  // Change the confirmation message to include the amount.
  confirmPay_->setMessage("Are you sure you want to pay $" + amount + " ?");

  // Change the default value for the prompt.
  promptAmount_->setDefaultValue(amount);

  // Change the text that shows the current amount.
  currentAmount_->setText("Current amount: $" + promptAmount_->defaultValue());
}

void JavascriptExample::confirmed()
{
  root()->addWidget(std::make_unique<WText>("<br/>Just payed $" +
                                              promptAmount_->defaultValue() + "."));
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return std::make_unique<JavascriptExample>(env);
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}

