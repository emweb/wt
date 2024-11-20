#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(PushButtonSize)
auto result =
    std::make_unique<Wt::WTemplate>(Wt::WString::tr("pushButtonSize-template"));

auto button = std::make_unique<Wt::WPushButton>("Large");
button->setStyleClass("btn-lg btn-large btn-secondary");
result->bindWidget("button-large", std::move(button));

button = std::make_unique<Wt::WPushButton>("Default");
result->bindWidget("button-default", std::move(button));

button = std::make_unique<Wt::WPushButton>("Small");
button->setStyleClass("btn-sm btn-small btn-secondary");
result->bindWidget("button-small", std::move(button));

SAMPLE_END(return std::move(result))
