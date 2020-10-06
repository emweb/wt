#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(Template)
auto t = std::make_unique<Wt::WTemplate>(Wt::WString::tr("WTemplate-example"));

t->bindWidget("name-edit", std::make_unique<Wt::WLineEdit>());
t->bindWidget("save-button", std::make_unique<Wt::WPushButton>("Save"));
t->bindWidget("cancel-button", std::make_unique<Wt::WPushButton>("Cancel"));

SAMPLE_END(return std::move(t))
