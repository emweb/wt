#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDialog.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WRegExpValidator.h>
#include <Wt/WText.h>

namespace {

extern void showDialog(Wt::WObject *owner, Wt::WText *out)
{
    auto dialog = owner->addChild(Wt::cpp14::make_unique<Wt::WDialog>("Go to cell"));

    Wt::WLabel *label =
        dialog->contents()->addWidget(Wt::cpp14::make_unique<Wt::WLabel>("Cell location (A1..Z999)"));
    Wt::WLineEdit *edit =
        dialog->contents()->addWidget(Wt::cpp14::make_unique<Wt::WLineEdit>());
    label->setBuddy(edit);

    dialog->contents()->addStyleClass("form-group");

    auto validator =
        std::make_shared<Wt::WRegExpValidator>("[A-Za-z][1-9][0-9]{0,2}");
    validator->setMandatory(true);
    edit->setValidator(validator);

    Wt::WPushButton *ok =
        dialog->footer()->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("OK"));
    ok->setDefault(true);
    if (wApp->environment().ajax())
      ok->disable();

    Wt::WPushButton *cancel =
        dialog->footer()->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("Cancel"));
    dialog->rejectWhenEscapePressed();

    edit->keyWentUp().connect([=] {
        ok->setDisabled(edit->validate() != Wt::ValidationState::Valid);
    });

    /*
     * Accept the dialog
     */
    ok->clicked().connect([=] {
        if (edit->validate() == Wt::ValidationState::Valid)
            dialog->accept();
    });

    /*
     * Reject the dialog
     */
    cancel->clicked().connect(dialog, &Wt::WDialog::reject);

    /*
     * Process the dialog result.
     */
    dialog->finished().connect([=] {
        if (dialog->result() == Wt::DialogCode::Accepted)
	    out->setText("New location: " + edit->text());
	else
	    out->setText("No location selected.");

        owner->removeChild(dialog);
    });

    dialog->show();
}

}
SAMPLE_BEGIN(Dialog)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WPushButton *button = container->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("Jump"));

Wt::WText *out = container->addWidget(Wt::cpp14::make_unique<Wt::WText>());
out->setStyleClass("help-block");

auto c = container.get();
button->clicked().connect([=] {
  showDialog(c, out);
});

SAMPLE_END(return std::move(container))
