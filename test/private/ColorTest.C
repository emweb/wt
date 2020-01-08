/*
 * Copyright (C) 2018 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "web/ColorUtils.h"

using namespace Wt::Color;

BOOST_AUTO_TEST_CASE( color_test1 )
{
  Wt::WColor red1 = parseCssColor("rgb(255, 0, 0)");
  BOOST_REQUIRE(red1 == Wt::StandardColor::Red);
  Wt::WColor red2 = parseCssColor("#f00");
  BOOST_REQUIRE(red2 == Wt::StandardColor::Red);
  Wt::WColor red3 = parseCssColor("#ff0000");
  BOOST_REQUIRE(red3 == Wt::StandardColor::Red);
  Wt::WColor red4 = parseCssColor("rgba(255, 0, 0, 1)");
  BOOST_REQUIRE(red4 == Wt::StandardColor::Red);
  Wt::WColor red5 = parseCssColor("#f00f");
  BOOST_REQUIRE(red5 == Wt::StandardColor::Red);
  Wt::WColor red6 = parseCssColor("#ff0000ff");
  BOOST_REQUIRE(red6 == Wt::StandardColor::Red);
}
