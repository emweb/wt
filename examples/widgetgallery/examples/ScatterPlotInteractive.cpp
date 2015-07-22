#include <Wt/Chart/WAxisSliderWidget>
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

#include "../treeview-dragdrop/CsvUtil.h"

SAMPLE_BEGIN(ScatterPlotInteractive)
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

/*
 * Creates the scatter plot.
 */
Wt::Chart::WCartesianChart *chart = new Wt::Chart::WCartesianChart(container);
chart->setBackground(Wt::WColor(220, 220, 220));
chart->setModel(model);
chart->setXSeriesColumn(0);
chart->setType(Wt::Chart::ScatterPlot);
chart->axis(Wt::Chart::XAxis).setScale(Wt::Chart::DateScale);
// Set maximum X zoom level to 16x zoom
chart->axis(Wt::Chart::XAxis).setMaxZoom(16.0);

/*
 * Add the second and the third column as line series.
 */
Wt::Chart::WDataSeries s(2, Wt::Chart::LineSeries);
s.setShadow(Wt::WShadow(3, 3, Wt::WColor(0, 0, 0, 127), 3));
chart->addSeries(s);

chart->resize(800, 400);

// Enable pan and zoom
chart->setPanEnabled(true);
chart->setZoomEnabled(true);

chart->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right); // Center horizontally

// Add a WAxisSliderWidget for the chart using the data series for column 2
Wt::Chart::WAxisSliderWidget *sliderWidget = new Wt::Chart::WAxisSliderWidget(chart, 2, container);
sliderWidget->resize(800, 80);
sliderWidget->setSelectionAreaPadding(40, Wt::Left | Wt::Right);
sliderWidget->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right); // Center horizontally

SAMPLE_END(return container)
