// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.

 *
 * See the LICENSE file for terms of use.
 */
#include <boost/bind.hpp>

#include <iostream>
#include <fstream>

#include <Wt/WRasterImage>
#include <Wt/WPainter>
#include <Wt/WPointF>
#include "Wt/Test/WTestEnvironment"

#include "WRasterTest.h"

void WRasterTest::test_dataUriImage()
{
  Wt::Test::WTestEnvironment environment;
  //Wt::WApplication app(environment);

  Wt::WRasterImage rasterImage("png", 80, 80);
  Wt::WPainter p(&rasterImage);

  {
    std::string uri 
      = "data:image/gif;base64,R0lGODdhMAAwAPAAAAAAAP///ywAAAAAMAAw"
      "AAAC8IyPqcvt3wCcDkiLc7C0qwyGHhSWpjQu5yqmCYsapyuvUUlvONmOZtfzgFz"
      "ByTB10QgxOR0TqBQejhRNzOfkVJ+5YiUqrXF5Y5lKh/DeuNcP5yLWGsEbtLiOSp"
      "a/TPg7JpJHxyendzWTBfX0cxOnKPjgBzi4diinWGdkF8kjdfnycQZXZeYGejmJl"
      "ZeGl9i2icVqaNVailT6F5iJ90m6mvuTS4OK05M0vDk0Q4XUtwvKOzrcd3iq9uis"
      "F81M1OIcR7lEewwcLp7tuNNkM3uNna3F2JQFo97Vriy/Xl4/f1cf5VWzXyym7PH"
      "hhx4dbgYKAAA7";
    
    Wt::WPainter::Image image(uri, 48, 48);
    p.drawImage(Wt::WPointF(0,0), image);
  }
  
  {
    bool error = false;
    try {
      std::string uri = "data:image/gif;,R0lGODdhMAAwAPAAAAAAAP///ywAAAAAMAAw";
    
      Wt::WPainter::Image image(uri, 48, 48);
      p.drawImage(Wt::WPointF(0,0), image);
    } catch (...) {
      error = true;
    }
    BOOST_REQUIRE(error);
  }
  
  {
    bool error = false;
    try {
      std::string uri 
	= "data:image/tiff;base64,R0lGODdhMAAwAPAAAAAAAP///ywAAAAAMAAw";
    
      Wt::WPainter::Image image(uri, 48, 48);
      p.drawImage(Wt::WPointF(0,0), image);
    } catch (...) {
      error = true;
    }
    BOOST_REQUIRE(error);
  }

  {
    bool error = false;
    try {
      std::string uri 
	= "data:;base64,R0lGODdhMAAwAPAAAAAAAP///ywAAAAAMAAw";
    
      Wt::WPainter::Image image(uri, 48, 48);
      p.drawImage(Wt::WPointF(0,0), image);
    } catch (...) {
      error = true;
    }
    BOOST_REQUIRE(error);
  }

  {
    bool error = false;
    try {
      std::string uri = "data:;base64,R0lGODdhMAAwAPAAAAAAAP///ywAAAAAMAAw";
    
      Wt::WPainter::Image image(uri, 48, 48);
      p.drawImage(Wt::WPointF(0,0), image);
    } catch (...) {
      error = true;
    }
    BOOST_REQUIRE(error);
  }

  {
    bool error = false;
    try {
      std::string uri = "data:image/gif;base64,";
    
      Wt::WPainter::Image image(uri, 48, 48);
      p.drawImage(Wt::WPointF(0,0), image);
    } catch (...) {
      error = true;
    }
    BOOST_REQUIRE(error);
  }
  
  p.end();
  std::ofstream f("data_uri_image.png");
  rasterImage.write(f);
}

WRasterTest::WRasterTest()
  : test_suite("raster_test_suite")
{
  add(BOOST_TEST_CASE
      (boost::bind(&WRasterTest::test_dataUriImage, this)));
}
