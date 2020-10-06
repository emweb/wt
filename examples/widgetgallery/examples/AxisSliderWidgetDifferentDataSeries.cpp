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

#ifdef WT_TARGET_JAVA
using namespace Wt;
#endif // WT_TARGET_JAVA

class SinModel : public Chart::WAbstractChartModel {
public:
  SinModel(double minimum, double maximum, int rows)
    : Chart::WAbstractChartModel(),
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

auto container = std::make_unique<WContainerWidget>();

auto chart =
    container->addNew<Chart::WCartesianChart>();
chart->setBackground(WColor(220, 220, 220));
chart->setType(Chart::ChartType::Scatter);

/*
 * Create a rough model to use for the WAxisSliderWidget
 */
auto roughModel = std::make_shared<SinModel>(-M_PI, M_PI, 100);
auto roughSeries =
    std::make_unique<Chart::WDataSeries>(1, Chart::SeriesType::Line);
auto roughSeries_ = roughSeries.get();
roughSeries_->setModel(roughModel);
roughSeries_->setXSeriesColumn(0);
// Add the rough series to the chart, but hide it!
roughSeries->setHidden(true);
chart->addSeries(std::move(roughSeries));

/*
 * Create a detailed model
 */
auto detailedModel = std::make_shared<SinModel>(-M_PI, M_PI, 10000);
auto seriesPtr =
    std::make_unique<Chart::WDataSeries>(1, Chart::SeriesType::Line);
auto series = seriesPtr.get();
series->setModel(detailedModel);
series->setXSeriesColumn(0);
series->setShadow(WShadow(3, 3, WColor(0, 0, 0, 127), 3));
chart->addSeries(std::move(seriesPtr));


chart->axis(Chart::Axis::X).setMaximumZoomRange(M_PI);
chart->axis(Chart::Axis::X).setMinimumZoomRange(M_PI / 16.0);
chart->axis(Chart::Axis::X).setMinimum(-3.5);
chart->axis(Chart::Axis::X).setMaximum(3.5);

chart->axis(Chart::Axis::Y).setMinimumZoomRange(0.1);
chart->axis(Chart::Axis::Y).setMinimum(-1.5);
chart->axis(Chart::Axis::Y).setMaximum(1.5);

chart->resize(800, 400);

// Enable pan and zoom
chart->setPanEnabled(true);
chart->setZoomEnabled(true);

// Enable on-demand loading
chart->setOnDemandLoadingEnabled(true);

chart->setMargin(WLength::Auto, Side::Left | Side::Right); // Center horizontally

// Add a WAxisSliderWidget for the chart using the data series for column 2
auto sliderWidget =
    container->addNew<Chart::WAxisSliderWidget>(roughSeries_);
sliderWidget->resize(800, 80);
sliderWidget->setSelectionAreaPadding(40, Side::Left | Side::Right);
sliderWidget->setMargin(WLength::Auto, Side::Left | Side::Right); // Center horizontally

SAMPLE_END(return std::move(container))
