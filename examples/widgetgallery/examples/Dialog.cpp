#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WDialog>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WRegExpValidator>
#include <Wt/WText>

namespace {

extern void showDialog(Wt::WText *out)
{
    Wt::WDialog *dialog = new Wt::WDialog("Go to cell");

    Wt::WLabel *label = new Wt::WLabel("Cell location (A1..Z999)",
				       dialog->contents());
    Wt::WLineEdit *edit = new Wt::WLineEdit(dialog->contents());
    label->setBuddy(edit);

    Wt::WRegExpValidator *validator = 
        new Wt::WRegExpValidator("[A-Za-z][1-9][0-9]{0,2}");
    validator->setMandatory(true);
    edit->setValidator(validator);

    Wt::WPushButton *ok = new Wt::WPushButton("OK", dialog->footer());
    ok->setDefault(true);
    ok->disable();

    Wt::WPushButton *cancel = new Wt::WPushButton("Cancel", dialog->footer());
    dialog->rejectWhenEscapePressed();

    edit->keyWentUp().connect(std::bind([=] () {
	ok->setDisabled(edit->validate() != Wt::WValidator::Valid);
    }));

    /*
     * Accept the dialog
     */
    ok->clicked().connect(std::bind([=] () {
	if (edit->validate())
	    dialog->accept();
    }));

    /*
     * Reject the dialog
     */
    cancel->clicked().connect(dialog, &Wt::WDialog::reject);

    /*
     * Process the dialog result.
     */
    dialog->finished().connect(std::bind([=] () {
	if (dialog->result() == Wt::WDialog::Accepted)
	    out->setText("New location: " + edit->text());
	else
	    out->setText("No location selected.");

	delete dialog;
    }));

    dialog->show();
}

}
SAMPLE_BEGIN(Dialog)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WPushButton *button = new Wt::WPushButton("Jump", container);

Wt::WText *out = new Wt::WText(container);
out->setStyleClass("help-block");

button->clicked().connect(std::bind([=] () {
      showDialog(out);
}));

SAMPLE_END(return container)
