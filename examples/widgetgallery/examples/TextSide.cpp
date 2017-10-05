#include <Wt/WLineEdit.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(TextSide)
auto result =
    Wt::cpp14::make_unique<Wt::WTemplate>(Wt::WString::tr("editSide-template"));

auto edit =
    Wt::cpp14::make_unique<Wt::WLineEdit>("Username");
edit->setStyleClass("span2");
result->bindWidget("name", std::move(edit));

edit =
    Wt::cpp14::make_unique<Wt::WLineEdit>();
edit->setStyleClass("span2");
result->bindWidget("amount1", std::move(edit));

edit =
    Wt::cpp14::make_unique<Wt::WLineEdit>();
edit->setStyleClass("span2");
result->bindWidget("amount2", std::move(edit));

SAMPLE_END(return std::move(result))
