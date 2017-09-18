#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(PushButtonAction)

auto result =
    Wt::cpp14::make_unique<Wt::WTemplate>(Wt::WString::tr("pushButtonAction-template"));

auto button = Wt::cpp14::make_unique<Wt::WPushButton>("Save");
auto button_ = result->bindWidget("button-save", std::move(button));
button_->setStyleClass("btn-primary");

button = Wt::cpp14::make_unique<Wt::WPushButton>("Cancel");
result->bindWidget("button-cancel", std::move(button));

SAMPLE_END(return std::move(result))
