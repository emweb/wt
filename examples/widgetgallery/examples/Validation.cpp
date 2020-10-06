#include <Wt/WIntValidator.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(Validation)

auto t = std::make_unique<Wt::WTemplate>(Wt::WString::tr("validation-template"));
t->addFunction("id", &Wt::WTemplate::Functions::id);

auto ageEdit = t->bindWidget("age", std::make_unique<Wt::WLineEdit>());

auto validator = std::make_shared<Wt::WIntValidator>(0, 150);
validator->setMandatory(true);
ageEdit->setValidator(validator);

auto button = t->bindWidget("button", std::make_unique<Wt::WPushButton>("Save"));

auto out = t->bindWidget("age-info", std::make_unique<Wt::WText>());
out->setInline(false);
out->hide();

button->clicked().connect([=] {
    out->show();
    if (ageEdit->validate() == Wt::ValidationState::Valid) {
        out->setText("Age of " + ageEdit->text() + " is saved!");
        out->setStyleClass("alert alert-success");
    } else {
        out->setText("The number must be in the range 0 to 150");
        out->setStyleClass("alert alert-danger");
    }
});

ageEdit->enterPressed().connect([=] {
    button->clicked().emit(Wt::WMouseEvent());
});

SAMPLE_END(return std::move(t))
