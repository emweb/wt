/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WString.h>


BOOST_AUTO_TEST_CASE(WString_test_empty_substition)
{
    Wt::WString tmplt = "{1}";
    Wt::WString tmplt2 = "{1}{2}";
    tmplt.arg("");
    tmplt2.arg("").arg("");

    BOOST_REQUIRE(tmplt.empty());
    BOOST_REQUIRE(tmplt2.empty());
}

BOOST_AUTO_TEST_CASE(WString_test_non_empty_substition)
{
    Wt::WString tmplt = "{1}";
    tmplt.arg("abc");

    BOOST_REQUIRE(!tmplt.empty());
}
