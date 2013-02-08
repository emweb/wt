#include <Wt/WContainerWidget>
#include <Wt/WDoubleSpinBox>
#include <Wt/WText>

SAMPLE_BEGIN(SpinBox)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

new Wt::WText("Enter a number between 0 and 100:", container);

Wt::WDoubleSpinBox *sb = new Wt::WDoubleSpinBox(container);
sb->setRange(0,100);
sb->setValue(50);
sb->setDecimals(2);
sb->setSingleStep(0.1);
sb->setMargin(10, Wt::Left | Wt::Right);

Wt::WText *out = new Wt::WText("", container);

sb->valueChanged().connect(std::bind([=] (double d) {
    out->setText(Wt::WString::fromUTF8("Spin box value changed to {1}.")
		 .arg(d));
}, std::placeholders::_1));

SAMPLE_END(return container)
