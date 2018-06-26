#include <Wt/WAnimation.h>
#include <Wt/WPanel.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(PanelCollapsible)
auto panel = Wt::cpp14::make_unique<Wt::WPanel>();
panel->setTitle("Collapsible panel");
panel->addStyleClass("centered-example");
panel->setCollapsible(true);

Wt::WAnimation animation(Wt::AnimationEffect::SlideInFromTop,
                         Wt::TimingFunction::EaseOut,
			 100);

panel->setAnimation(animation);
panel->setCentralWidget(Wt::cpp14::make_unique<Wt::WText>("This panel can be collapsed."));

SAMPLE_END(return std::move(panel))
