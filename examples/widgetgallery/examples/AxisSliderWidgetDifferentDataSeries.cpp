#include <Wt/Chart/WAbstractChartModel>
#include <Wt/Chart/WAxisSliderWidget>
#include <Wt/Chart/WCartesianChart>
#include <Wt/Chart/WDataSeries>
#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WObject>
#include <Wt/WShadow>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class SinModel : public Wt::Chart::WAbstractChartModel {
public:
  SinModel(double minimum, double maximum, int rows, Wt::WObject *parent = 0)
    : Wt::Chart::WAbstractChartModel(parent),
      minimum_(minimum),
      maximum_(maximum),
      rows_(rows)
  { }

  virtual double data(int row, int column) const
  {
    double x = minimum_ + row * (maximum_ - minimum_) / (rowCount() - 1);
    if (column == 0) {
      return x;
    } else {
      return std::sin(x) + std::sin(x * 100.0) / 40.0;
    }
  }

  virtual int columnCount() const
  {
    return 2;
  }

  virtual int rowCount() const
  {
    return rows_;
  }

  double minimum() const { return minimum_; }
  double maximum() const { return maximum_; }

private:
  double minimum_;
  double maximum_;
  int rows_;
};

SAMPLE_BEGIN(AxisSliderWidgetDifferentDataSeries)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::Chart::WCartesianChart *chart = new Wt::Chart::WCartesianChart(container);
chart->setBackground(Wt::WColor(220, 220, 220));
chart->setType(Wt::Chart::ScatterPlot);

/*
 * Create a rough model to use for the WAxisSliderWidget
 */
Wt::Chart::WAbstractChartModel *roughModel = new SinModel(-M_PI, M_PI, 100, container);
Wt::Chart::WDataSeries *roughSeries = new Wt::Chart::WDataSeries(1, Wt::Chart::LineSeries);
roughSeries->setModel(roughModel);
roughSeries->setXSeriesColumn(0);
// Add the rough series to the chart, but hide it!
roughSeries->setHidden(true);
chart->addSeries(roughSeries);

/*
 * Create a detailed model
 */
Wt::Chart::WAbstractChartModel *detailedModel = new SinModel(-M_PI, M_PI, 10000, container);
Wt::Chart::WDataSeries *series = new Wt::Chart::WDataSeries(1, Wt::Chart::LineSeries);
series->setModel(detailedModel);
series->setXSeriesColumn(0);
series->setShadow(Wt::WShadow(3, 3, Wt::WColor(0, 0, 0, 127), 3));
chart->addSeries(series);

chart->axis(Wt::Chart::XAxis).setMaximumZoomRange(M_PI);
chart->axis(Wt::Chart::XAxis).setMinimumZoomRange(M_PI / 16.0);
chart->axis(Wt::Chart::XAxis).setMinimum(-3.5);
chart->axis(Wt::Chart::XAxis).setMaximum(3.5);

chart->axis(Wt::Chart::YAxis).setMinimumZoomRange(0.1);
chart->axis(Wt::Chart::YAxis).setMinimum(-1.5);
chart->axis(Wt::Chart::YAxis).setMaximum(1.5);

chart->resize(800, 400);

// Enable pan and zoom
chart->setPanEnabled(true);
chart->setZoomEnabled(true);

// Enable on-demand loading
chart->setOnDemandLoadingEnabled(true);

chart->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right); // Center horizontally

// Add a WAxisSliderWidget for the chart using the data series for column 2
Wt::Chart::WAxisSliderWidget *sliderWidget = new Wt::Chart::WAxisSliderWidget(roughSeries, container);
sliderWidget->resize(800, 80);
sliderWidget->setSelectionAreaPadding(40, Wt::Left | Wt::Right);
sliderWidget->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right); // Center horizontally

SAMPLE_END(return container)
