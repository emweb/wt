#include <Wt/WContainerWidget>
#include <Wt/WGridLayout>
#include <Wt/WText>

SAMPLE_BEGIN(GridLayout)
Wt::WContainerWidget *container = new Wt::WContainerWidget();
container->setHeight(400);
container->setStyleClass("yellow-box");

Wt::WGridLayout *grid = new Wt::WGridLayout();
container->setLayout(grid);

for (int row = 0; row < 3; ++row) {
    for (int column = 0; column < 4; ++column) {
	Wt::WString cell = Wt::WString("Item ({1}, {2})").arg(row).arg(column);

	Wt::WText *t = new Wt::WText(cell);
	if (row == 1 || column == 1 || column == 2)
	    t->setStyleClass("blue-box");
	else
	    t->setStyleClass("green-box");

	grid->addWidget(t, row, column);
    }
}

grid->setRowStretch(1, 1);
grid->setColumnStretch(1, 1);
grid->setColumnStretch(2, 1);

SAMPLE_END(return container)
