/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WLength.h>

BOOST_AUTO_TEST_CASE( length_test_constructors )
{
  //default constructor
  {
  Wt::WLength a;
  BOOST_REQUIRE(a.isAuto());
  BOOST_REQUIRE(a.value() == -1);
  BOOST_REQUIRE(a.unit() == Wt::LengthUnit::Pixel);
  }

  //double constructor
  {
  Wt::WLength d(50.0);
  BOOST_REQUIRE(!d.isAuto());
  BOOST_REQUIRE(d.value() == 50.0);
  BOOST_REQUIRE(d.unit() == Wt::LengthUnit::Pixel);
  }

  {
  Wt::WLength d(99.0, Wt::LengthUnit::Centimeter);
  BOOST_REQUIRE(!d.isAuto());
  BOOST_REQUIRE(d.value() == 99.0);
  BOOST_REQUIRE(d.unit() == Wt::LengthUnit::Centimeter);
  }

  //int constructor
  {
  Wt::WLength i(10);
  BOOST_REQUIRE(!i.isAuto());
  BOOST_REQUIRE(i.value() == 10.0);
  BOOST_REQUIRE(i.unit() == Wt::LengthUnit::Pixel);
  }

  {
  Wt::WLength i(10, Wt::LengthUnit::Centimeter);
  BOOST_REQUIRE(!i.isAuto());
  BOOST_REQUIRE(i.value() == 10.0);
  BOOST_REQUIRE(i.unit() == Wt::LengthUnit::Centimeter);
  }

  {
  Wt::WLength i(0);
  BOOST_REQUIRE(!i.isAuto());
  BOOST_REQUIRE(i.value() == 0.0);
  BOOST_REQUIRE(i.unit() == Wt::LengthUnit::Pixel);
  }

  //string constructor
  {
  Wt::WLength s("10px");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 10.0);
  BOOST_REQUIRE(s.unit() == Wt::LengthUnit::Pixel);
  }
  
  {
  Wt::WLength s("15.2em");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.2);
  BOOST_REQUIRE(s.unit() == Wt::LengthUnit::FontEm);
  }

  {
  Wt::WLength s("15.2ex");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.2);
  BOOST_REQUIRE(s.unit() == Wt::LengthUnit::FontEx);
  }

  {
  Wt::WLength s("15.0in");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::LengthUnit::Inch);
  }

  {
  Wt::WLength s("15.0cm");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::LengthUnit::Centimeter);
  }

  {
  Wt::WLength s("15.0mm");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::LengthUnit::Millimeter);
  }

  {
  Wt::WLength s("15.0mm");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::LengthUnit::Millimeter);
  }

  {
  Wt::WLength s("15.0pt");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::LengthUnit::Point);
  }
  
  {
  Wt::WLength s("15.0pc");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::LengthUnit::Pica);
  }

  {
  Wt::WLength s("15.0%");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::LengthUnit::Percentage);
  }

  //add some random empty chars 
  {
  Wt::WLength s("  15.0   px   ");
  BOOST_REQUIRE(!s.isAuto());
  BOOST_REQUIRE(s.value() == 15.0);
  BOOST_REQUIRE(s.unit() == Wt::LengthUnit::Pixel);
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

BOOST_AUTO_TEST_CASE( length_test_css_text )
{
  //'auto' renders as the bare keyword
  {
  BOOST_CHECK(Wt::WLength::Auto.cssText() == "auto");
  BOOST_CHECK(Wt::WLength().cssText() == "auto");
  }

  //numeric units append the unit suffix
  {
  Wt::WLength l(1.5);
  BOOST_CHECK(l.cssText() == "1.5px");
  }

  {
  Wt::WLength l(15.0, Wt::LengthUnit::FontEm);
  BOOST_CHECK(l.cssText() == "15.0em");
  }

  {
  Wt::WLength l(15.0, Wt::LengthUnit::Percentage);
  BOOST_CHECK(l.cssText() == "15.0%");
  }

  {
  Wt::WLength l(15.0, Wt::LengthUnit::ViewportWidth);
  BOOST_CHECK(l.cssText() == "15.0vw");
  }
}

BOOST_AUTO_TEST_CASE( length_test_to_pixels )
{
  //'auto' is 0 pixels
  {
  BOOST_CHECK(Wt::WLength::Auto.toPixels() == 0);
  }

  //font-relative units scale with the font size
  {
  Wt::WLength l(2.0, Wt::LengthUnit::FontEm);
  BOOST_CHECK(l.toPixels(10.0) == 20.0);
  }

  {
  Wt::WLength l(2.0, Wt::LengthUnit::FontEx);
  BOOST_CHECK(l.toPixels(10.0) == 10.0);
  }

  //percentages are approximated relative to the font size
  {
  Wt::WLength l(50.0, Wt::LengthUnit::Percentage);
  BOOST_CHECK(l.toPixels(16.0) == 8.0);
  }

  //absolute units use fixed conversion factors
  {
  Wt::WLength l(10.0);
  BOOST_CHECK(l.toPixels() == 10.0);
  }

  {
  Wt::WLength l(1.0, Wt::LengthUnit::Inch);
  BOOST_CHECK(l.toPixels() == 72 * (4.0 / 3.0));
  }

  {
  Wt::WLength l(1.0, Wt::LengthUnit::Point);
  BOOST_CHECK(l.toPixels() == 4.0 / 3.0);
  }
}
