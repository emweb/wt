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

BOOST_AUTO_TEST_CASE(WString_test_append)
{
  // Tests a regression where strings with arguments were not regarded as literal.
  // This would result in WString trying to resolve a string with an empty key when
  // it was appended to. The string would then become "????" to indicate that the empty
  // key was not found.
  // Only localized strings should be regarded non-literal
  Wt::WString s = "{1}";
  s.arg("Hello");

  BOOST_REQUIRE_EQUAL(s, "Hello");

  BOOST_REQUIRE(s.literal());

  s += " {2}";
  s.arg("World");

  BOOST_REQUIRE_EQUAL(s, "Hello World");
}
