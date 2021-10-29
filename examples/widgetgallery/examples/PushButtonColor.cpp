#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(PushButtonColor)

auto result =
    std::make_unique<Wt::WTemplate>(Wt::WString::tr("pushButtonColor-template"));

auto button = std::make_unique<Wt::WPushButton>("Primary");
button->setStyleClass("btn-primary");
result->bindWidget("button-primary", std::move(button));

button = std::make_unique<Wt::WPushButton>("Secondary");
result->bindWidget("button-secondary", std::move(button));

button = std::make_unique<Wt::WPushButton>("Info");
button->setStyleClass("btn-info");
result->bindWidget("button-info", std::move(button));

button = std::make_unique<Wt::WPushButton>("Success");
button->setStyleClass("btn-success");
result->bindWidget("button-success", std::move(button));

button = std::make_unique<Wt::WPushButton>("Warning");
button->setStyleClass("btn-warning");
result->bindWidget("button-warning", std::move(button));

button = std::make_unique<Wt::WPushButton>("Danger");
button->setStyleClass("btn-danger");
result->bindWidget("button-danger", std::move(button));

button = std::make_unique<Wt::WPushButton>("Light");
button->setStyleClass("btn-light");
result->bindWidget("button-light", std::move(button));

button = std::make_unique<Wt::WPushButton>("Dark");
button->setStyleClass("btn-dark");
result->bindWidget("button-dark", std::move(button));

button = std::make_unique<Wt::WPushButton>("Outline");
button->setStyleClass("btn-outline-primary");
result->bindWidget("button-outline", std::move(button));

button = std::make_unique<Wt::WPushButton>("Link");
button->setStyleClass("btn-link");
result->bindWidget("button-link", std::move(button));

button = std::make_unique<Wt::WPushButton>("");
button->setStyleClass("btn-close");
result->bindWidget("button-close", std::move(button));

SAMPLE_END(return std::move(result))
