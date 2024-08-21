#include <Wt/WLabel.h>
#include <Wt/WPasswordEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTemplate.h>
#include <Wt/WString.h>

SAMPLE_BEGIN(PasswordEditDefault)

auto form = std::make_unique<Wt::WTemplate>(Wt::WString::tr("passwordEditDefault-template"));
form->addFunction("id", &Wt::WTemplate::Functions::id);

auto psw = form->bindWidget("password", std::make_unique<Wt::WPasswordEdit>());
psw->setPlaceholderText("Enter a password");
psw->setAutoComplete(Wt::AutoCompleteMode::Off);

auto button = form->bindWidget("save", std::make_unique<Wt::WPushButton>("Save"));

auto out = form->bindWidget("out", std::make_unique<Wt::WText>());

button->clicked().connect([=] {
    if (psw->validate() != Wt::ValidationState::Valid)
        out->setText("You should enter a password!");
    else {
        out->setText("You entered the password " + psw->text());
    }
});

SAMPLE_END(return std::move(form))