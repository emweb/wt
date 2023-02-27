#include <Wt/WCheckBox.h>
#include <Wt/WEmailEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

SAMPLE_BEGIN(EmailEdit)

auto result = std::make_unique<Wt::WTemplate>(Wt::WString::tr("emailEdit-template"));
auto tpl = result.get();
tpl->addFunction("id", &Wt::WTemplate::Functions::id);

tpl->bindString("label", "Email address: ", Wt::TextFormat::Plain);

auto edit = tpl->bindNew<Wt::WEmailEdit>("edit");

auto submit = tpl->bindNew<Wt::WPushButton>("submit", "Submit");

auto multiple = tpl->bindNew<Wt::WCheckBox>("multiple", "Allow multiple");
multiple->changed().connect([=]() {
  edit->setMultiple(multiple->isChecked());
});

auto pattern = tpl->bindNew<Wt::WCheckBox>("pattern", "Use pattern '.*@example[.]com'");
pattern->changed().connect([=]() {
  edit->setPattern(pattern->isChecked() ? ".*@example[.]com" : "");
});

tpl->bindEmpty("edit-info");

edit->validated().connect([=](const Wt::WValidator::Result& validationResult) {
  tpl->bindString("edit-info", validationResult.message());
});

submit->clicked().connect([=]() {
  edit->setMultiple(multiple->isChecked());
  edit->setPattern(pattern->isChecked() ? ".*@example[.]com" : "");
  edit->validate();
  tpl->bindString("output", Wt::WString("Entered email address: {1}").arg(edit->valueText()), Wt::TextFormat::Plain);
});

tpl->bindEmpty("output");

SAMPLE_END(return result)
