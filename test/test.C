#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>

using boost::unit_test_framework::test_suite;

#include "chart/WChartTest.h"
#include "dbo/DboTest.h"
#include "dbo/DboTest2.h"
#include "wdatetime/WDateTimeTest.h"

boost::unit_test::test_suite* init_unit_test_suite(int /* argc */, char * * const /* argv */)
{
  test_suite *top_test_suite = BOOST_TEST_SUITE("Master test suite");

  top_test_suite->add(new WDateTimeTest());
  top_test_suite->add(new WChartTest());
  top_test_suite->add(new DboTest());
  top_test_suite->add(new DboTest2());

  return top_test_suite;
}
