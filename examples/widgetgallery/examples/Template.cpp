#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WTemplate>

SAMPLE_BEGIN(Template)
Wt::WTemplate *t = new Wt::WTemplate(Wt::WString::tr("WTemplate-example"));

t->bindWidget("name-edit", new Wt::WLineEdit());
t->bindWidget("save-button", new Wt::WPushButton("Save"));
t->bindWidget("cancel-button", new Wt::WPushButton("Cancel"));

SAMPLE_END(return t)
