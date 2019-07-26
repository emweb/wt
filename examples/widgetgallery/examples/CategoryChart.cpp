#include <Wt/Chart/WCartesianChart.h>
#include <Wt/Chart/WDataSeries.h>
#include <Wt/WAbstractItemView.h>
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WItemDelegate.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WStandardItem.h>
#include <Wt/WTableView.h>

#include "../treeview-dragdrop/CsvUtil.h"

SAMPLE_BEGIN(CategoryChart)

auto container = cpp14::make_unique<WContainerWidget>();

auto model
    = csvToModel(WApplication::appRoot() + "category.csv");

if (!model)
    return std::move(container);

/*
 * Configure all model items as selectable and editable.
 */
for (int row = 0; row < model->rowCount(); ++row)
    for (int col = 0; col < model->columnCount(); ++col)
        model->item(row, col)->setFlags(ItemFlag::Editable);

/*
 * Shows a table, allowing editing of the model
 */
auto table = container->addNew<WTableView>();
table->setModel(model);
table->setSortingEnabled(true);
table->setColumnResizeEnabled(true);
table->setAlternatingRowColors(true);
table->setHeaderAlignment(0, AlignmentFlag::Center);
table->setColumnAlignment(0, AlignmentFlag::Center);
table->setRowHeight(28);
table->setHeaderHeight(28);
table->setMargin(10, Side::Top | Side::Bottom);
table->setMargin(WLength::Auto, Side::Left | Side::Right);
table->setWidth(4*120 + 80 + 5*7 + 2);

/*
 * Editing does not really work without Ajax, it would require an
 * additional button somewhere to confirm the edited value.
 */

if (WApplication::instance()->environment().ajax()) {
    table->setEditTriggers(EditTrigger::SingleClicked);
    table->setEditOptions(table->editOptions() | 
                          EditOption::SaveWhenClosed);
} else {
    table->setEditTriggers(EditTrigger::None);
}

/*
 * Use a delegate for the numeric data which rounds values sensibly.
 */
auto delegate = std::make_shared<WItemDelegate>();
delegate->setTextFormat("%.f");
table->setItemDelegate(delegate);

table->setColumnWidth(0, 80);
for (int i = 1; i < model->columnCount(); ++i)
    table->setColumnWidth(i, 120);

/*
 * Create the category chart.
 */
auto chart = container->addNew<Chart::WCartesianChart>();
#ifndef WT_TARGET_JAVA
chart->setModel(model);
#else // WT_TARGET_JAVA
chart->setModel(std::shared_ptr<WAbstractItemModel>(model));
#endif // WT_TARGET_JAVA
chart->setXSeriesColumn(0);
chart->setLegendEnabled(true);

/*
 * Provide ample space for the title, the X and Y axis and the legend.
 */
chart->setPlotAreaPadding(40, Side::Left | Side::Top | Side::Bottom);
chart->setPlotAreaPadding(120, Side::Right);

/*
 * Add all (but first) column as bar series.
 */
for (int column = 1; column < model->columnCount(); ++column) {
    auto series = cpp14::make_unique<Chart::WDataSeries>(column, Chart::SeriesType::Bar);
    series->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
    chart->addSeries(std::move(series));
}

chart->resize(600, 400);
chart->setMargin(WLength::Auto, Side::Left | Side::Right);

SAMPLE_END(return std::move(container))
