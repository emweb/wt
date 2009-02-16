#include "DialogWidgets.h"
#include "EventDisplayer.h"
#include "DeferredWidget.h"

#include <Wt/WText>
#include <Wt/WBreak>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WVBoxLayout>
#include <Wt/Ext/Button>
#include <Wt/Ext/Dialog>
#include <Wt/Ext/MessageBox>
#include <Wt/Ext/ProgressDialog>
#include <Wt/WBorderLayout>
#include <Wt/WFitLayout>
#include <Wt/WApplication>
#include <Wt/WString>
#ifdef WIN32
#include <windows.h>
#undef MessageBox
#endif

using namespace Wt;

DialogWidgets::DialogWidgets(EventDisplayer *ed)
  : ControlsWidget(ed, true)
{
  new WText(tr("dialogs-intro"), this);
}

void DialogWidgets::populateSubMenu(WMenu *menu)
{
  menu->addItem("WDialog", wDialog());
  menu->addItem("WMessageBox", wMessageBox());
  menu->addItem("Ext Dialogs",
		deferCreate(boost::bind(&DialogWidgets::eDialogs, this)));
}

WWidget *DialogWidgets::wDialog()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WDialog", result);
  new WText(tr("dialogs-WDialog"), result);
  WPushButton *button = new WPushButton("Familiar", result);
  button->clicked.connect(SLOT(this, DialogWidgets::custom));

  return result;
}

WWidget *DialogWidgets::wMessageBox()
{
  WContainerWidget *result = new WContainerWidget();

  topic("WMessageBox", result);
  new WText(tr("dialogs-WMessageBox"),
	    result);
  
  WContainerWidget *ex = new WContainerWidget(result);
  
  WVBoxLayout *vLayout = new WVBoxLayout();
  ex->setLayout(vLayout, AlignTop);
  vLayout->setContentsMargins(0, 0, 0, 0);
  vLayout->setSpacing(3);

  WPushButton *button;
  vLayout->addWidget(button = new WPushButton("One liner"));
  button->clicked.connect(SLOT(this, DialogWidgets::messageBox1));
  vLayout->addWidget(button = new WPushButton("Show some buttons"));
  button->clicked.connect(SLOT(this, DialogWidgets::messageBox2));
  vLayout->addWidget(button = new WPushButton("Need confirmation"));
  button->clicked.connect(SLOT(this, DialogWidgets::messageBox3));
  vLayout->addWidget(button = new WPushButton("Discard"));
  button->clicked.connect(SLOT(this, DialogWidgets::messageBox4));

  return result;
}

WWidget *DialogWidgets::eDialogs()
{
  WContainerWidget *result = new WContainerWidget();

  topic("Ext::Dialog", "Ext::MessageBox", "Ext::ProgressDialog", result);
  new WText(tr("dialogs-ExtDialog"), result);
  WContainerWidget *ex = new WContainerWidget(result);
  
  WVBoxLayout *vLayout = new WVBoxLayout();
  ex->setLayout(vLayout, AlignTop);
  vLayout->setContentsMargins(0, 0, 0, 0);
  vLayout->setSpacing(3);
  
  
  WPushButton *button;
  
  vLayout->addWidget(button = new WPushButton("Ext Message Box"));
  button->clicked.connect(SLOT(this, DialogWidgets::createExtMessageBox));
  vLayout->addWidget(button = new WPushButton("Ext Dialog"));
  button->clicked.connect(SLOT(this, DialogWidgets::createExtDialog));
  vLayout->addWidget(button = new WPushButton("Ext Progress Bar"));
  button->clicked.connect(SLOT(this, DialogWidgets::createExtProgress));

  return result;
}

void DialogWidgets::messageBox1()
{
  WMessageBox::show("Information",
		    "One-liner dialogs have a simple constructor", Ok);
  ed_->setStatus("Ok'ed");
}

void DialogWidgets::messageBox2()
{
  messageBox_
    = new WMessageBox("Question",
		      "This is a modal dialog that invokes a signal when a button is pushed",
		      NoIcon, Yes | No | Cancel);

  messageBox_
    ->buttonClicked.connect(SLOT(this, DialogWidgets::messageBoxDone));

  messageBox_->show();
}

void DialogWidgets::messageBox3()
{
  StandardButton
    result = WMessageBox::show("Push it",
			       "Yes/No questions can be tested by "
			       "checking show()'s return value",
			       Ok | Cancel);

  if (result == Ok)
    ed_->setStatus("Accepted!");
  else
    ed_->setStatus("Cancelled!");
}

void DialogWidgets::messageBox4()
{
  messageBox_
    = new WMessageBox("Your work",
		      "Provide your own button text.<br/>"
		      "Your work is not saved",
		      NoIcon, NoButton);

  messageBox_->addButton("Cancel modifications", Cancel);
  messageBox_->addButton("Continue modifying work", Ok);

  messageBox_
    ->buttonClicked.connect(SLOT(this, DialogWidgets::messageBoxDone));

  messageBox_->show();
}

void DialogWidgets::messageBoxDone(StandardButton result)
{
  switch (result) {
  case Ok:
    ed_->setStatus("Ok'ed"); break;
  case Cancel:
    ed_->setStatus("Cancelled!"); break;
  case Yes:
    ed_->setStatus("Me too!"); break;
  case No:
    ed_->setStatus("Me neither!"); break;
  default:
    ed_->setStatus("Unkonwn result?");
  }

  delete messageBox_;
  messageBox_ = 0;
}

void DialogWidgets::custom()
{
  WDialog dialog("Personalia");

  new WText("You can freely format the contents of a WDialog by "
	    "adding any widget you want to it.<br/>Here, we added WText, "
	    "WLineEdit and WPushButton to a dialog", dialog.contents());
  new WBreak(dialog.contents());
  new WText("Enter your name: ", dialog.contents());
  WLineEdit edit(dialog.contents());
  new WBreak(dialog.contents());
  WPushButton ok("Ok", dialog.contents());

  edit.enterPressed.connect(SLOT(&dialog, WDialog::accept));
  ok.clicked.connect(SLOT(&dialog, WDialog::accept));

  if (dialog.exec() == WDialog::Accepted) {
    ed_->setStatus("Welcome, " + edit.text());
  }
}

void DialogWidgets::createExtMessageBox()
{
  Ext::MessageBox *mb = new Ext::MessageBox();

  mb->setWindowTitle("Wt is magnificent");
  mb->setText("Isn't Wt the ruler of them all?");

  mb->setButtons(Wt::Yes);
  mb->finished.connect(SLOT(this, DialogWidgets::deleteExtDialog));

  mb->show();

  extDialog_ = mb;
}

void DialogWidgets::createExtDialog()
{
  Ext::Dialog *d = new Ext::Dialog();
  d->setWindowTitle("Ext::Dialog with WBorderLayout");
  d->resize(400,300);
  d->setStyleClass("dialog");

  Ext::Button *okButton = new Ext::Button("Ok");
  okButton->activated.connect(SLOT(d, Ext::Dialog::accept));
  d->addButton(okButton);
  okButton->setDefault(true);

  Ext::Button *cancelButton = new Ext::Button("Cancel");
  cancelButton->activated.connect(SLOT(d, Ext::Dialog::reject));
  d->addButton(cancelButton);

  WBorderLayout *layout = new WBorderLayout();
  d->setLayout(layout);

  Ext::Panel *west = new Ext::Panel();
  west->setTitle("West");
  west->setResizable(true);
  west->setCollapsible(true);
  west->resize(100, WLength());
  west->setLayout(new WFitLayout());
  west->layout()->addWidget(new WText("This is a resizable and collapsible "
				      "panel"));
  layout->addWidget(west, WBorderLayout::West);

  Ext::Panel *center = new Ext::Panel();
  center->setTitle("Center");

  WBorderLayout *nestedLayout = new WBorderLayout();
  center->setLayout(nestedLayout);

  Ext::Panel *nestedNorth = new Ext::Panel();
  nestedLayout->addWidget(nestedNorth, WBorderLayout::North);
  nestedNorth->resize(WLength(), 70);
  nestedNorth->layout()->addWidget(
    new WText("Ext Dialogs, like Wt Dialogs, can contain any widget. This "
	      "is a dialog with a layout manager. The left pane can be "
	      "resized."));

  Ext::Panel *nestedCenter = new Ext::Panel();
  nestedLayout->addWidget(nestedCenter, WBorderLayout::Center);
  nestedCenter->layout()->addWidget(new WText("This is simply WText, but "
					      "could have been any widget."));

  layout->addWidget(center, WBorderLayout::Center);

  d->show();
  extDialog_ = d;
  extDialog_->finished.connect(SLOT(this, DialogWidgets::deleteExtDialog));
}

void DialogWidgets::createExtProgress()
{
  Ext::ProgressDialog d("Please wait while calculating Pi ...", "Cancel", 0, 7);
  d.setWindowTitle("Calculator");

  d.show();

  for (unsigned i = 0; i < 7; ++i) {
    d.setValue(i);
    wApp->processEvents();

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
	       "It does not matter, Pi is overrated", Ok);
      break;
    }
  }
}

void DialogWidgets::deleteExtDialog()
{
  if (extDialog_->result() == Ext::Dialog::Accepted) {
    ed_->setStatus("Ext dialog accecpted");
  } else {
    ed_->setStatus("Ext dialog rejected");
  }
  delete extDialog_;
}

