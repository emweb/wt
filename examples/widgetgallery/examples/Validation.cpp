#include <Wt/WIntValidator>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WTemplate>
#include <Wt/WText>

SAMPLE_BEGIN(Validation)

Wt::WTemplate *t
    = new Wt::WTemplate(Wt::WString::tr("validation-template"));
t->addFunction("id", &Wt::WTemplate::Functions::id);

Wt::WLineEdit *ageEdit = new Wt::WLineEdit();
t->bindWidget("age", ageEdit);

// Create a validator that accepts integer values between 0 and 150.
Wt::WIntValidator *validator = new Wt::WIntValidator(0, 150);
validator->setMandatory(true);
ageEdit->setValidator(validator);

Wt::WPushButton *button = new Wt::WPushButton("Save");
t->bindWidget("button", button);

Wt::WText *out = new Wt::WText();
t->bindWidget("age-info", out);

button->clicked().connect(std::bind([=] () {
    if (ageEdit->validate() == Wt::WValidator::Valid) {
	out->setText("Age of " + ageEdit->text() + " is saved!");
    } else {
	out->setText("The number must be in the range 0 to 150");
    }
}));

SAMPLE_END(return t)
