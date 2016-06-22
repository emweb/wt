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
  SinModel(double minimum, double maximum, Wt::WObject *parent = 0)
    : Wt::Chart::WAbstractChartModel(parent),
      minimum_(minimum),
      maximum_(maximum)
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
    return 100;
  }

  double minimum() const { return minimum_; }
  double maximum() const { return maximum_; }

private:
  double minimum_;
  double maximum_;
};

struct ChartState : Wt::WObject {
  ChartState(Wt::WObject *parent = 0)
    : Wt::WObject(parent)
#ifndef WT_TARGET_JAVA
      , model(0)
#endif
  { }

#ifndef WT_TARGET_JAVA
  virtual ~ChartState()
  {
    delete model;
  }
#endif

  SinModel *model;
};

SAMPLE_BEGIN(AxisSliderWidgetDifferentDataSeries)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

ChartState *state = new ChartState(container);

/*
 * Start with a rough model for the fully zoomed out position.
 */
state->model = new SinModel(-M_PI, M_PI);

Wt::Chart::WCartesianChart *chart = new Wt::Chart::WCartesianChart(container);
chart->setBackground(Wt::WColor(220, 220, 220));
chart->setType(Wt::Chart::ScatterPlot);

/*
 * Create a rough model to use for the WAxisSliderWidget
 */
Wt::Chart::WAbstractChartModel *roughModel = new SinModel(-M_PI, M_PI, container);
Wt::Chart::WDataSeries *roughSeries = new Wt::Chart::WDataSeries(1, Wt::Chart::LineSeries);
roughSeries->setModel(roughModel);
roughSeries->setXSeriesColumn(0);
// Add the rough series to the chart, but hide it!
roughSeries->setHidden(true);
chart->addSeries(roughSeries);

/*
 * Add the second and the third column as line series.
 */
Wt::Chart::WDataSeries *series = new Wt::Chart::WDataSeries(1, Wt::Chart::LineSeries);
series->setModel(state->model);
series->setXSeriesColumn(0);
series->setShadow(Wt::WShadow(3, 3, Wt::WColor(0, 0, 0, 127), 3));
chart->addSeries(series);

chart->axis(Wt::Chart::XAxis).zoomRangeChanged().connect(std::bind([=] () {
  double minX = chart->axis(Wt::Chart::XAxis).zoomMinimum();
  double maxX = chart->axis(Wt::Chart::XAxis).zoomMaximum();
  /*
   * Determine the range of the model.
   */
  double dX = maxX - minX;
  minX = minX - dX / 2.0;
  if (minX < -M_PI)
    minX = -M_PI;
  maxX = maxX + dX / 2.0;
  if (maxX > M_PI)
    maxX = M_PI;
  if (state->model->minimum() != minX || state->model->maximum() != maxX) {
    // Change the model
    delete state->model;
    state->model = new SinModel(minX, maxX);
    series->setModel(state->model);
  }
}));

chart->axis(Wt::Chart::XAxis).setMinimumZoomRange(M_PI / 8.0);
chart->axis(Wt::Chart::XAxis).setMinimum(-3.5);
chart->axis(Wt::Chart::XAxis).setMaximum(3.5);

chart->axis(Wt::Chart::YAxis).setMinimumZoomRange(0.1);
chart->axis(Wt::Chart::YAxis).setMinimum(-1.5);
chart->axis(Wt::Chart::YAxis).setMaximum(1.5);

chart->resize(800, 400);

// Enable pan and zoom
chart->setPanEnabled(true);
chart->setZoomEnabled(true);

chart->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right); // Center horizontally

// Add a WAxisSliderWidget for the chart using the data series for column 2
Wt::Chart::WAxisSliderWidget *sliderWidget = new Wt::Chart::WAxisSliderWidget(roughSeries, container);
sliderWidget->resize(800, 80);
sliderWidget->setSelectionAreaPadding(40, Wt::Left | Wt::Right);
sliderWidget->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right); // Center horizontally

SAMPLE_END(return container)
