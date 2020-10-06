#include <Wt/WTableView.h>

#ifdef WT_EXAMPLE
#include "VirtualModel.cpp"
#endif // WT_EXAMPLE

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

SAMPLE_BEGIN(LargeTableView)

auto tableView = std::make_unique<WTableView>();
tableView->setModel(std::make_shared<VirtualModel>(10000, 50));
tableView->setRowHeaderCount(1); // treat first column as 'fixed' row headers
tableView->setSortingEnabled(false);
tableView->setAlternatingRowColors(true);
tableView->setRowHeight(28);
tableView->setHeaderHeight(28);
tableView->setSelectionMode(SelectionMode::Extended);
tableView->setEditTriggers(EditTrigger::None);

tableView->resize(650, 400);

SAMPLE_END(return std::move(tableView))
