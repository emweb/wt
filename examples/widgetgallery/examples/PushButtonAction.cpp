#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(PushButtonAction)

auto result =
    Wt::cpp14::make_unique<Wt::WTemplate>(Wt::WString::tr("pushButtonAction-template"));

auto button = result->bindWidget("button-save", Wt::cpp14::make_unique<Wt::WPushButton>("Save"));
button->setStyleClass("btn-primary");

result->bindWidget("button-cancel", Wt::cpp14::make_unique<Wt::WPushButton>("Cancel"));

SAMPLE_END(return std::move(result))
