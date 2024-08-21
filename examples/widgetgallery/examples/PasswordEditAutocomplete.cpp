#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPasswordEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTemplate.h>
#include <Wt/WString.h>

SAMPLE_BEGIN(PasswordEditAutocomplete)

auto form = std::make_unique<Wt::WTemplate>(Wt::WString::tr("PasswordEditAutocomplete-template"));
form->addFunction("id", &Wt::WTemplate::Functions::id);

auto username = form->bindWidget("username", std::make_unique<Wt::WLineEdit>());
username->setAutoComplete(Wt::AutoCompleteMode::Username);

auto psw = form->bindWidget("password", std::make_unique<Wt::WPasswordEdit>());
psw->setAutoComplete(Wt::AutoCompleteMode::CurrentPassword);

auto button = form->bindWidget("login", std::make_unique<Wt::WPushButton>("Login"));

auto out = form->bindWidget("out", std::make_unique<Wt::WText>());

button->clicked().connect([=] {
    if (username->text().empty()) {
        out->setText("You should enter your Username!");
    } else if (psw->validate() != Wt::ValidationState::Valid) {
        out->setText("You should enter your password!");
    } else {
        out->setText("You are logged in as " + username->text());
    }
});

SAMPLE_END(return std::move(form))