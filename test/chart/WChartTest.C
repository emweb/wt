// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.

 *
 * See the LICENSE file for terms of use.
 */

#include <boost/bind.hpp>

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

#include "WChartTest.h"

using namespace Wt;
using namespace Wt::Chart;

void WChartTest::test_WDateTimeChartMinutes()
{
  WStandardItemModel* model = new WStandardItemModel();

  WDate d(2009, 10, 1);
  WDateTime start(d, WTime(1, 0, 0));
  WDateTime end(d, WTime(2, 0, 0));

  WDateTime dt = start;
  int row = 0;
  model->insertColumns(0, 2);
  while (dt < end) {
    model->insertRow(model->rowCount());
    model->setData(row, 0, boost::any(dt));
    model->setData(row, 1, boost::any(row * 10));
    dt = dt.addSecs(60);
    row++;
  }

  plotTimeSeriesChart(model, "minutes.svg", DateTimeScale);

  delete model;
}
  
void WChartTest::test_WDateTimeChartHours()
{ 
  WStandardItemModel* model = new WStandardItemModel();

  WDate d(2009, 10, 1);
  WDateTime start(d, WTime(0, 0, 0));
  WDateTime end(d, WTime(23, 30, 0));

  WDateTime dt = start;
  int row = 0;
  model->insertColumns(0, 2);
  while (dt < end) {
    model->insertRow(model->rowCount());
    model->setData(row, 0, boost::any(dt));
    model->setData(row, 1, boost::any(row * 10));
    dt = dt.addSecs(60 * 60);
    row++;
  }

  plotTimeSeriesChart(model, "hours.svg", DateTimeScale);

  delete model;
}

void WChartTest::test_WDateTimeChartDays()
{
  WStandardItemModel* model = new WStandardItemModel();
  
  WDate start(2009, 10, 1);
  WDate end(2009, 11, 1);

  WDate d = start;
  int row = 0;
  model->insertColumns(0, 2);
  while (d < end) {
    model->insertRow(model->rowCount());
    model->setData(row, 0, boost::any(d));
    model->setData(row, 1, boost::any(row * 10));
    d = d.addDays(1);
    row++;
  }

  plotTimeSeriesChart(model, "days.svg", DateScale);

  delete model;
}
  
void WChartTest::test_WDateTimeChartWeeks()
{
  WStandardItemModel* model = new WStandardItemModel();

  WDate start(2009, 10, 1);
  WDate end(2009, 11, 1);

  WDate d = start;
  int row = 0;
  model->insertColumns(0, 2);
  while (d < end) {
    model->insertRow(model->rowCount());
    model->setData(row, 0, boost::any(d));
    model->setData(row, 1, boost::any(row * 10));
    d = d.addDays(7);
    row++;
  }

  plotTimeSeriesChart(model, "weeks.svg", DateScale);

  delete model;
}
  
void WChartTest::test_WDateTimeChartMonths()
{
  WStandardItemModel* model = new WStandardItemModel();

  WDate start(2008, 4, 1);
  WDate end(2008, 8, 1);
  
  WDate d = start;
  int row = 0;
  model->insertColumns(0, 2);
  while (d < end) {
    model->insertRow(model->rowCount());
    model->setData(row, 0, boost::any(d));
    model->setData(row, 1, boost::any(row * 10));
    d = d.addMonths(1);
    row++;
  }

  plotTimeSeriesChart(model, "months.svg", DateScale);

  delete model;
}

void WChartTest::plotTimeSeriesChart(WStandardItemModel* model, 
				     std::string fileName,
				     AxisScale xScale) 
{
  WCartesianChart *chart = new WCartesianChart();
  chart->setModel(model);
  chart->setXSeriesColumn(0);
  chart->setLegendEnabled(true);

  chart->setType(ScatterPlot);
  chart->axis(XAxis).setScale(xScale);

  chart->setPlotAreaPadding(100, Left);
  chart->setPlotAreaPadding(50, Top | Bottom);

  for (int i = 1; i < model->columnCount(); ++i) {
    WDataSeries s(i, LineSeries);
    chart->addSeries(s);
  }

  chart->setMargin(10, Top | Bottom);
  chart->setMargin(WLength::Auto, Left | Right);

  {
    WSvgImage image(400, 300);
    WPainter painter(&image);

    chart->paint(painter);

    painter.end();
    std::ofstream f(fileName.c_str());
    image.write(f);
    f.close();
  }

  delete chart;
}

WChartTest::WChartTest()
  : test_suite("wchart_test_suite")
{
  add(BOOST_TEST_CASE
      (boost::bind(&WChartTest::test_WDateTimeChartMinutes, this)));
  add(BOOST_TEST_CASE
      (boost::bind(&WChartTest::test_WDateTimeChartHours, this)));
  add(BOOST_TEST_CASE
      (boost::bind(&WChartTest::test_WDateTimeChartDays, this)));
  add(BOOST_TEST_CASE
      (boost::bind(&WChartTest::test_WDateTimeChartWeeks, this)));
  add(BOOST_TEST_CASE
      (boost::bind(&WChartTest::test_WDateTimeChartMonths, this)));
}
