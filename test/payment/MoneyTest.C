// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WException.h>
#include <Wt/Payment/Money.h>

BOOST_AUTO_TEST_CASE(money_test)
{
  {
    Wt::Payment::Money v1 = Wt::Payment::Money(13, 67, "EUR");
    Wt::Payment::Money v2 = Wt::Payment::Money(2, 75, "EUR");
    Wt::Payment::Money v3 = Wt::Payment::Money(4, 1, "EUR");
    double d = 100.9;

    BOOST_REQUIRE(v1.toString() == "13.67");
    BOOST_REQUIRE(v3.toString() == "4.01");

    BOOST_REQUIRE((v1 + v2).toString() == "16.42");
    BOOST_REQUIRE((v1 - v2).toString() == "10.92");
    BOOST_REQUIRE((v1 * d).toString() == "1379.30");
    BOOST_REQUIRE((v1 / d).toString() == "0.13");
    BOOST_REQUIRE((v3 * d).toString() == "404.60");

    Wt::Payment::Money v4 = (v1 + v2);
    v1+= v2;
    v2*= 8.1;
    BOOST_REQUIRE(v1.toString() == v4.toString());
    BOOST_REQUIRE(v2.toString() == "22.27");

    v2/= 8.1;
    //test - rounding error.
    BOOST_REQUIRE(v2.toString() == "2.74");
  }

}
