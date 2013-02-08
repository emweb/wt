#include <Wt/WTableView>

#ifdef WT_EXAMPLE
#include "VirtualModel.cpp"
#endif // WT_EXAMPLE

SAMPLE_BEGIN(LargeTableView)
Wt::WTableView *tableView = new Wt::WTableView();
tableView->setModel(new VirtualModel(10000, 50));

tableView->setRowHeaderCount(1); // treat first column as 'fixed' row headers
tableView->setSortingEnabled(false);
tableView->setAlternatingRowColors(true);
tableView->setRowHeight(28);
tableView->setHeaderHeight(28);

tableView->resize(650, 400);

SAMPLE_END(return tableView)
