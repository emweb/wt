#include <Wt/Chart/WCartesianChart.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WTimer.h>

#include <cmath>

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

SAMPLE_BEGIN(ScatterPlotCurve)
auto container = cpp14::make_unique<WContainerWidget>();

auto model =
    std::make_shared<WStandardItemModel>(40, 2);
model->setHeaderData(0, WString("X"));
model->setHeaderData(1, WString("Y = sin(X)"));

for (unsigned i = 0; i < 40; ++i) {
    double x = (static_cast<double>(i) - 20) / 4;

    model->setData(i, 0, x);
    model->setData(i, 1, std::sin(x));
}

/*
 * Create the scatter plot.
 */
Chart::WCartesianChart *chart = container->addNew<Chart::WCartesianChart>();
#ifndef WT_TARGET_JAVA
chart->setModel(model);        // Set the model.
#else // WT_TARGET_JAVA
chart->setModel(std::shared_ptr<WAbstractItemModel>(model));
#endif // WT_TARGET_JAVA
chart->setXSeriesColumn(0);    // Set the column that holds the X data.
chart->setLegendEnabled(true); // Enable the legend.

chart->setType(Chart::ChartType::Scatter);   // Set type to ScatterPlot.

// Typically, for mathematical functions, you want the axes to cross
// at the 0 mark:
chart->axis(Chart::Axis::X).setLocation(Chart::AxisValue::Zero);
chart->axis(Chart::Axis::Y).setLocation(Chart::AxisValue::Zero);

// Provide space for the X and Y axis and title.
chart->setPlotAreaPadding(120, Side::Right);
chart->setPlotAreaPadding(40, Side::Top | Side::Bottom);

// Add the curves
auto s = cpp14::make_unique<Chart::WDataSeries>(1, Chart::SeriesType::Curve);
s->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
chart->addSeries(std::move(s));

chart->resize(800, 300); // WPaintedWidget must be given explicit size.

chart->setMargin(10, Side::Top | Side::Bottom);            // Add margin vertically
chart->setMargin(WLength::Auto, Side::Left | Side::Right); // Center horizontally

SAMPLE_END(return std::move(container))
