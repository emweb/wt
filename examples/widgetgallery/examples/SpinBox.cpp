#include <Wt/WContainerWidget.h>
#include <Wt/WDoubleSpinBox.h>
#include <Wt/WLabel.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(SpinBox)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();
container->addStyleClass("form-group");

Wt::WLabel *label =
    container->addWidget(Wt::cpp14::make_unique<Wt::WLabel>("Enter a number (0 - 100):"));

Wt::WDoubleSpinBox *sb =
    container->addWidget(Wt::cpp14::make_unique<Wt::WDoubleSpinBox>());
sb->setRange(0,100);
sb->setValue(50);
sb->setDecimals(2);
sb->setSingleStep(0.1);

label->setBuddy(sb);

Wt::WText *out =
    container->addWidget(Wt::cpp14::make_unique<Wt::WText>(""));
out->addStyleClass("help-block");

sb->changed().connect([=] {
    if (sb->validate() == Wt::ValidationState::Valid) {
        out->setText(Wt::WString("Spin box value changed to {1}")
		     .arg(sb->text()));
    } else {
        out->setText(Wt::WString("Invalid spin box value!"));
    }
});

SAMPLE_END(return std::move(container))
