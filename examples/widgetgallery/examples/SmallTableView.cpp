#include <Wt/WApplication.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTableView.h>
#include "../treeview-dragdrop/CsvUtil.h"

SAMPLE_BEGIN(SmallTableView)
auto tableView = std::make_unique<WTableView>();
tableView->setModel(csvToModel(WApplication::appRoot() + "table.csv"));

tableView->setColumnResizeEnabled(false);
tableView->setColumnAlignment(0, AlignmentFlag::Center);
tableView->setHeaderAlignment(0, AlignmentFlag::Center);
tableView->setAlternatingRowColors(true);
tableView->setRowHeight(28);
tableView->setHeaderHeight(28);
tableView->setSelectionMode(SelectionMode::Single);
tableView->setEditTriggers(EditTrigger::None);

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

SAMPLE_END(return std::move(tableView))
