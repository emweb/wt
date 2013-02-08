#include <Wt/WPushButton>
#include <Wt/WTemplate>

SAMPLE_BEGIN(PushButtonColor)

Wt::WTemplate *result =
    new Wt::WTemplate(Wt::WString::tr("pushButtonColor-template"));

Wt::WPushButton *button = new Wt::WPushButton("Default");
result->bindWidget("button-default", button);

button = new Wt::WPushButton("Primary");
button->setStyleClass("btn-primary");
result->bindWidget("button-primary", button);

button = new Wt::WPushButton("Info");
button->setStyleClass("btn-info");
result->bindWidget("button-info", button);

button = new Wt::WPushButton("Success");
button->setStyleClass("btn-success");
result->bindWidget("button-success", button);

button = new Wt::WPushButton("Warning");
button->setStyleClass("btn-warning");
result->bindWidget("button-warning", button);

button = new Wt::WPushButton("Danger");
button->setStyleClass("btn-danger");
result->bindWidget("button-danger", button);

button = new Wt::WPushButton("Inverse");
button->setStyleClass("btn-inverse");
result->bindWidget("button-inverse", button);

button = new Wt::WPushButton("Link");
button->setStyleClass("btn-link");
result->bindWidget("button-link", button);

SAMPLE_END(return result)
