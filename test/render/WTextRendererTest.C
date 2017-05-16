#include <boost/test/unit_test.hpp>

#include <Wt/Render/WTextRenderer.h>
#include <iostream>
#include <boost/version.hpp>

#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104700
#  define CSS_PARSER
#endif

#ifdef CSS_PARSER

class MyTextRenderer : public Wt::Render::WTextRenderer
{
  virtual double pageWidth(int page) const override {return 0;}
  virtual double pageHeight(int page) const override {return 0;}
  virtual double margin(Wt::Side side) const override {return 0;}
  virtual Wt::WPaintDevice *startPage(int page) override{return 0;}
  virtual void endPage(Wt::WPaintDevice *device) override{}
  virtual Wt::WPainter *getPainter(Wt::WPaintDevice *device) override{return 0;}
};

BOOST_AUTO_TEST_CASE( WTextRenderer_testStylesheet )
{
  MyTextRenderer r;

  BOOST_REQUIRE( r.setStyleSheetText("h1{}") );
  BOOST_REQUIRE( r.setStyleSheetText("h1{}") );
  BOOST_REQUIRE( r.styleSheetText() == "h1{}" );
  BOOST_REQUIRE( r.setStyleSheetText("") );
  BOOST_REQUIRE( r.styleSheetText() == "" );

  BOOST_REQUIRE(  r.setStyleSheetText("h1{}") );
  BOOST_REQUIRE( !r.setStyleSheetText("h1{} #1bla{}") );
  BOOST_REQUIRE(  r.getStyleSheetParseErrors().size() );
  BOOST_REQUIRE( !r.setStyleSheetText("h1{} 1h{}") );
  BOOST_REQUIRE(  r.getStyleSheetParseErrors().size() );
  BOOST_REQUIRE(  r.styleSheetText() == "h1{}" );
  BOOST_REQUIRE(  r.setStyleSheetText("h1{}") );
  BOOST_REQUIRE( !r.getStyleSheetParseErrors().size() );
}

#endif // CSS_PARSER
