#include <Wt/WComboBox>

SAMPLE_BEGIN(ComboBox)
Wt::WComboBox *cb = new Wt::WComboBox();
cb->addItem("Heavy");   // 'Heavy' (index 0) is shown by default.
cb->addItem("Medium");
cb->addItem("Light");

SAMPLE_END(return cb)
