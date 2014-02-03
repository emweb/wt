#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WTemplate>
#include <Wt/WText>

SAMPLE_BEGIN(SimpleForm)
Wt::WTemplate *result =
    new Wt::WTemplate(Wt::WString::tr("simpleForm-template"));

Wt::WLineEdit *name = new Wt::WLineEdit();
result->bindWidget("name", name);
name->setEmptyText("first name");

Wt::WPushButton *button = new Wt::WPushButton("OK");
result->bindWidget("button", button);

Wt::WText *out = new Wt::WText("");
result->bindWidget("out", out);

button->clicked().connect(std::bind([=] () {
    out->setText("Hello, " + name->text() + "! I just want to help you... You"
                 + " could complete this simple form by adding validation.");
}));

SAMPLE_END(return result)

