#include <Wt/WCheckBox.h>
#include <Wt/WContainerWidget.h>

SAMPLE_BEGIN(CheckBoxStack)

auto result = Wt::cpp14::make_unique<Wt::WContainerWidget>();
Wt::WCheckBox *cb;

cb = result->addNew<Wt::WCheckBox>("Check me!");
cb->setInline(false);
cb->setChecked(true);

cb = result->addNew<Wt::WCheckBox>("Check me too!");
cb->setInline(false);

cb = result->addNew<Wt::WCheckBox>("Check me, I'm tristate!");
cb->setInline(false);
cb->setTristate();
cb->setCheckState(Wt::CheckState::PartiallyChecked);

SAMPLE_END(return std::move(result))
