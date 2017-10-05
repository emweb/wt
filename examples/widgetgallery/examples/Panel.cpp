#include <Wt/WPanel.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(Panel)
auto panel = Wt::cpp14::make_unique<Wt::WPanel>();
panel->addStyleClass("centered-example");
panel->setTitle("Terrific panel");
panel->setCentralWidget(Wt::cpp14::make_unique<Wt::WText>("This is a panel with a title."));

SAMPLE_END(return std::move(panel))
