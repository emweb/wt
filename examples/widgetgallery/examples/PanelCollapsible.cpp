#include <Wt/WAnimation>
#include <Wt/WPanel>
#include <Wt/WText>

SAMPLE_BEGIN(PanelCollapsible)
Wt::WPanel *panel = new Wt::WPanel();
panel->setTitle("Collapsible panel");
panel->addStyleClass("centered-example");
panel->setCollapsible(true);

Wt::WAnimation animation(Wt::WAnimation::SlideInFromTop,
			 Wt::WAnimation::EaseOut,
			 100);

panel->setAnimation(animation);
panel->setCentralWidget(new Wt::WText("This panel can be collapsed."));

SAMPLE_END(return panel)
