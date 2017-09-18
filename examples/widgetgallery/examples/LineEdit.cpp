#include <Wt/WContainerWidget.h>
#include <Wt/WIntValidator.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(LineEdit)

auto result =
    Wt::cpp14::make_unique<Wt::WTemplate>(Wt::WString::tr("lineEdit-template"));
result->addFunction("id", &Wt::WTemplate::Functions::id);

auto edit = Wt::cpp14::make_unique<Wt::WLineEdit>();
edit->setValidator(std::make_shared<Wt::WIntValidator>(0, 130));

result->bindString("label", "Age:");
result->bindWidget("edit", std::move(edit));

SAMPLE_END(return std::move(result))
