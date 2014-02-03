#include <Wt/WContainerWidget>
#include <Wt/WDoubleSpinBox>
#include <Wt/WLabel>
#include <Wt/WText>

SAMPLE_BEGIN(SpinBox)
Wt::WContainerWidget *container = new Wt::WContainerWidget();
container->addStyleClass("form-group");

Wt::WLabel *label = new Wt::WLabel("Enter a number (0 - 100):", container);

Wt::WDoubleSpinBox *sb = new Wt::WDoubleSpinBox(container);
sb->setRange(0,100);
sb->setValue(50);
sb->setDecimals(2);
sb->setSingleStep(0.1);

label->setBuddy(sb);

Wt::WText *out = new Wt::WText("", container);
out->addStyleClass("help-block");

sb->changed().connect(std::bind([=] () {
    if (sb->validate() == Wt::WValidator::Valid) {
        out->setText(Wt::WString::fromUTF8("Spin box value changed to {1}")
		     .arg(sb->text()));
    } else {
        out->setText(Wt::WString::fromUTF8("Invalid spin box value!"));
    }
}));

SAMPLE_END(return container)
