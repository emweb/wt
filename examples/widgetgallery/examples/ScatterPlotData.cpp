#include <Wt/Chart/WCartesianChart.h>
#include <Wt/Chart/WDataSeries.h>
#include <Wt/WAbstractItemModel.h>
#include <Wt/WAbstractItemView.h>
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDate.h>
#include <Wt/WEnvironment.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WItemDelegate.h>
#include <Wt/WShadow.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTableView.h>

#include "../treeview-dragdrop/CsvUtil.h"

SAMPLE_BEGIN(ScatterPlotData)
auto container = cpp14::make_unique<WContainerWidget>();

auto model
    = csvToModel(WApplication::appRoot() + "timeseries.csv");

if (!model)
    return std::move(container);

/*
 * Parses the first column as dates, to be able to use a date scale
 */
for (int row = 0; row < model->rowCount(); ++row) {
    WString s = asString(model->data(row, 0));
    WDate date = WDate::fromString(s, "dd/MM/yy");
    model->setData(row, 0, date);
  }

// Renders the data in a table.
WTableView *table = container->addNew<WTableView>();
table->setModel(model);
table->setSortingEnabled(false);
table->setColumnResizeEnabled(true);
table->setAlternatingRowColors(true);
table->setColumnAlignment(0, AlignmentFlag::Center);
table->setHeaderAlignment(0, AlignmentFlag::Center);
table->setRowHeight(28);
table->setHeaderHeight(28);
table->setColumnWidth(0, 80);
for (int column = 1; column < model->columnCount(); ++column)
    table->setColumnWidth(column, 90);
table->resize(783, 200);

/*
 * Use a delegate for the numeric data which rounds values sensibly.
 */
auto delegate = std::make_shared<WItemDelegate>();
delegate->setTextFormat("%.1f");
table->setItemDelegate(delegate);
table->setItemDelegateForColumn(0, std::make_shared<WItemDelegate>());

/*
 * Creates the scatter plot.
 */
Chart::WCartesianChart *chart = container->addNew<Chart::WCartesianChart>();
chart->setBackground(WColor(220, 220, 220));
#ifndef WT_TARGET_JAVA
chart->setModel(model);
#else // WT_TARGET_JAVA
chart->setModel(std::shared_ptr<WAbstractItemModel>(model));
#endif // WT_TARGET_JAVA
chart->setXSeriesColumn(0);
chart->setLegendEnabled(true);
chart->setType(Chart::ChartType::Scatter);
chart->axis(Chart::Axis::X).setScale(Chart::AxisScale::Date);

/*
 * Provide ample space for the title, the X and Y axis and the legend.
 */
chart->setPlotAreaPadding(40, Side::Left | Side::Top | Side::Bottom);
chart->setPlotAreaPadding(120, Side::Right);

/*
 * Add the second and the third column as line series.
 */
for (int i = 2; i < 4; ++i) {
    auto s = cpp14::make_unique<Chart::WDataSeries>(i, Chart::SeriesType::Line);
    s->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
    chart->addSeries(std::move(s));
}

chart->resize(800, 400);
chart->setMargin(WLength::Auto, Side::Left | Side::Right);

SAMPLE_END(return std::move(container))
