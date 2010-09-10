#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>

using boost::unit_test_framework::test_suite;

#include "chart/WChartTest.h"
#include "dbo/DboTest.h"
#include "dbo/DboTest2.h"
#include "models/WBatchEditProxyModelTest.h"
#include "utf8/Utf8Test.h"
#include "wdatetime/WDateTimeTest.h"

boost::unit_test::test_suite* init_unit_test_suite(int, char**)
{
  test_suite *tests = BOOST_TEST_SUITE("Wt test suite");

  tests->add(new DboTest());
  tests->add(new DboTest2());
  tests->add(new WBatchEditProxyModelTest());
  tests->add(new WChartTest());
  tests->add(new Utf8Test());
  tests->add(new WDateTimeTest());

  return tests;
}
