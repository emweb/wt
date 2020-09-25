/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WAny.h>

BOOST_AUTO_TEST_CASE( any_test )
{
  // Issue #7719
  Wt::cpp17::any value = 4ul;
  BOOST_REQUIRE_EQUAL(Wt::utf8("4"), Wt::asString(value));
}
