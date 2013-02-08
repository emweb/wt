#include <Wt/WContainerWidget>
#include <Wt/WIntValidator>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WTemplate>

SAMPLE_BEGIN(LineEdit)
Wt::WTemplate *result =
    new Wt::WTemplate(Wt::WString::tr("lineEdit-template"));

Wt::WLabel *label = new Wt::WLabel("Age:");
result->bindWidget("label", label);

Wt::WLineEdit *edit = new Wt::WLineEdit();
result->bindWidget("edit", edit);
edit->setValidator(new Wt::WIntValidator(0, 130));

// Select the text in the line edit if the label is clicked.
label->setBuddy(edit);

SAMPLE_END(return result)
