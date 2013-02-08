#include <Wt/WCheckBox>
#include <Wt/WContainerWidget>

SAMPLE_BEGIN(CheckBoxStack)
Wt::WContainerWidget *result = new Wt::WContainerWidget();
Wt::WCheckBox *cb;

cb = new Wt::WCheckBox("Check me!", result);
cb->setInline(false);
cb->setChecked(true);

cb = new Wt::WCheckBox("Check me too!", result);
cb->setInline(false);

cb = new Wt::WCheckBox("Check me, I'm tristate!", result);
cb->setInline(false);
cb->setTristate();
cb->setCheckState(Wt::PartiallyChecked);

SAMPLE_END(return result)
