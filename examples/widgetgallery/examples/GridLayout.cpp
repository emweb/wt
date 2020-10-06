#include <Wt/WContainerWidget.h>
#include <Wt/WGridLayout.h>
#include <Wt/WText.h>

SAMPLE_BEGIN(GridLayout)

auto container = std::make_unique<Wt::WContainerWidget>();
container->setHeight(400);
container->setStyleClass("yellow-box");

auto grid = container->setLayout(std::make_unique<Wt::WGridLayout>());

for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 4; ++column) {
        Wt::WString cell = Wt::WString("Item ({1}, {2})").arg(row).arg(column);

	auto text = std::make_unique<Wt::WText>(cell);
	if (row == 1 || column == 1 || column == 2)
	    text->setStyleClass("blue-box");
	else
	    text->setStyleClass("green-box");

        grid->addWidget(std::move(text), row, column);
    }
}

grid->setRowStretch(1, 1);
grid->setColumnStretch(1, 1);
grid->setColumnStretch(2, 1);

SAMPLE_END(return std::move(container))
