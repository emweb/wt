/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "web/WebUtils.h"

#include <limits>

BOOST_AUTO_TEST_CASE( RoundJsString_test1 )
{
  char buf[100];
  // Special cases
  BOOST_CHECK_EQUAL("NaN", Wt::Utils::round_js_str(std::numeric_limits<double>::quiet_NaN(), 3, buf));
  BOOST_CHECK_EQUAL("Infinity", Wt::Utils::round_js_str(std::numeric_limits<double>::infinity(), 3, buf));
  BOOST_CHECK_EQUAL("-Infinity", Wt::Utils::round_js_str(- std::numeric_limits<double>::infinity(), 3, buf));

  // Test for Spirit issue #688: https://github.com/boostorg/spirit/issues/688
  BOOST_CHECK_EQUAL(10., Wt::Utils::stod(Wt::Utils::round_js_str(9.999999999, 7, buf)));
  BOOST_CHECK_EQUAL(11., Wt::Utils::stod(Wt::Utils::round_js_str(10.999999999, 7, buf)));
  BOOST_CHECK_EQUAL(12., Wt::Utils::stod(Wt::Utils::round_js_str(11.999999999, 7, buf)));
  BOOST_CHECK_EQUAL(103., Wt::Utils::stod(Wt::Utils::round_js_str(102.999999999, 7, buf)));
  BOOST_CHECK_EQUAL(-10., Wt::Utils::stod(Wt::Utils::round_js_str(-9.999999999, 7, buf)));
  BOOST_CHECK_EQUAL(103., Wt::Utils::stod(Wt::Utils::round_js_str(102.999999999, 7, buf)));
  BOOST_CHECK_EQUAL(-103., Wt::Utils::stod(Wt::Utils::round_js_str(-102.999999999, 7, buf)));
}
