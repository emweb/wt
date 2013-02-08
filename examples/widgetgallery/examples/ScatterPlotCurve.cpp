#include <Wt/Chart/WCartesianChart>
#include <Wt/WContainerWidget>
#include <Wt/WStandardItemModel>

#include <cmath>

SAMPLE_BEGIN(ScatterPlotCurve)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WStandardItemModel *model = new Wt::WStandardItemModel(40, 2, container);
model->setHeaderData(0, Wt::WString("X"));
model->setHeaderData(1, Wt::WString("Y = sin(X)"));

for (unsigned i = 0; i < 40; ++i) {
    double x = (static_cast<double>(i) - 20) / 4;

    model->setData(i, 0, x);
    model->setData(i, 1, std::sin(x));
}

/*
 * Create the scatter plot.
 */
Wt::Chart::WCartesianChart *chart = new Wt::Chart::WCartesianChart(container);
chart->setModel(model);        // Set the model.
chart->setXSeriesColumn(0);    // Set the column that holds the X data.
chart->setLegendEnabled(true); // Enable the legend.

chart->setType(Wt::Chart::ScatterPlot);   // Set type to ScatterPlot.

// Typically, for mathematical functions, you want the axes to cross
// at the 0 mark:
chart->axis(Wt::Chart::XAxis).setLocation(Wt::Chart::ZeroValue);
chart->axis(Wt::Chart::YAxis).setLocation(Wt::Chart::ZeroValue);

// Provide space for the X and Y axis and title.
chart->setPlotAreaPadding(80, Wt::Left);
chart->setPlotAreaPadding(40, Wt::Top | Wt::Bottom);

// Add the curves
Wt::Chart::WDataSeries s(1, Wt::Chart::CurveSeries);
s.setShadow(Wt::WShadow(3, 3, Wt::WColor(0, 0, 0, 127), 3));
chart->addSeries(s);

chart->resize(800, 300); // WPaintedWidget must be given explicit size.

chart->setMargin(10, Wt::Top | Wt::Bottom);            // Add margin vertically
chart->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right); // Center horizontally

SAMPLE_END(return container)
