#include <Wt/WLabel.h>
#include <Wt/WPasswordEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WString.h>

SAMPLE_BEGIN(PasswordEditReveal)

auto form = std::make_unique<Wt::WTemplate>(Wt::WString::tr("PasswordEditReveal-template"));
form->addFunction("id", &Wt::WTemplate::Functions::id);

auto psw = form->bindWidget("password", std::make_unique<Wt::WPasswordEdit>());
psw->setPlaceholderText("Enter a password");
psw->setAutoComplete(Wt::AutoCompleteMode::Off);
psw->setRevealButtonEnabled(true);

auto toggle = form->bindWidget("toggle", std::make_unique<Wt::WPushButton>("Show"));

toggle->clicked().connect([=] {
    psw->setTextVisible(!psw->isTextVisible());
    toggle->setText(psw->isTextVisible() ? "Hide" : "Show");
});

psw->textVisibilityChanged().connect([=] {
    toggle->setText(psw->isTextVisible() ? "Hide" : "Show");
});

SAMPLE_END(return std::move(form))
