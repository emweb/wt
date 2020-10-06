#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(ComboBoxActivated)

auto container = std::make_unique<Wt::WContainerWidget>();

auto cb = container->addNew<Wt::WComboBox>();
cb->addItem("Heavy");
cb->addItem("Medium");
cb->addItem("Light");
cb->setCurrentIndex(1); // Show 'Medium' initially.
cb->setMargin(10, Wt::Side::Right);

auto out = container->addNew<Wt::WText>();
out->addStyleClass("help-block");

cb->changed().connect([=] {
    out->setText(Wt::WString("You selected {1}.")
		 .arg(cb->currentText()));
});

SAMPLE_END(return std::move(container))
