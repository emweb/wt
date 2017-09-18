#include <boost/test/unit_test.hpp>

#include <iostream>

#include "Wt/WWebWidget.h"

BOOST_AUTO_TEST_CASE( Escape_test1 )
{
  const std::string toEscape = "Support & training";
  const std::string toUnescape = "Support &amp; training";

  {
    std::string s = toEscape;
    Wt::WWebWidget::escapeText(s);
    BOOST_REQUIRE(s == toUnescape);
  }

  {
    std::string s = toUnescape;
    Wt::WWebWidget::unescapeText(s);
    BOOST_REQUIRE(s == toEscape);
  }
}

BOOST_AUTO_TEST_CASE( Escape_test2 )
{
  const std::string toUnescape = "&#34;";

  {
    std::string s = toUnescape;
    Wt::WWebWidget::unescapeText(s);
    BOOST_REQUIRE(s == "\"");
  }
}
