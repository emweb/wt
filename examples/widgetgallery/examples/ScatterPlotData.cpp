#include <Wt/Chart/WCartesianChart>
#include <Wt/Chart/WDataSeries>
#include <Wt/WAbstractItemModel>
#include <Wt/WAbstractItemView>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WDate>
#include <Wt/WEnvironment>
#include <Wt/WPaintedWidget>
#include <Wt/WItemDelegate>
#include <Wt/WShadow>
#include <Wt/WStandardItemModel>
#include <Wt/WTableView>

#include "../treeview-dragdrop/CsvUtil.h"

SAMPLE_BEGIN(ScatterPlotData)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WStandardItemModel *model
    = csvToModel(Wt::WApplication::appRoot() + "timeseries.csv", container);

if (!model)
    return container;

/*
 * Parses the first column as dates, to be able to use a date scale
 */
for (int row = 0; row < model->rowCount(); ++row) {
    Wt::WString s = Wt::asString(model->data(row, 0));
    Wt::WDate date = Wt::WDate::fromString(s, "dd/MM/yy");
    model->setData(row, 0, date);
  }

// Renders the data in a table.
Wt::WTableView *table = new Wt::WTableView(container);
table->setModel(model);
table->setSortingEnabled(false);
table->setColumnResizeEnabled(true);
table->setAlternatingRowColors(true);
table->setColumnAlignment(0, Wt::AlignCenter);
table->setHeaderAlignment(0, Wt::AlignCenter);
table->setRowHeight(28);
table->setHeaderHeight(28);
table->setColumnWidth(0, 80);
for (int column = 1; column < model->columnCount(); ++column)
    table->setColumnWidth(column, 90);
table->resize(783, 200);

/*
 * Use a delegate for the numeric data which rounds values sensibly.
 */
Wt::WItemDelegate *delegate = new Wt::WItemDelegate(table);
delegate->setTextFormat("%.1f");
table->setItemDelegate(delegate);
table->setItemDelegateForColumn(0, new Wt::WItemDelegate(table));

/*
 * Creates the scatter plot.
 */
Wt::Chart::WCartesianChart *chart = new Wt::Chart::WCartesianChart(container);
chart->setBackground(Wt::WColor(220, 220, 220));
chart->setModel(model);
chart->setXSeriesColumn(0);
chart->setLegendEnabled(true);
chart->setType(Wt::Chart::ScatterPlot);
chart->axis(Wt::Chart::XAxis).setScale(Wt::Chart::DateScale);

/*
 * Provide ample space for the title, the X and Y axis and the legend.
 */
chart->setPlotAreaPadding(40, Wt::Left | Wt::Top | Wt::Bottom);
chart->setPlotAreaPadding(120, Wt::Right);

/*
 * Add the second and the third column as line series.
 */
for (int i = 2; i < 4; ++i) {
    Wt::Chart::WDataSeries s(i, Wt::Chart::LineSeries);
    s.setShadow(Wt::WShadow(3, 3, Wt::WColor(0, 0, 0, 127), 3));
    chart->addSeries(s);
}

chart->resize(800, 400);
chart->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right);

SAMPLE_END(return container)
