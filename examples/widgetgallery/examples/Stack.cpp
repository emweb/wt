#include <Wt/WContainerWidget.h>
#include <Wt/WSpinBox.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(Stack)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

Wt::WSpinBox *sb =
    container->addWidget(Wt::cpp14::make_unique<Wt::WSpinBox>());
sb->setRange(0,2);

Wt::WStackedWidget *stack =
    container->addWidget(Wt::cpp14::make_unique<Wt::WStackedWidget>());
stack->addWidget(Wt::cpp14::make_unique<Wt::WText>("<strong>Stacked widget-index 0</strong>"
                               "<p>Hello</p>"));
stack->addWidget(Wt::cpp14::make_unique<Wt::WText>("<strong>Stacked widget-index 1</strong>"
                               "<p>This is Wt</p>"));
stack->addWidget(Wt::cpp14::make_unique<Wt::WText>("<strong>Stacked widget-index 2</strong>"
                               "<p>Do you like it?</p>"));

sb->changed().connect([=] {
    if (sb->validate() == Wt::ValidationState::Valid)
	stack->setCurrentIndex(sb->value());
});

SAMPLE_END(return std::move(container))
