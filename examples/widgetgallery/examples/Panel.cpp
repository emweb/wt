#include <Wt/WPanel>
#include <Wt/WText>

SAMPLE_BEGIN(Panel)
Wt::WPanel *panel = new Wt::WPanel();
panel->addStyleClass("centered-example");
panel->setTitle("Terrific panel");
panel->setCentralWidget(new Wt::WText("This is a panel with a title."));

SAMPLE_END(return panel)
