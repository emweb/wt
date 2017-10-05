#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(SimpleForm)
auto result =
    Wt::cpp14::make_unique<Wt::WTemplate>(Wt::WString::tr("simpleForm-template"));

auto name = result->bindWidget("name", Wt::cpp14::make_unique<Wt::WLineEdit>());
name->setPlaceholderText("first name");

auto button = result->bindWidget("button", Wt::cpp14::make_unique<Wt::WPushButton>("OK"));

auto out = result->bindWidget("out", Wt::cpp14::make_unique<Wt::WText>());

button->clicked().connect([=] {
    out->setText("Hello, " + name->text() + "! I just want to help you... You"
                 + " could complete this simple form by adding validation.");
});

SAMPLE_END(return std::move(result))

