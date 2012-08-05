/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fstream>

#include <Wt/Chart/WCartesianChart>
#include <Wt/Chart/WDataSeries>
#include <Wt/WStandardItemModel>
#include <Wt/WSvgImage>
#include <Wt/WPainter>
#include <Wt/WDate>
#include <Wt/WDateTime>
#include <Wt/WTime>

using namespace Wt;
using namespace Wt::Chart;

namespace {

double plotTimeSeriesChart(WStandardItemModel* model, 
			   std::string fileName,
			   AxisScale xScale)
{
  WCartesianChart chart;
  chart.setModel(model);
  chart.setXSeriesColumn(0);
  chart.setLegendEnabled(true);

  chart.setType(ScatterPlot);
  chart.axis(XAxis).setScale(xScale);
  chart.axis(YAxis).setScale(LogScale);

  chart.setPlotAreaPadding(100, Left);
  chart.setPlotAreaPadding(50, Top | Bottom);

  for (int i = 1; i < model->columnCount(); ++i) {
    WDataSeries s(i, LineSeries);
    chart.addSeries(s);
  }

  chart.setMargin(10, Top | Bottom);
  chart.setMargin(WLength::Auto, Left | Right);

  double result;

  {
    WSvgImage image(400, 300);
    WPainter painter(&image);

    chart.paint(painter);

    result = chart.axis(YAxis).maximum() - chart.axis(YAxis).minimum();

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
  WStandardItemModel model;

  WDate d(2009, 10, 1);
  WDateTime start(d, WTime(1, 24, 0));
  WDateTime end(d, WTime(2, 36, 0));

  WDateTime dt = start;
  int row = 0;
  model.insertColumns(0, 2);
  while (dt < end) {
    model.insertRow(model.rowCount());
    model.setData(row, 0, boost::any(dt));
    if (row % 10 == 0)
      model.setData(row, 1, boost::any());
    else
      model.setData(row, 1, boost::any(row * 10));
    dt = dt.addSecs(60);
    row++;
  }

  plotTimeSeriesChart(&model, "minutes.svg", DateTimeScale);
}
  
BOOST_AUTO_TEST_CASE( chart_test_WDateTimeChartHours )
{ 
  WStandardItemModel model;

  WDate d(2009, 10, 1);
  WDateTime start(d, WTime(1, 20, 0));
  WDateTime end(d, WTime(23, 30, 0));

  WDateTime dt = start;
  int row = 0;
  model.insertColumns(0, 2);
  while (dt < end) {
    model.insertRow(model.rowCount());
    model.setData(row, 0, boost::any(dt));
    model.setData(row, 1, boost::any(row * 10));
    dt = dt.addSecs(60);
    row++;
  }

  plotTimeSeriesChart(&model, "hours.svg", DateTimeScale);
}

BOOST_AUTO_TEST_CASE( chart_test_WDateTimeChartDays )
{
  WStandardItemModel model;
  
  WDate start(2009, 10, 1);
  WDate end(2009, 11, 10);

  WDate d = start;
  int row = 0;
  model.insertColumns(0, 2);
  while (d < end) {
    model.insertRow(model.rowCount());
    model.setData(row, 0, boost::any(d));
    model.setData(row, 1, boost::any(row * 10));
    d = d.addDays(1);
    row++;
  }

  plotTimeSeriesChart(&model, "days.svg", DateScale);
}

  
BOOST_AUTO_TEST_CASE( chart_test_WDateTimeChartWeeks )
{
  WStandardItemModel model;

  WDate start(2009, 10, 1);
  WDate end(2009, 11, 1);

  WDate d = start;
  int row = 0;
  model.insertColumns(0, 2);
  while (d < end) {
    model.insertRow(model.rowCount());
    model.setData(row, 0, boost::any(d));
    model.setData(row, 1, boost::any(row * 10));
    d = d.addDays(1);
    row++;
  }

  plotTimeSeriesChart(&model, "weeks.svg", DateScale);
}
  
BOOST_AUTO_TEST_CASE( chart_test_WDateTimeChartMonths )
{
  WStandardItemModel model;

  WDate start(2008, 4, 1);
  WDate end(2008, 12, 1);
  
  WDate d = start;
  int row = 0;
  model.insertColumns(0, 2);
  while (d < end) {
    model.insertRow(model.rowCount());
    model.setData(row, 0, boost::any(d));
    model.setData(row, 1, boost::any(row * 10));
    d = d.addDays(5);
    row++;
  }

  plotTimeSeriesChart(&model, "months.svg", DateScale);
}

BOOST_AUTO_TEST_CASE( chart_test_WDateTimeChart0Range )
{
  WStandardItemModel model;

  WDate start(2008, 4, 1);
  WDate end(2008, 12, 1);
  
  WDate d = start;
  int row = 0;
  model.insertColumns(0, 2);
  while (d < end) {
    model.insertRow(model.rowCount());
    model.setData(row, 0, boost::any(d));
    model.setData(row, 1, boost::any(20));
    d = d.addDays(5);
    row++;
  }

  double range = plotTimeSeriesChart(&model, "0range.svg", DateScale);

  BOOST_REQUIRE(range == 90);
}

