#include <Wt/WComboBox.h>

SAMPLE_BEGIN(ComboBox)

auto cb = Wt::cpp14::make_unique<Wt::WComboBox>();
cb->addItem("Heavy");   // 'Heavy' (index 0) is shown by default.
cb->addItem("Medium");
cb->addItem("Light");

SAMPLE_END(return std::move(cb))
