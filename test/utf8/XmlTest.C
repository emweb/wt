/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
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

  if (!bundle.resolveKey(locale, "test1"))
    return; // test.xml file not found

  Wt::LocalizedString result = bundle.resolveKey(locale, "test1");
  BOOST_REQUIRE(result);
  BOOST_REQUIRE(result.value == "<br/>");

  result = bundle.resolveKey(locale, "test2");
  BOOST_REQUIRE(result);
  BOOST_REQUIRE(result.value == "<div></div>");
}
