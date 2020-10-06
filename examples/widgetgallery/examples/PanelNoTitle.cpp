#include <Wt/WPanel.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(PanelNoTitle)
auto panel = std::make_unique<Wt::WPanel>();
panel->addStyleClass("centered-example");
panel->setCentralWidget(std::make_unique<Wt::WText>("This is a default panel."));

SAMPLE_END(return std::move(panel))
