#include <Wt/Chart/WCartesianChart>
#include <Wt/Chart/WDataSeries>
#include <Wt/WAbstractItemView>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WItemDelegate>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WTableView>

#include "../treeview-dragdrop/CsvUtil.h"

SAMPLE_BEGIN(CategoryChart)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WStandardItemModel *model
    = csvToModel(Wt::WApplication::appRoot() + "category.csv", container);

if (!model)
    return container;

/*
 * Configure all model items as selectable and editable.
 */
for (int row = 0; row < model->rowCount(); ++row)
    for (int col = 0; col < model->columnCount(); ++col)
        model->item(row, col)->setFlags(Wt::ItemIsEditable);

/*
 * Shows a table, allowing editing of the model
 */
Wt::WTableView *table = new Wt::WTableView(container);
table->setModel(model);
table->setSortingEnabled(true);
table->setColumnResizeEnabled(true);
table->setAlternatingRowColors(true);
table->setHeaderAlignment(0, Wt::AlignCenter);
table->setColumnAlignment(0, Wt::AlignCenter);
table->setRowHeight(28);
table->setHeaderHeight(28);
table->setMargin(10, Wt::Top | Wt::Bottom);
table->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right);
table->setWidth(4*120 + 80 + 5*7 + 2);

/*
 * Editing does not really work without Ajax, it would require an
 * additional button somewhere to confirm the edited value.
 */

if (Wt::WApplication::instance()->environment().ajax()) {
    table->setEditTriggers(Wt::WAbstractItemView::SingleClicked);
    table->setEditOptions(table->editOptions() | 
			  Wt::WAbstractItemView::SaveWhenClosed);
} else {
    table->setEditTriggers(Wt::WAbstractItemView::NoEditTrigger);
}

/*
 * Use a delegate for the numeric data which rounds values sensibly.
 */
Wt::WItemDelegate *delegate = new Wt::WItemDelegate(table);
delegate->setTextFormat("%.f");
table->setItemDelegate(delegate);

table->setColumnWidth(0, 80);
for (int i = 1; i < model->columnCount(); ++i)
    table->setColumnWidth(i, 120);

/*
 * Create the category chart.
 */
Wt::Chart::WCartesianChart *chart = new Wt::Chart::WCartesianChart(container);
chart->setModel(model);
chart->setXSeriesColumn(0);
chart->setLegendEnabled(true);

/*
 * Provide ample space for the title, the X and Y axis and the legend.
 */
chart->setPlotAreaPadding(40, Wt::Left | Wt::Top | Wt::Bottom);
chart->setPlotAreaPadding(120, Wt::Right);

/*
 * Add all (but first) column as bar series.
 */
for (int column = 1; column < model->columnCount(); ++column) {
    Wt::Chart::WDataSeries series(column, Wt::Chart::BarSeries);
    series.setShadow(Wt::WShadow(3, 3, Wt::WColor(0, 0, 0, 127), 3));
    chart->addSeries(series);
}

chart->resize(600, 400);
chart->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right);

SAMPLE_END(return container)
