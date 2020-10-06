#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(PushButtonColor)

auto result =
    std::make_unique<Wt::WTemplate>(Wt::WString::tr("pushButtonColor-template"));

auto button = std::make_unique<Wt::WPushButton>("Default");
result->bindWidget("button-default", std::move(button));

button = std::make_unique<Wt::WPushButton>("Primary");
button->setStyleClass("btn-primary");
result->bindWidget("button-primary", std::move(button));

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

button = std::make_unique<Wt::WPushButton>("Inverse");
button->setStyleClass("btn-inverse");
result->bindWidget("button-inverse", std::move(button));

button = std::make_unique<Wt::WPushButton>("Link");
button->setStyleClass("btn-link");
result->bindWidget("button-link", std::move(button));

SAMPLE_END(return std::move(result))
