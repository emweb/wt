#include <Wt/WCheckBox.h>
#include <Wt/WContainerWidget.h>

SAMPLE_BEGIN(CheckBoxInline)

auto result = Wt::cpp14::make_unique<Wt::WContainerWidget>();
Wt::WCheckBox *cb;

cb = result->addWidget(Wt::cpp14::make_unique<Wt::WCheckBox>("Check me!"));
cb->setChecked(true);

cb = result->addWidget(Wt::cpp14::make_unique<Wt::WCheckBox>("Check me too!"));

cb = result->addWidget(Wt::cpp14::make_unique<Wt::WCheckBox>("Check me, I'm tristate!"));
cb->setTristate();
cb->setCheckState(Wt::CheckState::PartiallyChecked);

SAMPLE_END(return std::move(result))
