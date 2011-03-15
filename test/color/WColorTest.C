// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "web/WtException.h"

#include <Wt/WColor>

BOOST_AUTO_TEST_CASE( color_test_constructors )
{
  //string constructors
  {
    Wt::WColor c("#f80");
    BOOST_REQUIRE(c.red() == 255);
    BOOST_REQUIRE(c.green() == 136);
    BOOST_REQUIRE(c.blue() == 0);
  }

  {
    Wt::WColor c("#12a0cf");
    BOOST_REQUIRE(c.red() == 18);
    BOOST_REQUIRE(c.green() == 160);
    BOOST_REQUIRE(c.blue() == 207);
  }

  {
    Wt::WColor c("rgb(18,160,207)");
    BOOST_REQUIRE(c.red() == 18);
    BOOST_REQUIRE(c.green() == 160);
    BOOST_REQUIRE(c.blue() == 207);
  }

  {
    Wt::WColor c("rgb( 18  , 160  ,  207 )");
    BOOST_REQUIRE(c.red() == 18);
    BOOST_REQUIRE(c.green() == 160);
    BOOST_REQUIRE(c.blue() == 207);
  }

  {
    Wt::WColor c("rgb( 50%  , 25%  ,  0%)");
    BOOST_REQUIRE(c.red() == 127);
    BOOST_REQUIRE(c.green() == 63);
    BOOST_REQUIRE(c.blue() == 0);
    BOOST_REQUIRE(c.alpha() == 255); 
  }

  {
    Wt::WColor c("rgba(18,160,207,50)");
    BOOST_REQUIRE(c.red() == 18);
    BOOST_REQUIRE(c.green() == 160);
    BOOST_REQUIRE(c.blue() == 207);
    BOOST_REQUIRE(c.alpha() == 50); 
  }

  {
    Wt::WColor c("rgba(18 , 160  ,  207 , 50)");
    BOOST_REQUIRE(c.red() == 18);
    BOOST_REQUIRE(c.green() == 160);
    BOOST_REQUIRE(c.blue() == 207);
    BOOST_REQUIRE(c.alpha() == 50); 
  }

  {
    Wt::WColor c("rgba( 50%  , 25%  ,  0%, 55)");
    BOOST_REQUIRE(c.red() == 127);
    BOOST_REQUIRE(c.green() == 63);
    BOOST_REQUIRE(c.blue() == 0);
    BOOST_REQUIRE(c.alpha() == 55); 
  }

  //try to mess things up
  {
    std::string exception;
    try {
      Wt::WColor c("#f8 0");
    } catch (Wt::WtException &e) {
      exception = e.what();
    }
    BOOST_REQUIRE(exception == 
		  "WColor: Could not parse rgb format: #f8 0");
  }
  
  {
    std::string exception;
    try {
      Wt::WColor c("#ff80");
    } catch (Wt::WtException &e) {
      exception = e.what();
    }
    BOOST_REQUIRE(exception == 
		  "WColor: Could not parse rgb format: #ff80");
  }

   {
    std::string exception;
    try {
      Wt::WColor c("rgb(30%,20%)");
    } catch (Wt::WtException &e) {
      exception = e.what();
    }
    BOOST_REQUIRE(exception == 
		  "WColor: Invalid argument count: rgb(30%,20%)");
   }

   {
    std::string exception;
    try {
      Wt::WColor c("rgb(30%,20%,50%,50)");
    } catch (Wt::WtException &e) {
      exception = e.what();
    }
    BOOST_REQUIRE(exception == 
		  "WColor: Invalid argument count: rgb(30%,20%,50%,50)");
   }

   {
    std::string exception;
    try {
      Wt::WColor c("rgb30%,20%,50%)");
    } catch (Wt::WtException &e) {
      exception = e.what();
    }
    BOOST_REQUIRE(exception == 
		  "WColor: Missing brackets in rgb format: rgb30%,20%,50%)");
   }

   {
    std::string exception;
    try {
      Wt::WColor c("rgb(30%,20%,50%");
    } catch (Wt::WtException &e) {
      exception = e.what();
    }
    BOOST_REQUIRE(exception == 
		  "WColor: Missing brackets in rgb format: rgb(30%,20%,50%");
   }

   {
    std::string exception;
    try {
      Wt::WColor c("rgba(30%,20%,50%,a)");
    } catch (Wt::WtException &e) {
      exception = e.what();
    }
    BOOST_REQUIRE(exception == 
		  "WColor: Illegal alpha value: a");
   }

   {
    std::string exception;
    try {
      Wt::WColor c("rgba(30%,20%,50%)");
    } catch (Wt::WtException &e) {
      exception = e.what();
    }
    BOOST_REQUIRE(exception == 
		  "WColor: Invalid argument count: rgba(30%,20%,50%)");
   }

   {
    std::string exception;
    try {
      Wt::WColor c("rgba(30%,20%,50%,50,x)");
    } catch (Wt::WtException &e) {
      exception = e.what();
    }
    BOOST_REQUIRE(exception == 
		  "WColor: Invalid argument count: rgba(30%,20%,50%,50,x)");
   }

   {
    std::string exception;
    try {
      Wt::WColor c("gold");
      c.blue();
    } catch (Wt::WtException &e) {
      exception = e.what();
    }
    BOOST_REQUIRE(exception == "WColor: No rgb values are available");
   }
}
