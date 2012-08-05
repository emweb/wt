/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WLength>

BOOST_AUTO_TEST_CASE( length_test_constructors )
{
  //default constructor
  {
  Wt::WLength a;
  BOOST_REQUIRE(a.isAuto());
  BOOST_REQUIRE(a.value() == -1);
  BOOST_REQUIRE(a.unit() == Wt::WLength::Pixel);
  }

  //double constructor
  {
  Wt::WLength d(50.0);
  BOOST_REQUIRE(!d.isAuto());
  BOOST_REQUIRE(d.value() == 50.0);
  BOOST_REQUIRE(d.unit() == Wt::WLength::Pixel);
  }

  {
  Wt::WLength d(99.0, Wt::WLength::Centimeter);
  BOOST_REQUIRE(!d.isAuto());
  BOOST_REQUIRE(d.value() == 99.0);
  BOOST_REQUIRE(d.unit() == Wt::WLength::Centimeter);
  }

  //int constructor
  {
  Wt::WLength i(10);
  BOOST_REQUIRE(!i.isAuto());
  BOOST_REQUIRE(i.value() == 10.0);
  BOOST_REQUIRE(i.unit() == Wt::WLength::Pixel);
  }

  {
  Wt::WLength i(10, Wt::WLength::Centimeter);
  BOOST_REQUIRE(!i.isAuto());
  BOOST_REQUIRE(i.value() == 10.0);
  BOOST_REQUIRE(i.unit() == Wt::WLength::Centimeter);
  }

  {
  Wt::WLength i(0);
  BOOST_REQUIRE(!i.isAuto());
  BOOST_REQUIRE(i.value() == 0.0);
  BOOST_REQUIRE(i.unit() == Wt::WLength::Pixel);
  }

  //string constructor
  {
  Wt::WLength s("10px");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 10.0);
  BOOST_REQUIRE(s.unit() == Wt::WLength::Pixel);
  }
  
  {
  Wt::WLength s("15.2em");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.2);
  BOOST_REQUIRE(s.unit() == Wt::WLength::FontEm);
  }

  {
  Wt::WLength s("15.2ex");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.2);
  BOOST_REQUIRE(s.unit() == Wt::WLength::FontEx);
  }

  {
  Wt::WLength s("15.0in");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::WLength::Inch);
  }

  {
  Wt::WLength s("15.0cm");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::WLength::Centimeter);
  }

  {
  Wt::WLength s("15.0mm");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::WLength::Millimeter);
  }

  {
  Wt::WLength s("15.0mm");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::WLength::Millimeter);
  }

  {
  Wt::WLength s("15.0pt");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::WLength::Point);
  }
  
  {
  Wt::WLength s("15.0pc");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::WLength::Pica);
  }

  {
  Wt::WLength s("15.0%");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::WLength::Percentage);
  }

  //add some random empty chars 
  {
  Wt::WLength s("  15.0   px   ");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::WLength::Pixel);
  }

  //try to mess things up
  {
    Wt::WLength s("px");
    BOOST_REQUIRE(s.isAuto());
  }

  {
    Wt::WLength s("100pn");
    BOOST_REQUIRE(s.isAuto());
  }
}
