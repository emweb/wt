#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(PushButtonSize)
auto result =
    std::make_unique<Wt::WTemplate>(Wt::WString::tr("pushButtonSize-template"));

auto button = std::make_unique<Wt::WPushButton>("Large");
button->setStyleClass("btn-lg");
result->bindWidget("button-large", std::move(button));

button = std::make_unique<Wt::WPushButton>("Default");
result->bindWidget("button-default", std::move(button));

button = std::make_unique<Wt::WPushButton>("Small");
button->setStyleClass("btn-sm");
result->bindWidget("button-small", std::move(button));

button = std::make_unique<Wt::WPushButton>("Mini");
button->setStyleClass("btn-xs");
result->bindWidget("button-mini", std::move(button));

SAMPLE_END(return std::move(result))
