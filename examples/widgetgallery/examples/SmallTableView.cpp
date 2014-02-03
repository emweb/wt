#include <Wt/WApplication>
#include <Wt/WStandardItemModel>
#include <Wt/WTableView>
#include "../treeview-dragdrop/CsvUtil.h"

SAMPLE_BEGIN(SmallTableView)
Wt::WTableView *tableView = new Wt::WTableView();
tableView->setModel(csvToModel(Wt::WApplication::appRoot() + "table.csv",
			       tableView));

tableView->setColumnResizeEnabled(false);
tableView->setColumnAlignment(0, Wt::AlignCenter);
tableView->setHeaderAlignment(0, Wt::AlignCenter);
tableView->setAlternatingRowColors(true);
tableView->setRowHeight(28);
tableView->setHeaderHeight(28);
tableView->setSelectionMode(Wt::SingleSelection);
tableView->setEditTriggers(Wt::WAbstractItemView::NoEditTrigger);

/*
 * Configure column widths and matching table width
 */
const int WIDTH = 120;
for (int i = 0; i < tableView->model()->columnCount(); ++i)
    tableView->setColumnWidth(i, 120);

/*
 * 7 pixels are padding/border per column
 * 2 pixels are border of the entire table
 */
tableView->setWidth((WIDTH + 7) * tableView->model()->columnCount() + 2);

SAMPLE_END(return tableView)
