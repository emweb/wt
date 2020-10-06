#include <Wt/Chart/WAxisSliderWidget.h>
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

#include "../treeview-dragdrop/CsvUtil.h"

SAMPLE_BEGIN(AxisSliderWidget)

auto container = std::make_unique<WContainerWidget>();

std::shared_ptr<WStandardItemModel> model
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
chart->setType(Chart::ChartType::Scatter);
chart->axis(Chart::Axis::X).setScale(Chart::AxisScale::Date);
double min = asNumber(model->data(0, 0));
double max = asNumber(model->data(model->rowCount() - 1, 0));
// Set maximum X zoom level to 16x zoom
chart->axis(Chart::Axis::X).setMinimumZoomRange((max - min) / 16.0);

/*
 * Add the second and the third column as line series.
 */
auto s = std::make_unique<Chart::WDataSeries>(2, Chart::SeriesType::Line);
auto s_ = s.get();
s_->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
chart->addSeries(std::move(s));

chart->resize(800, 400);

// Enable pan and zoom
chart->setPanEnabled(true);
chart->setZoomEnabled(true);

chart->setMargin(WLength::Auto, Side::Left | Side::Right); // Center horizontally

// Add a WAxisSliderWidget for the chart using the data series for column 2
auto sliderWidget = container->addNew<Chart::WAxisSliderWidget>(s_);
sliderWidget->resize(800, 80);
sliderWidget->setSelectionAreaPadding(40, Side::Left | Side::Right);
sliderWidget->setMargin(WLength::Auto, Side::Left | Side::Right); // Center horizontally

SAMPLE_END(return std::move(container))
