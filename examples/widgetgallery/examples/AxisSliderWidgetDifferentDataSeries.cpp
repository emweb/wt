#include <Wt/Chart/WAbstractChartModel.h>
#include <Wt/Chart/WAxisSliderWidget.h>
#include <Wt/Chart/WCartesianChart.h>
#include <Wt/Chart/WDataSeries.h>
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WObject.h>
#include <Wt/WShadow.h>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class SinModel : public Chart::WAbstractChartModel {
public:
  SinModel(double minimum, double maximum)
    : Chart::WAbstractChartModel(),
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

struct ChartState : WObject {
  ChartState()
    : WObject()
#ifndef WT_TARGET_JAVA
      , model(nullptr)
#endif
  { }

#ifndef WT_TARGET_JAVA
  virtual ~ChartState()
  {
  }
#endif

  std::shared_ptr<SinModel> model;
};

SAMPLE_BEGIN(AxisSliderWidgetDifferentDataSeries)

auto container = cpp14::make_unique<WContainerWidget>();

auto state = container->addChild(cpp14::make_unique<ChartState>());

/*
 * Start with a rough model for the fully zoomed out position.
 */
state->model = std::make_shared<SinModel>(-M_PI, M_PI);

auto chart =
    container->addWidget(cpp14::make_unique<Chart::WCartesianChart>());
chart->setBackground(WColor(220, 220, 220));
chart->setType(Chart::ChartType::Scatter);

/*
 * Create a rough model to use for the WAxisSliderWidget
 */
auto roughModel = std::make_shared<SinModel>(-M_PI, M_PI);
auto roughSeries =
    cpp14::make_unique<Chart::WDataSeries>(1, Chart::SeriesType::Line);
auto roughSeries_ = roughSeries.get();
roughSeries_->setModel(roughModel);
roughSeries_->setXSeriesColumn(0);
// Add the rough series to the chart, but hide it!
roughSeries->setHidden(true);
chart->addSeries(std::move(roughSeries));

/*
 * Add the second and the third column as line series.
 */
auto seriesPtr =
    cpp14::make_unique<Chart::WDataSeries>(1, Chart::SeriesType::Line);
auto series = seriesPtr.get();
series->setModel(state->model);
series->setXSeriesColumn(0);
series->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
chart->addSeries(std::move(seriesPtr));

chart->axis(Chart::Axis::X).zoomRangeChanged().connect([=] {
  double minX = chart->axis(Chart::Axis::X).zoomMinimum();
  double maxX = chart->axis(Chart::Axis::X).zoomMaximum();
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
    state->model = std::make_shared<SinModel>(minX, maxX);
    series->setModel(state->model);
  }
});

chart->axis(Chart::Axis::X).setMinimumZoomRange(M_PI / 8.0);
chart->axis(Chart::Axis::X).setMinimum(-3.5);
chart->axis(Chart::Axis::X).setMaximum(3.5);

chart->axis(Chart::Axis::Y).setMinimumZoomRange(0.1);
chart->axis(Chart::Axis::Y).setMinimum(-1.5);
chart->axis(Chart::Axis::Y).setMaximum(1.5);

chart->resize(800, 400);

// Enable pan and zoom
chart->setPanEnabled(true);
chart->setZoomEnabled(true);

chart->setMargin(WLength::Auto, Side::Left | Side::Right); // Center horizontally

// Add a WAxisSliderWidget for the chart using the data series for column 2
auto sliderWidget =
    container->addWidget(cpp14::make_unique<Chart::WAxisSliderWidget>(roughSeries_));
sliderWidget->resize(800, 80);
sliderWidget->setSelectionAreaPadding(40, Side::Left | Side::Right);
sliderWidget->setMargin(WLength::Auto, Side::Left | Side::Right); // Center horizontally

SAMPLE_END(return std::move(container))
