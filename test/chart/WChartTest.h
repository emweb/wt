// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCHART_TEST_H_
#define WCHART_TEST_H_

namespace Wt {
  class WStandardItemModel;
}

#include <Wt/Chart/WAxis>

#include <boost/test/unit_test.hpp>

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

class WChartTest : public test_suite
{
  void plotTimeSeriesChart(Wt::WStandardItemModel* model, 
			   std::string fileName,
			   Wt::Chart::AxisScale xScale);
public:
  void test_WDateTimeChartMinutes();
  void test_WDateTimeChartHours();
  void test_WDateTimeChartDays();
  void test_WDateTimeChartWeeks();
  void test_WDateTimeChartMonths();

  WChartTest();
};

#endif // WCHART_TEST_H_
