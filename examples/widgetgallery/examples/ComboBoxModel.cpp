#include <Wt/WAny.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WStringListModel.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(ComboBoxModel)

auto container = std::make_unique<Wt::WContainerWidget>();

auto cb = container->addNew<Wt::WComboBox>();
cb->setMargin(10, Wt::Side::Right);

auto model = std::make_shared<Wt::WStringListModel>();
model->addString("Belgium");
model->setData(0, 0, std::string("BE"), Wt::ItemDataRole::User);
model->addString("Netherlands");
model->setData(1, 0, std::string("NL"), Wt::ItemDataRole::User);
model->addString("United Kingdom");
model->setData(2, 0, std::string("UK"), Wt::ItemDataRole::User);
model->addString("United States");
model->setData(3, 0, std::string("US"), Wt::ItemDataRole::User);
model->setFlags(3, Wt::ItemFlag::Selectable);

cb->setNoSelectionEnabled(true);
cb->setModel(model);

auto out = container->addNew<Wt::WText>();
out->addStyleClass("help-block");

cb->changed().connect([=] {
    Wt::WString countryName = cb->currentText();
    int row = cb->currentIndex();
    Wt::WString countryCode = Wt::asString(model->data(model->index(row,0), Wt::ItemDataRole::User));
    out->setText(Wt::WString("You selected {1} with key {2}.").
		 arg(countryName).arg(countryCode));
});

SAMPLE_END(return std::move(container))
