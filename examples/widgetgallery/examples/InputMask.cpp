#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(InputMask)

auto result =
    Wt::cpp14::make_unique<Wt::WTemplate>(Wt::WString::tr("lineEdit-template"));
result->addFunction("id", &Wt::WTemplate::Functions::id);

auto edit = Wt::cpp14::make_unique<Wt::WLineEdit>();
edit->setTextSize(15);
edit->setInputMask("009.009.009.009;_");

result->bindString("label", "IP Address:");
result->bindWidget("edit", std::move(edit));

SAMPLE_END(return std::move(result))
