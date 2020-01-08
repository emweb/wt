/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fstream>

#include <Wt/Chart/WCartesianChart.h>
#include <Wt/Chart/WDataSeries.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WSvgImage.h>
#include <Wt/WPainter.h>
#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WTime.h>

using namespace Wt;
using namespace Wt::Chart;

namespace {

double plotTimeSeriesChart(const std::shared_ptr<WStandardItemModel>& model, 
			   std::string fileName,
			   AxisScale xScale)
{
  WCartesianChart chart;
  chart.setModel(model);
  chart.setXSeriesColumn(0);
  chart.setLegendEnabled(true);

  chart.setType(ChartType::Scatter);
  chart.axis(Axis::X).setScale(xScale);
  chart.axis(Axis::Y).setScale(AxisScale::Log);

  chart.setPlotAreaPadding(100, Side::Left);
  chart.setPlotAreaPadding(50, Side::Top | Side::Bottom);

  for (int i = 1; i < model->columnCount(); ++i) {
    auto s = cpp14::make_unique<WDataSeries>(i, SeriesType::Line);
    chart.addSeries(std::move(s));
  }

  chart.setMargin(10, Side::Top | Side::Bottom);
  chart.setMargin(WLength::Auto, Side::Left | Side::Right);

  double result;

  {
    WSvgImage image(400, 300);
    WPainter painter(&image);

    chart.paint(painter);

    result = 
      chart.axis(Axis::Y).maximum() - chart.axis(Axis::Y).minimum();

    painter.end();
    std::ofstream f(fileName.c_str(), std::ios::out | std::ios::binary);
    image.write(f);
    f.close();
  }

  return result;
}

} // end anonymous namespace

BOOST_AUTO_TEST_CASE( chart_test_WDateTimeChartMinutes )
{
  auto model = std::make_shared<WStandardItemModel>();

  WDate d(2009, 10, 1);
  WDateTime start(d, WTime(1, 24, 0));
  WDateTime end(d, WTime(2, 36, 0));

  WDateTime dt = start;
  int row = 0;
  model->insertColumns(0, 2);
  while (dt < end) {
    model->insertRow(model->rowCount());
    model->setData(row, 0, cpp17::any(dt));
    if (row % 10 == 0)
      model->setData(row, 1, cpp17::any());
    else
      model->setData(row, 1, cpp17::any(row * 10));
    dt = dt.addSecs(60);
    row++;
  }

  plotTimeSeriesChart(model, "minutes.svg", AxisScale::DateTime);
}
  
BOOST_AUTO_TEST_CASE( chart_test_WDateTimeChartHours )
{ 
  auto model = std::make_shared<WStandardItemModel>();

  WDate d(2009, 10, 1);
  WDateTime start(d, WTime(1, 20, 0));
  WDateTime end(d, WTime(23, 30, 0));

  WDateTime dt = start;
  int row = 0;
  model->insertColumns(0, 2);
  while (dt < end) {
    model->insertRow(model->rowCount());
    model->setData(row, 0, cpp17::any(dt));
    model->setData(row, 1, cpp17::any(row * 10));
    dt = dt.addSecs(60);
    row++;
  }

  plotTimeSeriesChart(model, "hours.svg", AxisScale::DateTime);
}

BOOST_AUTO_TEST_CASE( chart_test_WDateTimeChartDays )
{
  auto model = std::make_shared<WStandardItemModel>();
  
  WDate start(2009, 10, 1);
  WDate end(2009, 11, 10);

  WDate d = start;
  int row = 0;
  model->insertColumns(0, 2);
  while (d < end) {
    model->insertRow(model->rowCount());
    model->setData(row, 0, cpp17::any(d));
    model->setData(row, 1, cpp17::any(row * 10));
    d = d.addDays(1);
    row++;
  }

  plotTimeSeriesChart(model, "days.svg", AxisScale::Date);
}

  
BOOST_AUTO_TEST_CASE( chart_test_WDateTimeChartWeeks )
{
  auto model = std::make_shared<WStandardItemModel>();

  WDate start(2009, 10, 1);
  WDate end(2009, 11, 1);

  WDate d = start;
  int row = 0;
  model->insertColumns(0, 2);
  while (d < end) {
    model->insertRow(model->rowCount());
    model->setData(row, 0, cpp17::any(d));
    model->setData(row, 1, cpp17::any(row * 10));
    d = d.addDays(1);
    row++;
  }

  plotTimeSeriesChart(model, "weeks.svg", AxisScale::Date);
}
  
BOOST_AUTO_TEST_CASE( chart_test_WDateTimeChartMonths )
{
  auto model = std::make_shared<WStandardItemModel>();

  WDate start(2008, 4, 1);
  WDate end(2008, 12, 1);
  
  WDate d = start;
  int row = 0;
  model->insertColumns(0, 2);
  while (d < end) {
    model->insertRow(model->rowCount());
    model->setData(row, 0, cpp17::any(d));
    model->setData(row, 1, cpp17::any(row * 10));
    d = d.addDays(5);
    row++;
  }

  plotTimeSeriesChart(model, "months.svg", AxisScale::Date);
}

BOOST_AUTO_TEST_CASE( chart_test_WDateTimeChart0Range )
{
  auto model = std::make_shared<WStandardItemModel>();

  WDate start(2008, 4, 1);
  WDate end(2008, 12, 1);
  
  WDate d = start;
  int row = 0;
  model->insertColumns(0, 2);
  while (d < end) {
    model->insertRow(model->rowCount());
    model->setData(row, 0, cpp17::any(d));
    model->setData(row, 1, cpp17::any(20));
    d = d.addDays(5);
    row++;
  }

  double range = plotTimeSeriesChart(model, "0range.svg", AxisScale::Date);

  std::cerr << "Range: " << range << std::endl;

  BOOST_REQUIRE(range == 90);
}

