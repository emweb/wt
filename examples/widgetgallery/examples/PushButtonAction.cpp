#include <Wt/WPushButton>
#include <Wt/WTemplate>

SAMPLE_BEGIN(PushButtonAction)

Wt::WTemplate *result =
    new Wt::WTemplate(Wt::WString::tr("pushButtonAction-template"));

Wt::WPushButton *button = new Wt::WPushButton("Save");
result->bindWidget("button-save", button);
button->setStyleClass("btn-primary");

button = new Wt::WPushButton("Cancel");
result->bindWidget("button-cancel", button);

SAMPLE_END(return result)
