/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fstream>

#include <Wt/WSvgImage.h>
#include <Wt/WRectF.h>
#include <Wt/WPainter.h>
#include <Wt/WPen.h>

BOOST_AUTO_TEST_CASE( svg_test_drawWrappedText )
{
  static std::string text = 
    "ceci n'est pas un text et ceci n'est pas une pipe non plus";

  Wt::WSvgImage svgImage(1800, 800);
  Wt::WPainter p(&svgImage);
  
  //horizontal alignment
  {
  Wt::WRectF r(5, 5, 150, 100);
  p.drawText(r, Wt::AlignmentFlag::Left | Wt::AlignmentFlag::Top,
	     Wt::TextFlag::WordWrap, text);
  p.drawRect(r);
  }

  {
  Wt::WRectF r(160, 5, 150, 100);
  p.drawText(r, Wt::AlignmentFlag::Right | Wt::AlignmentFlag::Top,
	     Wt::TextFlag::WordWrap, text);
  p.drawRect(r);
  }

  {
  Wt::WRectF r(315, 5, 150, 100);
  p.drawText(r, Wt::AlignmentFlag::Center | Wt::AlignmentFlag::Top,
	     Wt::TextFlag::WordWrap, text);
  p.drawRect(r);
  }
  
  {
  Wt::WRectF r(470, 5, 150, 100);
  p.drawText(r, Wt::AlignmentFlag::Justify | Wt::AlignmentFlag::Top,
	     Wt::TextFlag::WordWrap, text);
  p.drawRect(r);
  }

  //red text
  {
  Wt::WRectF r(625, 5, 150, 100);
  p.setPen(Wt::WPen(Wt::StandardColor::Red));
  p.drawText(r, Wt::AlignmentFlag::Justify | Wt::AlignmentFlag::Top,
	     Wt::TextFlag::WordWrap, text);
  p.drawRect(r);
  }

  p.end();
  std::ofstream f("wrapped_text.svg");
  svgImage.write(f);
}

BOOST_AUTO_TEST_CASE( svg_test_drawSingleText )
{
  static std::string text = 
    "ceci n'est pas un text et ceci n'est pas une pipe non plus";

  Wt::WSvgImage svgImage(1800, 800);
  Wt::WPainter p(&svgImage);

  //horizontal alignment
  {
  Wt::WRectF r(5, 5, 150, 100);
  p.drawText(r, Wt::AlignmentFlag::Left | Wt::AlignmentFlag::Top, 
	     text);
  p.drawRect(r);
  }

  {
  Wt::WRectF r(5, 110, 150, 100);
  p.drawText(r, Wt::AlignmentFlag::Right | Wt::AlignmentFlag::Top,
	     text);
  p.drawRect(r);
  }

  {
  Wt::WRectF r(5, 215, 150, 100);
  p.drawText(r, Wt::AlignmentFlag::Center | Wt::AlignmentFlag::Top,
	     text);
  p.drawRect(r);
  }
  
  {
  Wt::WRectF r(5, 320, 150, 100);
  p.drawText(r, Wt::AlignmentFlag::Justify | Wt::AlignmentFlag::Top,
	     text);
  p.drawRect(r);
  }

  //red text
  {
  Wt::WRectF r(5, 425, 150, 100);
  p.setPen(Wt::WPen(Wt::StandardColor::Red));
  p.drawText(r, Wt::AlignmentFlag::Justify | Wt::AlignmentFlag::Top,
	     text);
  p.drawRect(r);
  }

  p.end();
  std::ofstream f("singleline_text.svg");
  svgImage.write(f);
}
