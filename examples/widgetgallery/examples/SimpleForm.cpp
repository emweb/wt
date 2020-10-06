#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(SimpleForm)
auto result =
    std::make_unique<Wt::WTemplate>(Wt::WString::tr("simpleForm-template"));

auto name = result->bindWidget("name", std::make_unique<Wt::WLineEdit>());
name->setPlaceholderText("first name");

auto button = result->bindWidget("button", std::make_unique<Wt::WPushButton>("OK"));

auto out = result->bindWidget("out", std::make_unique<Wt::WText>());

button->clicked().connect([=] {
    out->setText("Hello, " + name->text() + "! I just want to help you... You"
                 + " could complete this simple form by adding validation.");
});

SAMPLE_END(return std::move(result))

