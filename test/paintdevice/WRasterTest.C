/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WConfig.h>

#ifdef WT_HAS_WRASTERIMAGE

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fstream>

#include <Wt/WRasterImage.h>
#include <Wt/WPainter.h>
#include <Wt/WPointF.h>
#include <Wt/Test/WTestEnvironment.h>
#include <Wt/Render/WTextRenderer.h>

namespace {
  using namespace Wt;

  class MultiLineTextRenderer : public Wt::Render::WTextRenderer
  {
  public:
    MultiLineTextRenderer(WPainter& painter, const WRectF& rect)
      : painter_(painter),
	rect_(rect)
    { }
    
    virtual double pageWidth(int page) const override {
      return rect_.right();
    }
    
    virtual double pageHeight(int page) const override {
      return 1E9;
    }
    
    virtual double margin(Side side) const override {
      switch (side) {
      case Side::Top: return rect_.top(); break;
      case Side::Left: return rect_.left(); break;
      default:
      return 0;
      }
    }

    virtual WPaintDevice *startPage(int page) override {
      if (page > 0)
	assert(false);
      
      return painter_.device();
    }
    
    virtual void endPage(WPaintDevice *device) override {
    }

    virtual WPainter *getPainter(WPaintDevice *device) override {
      return &painter_;
    }
    
  private:
    WPainter& painter_;
    WRectF    rect_;
  };
}

BOOST_AUTO_TEST_CASE( raster_test_textRenderer )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WRasterImage rasterImage("png", 357, 193);
  {
    Wt::WPainter p(&rasterImage);
    std::string text = 
      "<table style=\"width:357px;\"><tr><td style=\"padding:0px;height:"
      "193px;color:rgb(247,17,117);text-align:left;vertical-align:top;"
      "font-family: Arial;font-size: 60.0pt;font-weight: normal;\">"
      "xxx</td></tr></table>";
    MultiLineTextRenderer renderer(p, WRectF(0, 0, 357, 193));
    renderer.render(text);
  }

  std::ofstream f("text_render_image_1.png");
  rasterImage.write(f);
}

BOOST_AUTO_TEST_CASE( raster_test_text_embedded_stylesheet )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WRasterImage rasterImage("png", 357, 193);
  {
    Wt::WPainter p(&rasterImage);
    std::string text =
      "<style>"
        "table {width:357px;}"
        "td {padding: 0px; height: 193px; color: #f71175;"
            "text-align: left; vertical-align: top; font-family: Arial;"
            "font-size: 60.0pt; font-weight: normal;}"
      "</style>"
      "<table><tr><td>xxx</td></tr></table>";
    MultiLineTextRenderer renderer(p, WRectF(0, 0, 357, 193));
    renderer.render(text);
  }

  std::ofstream f("text_render_image_2.png");
  rasterImage.write(f);
}

BOOST_AUTO_TEST_CASE( raster_test_dataUriImage )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

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

#endif
