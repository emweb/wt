#include <Wt/WContainerWidget>
#include <Wt/WSpinBox>
#include <Wt/WStackedWidget>
#include <Wt/WText>

SAMPLE_BEGIN(Stack)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WSpinBox *sb = new Wt::WSpinBox(container);
sb->setRange(0,2);

Wt::WStackedWidget *stack = new Wt::WStackedWidget(container);
stack->addWidget(new Wt::WText("<strong>Stacked widget-index 0</strong>"
                               "<p>Hello</p>"));
stack->addWidget(new Wt::WText("<strong>Stacked widget-index 1</strong>"
                               "<p>This is Wt</p>"));
stack->addWidget(new Wt::WText("<strong>Stacked widget-index 2</strong>"
                               "<p>Do you like it?</p>"));

sb->changed().connect(std::bind([=] () {
    if (sb->validate())
	stack->setCurrentIndex(sb->value());
}));

SAMPLE_END(return container)
