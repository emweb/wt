#include <Wt/WPanel>
#include <Wt/WText>

SAMPLE_BEGIN(PanelNoTitle)
Wt::WPanel *panel = new Wt::WPanel();
panel->addStyleClass("centered-example");
panel->setCentralWidget(new Wt::WText("This is a default panel."));

SAMPLE_END(return panel)
