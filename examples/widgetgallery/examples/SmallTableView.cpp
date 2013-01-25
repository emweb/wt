#include <Wt/WStandardItemModel>
#include <Wt/WTableView>

SAMPLE_BEGIN(SmallTableView)
Wt::WTableView *tableView = new Wt::WTableView();
tableView->setModel(csvToModel(Wt::WApplication::appRoot() + "table.csv"));

tableView->setColumnResizeEnabled(false);
tableView->setColumnAlignment(0, Wt::AlignCenter);
tableView->setHeaderAlignment(0, Wt::AlignCenter);
tableView->setAlternatingRowColors(true);
tableView->setRowHeight(28);
tableView->setHeaderHeight(28);
tableView->setSelectionMode(Wt::SingleSelection);

/*
 * Configure column widths and matching table width
 */
for (int i = 0; i < tableView->model()->columnCount(); ++i)
  tableView->setColumnWidth(i, 120);

tableView->setWidth(127 * tableView->model()->columnCount());

SAMPLE_END(return tableView)
