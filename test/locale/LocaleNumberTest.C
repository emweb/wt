/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>
#include <Wt/WLocale.h>

using namespace Wt;

BOOST_AUTO_TEST_CASE( formatCLocale )
{
  double d = 12345678.2345;

  WLocale l;

  WString s = l.toString(d);

  BOOST_REQUIRE(s == "12345678.2345");
}

BOOST_AUTO_TEST_CASE( formatGrouping )
{
  double d = 12345678.2345;

  WLocale l;
  l.setGroupSeparator(",");

  WString s = l.toString(d);

  BOOST_REQUIRE(s == "12,345,678.2345");

  s = l.toString(d / 10);

  BOOST_REQUIRE(s == "1,234,567.82345");
  BOOST_REQUIRE((l.toDouble(s) - d / 10) < 1E-6);

  s = l.toString(d / 100);

  BOOST_REQUIRE(s == "123,456.782345");
  BOOST_REQUIRE((l.toDouble(s) - d / 100) < 1E-7);

  s = l.toString(d / 1E30);

  BOOST_REQUIRE(s == "1.23456782345e-23");

  s = l.toString(d * 1E30);

  BOOST_REQUIRE(s == "1.23456782345e+37");

  s = l.toString(d / -1E30);

  BOOST_REQUIRE(s == "-1.23456782345e-23");

  s = l.toString(d * -1E30);

  BOOST_REQUIRE(s == "-1.23456782345e+37");
}

BOOST_AUTO_TEST_CASE( formatNegGrouping )
{
  double d = -12345678.2345;

  WLocale l;
  l.setGroupSeparator(",");

  WString s = l.toString(d);

  BOOST_REQUIRE(s == "-12,345,678.2345");

  s = l.toString(d / 10);

  BOOST_REQUIRE(s == "-1,234,567.82345");
  BOOST_REQUIRE((l.toDouble(s) - d / 10) < 1E-6);

  s = l.toString(d / 100);

  BOOST_REQUIRE(s == "-123,456.782345");
  BOOST_REQUIRE((l.toDouble(s) - d / 100) < 1E-7);

  s = l.toString(d / 1E30);

  BOOST_REQUIRE(s == "-1.23456782345e-23");

  s = l.toString(d * 1E30);

  BOOST_REQUIRE(s == "-1.23456782345e+37");

  s = l.toString(d / -1E30);

  BOOST_REQUIRE(s == "1.23456782345e-23");

  s = l.toString(d * -1E30);

  BOOST_REQUIRE(s == "1.23456782345e+37");
}
