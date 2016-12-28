/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <Wt/WMessageResourceBundle.h>

BOOST_AUTO_TEST_CASE( Xml_test )
{
  Wt::WLocale locale;
  Wt::WMessageResourceBundle bundle;
  bundle.use("test");

  std::string result;

  if (!bundle.resolveKey(locale, "test1", result))
    return; // test.xml file not found

  BOOST_REQUIRE(bundle.resolveKey(locale, "test1", result));
  BOOST_REQUIRE(result == "<br/>");

  BOOST_REQUIRE(bundle.resolveKey(locale, "test2", result));
  BOOST_REQUIRE(result == "<div></div>");
}
