#include <Wt/WIntValidator>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WTemplate>
#include <Wt/WText>

SAMPLE_BEGIN(Validation)

Wt::WTemplate *t = new Wt::WTemplate(Wt::WString::tr("validation-template"));
t->addFunction("id", &Wt::WTemplate::Functions::id);

Wt::WLineEdit *ageEdit = new Wt::WLineEdit();
t->bindWidget("age", ageEdit);

Wt::WIntValidator *validator = new Wt::WIntValidator(0, 150);
validator->setMandatory(true);
ageEdit->setValidator(validator);

Wt::WPushButton *button = new Wt::WPushButton("Save");
t->bindWidget("button", button);

Wt::WText *out = new Wt::WText();
out->setInline(false);
out->hide();
t->bindWidget("age-info", out);

button->clicked().connect(std::bind([=] () {
    out->show();
    if (ageEdit->validate() == Wt::WValidator::Valid) {
	out->setText("Age of " + ageEdit->text() + " is saved!");
	out->setStyleClass("alert alert-success");
    } else {
	out->setText("The number must be in the range 0 to 150");
	out->setStyleClass("alert alert-danger");
    }
}));

ageEdit->enterPressed().connect(std::bind([=] () {
    button->clicked().emit(Wt::WMouseEvent());
}));

SAMPLE_END(return t)
