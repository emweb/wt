/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WException.h>
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

BOOST_AUTO_TEST_CASE( length_test_keyword_parsing )
{
  //the content sizing keywords parse as keyword units without a value
  {
  Wt::WLength s("fit-content");
  BOOST_CHECK(!s.isAuto());
  BOOST_CHECK(s.unit() == Wt::LengthUnit::FitContent);
  BOOST_CHECK(s.value() == -1);
  }

  {
  Wt::WLength s("max-content");
  BOOST_CHECK(!s.isAuto());
  BOOST_CHECK(s.unit() == Wt::LengthUnit::MaxContent);
  BOOST_CHECK(s.value() == -1);
  }

  {
  Wt::WLength s("min-content");
  BOOST_CHECK(!s.isAuto());
  BOOST_CHECK(s.unit() == Wt::LengthUnit::MinContent);
  BOOST_CHECK(s.value() == -1);
  }

  //surrounding whitespace is ignored, like for numeric lengths
  {
  Wt::WLength s("  fit-content  ");
  BOOST_CHECK(!s.isAuto());
  BOOST_CHECK(s.unit() == Wt::LengthUnit::FitContent);
  }

  {
  Wt::WLength s(" auto ");
  BOOST_CHECK(s.isAuto());
  }

  //keywords are case-sensitive, like 'auto' and the unit suffixes
  {
  Wt::WLength s("Fit-Content");
  BOOST_CHECK(s.isAuto());
  }

  //the fit-content() functional form is not supported
  {
  Wt::WLength s("fit-content(200px)");
  BOOST_CHECK(s.isAuto());
  }

  //a number followed by a keyword is not a valid length
  {
  Wt::WLength s("10fit-content");
  BOOST_CHECK(s.isAuto());
  }
}

BOOST_AUTO_TEST_CASE( length_test_keyword_unit_constructor )
{
  //keyword units can be constructed without a value, equivalent to
  //parsing the bare keyword
  {
  Wt::WLength l(Wt::LengthUnit::FitContent);
  BOOST_CHECK(!l.isAuto());
  BOOST_CHECK(l.unit() == Wt::LengthUnit::FitContent);
  BOOST_CHECK(l.value() == -1);
  BOOST_CHECK(l == Wt::WLength("fit-content"));
  }

  {
  Wt::WLength l(Wt::LengthUnit::MaxContent);
  BOOST_CHECK(!l.isAuto());
  BOOST_CHECK(l.unit() == Wt::LengthUnit::MaxContent);
  BOOST_CHECK(l.value() == -1);
  BOOST_CHECK(l == Wt::WLength("max-content"));
  }

  {
  Wt::WLength l(Wt::LengthUnit::MinContent);
  BOOST_CHECK(!l.isAuto());
  BOOST_CHECK(l.unit() == Wt::LengthUnit::MinContent);
  BOOST_CHECK(l.value() == -1);
  BOOST_CHECK(l == Wt::WLength("min-content"));
  }

  //the constructor is implicit, like the double constructor
  {
  Wt::WLength l = Wt::LengthUnit::MaxContent;
  BOOST_CHECK(l.cssText() == "max-content");
  }

  //units with a numeric component cannot be constructed without a value
  {
  BOOST_CHECK_THROW(Wt::WLength{Wt::LengthUnit::Pixel}, Wt::WException);
  BOOST_CHECK_THROW(Wt::WLength{Wt::LengthUnit::Percentage}, Wt::WException);
  BOOST_CHECK_THROW(Wt::WLength{Wt::LengthUnit::ViewportMax}, Wt::WException);
  }
}

BOOST_AUTO_TEST_CASE( length_test_keyword_css_text )
{
  //keyword units render as the bare keyword
  {
  BOOST_CHECK(Wt::WLength("fit-content").cssText() == "fit-content");
  BOOST_CHECK(Wt::WLength("max-content").cssText() == "max-content");
  BOOST_CHECK(Wt::WLength("min-content").cssText() == "min-content");
  }

  //the value passed to the constructor is ignored for keyword units
  {
  Wt::WLength l(42.0, Wt::LengthUnit::MaxContent);
  BOOST_CHECK(l.cssText() == "max-content");
  BOOST_CHECK(l.value() == -1);
  }
}

BOOST_AUTO_TEST_CASE( length_test_keyword_to_pixels )
{
  //content-dependent sizes cannot be approximated server-side
  {
  BOOST_CHECK(Wt::WLength("fit-content").toPixels() == 0);
  BOOST_CHECK(Wt::WLength("max-content").toPixels() == 0);
  BOOST_CHECK(Wt::WLength("min-content").toPixels() == 0);
  }

  {
  Wt::WLength l(100.0, Wt::LengthUnit::FitContent);
  BOOST_CHECK(l.toPixels() == 0);
  }
}

BOOST_AUTO_TEST_CASE( length_test_keyword_equality )
{
  //keyword lengths are canonical: the value does not participate
  {
  BOOST_CHECK(Wt::WLength("fit-content") ==
                Wt::WLength(7.0, Wt::LengthUnit::FitContent));
  BOOST_CHECK(Wt::WLength("fit-content") != Wt::WLength("max-content"));
  BOOST_CHECK(Wt::WLength("fit-content") != Wt::WLength::Auto);
  }

  //arithmetic keeps keyword lengths canonical
  {
  Wt::WLength l = Wt::WLength("fit-content") * 2;
  BOOST_CHECK(l.cssText() == "fit-content");
  BOOST_CHECK(l == Wt::WLength("fit-content"));
  }
}
