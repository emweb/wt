#include <Wt/WContainerWidget>
#include <Wt/WDoubleSpinBox>
#include <Wt/WText>

SAMPLE_BEGIN(SpinBox)
Wt::WContainerWidget *container = new Wt::WContainerWidget();
container->addStyleClass("control-group");

new Wt::WText("Enter a number between 0 and 100:", container);

Wt::WDoubleSpinBox *sb = new Wt::WDoubleSpinBox(container);
sb->setRange(0,100);
sb->setValue(50);
sb->setDecimals(2);
sb->setSingleStep(0.1);
sb->setMargin(10, Wt::Left | Wt::Right);

Wt::WText *out = new Wt::WText("", container);

sb->changed().connect(std::bind([=] () {
    if (sb->validate() == Wt::WValidator::Valid) {
        out->setText(Wt::WString::fromUTF8("Spin box value changed to {1}")
		     .arg(sb->text()));
    } else {
        out->setText(Wt::WString::fromUTF8("Invalid spin box value!"));
    }
}));

SAMPLE_END(return container)
