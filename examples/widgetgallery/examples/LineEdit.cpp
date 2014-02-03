#include <Wt/WContainerWidget>
#include <Wt/WIntValidator>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WTemplate>

SAMPLE_BEGIN(LineEdit)
Wt::WTemplate *result = new Wt::WTemplate(Wt::WString::tr("lineEdit-template"));
result->addFunction("id", &Wt::WTemplate::Functions::id);

Wt::WLineEdit *edit = new Wt::WLineEdit();
edit->setValidator(new Wt::WIntValidator(0, 130));

result->bindString("label", "Age:");
result->bindWidget("edit", edit);

SAMPLE_END(return result)
