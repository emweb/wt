/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "web/InfraUtils.h"

BOOST_AUTO_TEST_CASE( InfraUtils_stripNewlines )
{
  using Wt::WHATWG::Infra::stripNewlines;

  std::string s = "one\ntwo\rthree";
  stripNewlines(s);
  BOOST_REQUIRE_EQUAL(s, "onetwothree");

  s = "one   \ntwo \r\r three\n";
  stripNewlines(s);
  BOOST_REQUIRE_EQUAL(s, "one   two  three");

  s = "one\r\ntwo\r\n";
  stripNewlines(s);
  BOOST_REQUIRE_EQUAL(s, "onetwo");

  s = "\n\r\n\r\n";
  stripNewlines(s);
  BOOST_REQUIRE_EQUAL(s, "");

  s = "";
  stripNewlines(s);
  BOOST_REQUIRE_EQUAL(s, "");
}

BOOST_AUTO_TEST_CASE( InfraUtils_trim )
{
  using Wt::WHATWG::Infra::trim;

  std::string s = "    \t\r\f\n  \t\r  ";
  trim(s);
  BOOST_REQUIRE_EQUAL(s, "");

  s = "   this is a string    ";
  trim(s);
  BOOST_REQUIRE_EQUAL(s, "this is a string");

  s = "";
  trim(s);
  BOOST_REQUIRE_EQUAL(s, "");

  s = "    \fone two";
  trim(s);
  BOOST_REQUIRE_EQUAL(s, "one two");

  s = "one two  \f\r\n ";
  trim(s);
  BOOST_REQUIRE_EQUAL(s, "one two");
}
