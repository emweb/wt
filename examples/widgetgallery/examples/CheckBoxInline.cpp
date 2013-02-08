#include <Wt/WCheckBox>
#include <Wt/WContainerWidget>

SAMPLE_BEGIN(CheckBoxInline)
Wt::WContainerWidget *result = new Wt::WContainerWidget();
Wt::WCheckBox *cb;

cb = new Wt::WCheckBox("Check me!", result);
cb->setChecked(true);

cb = new Wt::WCheckBox("Check me too!", result);

cb = new Wt::WCheckBox("Check me, I'm tristate!", result);
cb->setTristate();
cb->setCheckState(Wt::PartiallyChecked);

SAMPLE_END(return result)
