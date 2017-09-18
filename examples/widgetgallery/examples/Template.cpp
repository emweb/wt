#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(Template)
auto t = Wt::cpp14::make_unique<Wt::WTemplate>(Wt::WString::tr("WTemplate-example"));

t->bindWidget("name-edit", Wt::cpp14::make_unique<Wt::WLineEdit>());
t->bindWidget("save-button", Wt::cpp14::make_unique<Wt::WPushButton>("Save"));
t->bindWidget("cancel-button", Wt::cpp14::make_unique<Wt::WPushButton>("Cancel"));

SAMPLE_END(return std::move(t))
