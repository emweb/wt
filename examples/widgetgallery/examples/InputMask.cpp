#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WTemplate>

SAMPLE_BEGIN(InputMask)
Wt::WTemplate *result = new Wt::WTemplate(Wt::WString::tr("lineEdit-template"));
result->addFunction("id", &Wt::WTemplate::Functions::id);

Wt::WLineEdit *edit = new Wt::WLineEdit();
edit->setTextSize(15);
edit->setInputMask("009.009.009.009;_");

result->bindString("label", "IP Address:");
result->bindWidget("edit", edit);

SAMPLE_END(return result)
