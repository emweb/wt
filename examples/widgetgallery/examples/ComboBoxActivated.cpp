#include <Wt/WComboBox>
#include <Wt/WContainerWidget>
#include <Wt/WText>

SAMPLE_BEGIN(ComboBoxActivated)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WComboBox *cb = new Wt::WComboBox(container);
cb->addItem("Heavy");
cb->addItem("Medium");
cb->addItem("Light");
cb->setCurrentIndex(1); // Show 'Medium' initially.
cb->setMargin(10, Wt::Right);

Wt::WText *out = new Wt::WText(container);
out->addStyleClass("help-block");

cb->changed().connect(std::bind([=] () {
    out->setText(Wt::WString::fromUTF8("You selected {1}.")
		 .arg(cb->currentText()));
}));

SAMPLE_END(return container)
