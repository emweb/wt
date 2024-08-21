#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPasswordEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTemplate.h>
#include <Wt/WString.h>

SAMPLE_BEGIN(PasswordEditNative)

auto form = std::make_unique<Wt::WTemplate>(Wt::WString::tr("PasswordEditNative-template"));
form->addFunction("id", &Wt::WTemplate::Functions::id);

auto username = form->bindWidget("username", std::make_unique<Wt::WLineEdit>());
username->setAutoComplete(Wt::AutoCompleteMode::Username);

auto psw = form->bindWidget("password", std::make_unique<Wt::WPasswordEdit>());
psw->setAutoComplete(Wt::AutoCompleteMode::NewPassword);
psw->setNativeControl(true);

// This pattern forces the password to contain at least one uppercase, one lowercase and one digit.
// This is just an example! We discourage you to use this regex as a safe production pattern.
psw->setPattern("^(?=.*\\d)(?=.*[a-z])(?=.*[A-Z]).{1,}$");

// Sets the minimum length of the password to 5 characters.
// Again, this is just an example! We strongly encourage you to have a higher minimum length for production.
psw->setMinLength(5);
psw->setToolTip("Password should be at least of lenth 5 and contain an uppercase letter, a lowercase letter and a number.");

SAMPLE_END(return std::move(form))