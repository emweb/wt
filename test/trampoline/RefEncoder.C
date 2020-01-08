/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <sstream>
#include <Wt/WAnchor.h>
#include <Wt/WImage.h>
#include <Wt/WText.h>
#include <Wt/Test/WTestEnvironment.h>
#include <Wt/WApplication.h>

using namespace Wt;

BOOST_AUTO_TEST_CASE(test_trampoline1)
{
  Wt::Test::WTestEnvironment env;
  env.setSessionIdInUrl(true);

  Wt::WApplication app(env);

  {
    Wt::WText t("<div style=\"background-image: "
		"url(http://www.google.be)\"></div>");

    std::stringstream s;
    t.htmlText(s);

    std::cerr << s.str() << std::endl;
    std::size_t red = s.str().find
      ("?request=redirect&amp;url=http%3a%2f%2fwww.google.be&amp;hash=");

    BOOST_REQUIRE(red != std::string::npos);
  }

  {
    Wt::WText t("<div style=\"background-image: "
		"url('http://www.google.be')\"></div>");
    std::stringstream s;
    t.htmlText(s);

    std::size_t red = s.str().find
      ("?request=redirect&amp;url=http%3a%2f%2fwww.google.be&amp;hash=");

    BOOST_REQUIRE(red != std::string::npos);
  }

  {
    Wt::WText t("<div style=\"background-image: "
		"url('http://www.google.be')\">"
		"<span style=\"background-image: "
		"url('http://www.webtoolkit.eu')\"></span></div>");
    std::stringstream s;
    t.htmlText(s);

    std::size_t red = s.str().find
      ("?request=redirect&amp;url=http%3a%2f%2fwww.google.be&amp;hash=");

    BOOST_REQUIRE(red != std::string::npos);

    red = s.str().find
      ("?request=redirect&amp;url=http%3a%2f%2fwww.webtoolkit.eu&amp;hash=");

    BOOST_REQUIRE(red != std::string::npos);
  }

  {
    Wt::WImage img("http://www.google.be");
    
    std::stringstream s;
    img.htmlText(s);

    std::size_t red = s.str().find
      ("?request=redirect&amp;url=http%3a%2f%2fwww.google.be&amp;hash=");

    BOOST_REQUIRE(red != std::string::npos);
  }

  {
    Wt::WAnchor img("http://www.google.be");
    
    std::stringstream s;
    img.htmlText(s);

    std::size_t red = s.str().find
      ("?request=redirect&amp;url=http%3a%2f%2fwww.google.be&amp;hash=");

    BOOST_REQUIRE(red != std::string::npos);
  }

  {
    Wt::WContainerWidget w;
    w.decorationStyle().setBackgroundImage("http://www.google.be");

    std::stringstream s;
    w.htmlText(s);

    std::cerr << s.str() << std::endl;

    std::size_t red = s.str().find
      ("?request=redirect&amp;url=http%3a%2f%2fwww.google.be&amp;hash=");

    BOOST_REQUIRE(red != std::string::npos);
  }    
}
