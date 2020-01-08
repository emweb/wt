// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WException.h>
#include <Wt/WColor.h>

BOOST_AUTO_TEST_CASE( color_test_constructors )
{
  //string constructors
  {
    Wt::WColor c("#f80");
    BOOST_REQUIRE(c.red() == 255);
    BOOST_REQUIRE(c.green() == 136);
    BOOST_REQUIRE(c.blue() == 0);
    BOOST_REQUIRE(c.alpha() == 255); 
    BOOST_REQUIRE(c.cssText() == "#f80");
  }

  {
    Wt::WColor c("#12a0cf");
    BOOST_REQUIRE(c.red() == 18);
    BOOST_REQUIRE(c.green() == 160);
    BOOST_REQUIRE(c.blue() == 207);
    BOOST_REQUIRE(c.alpha() == 255); 
  }

  {
    Wt::WColor c("#FF0000");
    BOOST_REQUIRE(c.red() == 255);
    BOOST_REQUIRE(c.green() == 0);
    BOOST_REQUIRE(c.blue() == 0);
    BOOST_REQUIRE(c.alpha() == 255); 
  }

  {
    Wt::WColor c("rgb(18,160,207)");
    BOOST_REQUIRE(c.red() == 18);
    BOOST_REQUIRE(c.green() == 160);
    BOOST_REQUIRE(c.blue() == 207);
    BOOST_REQUIRE(c.alpha() == 255); 
  }

  {
    Wt::WColor c("rgb( 18  , 160  ,  207 )");
    BOOST_REQUIRE(c.red() == 18);
    BOOST_REQUIRE(c.green() == 160);
    BOOST_REQUIRE(c.blue() == 207);
    BOOST_REQUIRE(c.alpha() == 255);
  }

  {
    Wt::WColor c("rgb( 50%  , 25%  ,  0%)");
    BOOST_REQUIRE(c.red() == 127);
    BOOST_REQUIRE(c.green() == 63);
    BOOST_REQUIRE(c.blue() == 0);
    BOOST_REQUIRE(c.alpha() == 255); 
  }

  {
    Wt::WColor c("rgba(18,160,207,0.2)");
    BOOST_REQUIRE(c.red() == 18);
    BOOST_REQUIRE(c.green() == 160);
    BOOST_REQUIRE(c.blue() == 207);
    BOOST_REQUIRE(c.alpha() == 51); 
  }

  {
    Wt::WColor c("rgba(18 , 160  ,  207 , 0.2)");
    BOOST_REQUIRE(c.red() == 18);
    BOOST_REQUIRE(c.green() == 160);
    BOOST_REQUIRE(c.blue() == 207);
    BOOST_REQUIRE(c.alpha() == 51); 
  }

  {
    Wt::WColor c("rgba( 50%  , 25%  ,  0%, 0.22)");
    BOOST_REQUIRE(c.red() == 127);
    BOOST_REQUIRE(c.green() == 63);
    BOOST_REQUIRE(c.blue() == 0);
    BOOST_REQUIRE(c.alpha() == 56); 
  }

  {
    Wt::WColor c("#ff80");
    BOOST_REQUIRE(c.red() == 255 && c.green() == 255 && c.blue() == 0x88 && c.alpha() == 0);
  }

  //try to mess things up
  {
    Wt::WColor c("#f8 0");
    BOOST_REQUIRE(c.red() == 0 && c.green() == 0 && c.blue() == 0);
  }

  {
    Wt::WColor c("rgb(30%,20%)");
    BOOST_REQUIRE(c.red() == 0 && c.green() == 0 && c.blue() == 0);
  }

  {
    Wt::WColor c("rgb(30%,20%,50%,0.2)");
    BOOST_REQUIRE(c.red() == 0 && c.green() == 0 && c.blue() == 0);
  }

  {
    Wt::WColor c("rgb30%,20%,50%)");
    BOOST_REQUIRE(c.red() == 0 && c.green() == 0 && c.blue() == 0);
  }

  {
    Wt::WColor c("rgb(30%,20%,50%");
    BOOST_REQUIRE(c.red() == 0 && c.green() == 0 && c.blue() == 0);
  }

  {
    Wt::WColor c("rgba(30%,20%,50%,a)");
    BOOST_REQUIRE(c.alpha() == 255);
  }

  {
    Wt::WColor c("rgba(30%,20%,50%)");
    BOOST_REQUIRE(c.red() == 0 && c.green() == 0 && c.blue() == 0);
  }

  {
    Wt::WColor c("rgba(30%,20%,50%,0.2,x)");
    BOOST_REQUIRE(c.red() == 0 && c.green() == 0 && c.blue() == 0);
  }

  {
    Wt::WColor c("gold");
    BOOST_REQUIRE(c.red() == 0 && c.green() == 0 && c.blue() == 0);
    BOOST_REQUIRE(c.cssText() == "gold");
  }
}
