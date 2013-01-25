#include <Wt/WComboBox>
#include <Wt/WContainerWidget>
#include <Wt/WText>

SAMPLE_BEGIN(ComboBoxModel)

Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WComboBox *cb = new Wt::WComboBox(container);
cb->setMargin(10, Wt::Right);

Wt::WAbstractItemModel *model = cb->model();

model->insertRows(0, 4);
model->setData(0, 0, std::string("Belgium"), Wt::DisplayRole);
model->setData(0, 0, std::string("BE"), Wt::UserRole);
model->setData(1, 0, std::string("Netherlands"), Wt::DisplayRole);
model->setData(1, 0, std::string("NL"), Wt::UserRole);
model->setData(2, 0, std::string("United Kingdom"), Wt::DisplayRole);
model->setData(2, 0, std::string("UK"), Wt::UserRole);
model->setData(3, 0, std::string("United States"), Wt::DisplayRole);
model->setData(3, 0, std::string("US"), Wt::UserRole);

Wt::WText *out = new Wt::WText(container);

cb->changed().connect(std::bind([=] () {
        Wt::WString countryName = cb->currentText();
        int row = cb->currentIndex();
        std::string countryCode = boost::any_cast<std::string>(
            model->data(model->index(row,0), Wt::UserRole));
        out->setText(Wt::WString::fromUTF8("You selected {1} with key {2}.").
		     arg(countryName).arg(countryCode));
}));

SAMPLE_END(return container)
