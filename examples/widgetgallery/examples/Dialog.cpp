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
#ifndef WT_TARGET_JAVA
    auto dialog = owner->addChild(std::make_unique<Wt::WDialog>("Go to cell"));
#else // WT_TARGET_JAVA
    auto dialog = new Wt::WDialog("Go to cell");
#endif // WT_TARGET_JAVA

    Wt::WLabel *label =
        dialog->contents()->addNew<Wt::WLabel>("Cell location (A1..Z999)");
    Wt::WLineEdit *edit =
        dialog->contents()->addNew<Wt::WLineEdit>();
    label->setBuddy(edit);

    dialog->contents()->addStyleClass("form-group");

    auto validator =
        std::make_shared<Wt::WRegExpValidator>("[A-Za-z][1-9][0-9]{0,2}");
    validator->setMandatory(true);
    edit->setValidator(validator);

    Wt::WPushButton *ok =
        dialog->footer()->addNew<Wt::WPushButton>("OK");
    ok->setDefault(true);
    if (wApp->environment().ajax())
      ok->disable();

    Wt::WPushButton *cancel =
        dialog->footer()->addNew<Wt::WPushButton>("Cancel");
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

#ifndef WT_TARGET_JAVA
        owner->removeChild(dialog);
#else // WT_TARGET_JAVA
        delete dialog;
#endif // WT_TARGET_JAVA
    });

    dialog->show();
}

}
SAMPLE_BEGIN(Dialog)
auto container = std::make_unique<Wt::WContainerWidget>();

Wt::WPushButton *button = container->addNew<Wt::WPushButton>("Jump");

Wt::WText *out = container->addNew<Wt::WText>();
out->setStyleClass("help-block");

auto c = container.get();
button->clicked().connect([=] {
  showDialog(c, out);
});

SAMPLE_END(return std::move(container))
